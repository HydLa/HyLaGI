#include "AffineApproximator.h"
#include <boost/lexical_cast.hpp>
#include "TreeInfixPrinter.h"
#include <exception>
#include "Backend.h"
#include "Logger.h"
#include "TimeModifier.h"
#include "VariableFinder.h"
#include "Variable.h"

using namespace std;
using namespace boost;
using namespace hydla::simulator;

namespace hydla {
namespace interval {

AffineApproximator* AffineApproximator::affine_translator = NULL;

AffineApproximator* AffineApproximator::get_instance()
{
  if(affine_translator == NULL)affine_translator = new AffineApproximator();
  return affine_translator;
}

AffineApproximator::AffineApproximator(): epsilon_index(0)
{}

AffineApproximator::~AffineApproximator()
{}

void AffineApproximator::set_simulator(Simulator* s)
{
  simulator = s;
}

void AffineApproximator::reduce_dummy_variables(ub::vector<affine_t> &formulas, int limit)
{
  std::map<int, int> index_map = kv::epsilon_reduce(formulas, limit);
  if(!index_map.empty())
  {
    parameter_idx_map.clear();
    for(auto pair : index_map)
    {
      HYDLA_LOGGER_DEBUG_VAR(pair.first);
      HYDLA_LOGGER_DEBUG_VAR(pair.second);
      parameter_idx_map_t::right_iterator r_it = parameter_idx_map.right.find(pair.first);
      if(r_it == parameter_idx_map.right.end())continue;
      if(pair.second == -1)
      {
        parameter_idx_map.right.erase(r_it);
      }
      else
      {
        parameter_idx_map.right.replace_key(r_it, pair.second);
      }
    }
  }
}

value_t AffineApproximator::translate_into_symbolic_value(const affine_t& affine_value, parameter_map_t &parameter_map)
{
  value_t ret(affine_value.a(0));
  simulator->backend->call("transformToRational", 1, "vln", "vl", &ret, &ret);
  double sum = 0;
  int available_index;
  for(int i = 1; i < affine_value.a.size(); i++)
  {
    if(affine_value.a(i) == 0)continue;
    if(parameter_idx_map.right.find(i)
       == parameter_idx_map.right.end())
    {
      // 新規に追加されるダミー変数は1つにまとめる（この時点では他の変数と関係を持っていないため）
      // TODO: kvライブラリ内のダミー変数も削除する？結果に誤りは生まれないはずだが、インデックスが無駄に増える。
      sum += affine_value.a(i);
      available_index = i;
    }
    else
    {
      parameter_idx_map_t::right_iterator r_it = parameter_idx_map.right.find(i);
      // convert floating point into rational
      value_t val = value_t(affine_value.a(i));
      simulator->backend->call("transformToRational", 1, "vln", "vl", &val, &val);
      ret = ret + val * value_t(r_it->second);
    }
  }
  if(sum != 0)
  {
    range_t range;
    range.set_lower_bound(value_t("-1"), true);
    range.set_upper_bound(value_t("1"), true);
    // parameters whose differential counts are -1 are regarded as dummy variables
    parameter_t param = simulator->introduce_parameter("affine", -1, ++epsilon_index, range);
    parameter_idx_map.insert(parameter_idx_t(param, available_index));
    parameter_map[param] = range_t(value_t(-1), value_t(1));
    value_t val = value_t(sum);
    simulator->backend->call("transformToRational", 1, "vln", "vl", &val, &val);
    ret = ret + val * value_t(param);
  }

  // reset rounding mode
  kv::hwround::roundnear();

  kv::interval<double> itv = to_interval(affine_value);
  HYDLA_LOGGER_DEBUG_VAR(itv);
  return ret;
}

value_t AffineApproximator::approximate(node_sptr& node, parameter_map_t &parameter_map)
{
  AffineTreeVisitor visitor(parameter_idx_map);
  AffineOrInteger val = visitor.approximate(node);
  if(val.is_integer)return value_t(val.integer);

  affine_t affine_value = val.affine_value;
  // 試験的にダミー変数の削減をしてみる TODO:外部から削減タイミングを指定するようにする
/*
  ub::vector<affine_t> formulas(1);
  formulas(0) = affine_value;
  reduce_dummy_variables(formulas, 1);
  affine_value = formulas(0);  
  HYDLA_LOGGER_DEBUG_VAR(affine_t::maxnum());
  HYDLA_LOGGER_DEBUG_VAR(affine_value);
*/
  // set rounding mode
  kv::hwround::roundup();

  value_t result_value = translate_into_symbolic_value(affine_value, parameter_map);  
  return result_value;
}


void AffineApproximator::approximate(const variable_t &variable_to_approximate, variable_map_t&variable_map, parameter_map_t &parameter_map, node_sptr condition)
{
  range_t val = variable_map[variable_to_approximate];
  assert(val.unique());
  node_sptr node = val.get_unique().get_node();
  value_t affine = approximate(node, parameter_map);
  variable_map[variable_to_approximate] = affine;
  if(condition.get() != nullptr)
  {
    //TODO: deal with general case (currently only for '=')
    assert(typeid(*condition) == typeid(symbolic_expression::Equal));
    //Check whether the condition has approximated variable
    simulator::VariableFinder finder;
    finder.visit_node(condition, false);
    if(finder.include_variable(variable_to_approximate) || finder.include_variable_prev(variable_to_approximate))
    {
      VariableFinder::variable_set_t variables = finder.get_all_variable_set();
      if(variables.size() > 2)
      {
        //TODO: approximate n-2 variables
        assert(0);
      }
      // TODO: 本来ならここで離散変化条件に関わる変数を全部考慮に入れないといけない（現状だと離散変化条件が直接言及している変数しか考慮していない）
      variable_t remain_var;
      for(auto var : variables)
      {
        if(var != variable_to_approximate)
        {
          remain_var = var;
          break;
        }
      }
      Value consistent_value;
      simulator->backend->call("calculateConsistentValue", 4,
                                "ecvnmvnmp", "vl",
                                &condition, &remain_var, &variable_map, &parameter_map, &consistent_value);
      variable_map[remain_var] = consistent_value;
    }
  }
}


void AffineApproximator::approximate_time(value_t& time, const variable_map_t& ip_map, variable_map_t& prev_map, parameter_map_t &parameter_map, node_sptr condition)
{
  node_sptr node = time.get_node();
  time = approximate(node, parameter_map);
  if(condition.get() != nullptr)
  {
    //TODO: deal with general case (currently only for '=')
    assert(typeid(*condition) == typeid(symbolic_expression::Equal));
    //Check whether the condition has approximated variable
    simulator::VariableFinder finder;
    finder.visit_node(condition, false);
    VariableFinder::variable_set_t variables = finder.get_all_variable_set();
    
    // TODO: 本来ならここで離散変化条件に関わる変数を全部考慮に入れないといけない
    // 再計算する変数を１つだけ決める（暫定的に最初の要素とする）
    if(variables.size() <= 1)return;
    variable_t recalculate_variable = *(variables.begin());
    for(auto vm_it = prev_map.begin(); vm_it != prev_map.end(); vm_it++)
    {
      // それ以外は新しいtを代入する．
      // TODO: 使用する時刻はタイムシフト前の方が計算が楽なはず
      TimeModifier modifier(*simulator->backend);
      prev_map[vm_it->first] = modifier.substitute_time(time, ip_map.find(vm_it->first)->second);
    }

    Value consistent_value;
    simulator->backend->call("calculateConsistentValue", 4,
                             "ecvnmvnmp", "vl",
                             &condition, &recalculate_variable, &prev_map, &parameter_map, &consistent_value);
    prev_map[recalculate_variable] = consistent_value;
  }
}



}
}
