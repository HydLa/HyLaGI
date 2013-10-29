#ifndef _INCLUDED_HYDLA_UNSAT_CORE_FINDER_H_
#define _INCLUDED_HYDLA_UNSAT_CORE_FINDER_H_

#include "SymbolicTypes.h"

#include "ParseTree.h"
#include "Simulator.h"
#include "Backend.h"

namespace hydla{
namespace simulator{
namespace symbolic {

class UnsatCoreFinder
{
public:
  UnsatCoreFinder(const boost::shared_ptr<hydla::backend::Backend> &back);
  virtual ~UnsatCoreFinder();

  typedef std::map<std::string, hydla::parse_tree::node_sptr> false_map_t;

  void print_unsat_cores(std::map<hydla::parse_tree::node_sptr,std::string> S,std::map<const std::string,int> S4C);

  void find_unsat_core(const module_set_sptr& ms,std::map<hydla::parse_tree::node_sptr,std::string> S,std::map<const std::string,int> S4C,simulation_todo_sptr_t&,const variable_map_t&);

  void check_all_module_set();

  bool check_inconsistency();

  bool check_unsat_core(std::map<hydla::parse_tree::node_sptr,std::string> S,std::map<const std::string,int> S4C, const module_set_sptr& ms, simulation_todo_sptr_t&, const variable_map_t&);

  void add_constraints(std::map<hydla::parse_tree::node_sptr,std::string> S,std::map<const std::string,int> S4C);

protected:

  boost::shared_ptr<hydla::backend::Backend> backend_;
  Phase phase_;
};

}
}
}
#endif //_INCLUDED_HYDLA_UNSAT_CORE_FINDER_H_

