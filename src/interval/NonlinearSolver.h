#pragma once
#include <vector>
#include "Simulator.h"
#include "ModuleSet.h"
#include "IncrementalModuleSet.h"
#include "RelationGraph.h"
#include "NonlinearFunctionApproximator.h"
#include "Logger.h"
#include "Opts.h"

namespace hydla {
namespace interval {

class NonlinearSolver{
public:
  NonlinearSolver() = default;

  void init(hierarchy::ModuleSetContainer*, simulator::RelationGraph*, simulator::variable_map_t& variable_map, const Opts*);

  /*
  void addPP();
  void addIP(int num);
  void reset();

  void dump();

  const std::vector<simulator::constraint_t>& getTempAddedConstraints()const
  {
    return tempAddedConstraints1;
  }
  const std::vector<hierarchy::ModuleSet>& getTempAddedModuleSets()const
  {
    return tempAddedModuleSets1;
  }
  */

  void updateNum(int num)
  {
    //++currentNum;
    currentPosition = num;
    HYDLA_LOGGER_DEBUG("BREAK currentPosition: ", currentPosition);
  }
  
  void addPP();
  void addIP();
  void reset();

  void dump();

  const std::vector<simulator::constraint_t>& getTempAddedConstraints()const
  {
    //return currentNum % 2 == 1 ? tempAddedConstraints1 : tempAddedConstraints2;
    return tempAddedConstraints1;
  }
  const std::vector<hierarchy::ModuleSet>& getTempAddedModuleSets()const
  {
    //return currentNum % 2 == 1 ? tempAddedModuleSets1 : tempAddedModuleSets2;
    return tempAddedModuleSets1;
  }
  
  std::vector<simulator::constraint_t>& getAllAddedConstraints()
  {
    return allAddedConstraints;
  }

private:
  std::vector<simulator::constraint_t> tempAddedConstraints1;
  std::vector<hierarchy::ModuleSet> tempAddedModuleSets1;

  std::vector<simulator::constraint_t> tempAddedConstraints2;
  std::vector<hierarchy::ModuleSet> tempAddedModuleSets2;

  //for destruction only
  std::vector<simulator::constraint_t> allAddedConstraints;

  NonlinearFunctionApproximator approximator;
  int currentPosition = -1;

  hierarchy::IncrementalModuleSet* pIMS;
  simulator::RelationGraph* pRG;
  const Opts* pOpts;

  int currentNum = 0;

};

} //namespace interval
} //namespace hydla
