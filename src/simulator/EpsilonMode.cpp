
#include <iostream>

#include "Backend.h"
#include "EpsilonMode.h"
#include "ValueModifier.h"

using namespace std;

using namespace hydla::backend;

using namespace hydla::hierarchy;
using namespace hydla::simulator;
using namespace hydla::symbolic_expression;
using namespace hydla::logger;

using namespace hydla::symbolic_expression;
using namespace hydla::backend;

void hydla::simulator::cut_high_order_epsilon(backend_sptr_t backend_,
                                              phase_result_sptr_t &phase,
                                              int diff_cnt) {
  Parameter par("eps", 0, 1);
  value_t time_ret;
  backend_->call("cutHighOrderVariable", true, 3, "vlnpi", "vl",
                 &phase->current_time, &par, &diff_cnt, &time_ret);
  phase->current_time = time_ret;
  for (auto var_entry : phase->variable_map) {
    Variable var = var_entry.first;
    range_t range = var_entry.second;
    if (range.undefined()) {
      continue;
    } else if (range.unique()) {
      value_t value_ret;
      simulator::value_t val = range.get_unique_value();
      backend_->call("cutHighOrderVariable", true, 3, "vlnpi", "vl", &val, &par,
                     &diff_cnt, &value_ret);
      phase->variable_map[var] = value_ret;
    } else {
      for (ValueRange::uint i = 0; i < range.get_lower_cnt(); i++) {
        ValueRange::bound_t bd = range.get_lower_bound(i);
        value_t val = bd.value;
        value_t ret;
        backend_->call("cutHighOrderVariable", true, 3, "vlnpi", "vl", &val,
                       &par, &diff_cnt, &ret);
        range.set_lower_bound(ret, bd.include_bound);
      }
      for (ValueRange::uint i = 0; i < range.get_upper_cnt(); i++) {
        ValueRange::bound_t bd = range.get_upper_bound(i);
        value_t val = bd.value;
        value_t ret;
        backend_->call("cutHighOrderVariable", true, 3, "vlnpi", "vl", &val,
                       &par, &diff_cnt, &ret);
        range.set_upper_bound(ret, bd.include_bound);
      }
      phase->variable_map[var] = range;
    }
  }
}

pp_time_result_t
hydla::simulator::reduce_unsuitable_case(pp_time_result_t original_result,
                                         backend_sptr_t backend_,
                                         phase_result_sptr_t &phase) {
  pp_time_result_t ret;
  bool unsuitable;
  for (auto original_candidate : original_result) {
    HYDLA_LOGGER_DEBUG("#epsilon checking time", original_candidate.time);
    unsuitable = false;
    HYDLA_LOGGER_DEBUG_VAR(original_candidate.parameter_constraint);
    backend_->call("unsuitableCase", true, 1, "csn", "b",
                   &original_candidate.parameter_constraint, &unsuitable);
    if (!unsuitable) {
      ret.push_back(original_candidate);
    }
  }
  return ret;
}

find_min_time_result_t
hydla::simulator::reduce_unsuitable_case(find_min_time_result_t original_result,
                                         backend_sptr_t backend_,
                                         phase_result_sptr_t &phase) {
  find_min_time_result_t ret;
  bool unsuitable;
  for (auto original_candidate : original_result) {
    HYDLA_LOGGER_DEBUG("#epsilon checking time", original_candidate.time);
    unsuitable = false;
    HYDLA_LOGGER_DEBUG_VAR(original_candidate.parameter_constraint);
    backend_->call("unsuitableCase", true, 1, "csn", "b",
                   &original_candidate.parameter_constraint, &unsuitable);
    if (!unsuitable) {
      ret.push_back(original_candidate);
    }
  }
  return ret;
}

find_min_time_result_t hydla::simulator::calculate_tmp_result(
    phase_result_sptr_t &phase, value_t time_limit,
    symbolic_expression::node_sptr trigger, backend_sptr_t backend,
    variable_map_t vm) {
  bool iszero = false;
  bool same_id = false;

  find_min_time_result_t find_result, ret;

  HYDLA_LOGGER_DEBUG("come calculate tmp result");

  backend->call("findMinTime", true, 3, "etmvtvlt", "f", &trigger, &vm,
                &time_limit, &find_result);
  for (auto find_candidate : find_result) {
    value_t time = find_candidate.time;
    backend->call("isZero", true, 1, "vln", "b", &time, &iszero);

    // 前回の離散変化条件のidとの比較
    for (auto entry : phase->parent->parent->discrete_asks) {
      if (trigger == entry.first) {
        same_id = true;
        break;
      }
    }
    // 時刻をずらす場合
    if (iszero && same_id) {
      // 時刻ずらし
      find_min_time_result_t tmp_time_result;

      node_sptr tmp_val;
      tmp_val = node_sptr(new Number("-1"));
      tmp_val = node_sptr(new Times(tmp_val, find_candidate.time.get_node()));

      variable_map_t shifted_vm;
      ValueModifier modifier(backend);
      shifted_vm = modifier.apply_function("exprTimeshift", tmp_val, vm);

      backend->call("resetConstraintForVariable", true, 0, "", "");
      backend->call("addConstraint", true, 1, "mv0t", "", &shifted_vm);
      value_t eps_time_limit = time_limit;
      eps_time_limit -= find_candidate.time;
      // 二回目
      tmp_time_result = calculate_tmp_result(phase, eps_time_limit, trigger,
                                             backend, shifted_vm);

      for (auto &tmp_candidate : tmp_time_result) {
        tmp_candidate.time = find_candidate.time + tmp_candidate.time;
        ret.push_back(tmp_candidate);
      }
    } else {
      ret.push_back(find_candidate);
    }
  }
  HYDLA_LOGGER_DEBUG("exit calculate tmp result");

  return ret;
}
