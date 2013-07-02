#include "Dumpers.h"

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


std::ostream& operator<<(std::ostream& s, const constraints_t& a)
{
  s << NodeDumper(a.begin(), a.end());
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


std::ostream& operator<<(std::ostream& s, const phase_result_sptr_t& phase)
{
  s << "%% PhaseType: " << phase->phase << std::endl;
  s << "%% id: " <<  phase->id          << std::endl;
  s << "%% step: " <<  phase->step << std::endl;
  
  if(phase->current_time.get())
  {
    s << "%% current_time: " << *phase->current_time << std::endl;
  }
  if(phase->end_time.get())
  {
    s << "%% end_time: " << *phase->end_time << std::endl;
  }
  s << "--- variable map ---" << std::endl;
  s << phase->variable_map    << std::endl;
  s << "--- parameter map ---"          << std::endl;
  s << phase->parameter_map << std::endl;
  
  return s;
}

std::ostream& operator<<(std::ostream& s, const simulation_todo_sptr_t& todo)
{
  s << "%% PhaseType: " << todo->phase << std::endl;
  s << "%% id: " <<  todo->id          << std::endl;
  s << "%% time: " << *todo->current_time << std::endl;
  s << "--- parent phase result ---" << std::endl;
  s << todo->parent << std::endl;
  s << "--- temporary_constraints ---"  << std::endl; 
  s << todo->temporary_constraints      << std::endl;
  s << "--- parameter map ---"          << std::endl;
  s << todo->parameter_map << std::endl;
  
  return s;
}


std::ostream& operator<<(std::ostream& s, const module_set_sptr& m)
{
  s << m->get_name()
    << "\n"
    << m->get_infix_string();
  return s;
}



std::ostream& operator<<(std::ostream& s, const parameter_map_t& pm){
  parameter_map_t::const_iterator it  = pm.begin();
  parameter_map_t::const_iterator end = pm.end();
  for(; it!=end; ++it) {
    s << *(it->first) << " <=> " << it->second << "\n";
  }
  return s;
}


std::ostream& operator<<(std::ostream& s, const variable_map_t& vm){
  variable_map_t::const_iterator it  = vm.begin();
  variable_map_t::const_iterator end = vm.end();
  
  for(; it!=end; ++it) {
    if(it->second.get()){
      s << *(it->first) << " <=> " << *(it->second) << "\n";
    }else{
      s << *(it->first) << " <=> UNDEF\n";
    }
  }
  
  return s;
}



std::ostream& operator<<(std::ostream& s, const variable_range_map_t& vm){
  variable_range_map_t::const_iterator it  = vm.begin();
  variable_range_map_t::const_iterator end = vm.end();
  
  for(; it!=end; ++it) {
    s << *(it->first) << "(" << it->first << ") " << " <=> " << it->second << "\n";
  }
  
  return s;
}

}
}
