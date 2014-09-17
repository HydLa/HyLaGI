#include "PhaseResult.h"
#include "Parameter.h"
#include "Simulator.h"

using namespace std;


namespace {
  struct NodeDumper {
      
    template<typename T>
    NodeDumper(T it, T end) 
    {
      for(; it!=end; ++it) {
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


namespace hydla {
namespace simulator {


PhaseResult::PhaseResult():cause_for_termination(NONE), parent(nullptr), full_information(nullptr)
{
}

PhaseResult::~PhaseResult()
{
  for(auto child : children)
  {
    child->parent = nullptr;
  }
}

void PhaseResult::generate_full_information()
{
  PhaseResult *ancestor = parent;
  std::list<PhaseResult *> ancestor_list;
  ancestor_list.push_back(this);
  while(ancestor->parent != nullptr && ancestor->full_information == nullptr)
  {
    ancestor_list.push_back(ancestor);
    ancestor = ancestor->parent;
  }
  assert(ancestor->full_information != nullptr);
  full_information = new FullInformation(*ancestor->full_information);
  for(auto r_it = ancestor_list.rbegin(); r_it != ancestor_list.rend(); r_it++)
  {
    for(auto ask : (*r_it)->diff_positive_asks)
    {
      full_information->negative_asks.erase(ask);
      full_information->positive_asks.insert(ask);
    }
    for(auto ask : (*r_it)->diff_negative_asks)
    {
      full_information->positive_asks.erase(ask);
      full_information->negative_asks.insert(ask);
    }
  }
}

ask_set_t PhaseResult::get_all_positive_asks()
{
  if(full_information == nullptr)generate_full_information();
  return full_information->positive_asks;
}

ask_set_t PhaseResult::get_all_negative_asks()
{
  if(full_information == nullptr)generate_full_information();
  return full_information->negative_asks;
}

void PhaseResult::set_full_information(FullInformation *info)
{
  full_information = info;
}


PhaseResult::PhaseResult(const SimulationTodo& todo, const CauseForTermination& cause):
  phase_type(todo.phase_type),
  current_time(todo.current_time),
  parameter_map(todo.parameter_map),
  diff_positive_asks(todo.positive_asks),
  diff_negative_asks(todo.negative_asks),
  step(todo.parent->step + 1),
  cause_for_termination(cause),
  parent(todo.parent.get()),
  full_information(nullptr)
{
}


ostream& operator<<(std::ostream& s, const PhaseResult& phase)
{
  s << "%% PhaseType: " << phase.phase_type << endl;
  s << "%% id: " <<  phase.id          << endl;
  s << "%% step: " <<  phase.step << endl;
  
  if(!phase.current_time.undefined())
  {
    s << "%% current_time: " << phase.current_time << endl;
  }
  if(!phase.end_time.undefined())
  {
    s << "%% end_time: " << phase.end_time << endl;
  }
  s << "--- variable map ---" << endl;
  s << phase.variable_map    << endl;
  s << "--- parameter map ---"          << endl;
  s << phase.parameter_map << endl;
  return s;
}


ostream& operator<<(std::ostream& s, const variable_map_t& vm){
  variable_map_t::const_iterator it  = vm.begin();
  variable_map_t::const_iterator end = vm.end();
  
  for(; it!=end; ++it) {
    s << it->first << " <=> " << it->second << "\n";
  }
  
  return s;
}


ostream& operator<<(std::ostream& s, const parameter_map_t& pm){
  parameter_map_t::const_iterator it  = pm.begin();
  parameter_map_t::const_iterator end = pm.end();
  for(; it!=end; ++it) {
    s << it->first << " <=> " << it->second << "\n";
  }
  return s;
}

ostream& operator<<(std::ostream& s, const ask_set_t& a)
{
  s << NodeDumper(a.begin(), a.end());
  return s;
}

ostream& operator<<(std::ostream& s, const tells_t& a)
{
  s << NodeDumper(a.begin(), a.end());
  return s;
}

ostream& operator<<(std::ostream& s, const constraints_t &a)
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
  for(change_variables_t::iterator it = a.begin(); it != a.end(); it++, i++)
    s << *it << (i<a.size() ? " , " : "");
  s << endl;
  return s;
}


} // namespace simulator
} // namespace hydla 
