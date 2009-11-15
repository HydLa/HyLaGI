#ifndef _INCLUDED_HYDLA_SIMULATOR_H_
#define _INCLUDED_HYDLA_SIMULATOR_H_

#include <iostream>

#include <string>
#include <map>
#include <set>
#include <queue>

#include <boost/bind.hpp>

#include "ModuleSetContainer.h"

namespace hydla {
namespace simulator {

template<typename VariableType, typename ValueType>
class Simulator
{
public:
  typedef struct ConstraintStore_ {
    std::map<VariableType, ValueType> variable_list_;
  } ConstraintStore;

  typedef struct State_ {
//    std::set<ask_index_t> entailed_ask;
//    std::vector<boost::shared_ptr<hydla::parse_tree::Always> > expanded_always;
    bool initial_time;
  } State;

  Simulator()
  {}
  
  virtual ~Simulator()
  {}

  void simulate(boost::shared_ptr<hydla::ch::ModuleSetContainer> msc)
  {
    //state_queue_
    State state;
    msc->dispatch(
      boost::bind(&Simulator<VariableType, ValueType>::execute_module_set, 
                  this, 
                  _1,
                  &state));
    
  }

  bool execute_module_set(hydla::ch::module_set_sptr ms, State* state)
  {
    std::cout << ms->get_name() << std::endl;
    std::cout << ms->get_tree_dump() << std::endl;

//     point_phase();
//     interval_phase();

    return false;
  }


protected:
//   bool point_phase(module_set_sptr& ms, 
//                    std::vector<t> prev_vars)
//   {
//     ConsistencyCheckerPoint ccp(ms, ml_);
//     EntailmentCheckerPoint  ecp(ms, ml_);


//     bool expanded = true;
//     while(expanded) {
//       // 制約が充足しているかどうか確認
//       if(!ctv.is_consistent(entailed_ask)) return false;

//       // 成り立つ含意制約を調べる
//       expanded = ecp.entail_check(entailed_ask);
//     }
  
//     return true;
//   }

//   bool interval_phase(module_set_sptr& ms,
//                       std::vector<t> initial_vars)
//   {
//     ConsistencyCheckerInterval cci(ms, ml_);
//     EntailmentCheckerInterval  eci(ms, ml_);

//     std::vector<ask_index_t> entailed_ask;
//     bool expanded = true;
//     while(expanded) {
//       // 制約が充足しているかどうか確認
//       if(!ctv.is_consistent(entailed_ask)) return false;

//       // 成り立つ含意制約を調べる
//       expanded = ecp.entail_check(entailed_ask);
//     }

//     return false;
//   }

private:
  std::queue<State> state_queue_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_H_
