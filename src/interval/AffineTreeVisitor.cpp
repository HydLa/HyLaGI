#include "AffineTreeVisitor.h"
#include "TreeInfixPrinter.h"
#include <exception>
#include "Backend.h"
#include "Logger.h"
#include "kv/affine.hpp"

using namespace std;
using namespace hydla::symbolic_expression;
using namespace boost;

#define HYDLA_LOGGER_NODE_VALUE \
  ;
//  HYDLA_LOGGER_DEBUG("node: ", node->get_node_type_name(), ", expr: ", get_infix_string(node), ", current_val: ", current_val_)

#define HYDLA_LOGGER_NODE_VISIT \
  ;
//  HYDLA_LOGGER_DEBUG("visit node: ", node->get_node_type_name(), ", expr: ", get_infix_string(node))


namespace hydla {
namespace interval {

/// share constants to take advantage of dependency
itvd AffineTreeVisitor::pi = kv::constants<kv::interval<double> >::pi();
itvd AffineTreeVisitor::e = kv::constants<kv::interval<double> >::e();

class ApproximateException:public std::runtime_error{
public:
  ApproximateException(const std::string& msg):
    std::runtime_error("error occurred in approximation: " + msg){}
};

AffineTreeVisitor::AffineTreeVisitor(parameter_idx_map_t &map, variable_map_t &vm):parameter_idx_map_(&map), variable_map(vm)
{}

AffineTreeVisitor::AffineTreeVisitor(variable_map_t &vm): variable_map(vm), pm_external(true)
{
  parameter_idx_map_ = new parameter_idx_map_t();
  pm_external = true;
}



AffineTreeVisitor::~AffineTreeVisitor()
{
  if(pm_external)delete parameter_idx_map_;
}

void AffineTreeVisitor::set_current_time(itvd itv)
{
  ++time_idx;
  current_time = itv;
}

AffineMixedValue AffineTreeVisitor::approximate(const node_sptr &node)
{
  differential_count = 0;
  accept(node);
  return current_val_;
}

void AffineTreeVisitor::visit(std::shared_ptr<hydla::symbolic_expression::Plus> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  accept(node->get_lhs());
  AffineMixedValue lhs = current_val_;
  accept(node->get_rhs());
  AffineMixedValue rhs = current_val_;
  current_val_ = lhs + rhs;
  HYDLA_LOGGER_NODE_VALUE;
  return;
}

void AffineTreeVisitor::visit(std::shared_ptr<hydla::symbolic_expression::Subtract> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  accept(node->get_lhs());
  AffineMixedValue lhs = current_val_;
  accept(node->get_rhs());
  AffineMixedValue rhs = current_val_;
  current_val_ = lhs - rhs;
  HYDLA_LOGGER_NODE_VALUE;
  return;
}

void AffineTreeVisitor::visit(std::shared_ptr<hydla::symbolic_expression::Times> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  accept(node->get_lhs());
  AffineMixedValue lhs = current_val_;
  accept(node->get_rhs());
  AffineMixedValue rhs = current_val_;
  current_val_ = lhs * rhs;
  HYDLA_LOGGER_NODE_VALUE;
  return;
}

void AffineTreeVisitor::visit(std::shared_ptr<hydla::symbolic_expression::Divide> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  accept(node->get_lhs());
  AffineMixedValue lhs = current_val_;
  accept(node->get_rhs());
  AffineMixedValue rhs = current_val_;
  current_val_ = lhs / rhs;
  HYDLA_LOGGER_NODE_VALUE;
  return;
}

void AffineTreeVisitor::visit(std::shared_ptr<hydla::symbolic_expression::Power> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  accept(node->get_lhs());
  AffineMixedValue lhs = current_val_;  
  // TODO: 文字列以外で判定する
  std::string rhs_str = get_infix_string(node->get_rhs());
  if(rhs_str == "1/2")
  {
    if(lhs.type == INTEGER)current_val_ = sqrt(itvd(lhs.integer));
    else if(lhs.type == INTERVAL)current_val_ = AffineMixedValue(sqrt(lhs.interval));
    else current_val_ = sqrt(lhs.affine_value);
  }
  else if(rhs_str == "(-1)/2" || rhs_str == "-1/2")
  {
    if(lhs.type == INTEGER)current_val_ = 1/sqrt(itvd(lhs.integer));
    else if(lhs.type == INTERVAL)current_val_ = 1/sqrt(lhs.interval);
    else current_val_ = 1/sqrt(lhs.affine_value);
  }
  else if(rhs_str == "2" && lhs.type == AFFINE)
  {
    current_val_ = square(lhs.affine_value);
  }
  else
  {
    accept(node->get_rhs());
    AffineMixedValue rhs = current_val_;
    current_val_ = lhs ^ rhs;
  }
  HYDLA_LOGGER_NODE_VALUE;
}

void AffineTreeVisitor::visit(std::shared_ptr<hydla::symbolic_expression::Negative> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  accept(node->get_child());
  current_val_ = -current_val_;
  HYDLA_LOGGER_NODE_VALUE;
  return;
}

void AffineTreeVisitor::visit(std::shared_ptr<hydla::symbolic_expression::Positive> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  accept(node->get_child());
  HYDLA_LOGGER_NODE_VALUE;
  return;
}

void AffineTreeVisitor::visit(std::shared_ptr<hydla::symbolic_expression::Pi> node)
{
  current_val_.interval = pi;
  current_val_.type = INTERVAL;
}

void AffineTreeVisitor::visit(std::shared_ptr<hydla::symbolic_expression::E> node)
{
  current_val_ = AffineMixedValue(e);
}

void AffineTreeVisitor::visit(std::shared_ptr<hydla::symbolic_expression::Number> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  std::string number_str = node->get_number();

  // try translation to int
  try{
    int integer = stoi(number_str);
    current_val_ = AffineMixedValue(integer);
    HYDLA_LOGGER_NODE_VALUE;
    return;
  }catch(const std::logic_error &){
  }


  //try approximation as double with upper rounding
  kv::interval<double> itv = kv::interval<double>(number_str);
  current_val_ = AffineMixedValue(itv);
  HYDLA_LOGGER_NODE_VALUE;
}

void AffineTreeVisitor::visit(std::shared_ptr<hydla::symbolic_expression::Float> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  current_val_ = AffineMixedValue(itvd(node->get_number()));
  HYDLA_LOGGER_NODE_VALUE;
}

void AffineTreeVisitor::visit(std::shared_ptr<hydla::symbolic_expression::Function> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  std::string name = node->get_name();
  if(name == "log")
  {
    if(node->get_arguments_size() != 1)invalid_node(*node);
    accept(node->get_argument(0) );
    if(current_val_.type == INTEGER)current_val_.interval = log(itvd(current_val_.integer));
    else if(current_val_.type == INTERVAL)current_val_.interval = log(current_val_.interval);
    else current_val_.affine_value = log(current_val_.affine_value);
  }
  else if(name == "sin")
  {
    if(node->get_arguments_size() != 1)invalid_node(*node);
    accept(node->get_argument(0) );
    if(current_val_.type == INTEGER)current_val_.interval = sin(itvd(current_val_.integer));
    if(current_val_.type == INTERVAL)current_val_.interval = sin(current_val_.interval);
    else current_val_.affine_value = sin(current_val_.affine_value);
  }
  else if(name == "cos")
  {
    if(node->get_arguments_size() != 1)invalid_node(*node);
    accept(node->get_argument(0) );
    if(current_val_.type == INTEGER)current_val_.interval = cos(itvd(current_val_.integer));
    if(current_val_.type == INTERVAL)current_val_.interval = cos(current_val_.interval);
    else current_val_.affine_value = cos(current_val_.affine_value);
  }
  else if(name == "sinh")
  {
    if(node->get_arguments_size() != 1)invalid_node(*node);
    accept(node->get_argument(0) );
    if(current_val_.type == INTEGER)current_val_.interval = sinh(itvd(current_val_.integer));
    if(current_val_.type == INTERVAL)current_val_.interval = sinh(current_val_.interval);
    else current_val_.affine_value = sinh(current_val_.affine_value);
  }
  else if(name == "cosh")
  {
    if(node->get_arguments_size() != 1)invalid_node(*node);
    accept(node->get_argument(0) );
    if(current_val_.type == INTEGER)current_val_.interval = cosh(itvd(current_val_.integer));
    if(current_val_.type == INTERVAL)current_val_.interval = cosh(current_val_.interval);
    else current_val_.affine_value = cosh(current_val_.affine_value);
  }
  else
  {
    invalid_node(*node);
  }
  HYDLA_LOGGER_NODE_VALUE;
}

void AffineTreeVisitor::visit(std::shared_ptr<hydla::symbolic_expression::Parameter> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  current_val_ = affine_t();
  parameter_t param(node->get_name(),
                    node->get_differential_count(),
                    node->get_phase_id());
  parameter_idx_map_t::left_iterator it = parameter_idx_map_->left.find(param);
  int idx;
  if(it == parameter_idx_map_->left.end())
  {
    idx = ++affine_t::maxnum();
    parameter_idx_map_->insert(
      parameter_idx_t(param, affine_t::maxnum()));
  }
  else
  {
    idx = it->second;
  }
  current_val_.affine_value.a.resize(idx + 1);
  current_val_.affine_value.a(idx) = 1;
  current_val_.affine_value.er = 0;
  HYDLA_LOGGER_NODE_VALUE;
  return;
}



void AffineTreeVisitor::visit(std::shared_ptr<symbolic_expression::Variable> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  simulator::Variable variable(node->get_name(), differential_count);
  if(variable_map.find(variable) == variable_map.end())throw ApproximateException("unknown variable: " + variable.get_string() );
  if(!variable_map[variable].unique())throw ApproximateException("the value of a variable must be unique: "+ variable.get_string());
  accept(variable_map[variable].get_unique_value().get_node());
  HYDLA_LOGGER_NODE_VALUE;
  return;
}


void AffineTreeVisitor::visit(std::shared_ptr<Differential> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  differential_count++;
  // TODO: 変数以外の微分値は扱えないので、その判定もしたい。
  accept(node->get_child());
  differential_count--;
  return;
}

void AffineTreeVisitor::visit(std::shared_ptr<SymbolicT> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  current_val_ = AffineMixedValue(affine_t());
  parameter_t param("t",
                    -1,
                    time_idx);
  parameter_idx_map_t::left_iterator it = parameter_idx_map_->left.find(param);
  int idx;
  if(it == parameter_idx_map_->left.end())
  {
    idx = ++affine_t::maxnum();
    parameter_idx_map_->insert(
      parameter_idx_t(param, affine_t::maxnum()));
  }
  else
  {
    idx = it->second;
  }
  current_val_.affine_value = affine_t(current_time);
  HYDLA_LOGGER_NODE_VALUE;
  return;
}

void AffineTreeVisitor::invalid_node(symbolic_expression::Node& node)
{
  throw ApproximateException("invalid node: " + node.get_string());
}


#define DEFINE_INVALID_NODE(NODE_NAME)                           \
void AffineTreeVisitor::visit(std::shared_ptr<NODE_NAME> node) \
{                                                                \
  HYDLA_LOGGER_DEBUG("");                                        \
  invalid_node(*node);                                           \
}

DEFINE_INVALID_NODE(ConstraintDefinition)
DEFINE_INVALID_NODE(ProgramDefinition)
DEFINE_INVALID_NODE(ConstraintCaller)
DEFINE_INVALID_NODE(ProgramCaller)
DEFINE_INVALID_NODE(Constraint)
DEFINE_INVALID_NODE(Ask)
DEFINE_INVALID_NODE(Tell)

DEFINE_INVALID_NODE(Equal)
DEFINE_INVALID_NODE(UnEqual)

DEFINE_INVALID_NODE(Less)
DEFINE_INVALID_NODE(LessEqual)

DEFINE_INVALID_NODE(Greater)
DEFINE_INVALID_NODE(GreaterEqual)

DEFINE_INVALID_NODE(LogicalAnd)
DEFINE_INVALID_NODE(LogicalOr)

DEFINE_INVALID_NODE(Weaker)
DEFINE_INVALID_NODE(Parallel)

DEFINE_INVALID_NODE(Always)

DEFINE_INVALID_NODE(Previous)

DEFINE_INVALID_NODE(Print)
DEFINE_INVALID_NODE(PrintPP)
DEFINE_INVALID_NODE(PrintIP)
DEFINE_INVALID_NODE(Scan)
DEFINE_INVALID_NODE(Exit)
DEFINE_INVALID_NODE(Abort)
DEFINE_INVALID_NODE(SVtimer)

DEFINE_INVALID_NODE(Not)

DEFINE_INVALID_NODE(UnsupportedFunction)

DEFINE_INVALID_NODE(ImaginaryUnit)
DEFINE_INVALID_NODE(Infinity)
DEFINE_INVALID_NODE(True)
DEFINE_INVALID_NODE(False)

DEFINE_INVALID_NODE(ProgramList)
DEFINE_INVALID_NODE(ConditionalProgramList)
DEFINE_INVALID_NODE(ExpressionList)
DEFINE_INVALID_NODE(ConditionalExpressionList)
DEFINE_INVALID_NODE(EachElement)
DEFINE_INVALID_NODE(DifferentVariable)
DEFINE_INVALID_NODE(ExpressionListElement)
DEFINE_INVALID_NODE(ExpressionListCaller)
DEFINE_INVALID_NODE(ExpressionListDefinition)
DEFINE_INVALID_NODE(ProgramListElement)
DEFINE_INVALID_NODE(ProgramListCaller)
DEFINE_INVALID_NODE(ProgramListDefinition)
DEFINE_INVALID_NODE(Range)
DEFINE_INVALID_NODE(Union)
DEFINE_INVALID_NODE(Intersection)
DEFINE_INVALID_NODE(SumOfList)
DEFINE_INVALID_NODE(MulOfList)
DEFINE_INVALID_NODE(SizeOfList)

}
}
