#ifndef _INCLUDED_HYDLA_DUMPERS_H_
#define _INCLUDED_HYDLA_DUMPERS_H_

#include "Simulator.h"


std::ostream& operator<<(std::ostream& s, const hydla::simulator::parameter_map_t& pm);
std::ostream& operator<<(std::ostream& s, const hydla::simulator::variable_map_t& vm);
std::ostream& operator<<(std::ostream& s, const hydla::simulator::constraints_t& a);
std::ostream& operator<<(std::ostream& s, const hydla::simulator::ask_set_t& a);
std::ostream& operator<<(std::ostream& s, const hydla::simulator::tells_t& a);
std::ostream& operator<<(std::ostream& s, const hydla::simulator::collected_tells_t& a);
std::ostream& operator<<(std::ostream& s, const hydla::simulator::expanded_always_t& a);
std::ostream& operator<<(std::ostream& s, const hydla::simulator::PhaseResult& phase);
std::ostream& operator<<(std::ostream& s, const hydla::simulator::simulation_todo_sptr_t& todo);
std::ostream& operator<<(std::ostream& s, const hydla::simulator::module_set_sptr& m);



#endif // _INCLUDED_HYDLA_DUMPERS_H_
