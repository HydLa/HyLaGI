#pragma once
#include <vector>
#include "Simulator.h"
#include "ModuleSet.h"
#include "IncrementalModuleSet.h"
#include "RelationGraph.h"

namespace hydla {
namespace interval {
using namespace hydla::symbolic_expression;

using PNode = boost::shared_ptr<Node>;

inline PNode NewConstraint(PNode p) { return static_cast<PNode>(boost::shared_ptr<Constraint>(new Constraint(p))); }
inline PNode NewAlways(PNode p) { return static_cast<PNode>(boost::shared_ptr<Always>(new Always(p))); }
inline PNode NewPrev(PNode p) { return static_cast<PNode>(boost::shared_ptr<Previous>(new Previous(p))); }
inline PNode NewD(PNode p) { return static_cast<PNode>(boost::shared_ptr<Differential>(new Differential(p))); }
inline PNode NewEqual(PNode p, PNode q) { return static_cast<PNode>(boost::shared_ptr<Equal>(new Equal(p, q))); }
inline PNode NewGreaterEq(PNode p, PNode q) { return static_cast<PNode>(boost::shared_ptr<GreaterEqual>(new GreaterEqual(p, q))); }
inline PNode NewLessEq(PNode p, PNode q) { return static_cast<PNode>(boost::shared_ptr<LessEqual>(new LessEqual(p, q))); }
inline PNode NewAdd(PNode p, PNode q) { return static_cast<PNode>(boost::shared_ptr<Plus>(new Plus(p, q))); }
inline PNode NewMul(PNode p, PNode q) { return static_cast<PNode>(boost::shared_ptr<Times>(new Times(p, q))); }
inline PNode NewAsk(PNode p, PNode q) { return static_cast<PNode>(boost::shared_ptr<Ask>(new Ask(p, q))); }
inline PNode NewAnd(PNode p, PNode q) { return static_cast<PNode>(boost::shared_ptr<LogicalAnd>(new LogicalAnd(p, q))); }
inline PNode NewNum(const std::string& n) { return static_cast<PNode>(boost::shared_ptr<Number>(new Number(n))); }
inline PNode NewVar(const std::string& n) { return static_cast<PNode>(boost::shared_ptr<Variable>(new Variable(n))); }

struct DynamicModule
{
  DynamicModule() = default;
  DynamicModule(const std::string& name, simulator::constraint_t constraint):
    moduleSet(name, constraint),
    constraint(constraint)
  {}

  hierarchy::ModuleSet moduleSet;
  simulator::constraint_t constraint;
};

struct FunctionBounds
{
  FunctionBounds() = default;
  void add(const DynamicModule& bound)
  {
    bounds.push_back(bound);
  }
  size_t size()const
  {
    return bounds.size();
  }
  std::vector<DynamicModule>::iterator begin(){return bounds.begin();}
  std::vector<DynamicModule>::const_iterator begin()const{return bounds.begin();}
  std::vector<DynamicModule>::iterator end(){return bounds.end();}
  std::vector<DynamicModule>::const_iterator end()const{return bounds.end();}
  std::vector<DynamicModule> bounds;
  int x = 2;
};

class NonlinearFunctionApproximator{
public:
  NonlinearFunctionApproximator() = default;

  int init(double init_v1, double init_v2);

  const std::vector<node_sptr>& getGuard(int i)const
  {
    return guards[i];
  }
  const FunctionBounds& getJump(int i)const
  {
    //HYDLA_LOGGER_DEBUG("BREAK FunctionBounds size: ", jumps.size());
    //std::cout << "BREAK FunctionBounds size: " << jumps.size() << std::endl;
    return jumps[i];
  }
  const DynamicModule& getUpdate(int i)const
  {
    //HYDLA_LOGGER_DEBUG("BREAK DynamicModules size: ", updates.size());
    //std::cout << "BREAK DynamicModules size: " << updates.size() << std::endl;
    return updates[i];
  }

private:
  std::vector<FunctionBounds> jumps;
  std::vector<std::vector<node_sptr>> guards;
  std::vector<DynamicModule> updates;
  
};

} //namespace interval
} //namespace hydla
