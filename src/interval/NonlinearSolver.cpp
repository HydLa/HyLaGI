#include "Logger.h"
#include "NonlinearSolver.h"
#include "Opts.h"

namespace hydla
{
namespace interval
{
void NonlinearSolver::init(hierarchy::ModuleSetContainer* pMSC_, simulator::RelationGraph* pRG_, simulator::variable_map_t& variable_map, const Opts* pOpts_)
{
  pIMS = dynamic_cast<hierarchy::IncrementalModuleSet*>(pMSC_);
  pRG = pRG_;
  pOpts = pOpts_;

  const std::string positionVar = "position1";
  const std::string widthVar = "w";
  const std::string var1 = "theta1";
  const std::string var2 = "theta2";

  //const double init_v1 = 2.01;
  //const double init_v2 = 2.01;
  const int init_v1 = 1;
  const int init_v2 = 0;
  const std::string init_v1_str = std::to_string(init_v1);
  const std::string init_v2_str = std::to_string(init_v2);

  currentPosition = approximator.init(init_v1, init_v2);

  {
    //position1 = 3 & position1' = 0.
    //simulator::constraint_t initialConstraint = NewConstraint(NewEqual(NewVar(positionVar), NewNum(std::to_string(currentPosition))));
    simulator::constraint_t initialConstraint = NewConstraint(
        NewAnd(
            NewEqual(NewVar(positionVar), NewNum(std::to_string(currentPosition))),
            NewEqual(NewD(NewVar(positionVar)), NewNum("0"))));

    variable_map[simulator::Variable(positionVar, 0)] = simulator::ValueRange(simulator::Value(currentPosition));

    hierarchy::ModuleSet msInitial("InitPos", initialConstraint);
    hierarchy::IncrementalModuleSet imsInitial(msInitial);
    pIMS->add_parallel(imsInitial);
    for (auto m : msInitial)
    {
      pRG->add(m);
    }
  }

  {
    //theta1 = 2 & theta2 = 2
    simulator::constraint_t initialTheta = NewConstraint(
        NewAnd(
            NewEqual(NewVar(var1), NewNum(init_v1_str)),
            NewEqual(NewVar(var2), NewNum(init_v2_str))));

    variable_map[simulator::Variable(var1, 0)] = simulator::ValueRange(simulator::Value(init_v1));
    variable_map[simulator::Variable(var2, 0)] = simulator::ValueRange(simulator::Value(init_v2));

    hierarchy::ModuleSet msInitialTheta("InitTheta", initialTheta);
    hierarchy::IncrementalModuleSet imsInitialTheta(msInitialTheta);
    pIMS->add_parallel(imsInitialTheta);
    for (auto m : msInitialTheta)
    {
      pRG->add(m);
    }
  }

  {
    //[](position1' = 0)
    simulator::constraint_t positionConstraint = NewAlways(NewEqual(NewD(NewVar(positionVar)), NewNum("0")));
    hierarchy::ModuleSet msPosition("ConstPos", positionConstraint);
    hierarchy::IncrementalModuleSet imsPosition(msPosition);

    variable_map[simulator::Variable(positionVar, 1)] = simulator::ValueRange(simulator::Value("0"));

    simulator::constraint_t trueConstraint = NewAlways(NewEqual(NewNum("0"), NewNum("0")));
    hierarchy::ModuleSet msTrue("Required", trueConstraint);
    hierarchy::IncrementalModuleSet imsTrue(msTrue);

    imsTrue.add_weak(imsPosition);
    pIMS->add_parallel(imsTrue);
    for (auto m : msPosition)
    {
      pRG->add(m);
    }
    for (auto m : msTrue)
    {
      pRG->add(m);
    }
  }

  {
    //[](w' = 0)
    simulator::constraint_t widthConstraint = NewAlways(NewEqual(NewD(NewVar(widthVar)), NewNum("0")));
    hierarchy::ModuleSet msWidth("ConstWidth", widthConstraint);
    hierarchy::IncrementalModuleSet imsWidth(msWidth);

    pIMS->add_parallel(imsWidth);
    for (auto m : msWidth)
    {
      pRG->add(m);
    }
  }

  //addIP();
}

//#ifdef commentout2
void NonlinearSolver::addPP()
{
}

void NonlinearSolver::addIP()
{
  /*
  if (currentPosition == -1)
  {
    currentPosition = approximator.init();
  }
  */

  const auto& currentJumps = approximator.getJump(currentPosition);
  const auto& currentUpdate = approximator.getUpdate(currentPosition);

  const size_t jumpSize = currentJumps.size();

  const std::string positionVar = "position1";
  const std::string widthVar = "w";
  const std::string var1 = "theta1";
  const std::string var2 = "theta2";

  //[](position1' = 0)
  //simulator::constraint_t positionConstraint = NewAlways(NewEqual(NewD(NewVar(positionVar)), NewNum("0")));
  //hierarchy::ModuleSet msPosition("ConstPos", positionConstraint);
  //hierarchy::IncrementalModuleSet imsPosition(msPosition);

  //tempAddedConstraints1.push_back(positionConstraint);
  //tempAddedModuleSets1.push_back(msPosition);

  HYDLA_LOGGER_DEBUG("BREAK addIP 1");

  hierarchy::IncrementalModuleSet imsUpdate(currentUpdate.moduleSet);
  HYDLA_LOGGER_DEBUG("BREAK addIP 1-2");
  for (auto m : currentUpdate.moduleSet)
  {
    pRG->add(m);
  }
  HYDLA_LOGGER_DEBUG("BREAK addIP 1-3");
  pIMS->add_parallel(imsUpdate);
  HYDLA_LOGGER_DEBUG("BREAK addIP 1-4");

  tempAddedConstraints1.push_back(currentUpdate.constraint);
  tempAddedModuleSets1.push_back(currentUpdate.moduleSet);

  hierarchy::IncrementalModuleSet imsJumps;

  HYDLA_LOGGER_DEBUG("BREAK addIP 2");
  for (const auto& jump : currentJumps)
  {
    for (auto m : jump.moduleSet)
    {
      pRG->add(m);
    }
    hierarchy::IncrementalModuleSet tempModule(jump.moduleSet);
    imsJumps.add_parallel(tempModule);

    tempAddedConstraints1.push_back(jump.constraint);
    tempAddedModuleSets1.push_back(jump.moduleSet);
  }
  HYDLA_LOGGER_DEBUG("BREAK addIP 3");
  //imsJumps.add_weak(imsPosition);
  pIMS->add_parallel(imsJumps);
  HYDLA_LOGGER_DEBUG("BREAK addIP 4");

  const auto& currentGuards = approximator.getGuard(currentPosition);
  pOpts->guards_to_interval_newton.clear();
  for (const node_sptr guard : currentGuards)
  {
    HYDLA_LOGGER_DEBUG("BREAK addIP add guard: ", get_infix_string(guard));
    pOpts->guards_to_interval_newton.insert(get_infix_string(guard));
    tempAddedConstraints1.push_back(guard);
  }

  /*
  const int num = currentNum;

  HYDLA_LOGGER_DEBUG("NONLINEAR : p=", num);
  using hydla::symbolic_expression::Always;
  using hydla::symbolic_expression::Ask;
  using hydla::symbolic_expression::Constraint;
  using hydla::symbolic_expression::ConstraintCaller;
  using hydla::symbolic_expression::Differential;
  using hydla::symbolic_expression::Equal;
  using hydla::symbolic_expression::LogicalAnd;
  using hydla::symbolic_expression::Node;
  using hydla::symbolic_expression::Number;
  using hydla::symbolic_expression::Previous;
  using hydla::symbolic_expression::Variable;

  auto removeGuard1 =
      boost::shared_ptr<Equal>(new Equal(
          boost::shared_ptr<Previous>(new Previous(boost::shared_ptr<Variable>(new Variable("x")))),
          boost::shared_ptr<Number>(new Number("3"))));

  auto removeGuard2 =
      boost::shared_ptr<Equal>(new Equal(
          boost::shared_ptr<Previous>(new Previous(boost::shared_ptr<Variable>(new Variable("x")))),
          boost::shared_ptr<Number>(new Number("14"))));

  auto removeGuard3 =
      boost::shared_ptr<Equal>(new Equal(
          boost::shared_ptr<Previous>(new Previous(boost::shared_ptr<Variable>(new Variable("x")))),
          boost::shared_ptr<Number>(new Number("11"))));

  const int state = num % 2;
  auto currentGuard = (state == 1 ? removeGuard1 : removeGuard2);

  simulator::constraint_t constraintB = static_cast<boost::shared_ptr<Node>>(
      boost::shared_ptr<Constraint>(new Constraint(
          boost::shared_ptr<Always>(new Always(
              boost::shared_ptr<Ask>(new Ask(
                  currentGuard,
                  boost::shared_ptr<Equal>(new Equal(
                      boost::shared_ptr<Variable>(new Variable("x")),
                      boost::shared_ptr<Number>(new Number("0")))))))))));

  simulator::constraint_t constraintC = static_cast<boost::shared_ptr<Node>>(
      boost::shared_ptr<Constraint>(new Constraint(
          boost::shared_ptr<Always>(new Always(
              boost::shared_ptr<Ask>(new Ask(
                  removeGuard3,
                  boost::shared_ptr<Equal>(new Equal(
                      boost::shared_ptr<Variable>(new Variable("x")),
                      boost::shared_ptr<Number>(new Number("0")))))))))));

  hierarchy::ModuleSet msB("TestB", constraintB);
  hierarchy::IncrementalModuleSet imsB(msB);
  hierarchy::ModuleSet msC("TestC", constraintC);
  hierarchy::IncrementalModuleSet imsC(msC);

  imsB.add_parallel(imsC);

  pIMS->add_parallel(imsB);
  for (auto m : msB)
  {
    pRG->add(m);
  }
  for (auto m : msC)
  {
    pRG->add(m);
  }

  if (state == 1)
  {
    tempAddedConstraints1.push_back(static_cast<boost::shared_ptr<Node>>(currentGuard));
    tempAddedModuleSets1.push_back(msB);

    tempAddedConstraints1.push_back(static_cast<boost::shared_ptr<Node>>(removeGuard3));
    tempAddedModuleSets1.push_back(msC);
  }
  else
  {
    tempAddedConstraints2.push_back(static_cast<boost::shared_ptr<Node>>(currentGuard));
    tempAddedModuleSets2.push_back(msB);

    tempAddedConstraints2.push_back(static_cast<boost::shared_ptr<Node>>(removeGuard3));
    tempAddedModuleSets2.push_back(msC);
  }
  allAddedConstraints.push_back(static_cast<boost::shared_ptr<Node>>(currentGuard));
*/
}

void NonlinearSolver::reset()
{
  for (auto ms : tempAddedModuleSets1)
  {
    pIMS->remove(ms);
  }
  for (simulator::constraint_t p : tempAddedConstraints1)
  {
    pRG->remove(p);
  }

  tempAddedModuleSets1.clear();
  tempAddedConstraints1.clear();
  /*
  const int num = currentNum;
  const int state = num % 2;

  if (state == 1)
  {
    for (auto ms : tempAddedModuleSets1)
    {
      pIMS->remove(ms);
    }
    for (simulator::constraint_t p : tempAddedConstraints1)
    {
      pRG->remove(p);
      //pRG->set_expanded_atomic(p, false);
    }

    tempAddedModuleSets1.clear();
    tempAddedConstraints1.clear();
  }
  else
  {
    for (auto ms : tempAddedModuleSets2)
    {
      pIMS->remove(ms);
    }
    for (simulator::constraint_t p : tempAddedConstraints2)
    {
      pRG->remove(p);
      //pRG->set_expanded_atomic(p, false);
    }

    tempAddedModuleSets2.clear();
    tempAddedConstraints2.clear();
  }
*/
}
//#endif

void NonlinearSolver::dump()
{
  //pIMS->dump_priority_data_for_graphviz(std::cout);
  //pRG->dump_graph(std::cout);
  for (auto p : pRG->get_constraints())
  {
    HYDLA_LOGGER_DEBUG("BREAK NonlinearSolver::dump ", get_infix_string(p));
  }
}

}  //namespace interval
}  //namespace hydla
