#include "PhaseResult.h"
#include "DefaultParameter.h"
#include "Simulator.h"



namespace {
  struct NodeDumper {
      
    template<typename T>
    NodeDumper(T it, T end) 
    {
      for(; it!=end; ++it) {
        ss << **it << "\n";
      }
    }

    NodeDumper(const NodeDumper& rhs)
    {
      ss << rhs.ss.rdbuf();
    }

    friend std::ostream& operator<<(std::ostream& s, const NodeDumper& nd)
    {
      s << nd.ss.str();
      return s;
    }

    std::stringstream ss;
  };
}


namespace hydla {
namespace simulator {


PhaseResult::PhaseResult():cause_for_termination(NONE)
{
}

bool ParameterComparator::operator()(const DefaultParameter x,const DefaultParameter y) const { return x < y; }



PhaseResult::PhaseResult(const SimulationTodo& todo, const CauseForTermination& cause):
  phase_type(todo.phase_type),
  current_time(todo.current_time),
  parameter_map(todo.parameter_map),
  positive_asks(todo.positive_asks),
  negative_asks(todo.negative_asks),
  step(todo.parent->step + 1),
  module_set_container(todo.module_set_container),
  cause_for_termination(cause),
  parent(todo.parent)
{
}



std::ostream& operator<<(std::ostream& s, const PhaseResult& phase)
{
  s << "%% PhaseType: " << phase.phase_type << std::endl;
  s << "%% id: " <<  phase.id          << std::endl;
  s << "%% step: " <<  phase.step << std::endl;
  
  if(!phase.current_time.undefined())
  {
    s << "%% current_time: " << phase.current_time << std::endl;
  }
  if(!phase.end_time.undefined())
  {
    s << "%% end_time: " << phase.end_time << std::endl;
  }
  s << "--- variable map ---" << std::endl;
  s << phase.variable_map    << std::endl;
  s << "--- parameter map ---"          << std::endl;
  s << phase.parameter_map << std::endl;
  
  return s;
}


std::ostream& operator<<(std::ostream& s, const variable_map_t& vm){
  variable_map_t::const_iterator it  = vm.begin();
  variable_map_t::const_iterator end = vm.end();
  
  for(; it!=end; ++it) {
    s << it->first << " <=> " << it->second << "\n";
  }
  
  return s;
}


std::ostream& operator<<(std::ostream& s, const parameter_map_t& pm){
  parameter_map_t::const_iterator it  = pm.begin();
  parameter_map_t::const_iterator end = pm.end();
  for(; it!=end; ++it) {
    s << it->first << " <=> " << it->second << "\n";
  }
  return s;
}





std::ostream& operator<<(std::ostream& s, const ask_set_t& a)
{
  s << NodeDumper(a.begin(), a.end());
  return s;
}

std::ostream& operator<<(std::ostream& s, const tells_t& a)
{
  s << NodeDumper(a.begin(), a.end());
  return s;
}

std::ostream& operator<<(std::ostream& s, const collected_tells_t& a)
{
  s << NodeDumper(a.begin(), a.end());
  return s;
}

std::ostream& operator<<(std::ostream& s, const expanded_always_t& a)
{
  s << NodeDumper(a.begin(), a.end());
  return s;
}

std::ostream& operator<<(std::ostream& s, const module_set_sptr& m)
{
  s << m->get_name()
    << "\n"
    << m->get_infix_string();
  return s;
}

std::ostream& operator<<(std::ostream& s, const change_variables_t& a)
{
  s << "%% ChangedVariables: ";
  change_variables_t::iterator it = a.begin();
  s << *it;
  it++;
  for(; it != a.end(); it++)
    s << ", " << *it;
  s << std::endl;
  return s;
}


} // namespace simulator
} // namespace hydla 
