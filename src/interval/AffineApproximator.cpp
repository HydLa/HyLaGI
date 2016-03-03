#include "AffineApproximator.h"
#include <boost/lexical_cast.hpp>
#include "TreeInfixPrinter.h"
#include <exception>
#include "Backend.h"
#include "Logger.h"
#include "ValueModifier.h"
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

void AffineApproximator::reduce_dummy_variables(kv::ub::vector<affine_t> &formulas, int limit)
{
  assert(0);
/*
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
*/
}

value_t AffineApproximator::translate_into_symbolic_value(const affine_t& affine_value, parameter_map_t &parameter_map)
{
  value_t ret(affine_value.a(0));
  simulator->backend->call("transformToRational", true, 1, "vln", "vl", &ret, &ret);
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
      simulator->backend->call("transformToRational", true, 1, "vln", "vl", &val, &val);
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
    simulator->backend->call("transformToRational", true, 1, "vln", "vl", &val, &val);
    ret = ret + val * value_t(param);
  }

  // reset rounding mode
  kv::hwround::roundnear();

  kv::interval<double> itv = to_interval(affine_value);
  HYDLA_LOGGER_DEBUG_VAR(itv);
  return ret;
}

value_t AffineApproximator::approximate(node_sptr& node, parameter_map_t &parameter_map, variable_map_t &variable_map)
{
  AffineTreeVisitor visitor(parameter_idx_map, variable_map);
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

void AffineApproximator::approximate_time(value_t& time, const variable_map_t& ip_map, variable_map_t& prev_map, parameter_map_t &parameter_map, node_sptr condition)
{
  node_sptr node = time.get_node();
  time = approximate(node, parameter_map, prev_map);
}


}
}
