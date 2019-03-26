#include "PhaseResult.h"
#include "Parameter.h"
#include "Simulator.h"
#include "Backend.h"
#include "HydLaError.h"

#include <sstream>

using namespace std;

namespace
{
struct NodeDumper
{
  template <typename T>
  NodeDumper(T it, T end)
  {
    for (; it != end; ++it)
    {
      ss << hydla::symbolic_expression::get_infix_string(*it) << "\n";
    }
  }

  NodeDumper(const NodeDumper& rhs)
  {
    ss << rhs.ss.rdbuf();
  }

  friend ostream& operator<<(std::ostream& s, const NodeDumper& nd)
  {
    s << nd.ss.str();
    return s;
  }

  stringstream ss;
};
}

namespace hydla
{
namespace simulator
{
backend::Backend* PhaseResult::backend = nullptr;

PhaseResult::PhaseResult() : phase_type(InvalidPhase), simulation_state(NOT_SIMULATED), parent(nullptr)
{
}

PhaseResult::~PhaseResult()
{
  for (auto child : children)
  {
    child->parent = nullptr;
  }
}

void PhaseResult::generate_full_information() const
{
  PhaseResult* ancestor = parent;
  std::list<const PhaseResult*> ancestor_list;
  ancestor_list.push_back(this);
  while (ancestor->parent != nullptr && !ancestor->full_information)
  {
    ancestor_list.push_back(ancestor);
    ancestor = ancestor->parent;
  }
  assert(ancestor->full_information);
  full_information = ancestor->full_information;
  for (auto r_it = ancestor_list.rbegin(); r_it != ancestor_list.rend(); r_it++)
  {
    for (auto ask : (*r_it)->diff_positive_asks)
    {
      full_information->negative_asks.erase(ask);
      full_information->positive_asks.insert(ask);
    }
    for (auto ask : (*r_it)->diff_negative_asks)
    {
      full_information->positive_asks.erase(ask);
      full_information->negative_asks.insert(ask);
    }
  }
}

asks_t PhaseResult::get_diff_negative_asks() const
{
  return diff_negative_asks;
}

asks_t PhaseResult::get_diff_positive_asks() const
{
  return diff_positive_asks;
}

void PhaseResult::add_diff_positive_asks(const asks_t& asks)
{
  full_information = boost::none;
  diff_positive_asks.insert(asks.begin(), asks.end());
}

void PhaseResult::add_diff_negative_asks(const asks_t& asks)
{
  full_information = boost::none;
  diff_negative_asks.insert(asks.begin(), asks.end());
}

void PhaseResult::add_diff_positive_ask(const ask_t& ask)
{
  full_information = boost::none;
  diff_positive_asks.insert(ask);
}

void PhaseResult::add_diff_negative_ask(const ask_t& ask)
{
  full_information = boost::none;
  diff_negative_asks.insert(ask);
}

asks_t PhaseResult::get_all_positive_asks() const
{
  if (!full_information)
    generate_full_information();
  return full_information->positive_asks;
}

asks_t PhaseResult::get_all_negative_asks() const
{
  if (!full_information)
    generate_full_information();
  return full_information->negative_asks;
}

void PhaseResult::set_parameter_constraint(const ConstraintStore& cons)
{
  parameter_constraint = cons;
  parameter_maps = boost::none;
}

void PhaseResult::add_parameter_constraint(const constraint_t& cons)
{
  parameter_constraint.add_constraint(cons);
  parameter_maps = boost::none;
}

void PhaseResult::add_parameter_constraint(const ConstraintStore& cons)
{
  parameter_constraint.add_constraint_store(cons);
  parameter_maps = boost::none;
}

void PhaseResult::remove_parameter_constraint(const constraint_t& eraseGuard)
{
  parameter_constraint.remove_constraint(eraseGuard);
  //parameter_maps = boost::none;
}

bool PhaseResult::remove_asks(const constraint_t eraseGuard)
{
  bool anyErased = false;

  HYDLA_LOGGER_DEBUG("BREAK PhaseResult::remove_asks");

  if (parent)
  {
    for (auto it = parent->next_pp_candidate_map.begin(); it != parent->next_pp_candidate_map.end();)
    {
      const bool result = it->first->has_sub_tree(*eraseGuard);
      HYDLA_LOGGER_DEBUG("BREAK next_pp_candidate - ", get_infix_string(it->first), " : ", result ? "true" : "false");
      if (result)
      {
        it = parent->next_pp_candidate_map.erase(it);
        anyErased = true;
      }
      else
      {
        ++it;
      }
    }
  }
  {
    auto& asks = discrete_asks;
    for (auto it = asks.begin(); it != asks.end();)
    {
      const bool result = it->first->has_sub_tree(*eraseGuard);
      HYDLA_LOGGER_DEBUG("BREAK asks: ", get_infix_string(eraseGuard), ", discrete_ask:  ", get_infix_string(it->first), "  => ", result ? "true" : "false");
      if (result)
      {
        it = asks.erase(it);
        anyErased = true;
      }
      else
      {
        ++it;
      }
    }
  }

  for (auto it = diff_positive_asks.begin(); it != diff_positive_asks.end();)
  {
    const bool result = (*it)->has_sub_tree(*eraseGuard);
    HYDLA_LOGGER_DEBUG("BREAK diff_positive_asks - ", get_infix_string(*it), " : ", result ? "true" : "false");
    if (result)
    {
      it = diff_positive_asks.erase(it);
      anyErased = true;
    }
    else
    {
      ++it;
    }
  }
  for (auto it = diff_negative_asks.begin(); it != diff_negative_asks.end();)
  {
    const bool result = (*it)->has_sub_tree(*eraseGuard);
    HYDLA_LOGGER_DEBUG("BREAK diff_negative_asks - ", get_infix_string(*it), " : ", result ? "true" : "false");
    if (result)
    {
      it = diff_negative_asks.erase(it);
      anyErased = true;
    }
    else
    {
      ++it;
    }
  }

  return anyErased;
}

ConstraintStore PhaseResult::get_parameter_constraint() const
{
  return parameter_constraint;
}

std::vector<parameter_map_t> PhaseResult::get_parameter_maps() const
{
  if (!parameter_maps)
  {
    if (parameter_constraint.size() == 0)
    {
      parameter_maps = std::vector<parameter_map_t>(1);
    }
    else
    {
      if (!backend)
        throw HYDLA_ERROR("backend has not been set yet.");
      std::vector<parameter_map_t> par_maps;
      backend->call("createParameterMaps", true, 1, "csn", "mps", &parameter_constraint, &par_maps);
      parameter_maps = par_maps;
    }
  }
  return *parameter_maps;
}

void PhaseResult::set_full_information(FullInformation& info)
{
  full_information = info;
}

string PhaseResult::get_string() const
{
  std::stringstream sstr;
  sstr << *this;
  return sstr.str();
}

PhaseResult& PhaseResult::operator=(const PhaseResult& other)
{
  phase_type = other.phase_type;
  id = other.id;
  step = other.step;

  current_time = other.current_time;
  end_time = other.end_time;

  variable_map = other.variable_map;
  prev_map = other.prev_map;

  additional_parameter_constraint = other.additional_parameter_constraint;
  additional_constraint_store = other.additional_constraint_store;
  diff_sum = other.diff_sum;
  discrete_differential_set = other.discrete_differential_set;

  module_diff = other.module_diff;

  unadopted_ms = other.unadopted_ms;
  unadopted_mss = other.unadopted_mss;
  inconsistent_module_sets = other.inconsistent_module_sets;
  inconsistent_constraints = other.inconsistent_constraints;
  next_pp_candidate_map = other.next_pp_candidate_map;
  guard_time_map = other.guard_time_map;
  always_list = other.always_list;

  simulation_state = other.simulation_state;
  parent = other.parent;
  children = other.children;
  todo_list = other.todo_list;

  discrete_asks = other.discrete_asks;
  discrete_guards = other.discrete_guards;

  profile = other.profile;

  diff_positive_asks = other.diff_positive_asks;
  diff_negative_asks = other.diff_negative_asks;
  parameter_constraint = other.parameter_constraint;
  parameter_maps = other.parameter_maps;
  full_information = other.full_information;

  return *this;
}

ostream& operator<<(std::ostream& s, const PhaseResult& phase)
{
  s << "%% PhaseType: " << phase.phase_type << endl;
  s << "%% id: " << phase.id << endl;
  s << "%% step: " << phase.step << endl;

  if (phase.parent != nullptr)
    s << "%% parent_id:" << phase.parent->id << endl;
  else
    s << "%% no parent" << endl;

  s << "%% unadopted modules: " << phase.unadopted_ms.get_name() << endl;
  if (!phase.inconsistent_module_sets.empty())
  {
    for (auto module_set : phase.inconsistent_module_sets)
    {
      s << "%% inconsistent modules: " << module_set.get_name() << endl;
    }
  }
  if (!phase.inconsistent_constraints.empty())
  {
    s << "%% inconsistent constraints: ";
    bool first = true;
    for (auto constraint : phase.inconsistent_constraints)
    {
      if (!first)
        s << "\t\t";
      s << constraint << endl;
      first = false;
    }
  }

  s << "%% positive_asks" << endl;
  for (auto ask : phase.get_diff_positive_asks())
  {
    s << get_infix_string(ask) << endl;
  }
  s << "%% negative_asks" << endl;
  for (auto ask : phase.get_diff_negative_asks())
  {
    s << get_infix_string(ask) << endl;
  }

  if (!phase.current_time.undefined())
  {
    s << "%% current_time: " << phase.current_time << endl;
  }
  if (!phase.end_time.undefined())
  {
    s << "%% end_time: " << phase.end_time << endl;
  }
  s << "--- variable map ---" << endl;
  s << phase.variable_map << endl;
  s << "--- parameter constraint ---" << endl;
  s << phase.get_parameter_constraint() << endl;
  return s;
}

ostream& operator<<(std::ostream& s, const variable_map_t& vm)
{
  variable_map_t::const_iterator it = vm.begin();
  variable_map_t::const_iterator end = vm.end();

  for (; it != end; ++it)
  {
    s << it->first << " <=> " << it->second << "\n";
  }

  return s;
}

ostream& operator<<(std::ostream& s, const parameter_map_t& pm)
{
  parameter_map_t::const_iterator it = pm.begin();
  parameter_map_t::const_iterator end = pm.end();
  for (; it != end; ++it)
  {
    s << it->first << " <=> " << it->second << "\n";
  }
  return s;
}

ostream& operator<<(std::ostream& s, const asks_t& a)
{
  s << NodeDumper(a.begin(), a.end());
  return s;
}

ostream& operator<<(std::ostream& s, const tells_t& a)
{
  s << NodeDumper(a.begin(), a.end());
  return s;
}

ostream& operator<<(std::ostream& s, const constraints_t& a)
{
  s << NodeDumper(a.begin(), a.end());
  return s;
}

ostream& operator<<(std::ostream& s, const module_set_t& m)
{
  s << m.get_name()
    << "\n"
    << m.get_infix_string();
  return s;
}

ostream& operator<<(std::ostream& s, const change_variables_t& a)
{
  s << "%% ChangedVariables: ";
  int i = 1;
  for (change_variables_t::iterator it = a.begin(); it != a.end(); it++, i++)
    s << *it << (i < a.size() ? " , " : "");
  s << endl;
  return s;
}

ostream& operator<<(std::ostream& os, const pp_time_result_t& result)
{
  for (auto candidate : result)
  {
    os << "time: " << candidate.time << endl;
    os << "--- parameter constraint ---\n" << candidate.parameter_constraint << endl;
    os << "--- discrete guards ---" << endl;
    for (auto guard : candidate.discrete_asks)
    {
      os << "guard: " << get_infix_string(guard.first) << " on_time: " << guard.second << endl;
    }
    os << endl;
  }
  return os;
}

ostream& operator<<(std::ostream& os, const FindMinTimeCandidate& candidate)
{
  os << "time: " << candidate.time << endl;
  os << "on_time: " << candidate.on_time << endl;
  os << "--- parameter constraint ---\n" << candidate.parameter_constraint << endl;

  return os;
}

}  // namespace simulator
}  // namespace hydla
