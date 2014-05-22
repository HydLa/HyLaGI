#ifndef _INCLUDED_HYDLA_CONSISTENCY_CHECKER_H_
#define _INCLUDED_HYDLA_CONSISTENCY_CHECKER_H_

#include <iostream>
#include <string>
#include <cassert>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "Timer.h"
#include "Logger.h"
#include "PhaseResult.h"
#include "RelationGraph.h"
#include "Simulator.h"
#include "UnsatCoreFinder.h"
#include "AnalysisResultChecker.h"

namespace hydla {

namespace simulator {

struct CheckConsistencyResult
{
  ConstraintStore consistent_store, inconsistent_store;
};

class ConsistencyChecker{

public:
  typedef hydla::symbolic_expression::node_sptr node_sptr;

  ConsistencyChecker();
  ConsistencyChecker(backend_sptr_t back);
  ConsistencyChecker(ConsistencyChecker&);

  virtual ~ConsistencyChecker();

  typedef hierarchy::module_set_sptr              modulse_set_sptr;
  typedef std::set< std::string > change_variables_t;

  CheckConsistencyResult check_consistency(const ConstraintStore& constraint_store, const PhaseType& phase);
  
  CheckConsistencyResult check_consistency(const ConstraintStore& constraint_store, const continuity_map_t&, const PhaseType& phase);
  void set_backend(backend_sptr_t back);
  
  void add_continuity(const continuity_map_t&, const PhaseType &phase);
  CheckConsistencyResult call_backend_check_consistency(const PhaseType &phase);
protected:

  typedef enum{
    ENTAILED,
    CONFLICTING,
    BRANCH_VAR,
    BRANCH_PAR
  } CheckEntailmentResult;

private:
  backend_sptr_t backend;
};


} //namespace simulator
} //namespace hydla

#endif // _INCLUDED_HYDLA_PHASE_SIMULATOR_H_
