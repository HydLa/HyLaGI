#include "IntervalTreeVisitor.h"
#include "Logger.h"
#include <exception>
#include <boost/lexical_cast.hpp>

namespace hydla
{
namespace interval
{

using namespace hydla::symbolic_expression;

itvd IntervalTreeVisitor::pi = kv::constants<itvd>::pi();
itvd IntervalTreeVisitor::e = kv::constants<itvd>::e();

class IntervalException : public std::runtime_error
{
  public:
  IntervalException(const std::string& msg):
    std::runtime_error("error occured in interval calculation: " + msg){}
};


IntervalTreeVisitor::IntervalTreeVisitor()
{
}


itvd IntervalTreeVisitor::get_interval_value(const node_sptr& node, itvd *t, parameter_map_t *map)
{
  time_interval = t;
  parameter_map = map;
  accept(node);
  if(current_value.is_integer)
    return itvd(current_value.integer);
  else
    return current_value.interval_value;
}


void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Plus> node)
{
  accept(node->get_lhs());
  IntervalOrInteger lhs = current_value;
  accept(node->get_rhs());
  IntervalOrInteger rhs = current_value;
  current_value = lhs + rhs;
  // HYDLA_LOGGER_DEBUG("Plus : ", current_value.interval_value);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Subtract> node)
{
  accept(node->get_lhs());
  IntervalOrInteger lhs = current_value;
  accept(node->get_rhs());
  IntervalOrInteger rhs = current_value;
  current_value = lhs - rhs;
  // HYDLA_LOGGER_DEBUG("Subtract : ", current_value.interval_value);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Times> node)
{
  accept(node->get_lhs());
  IntervalOrInteger lhs = current_value;
  accept(node->get_rhs());
  IntervalOrInteger rhs = current_value;
  current_value = lhs * rhs;
  // HYDLA_LOGGER_DEBUG("Times : ", current_value.interval_value);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Divide> node)
{
  accept(node->get_lhs());
  IntervalOrInteger lhs = current_value;
  accept(node->get_rhs());
  IntervalOrInteger rhs = current_value;
  current_value = lhs / rhs;
  // HYDLA_LOGGER_DEBUG("Divide : ", current_value.interval_value);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Power> node)
{
  accept(node->get_lhs());
  IntervalOrInteger lhs = current_value;
  accept(node->get_rhs());
  IntervalOrInteger rhs = current_value;

  itvd base;
  if(lhs.is_integer)
    base = itvd(lhs.integer);
  else
    base = lhs.interval_value;
  // TODO: avoid string comparison
  if(rhs.is_integer)
  {
    current_value.interval_value = pow(base, rhs.integer);
  }
  else if(get_infix_string(node->get_rhs()) == "1/2")
  {
    current_value.interval_value = sqrt(base);
  }
  else
    current_value.interval_value = pow(base, rhs.interval_value);

  current_value.is_integer = false;
  // HYDLA_LOGGER_DEBUG("Power : ", current_value.interval_value);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Negative> node)
{
  accept(node->get_child());
  current_value = -current_value;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Positive> node)
{
  accept(node->get_child());
}


void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Pi> node)
{
  current_value.interval_value = pi;
  current_value.is_integer = false;
  // HYDLA_LOGGER_DEBUG("Pi : ", current_value.interval_value);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::E> node)
{
  current_value.interval_value = e;
  current_value.is_integer = false;
  // HYDLA_LOGGER_DEBUG("E : ", current_value.interval_value);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Number> node)
{
  std::string number_str = node->get_number();

  // try translation to int
  try
  {
    int integer = boost::lexical_cast<int>(number_str);
    current_value.is_integer = true;
    current_value.integer = integer;
    // HYDLA_LOGGER_DEBUG("Number : ", current_value.integer);
    // HYDLA_LOGGER_NODE_VALUE;
    return;
  }
  catch(const boost::bad_lexical_cast &e)
  {
    // do nothing
  }

  itvd itv = itvd(number_str);
  current_value.interval_value = itv;
  current_value.is_integer = false;
  // HYDLA_LOGGER_DEBUG("Number : ", current_value.interval_value);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Float> node)
{
  current_value.interval_value = itvd(node->get_number());
  current_value.is_integer = false;
  // HYDLA_LOGGER_DEBUG("Float : ", current_value.interval_value);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Function> node)
{
  std::string name = node->get_name();
  itvd arg;
  if(name == "sin")
  {
    if(node->get_arguments_size() != 1)
    {
      invalid_node(*node);
    }
    accept(node->get_argument(0));
    
    if(current_value.is_integer)
    {
      arg = itvd(current_value.integer);
    }
    else
    {
      arg = current_value.interval_value;
    }

    current_value.interval_value = sin(arg);
    current_value.is_integer = false;
    // HYDLA_LOGGER_DEBUG("Sin : ", current_value.interval_value);
  }
  else if(name == "cos")
  {
    if(node->get_arguments_size() != 1)
    {
      invalid_node(*node); 
    }
    accept(node->get_argument(0));

    if(current_value.is_integer)
    {
      arg = itvd(current_value.integer);
    }
    else
    {
      arg = current_value.interval_value;
    }
    
    current_value.interval_value = cos(arg);
    current_value.is_integer = false;
    // HYDLA_LOGGER_DEBUG("Cos : ", current_value.interval_value);
  }
  else if(name == "tan")
  {
    if(node->get_arguments_size() != 1)
    {
      invalid_node(*node);
    }
    accept(node->get_argument(0));

    if(current_value.is_integer)
    {
      arg = itvd(current_value.integer);
    }
    else
    {
      arg = current_value.interval_value;
    }
    
    current_value.interval_value = tan(arg);
    current_value.is_integer = false;
  }
  else if(name == "log")
  {
    if(node->get_arguments_size() != 1)
    {
      invalid_node(*node);
    }
    accept(node->get_argument(0));

    if(current_value.is_integer)
    {
      arg = itvd(current_value.integer);
    }
    else
    {
      arg = current_value.interval_value;
    }
    
    current_value.interval_value = log(arg);
    current_value.is_integer = false;
    // HYDLA_LOGGER_DEBUG("Log : ", current_value.interval_value);
  }
  else if(name == "sinh")
  {
    if(node->get_arguments_size() != 1)
    {
      invalid_node(*node);
    }
    accept(node->get_argument(0));

    if(current_value.is_integer)
    {
      arg = itvd(current_value.integer);
    }
    else
    {
      arg = current_value.interval_value;
    }
    
    current_value.interval_value = sinh(arg);
    current_value.is_integer = false;
  }
  else if(name == "cosh")
  {
    if(node->get_arguments_size() != 1)
    {
      invalid_node(*node);
    }
    accept(node->get_argument(0));

    if(current_value.is_integer)
    {
      arg = itvd(current_value.integer);
    }
    else
    {
      arg = current_value.interval_value;
    }
    
    current_value.interval_value = cosh(arg);
    current_value.is_integer = false;
  }
  else if(name == "tanh")
  {
    if(node->get_arguments_size() != 1)
    {
      invalid_node(*node);
    }
    accept(node->get_argument(0));

    if(current_value.is_integer)
    {
      arg = itvd(current_value.integer);
    }
    else
    {
      arg = current_value.interval_value;
    }
    
    current_value.interval_value = tanh(arg);
    current_value.is_integer = false;
  }
  else
  {
    invalid_node(*node);
  }
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::SymbolicT> node)
{
  if(time_interval == nullptr)invalid_node(*node);
  current_value.interval_value = *time_interval;
  current_value.is_integer = false;
  // HYDLA_LOGGER_DEBUG("SymbolicT : ", current_value.interval_value);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Parameter> node)
{
  if(parameter_map == nullptr)invalid_node(*node);
  parameter_t param(node->get_name(),
                    node->get_differential_count(),
                    node->get_phase_id());
  auto param_it = parameter_map->find(param);
  if(param_it == parameter_map->end())invalid_node(*node);

  range_t range = param_it->second;;

  if(range.unique())
  {
    accept(range.get_unique_value().get_node());
  }
  else
  {
    value_t lower_value = (range.get_lower_bound()).value;
    accept(lower_value.get_node());
    itvd lower_itvd;
    if(current_value.is_integer)
      lower_itvd = itvd(current_value.integer);
    else
      lower_itvd = current_value.interval_value;

    value_t uppper_value = (range.get_upper_bound()).value;
    accept(uppper_value.get_node());
    itvd upper_itvd;
    if(current_value.is_integer)
      upper_itvd = itvd(current_value.integer);
    else
      upper_itvd = current_value.interval_value;

    current_value.interval_value = itvd(lower_itvd.lower(), upper_itvd.upper());
    current_value.is_integer = false;
  }
}


void IntervalTreeVisitor::invalid_node(symbolic_expression::Node &node)
{
  throw IntervalException("invalid node: " + node.get_string());
}

void IntervalTreeVisitor::debug_print(std::string str, itvd x)
{
  std::cout << str << x << "\n";
}


#define DEFINE_INVALID_NODE(NODE_NAME)                                \
  void IntervalTreeVisitor::visit(boost::shared_ptr<NODE_NAME> node)  \
{                                                                \
  HYDLA_LOGGER_DEBUG("");                                        \
  invalid_node(*node);                                           \
}


DEFINE_INVALID_NODE(Variable)
DEFINE_INVALID_NODE(Differential)

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
DEFINE_INVALID_NODE(SizeOfList)


} // namespace interval
} // namespace hydla
