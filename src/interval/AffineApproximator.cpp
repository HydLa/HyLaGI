#include "AffineApproximator.h"
#include <boost/lexical_cast.hpp>
#include "TreeInfixPrinter.h"
#include <exception>
#include "Backend.h"
#include "Logger.h"
#include "ValueModifier.h"
#include "VariableFinder.h"
#include "Variable.h"
#include "HydLaError.h"


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
  std::map<int, int> index_map = kv::epsilon_reduce(formulas, limit);
  for(auto pair : index_map)
  {
    HYDLA_LOGGER_DEBUG_VAR(pair.first);
    HYDLA_LOGGER_DEBUG_VAR(pair.second);
    parameter_idx_map_t::right_iterator r_it = parameter_idx_map.right.find(pair.second);
    if(r_it == parameter_idx_map.right.end())
    {
      HYDLA_LOGGER_DEBUG("Not Found");
      continue;
    }

    if(pair.second == -1)
    {
      parameter_idx_map.right.erase(r_it);
    }
    else
    {
      parameter_idx_map.right.replace_key(r_it, pair.first);
    }
  }
}

value_t AffineApproximator::translate_into_symbolic_value(const affine_t& affine_value, parameter_map_t &parameter_map)
{
  // set rounding mode
  kv::hwround::roundup();
  value_t ret(affine_value.a(0));
  simulator->backend->call("transformToRational", true, 1, "vln", "vl", &ret, &ret);
  double sum = 0;
  int available_index;
  for(int i = 1; i < affine_value.a.size(); i++)
  {
    if(affine_value.a(i) == 0)continue;
    parameter_idx_map_t::right_iterator r_it = parameter_idx_map.right.find(i);
    if(r_it == parameter_idx_map.right.end())
    {
      // 新規に追加されるダミー変数は1つにまとめる（この時点では他の変数と関係を持っていないため）
      // TODO: kvライブラリ内のダミー変数も削除する？結果に誤りは生まれないはずだが、インデックスが無駄に増える。
      sum += affine_value.a(i);
      available_index = i;
    }
    else
    {
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

void AffineApproximator::approximate(const simulator::variable_set_t &vars_to_approximate,  variable_map_t &variable_map, parameter_map_t &parameter_map, value_t &time)
{
  parameter_idx_map.clear();
  affine_t::maxnum() = 0;
  AffineTreeVisitor visitor(parameter_idx_map, variable_map);
  std::map<simulator::variable_t, affine_t> affine_map;
  for(auto var: vars_to_approximate)
  {
    simulator::range_t range = variable_map[var];
    HYDLA_ASSERT(range.unique());
    HYDLA_LOGGER_DEBUG_VAR(var);
    HYDLA_LOGGER_DEBUG_VAR(range);
    node_sptr expr = range.get_unique_value().get_node();
    AffineMixedValue val = visitor.approximate(expr);

    if(val.type == INTEGER)
    {
      variable_map[var] = value_t(val.integer);
    }
    else if(val.type == INTERVAL)
    {
      affine_map[var] = val.interval;
    }
    else
    {
      affine_map[var] = val.affine_value;
    }
  }


  bool time_is_affine = false;
  // approximate time
  node_sptr time_expr = time.get_node();
  HYDLA_LOGGER_DEBUG_VAR(time_expr);
  AffineMixedValue time_val = visitor.approximate(time_expr);
  if(time_val.type != INTEGER)
  {
    time_is_affine = true;
  }

  // 試験的にダミー変数の削減をしてみる TODO:外部から削減タイミングを指定するようにする
  kv::ub::vector<affine_t> formulas(affine_map.size() + (time_is_affine?1:0));
  int i = 0;
  if(time_is_affine)
  {
    formulas[0] = time_val.type == INTERVAL?affine_t(time_val.interval):time_val.affine_value;
    i = 1;
  }
  

  std::map<variable_t, int> var_index_map;
  
  for(auto element: affine_map)
  {
    formulas(i) = element.second;
    var_index_map[element.first] = i;
    ++i;
  }
  reduce_dummy_variables(formulas, formulas.size());

  if(time_is_affine)
  {
    time = translate_into_symbolic_value(formulas(0), parameter_map);
  }

  for(auto element: affine_map)
  {
    simulator::variable_t var = element.first;
    affine_t affine = formulas[var_index_map[var] ];
    HYDLA_LOGGER_DEBUG_VAR(var);
    value_t result_value = translate_into_symbolic_value(affine, parameter_map);
    variable_map[var] = result_value;
  }

  return;
}


}
}
