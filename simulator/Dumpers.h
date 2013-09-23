#ifndef _INCLUDED_HYDLA_DUMPERS_H_
#define _INCLUDED_HYDLA_DUMPERS_H_

#include "Simulator.h"


namespace hydla {
namespace simulator {
std::ostream& operator<<(std::ostream& s, const parameter_map_t& pm);
std::ostream& operator<<(std::ostream& s, const variable_map_t& vm);
std::ostream& operator<<(std::ostream& s, const constraints_t& a);
std::ostream& operator<<(std::ostream& s, const ask_set_t& a);
std::ostream& operator<<(std::ostream& s, const tells_t& a);
std::ostream& operator<<(std::ostream& s, const collected_tells_t& a);
std::ostream& operator<<(std::ostream& s, const expanded_always_t& a);
std::ostream& operator<<(std::ostream& s, const phase_result_sptr_t& phase);
std::ostream& operator<<(std::ostream& s, const simulation_todo_sptr_t& todo);
std::ostream& operator<<(std::ostream& s, const module_set_sptr& m);
}
}



#endif // _INCLUDED_HYDLA_DUMPERS_H_
