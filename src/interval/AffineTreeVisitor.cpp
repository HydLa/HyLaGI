#include "AffineTreeVisitor.h"
#include <boost/lexical_cast.hpp>
#include "TreeInfixPrinter.h"
#include <exception>
#include "Backend.h"
#include "Logger.h"
#include "kv/affine.hpp"

using namespace std;
using namespace hydla::symbolic_expression;
using namespace boost;

#define HYDLA_LOGGER_NODE_VALUE \
  HYDLA_LOGGER_DEBUG("node: ", node->get_node_type_name(), ", expr: ", get_infix_string(node), ", current_val: ", current_val_)

#define HYDLA_LOGGER_NODE_VISIT \
  HYDLA_LOGGER_DEBUG("visit node: ", node->get_node_type_name(), ", expr: ", get_infix_string(node))


namespace hydla {
namespace interval {

/// share constants to take advantage of dependency
affine_t AffineTreeVisitor::pi = affine_t(kv::constants<kv::interval<double> >::pi());
affine_t AffineTreeVisitor::e = affine_t(kv::constants<kv::interval<double> >::e());

class ApproximateException:public std::runtime_error{
public:
  ApproximateException(const std::string& msg):
    std::runtime_error("error occurred in approximation: " + msg){}
};

AffineTreeVisitor::AffineTreeVisitor(parameter_idx_map_t &map, variable_map_t &vm):parameter_idx_map_(map), variable_map(vm)
{}

AffineTreeVisitor::~AffineTreeVisitor()
{}

AffineOrInteger AffineTreeVisitor::approximate(node_sptr &node)
{
  differential_count = 0;
  accept(node);
  return current_val_;
}

affine_t AffineTreeVisitor::pow(affine_t affine, int exp)
{
  bool is_negative = exp < 0;
  exp = is_negative?-exp:exp;
  affine_t power_val(1);
  while(exp != 0)
  {
    if(exp & 1)
    {
      power_val = power_val * affine;
    }
    affine *= affine;
    exp /= 2;
  }
  return power_val;
}

AffineOrInteger AffineTreeVisitor::pow(AffineOrInteger x, AffineOrInteger y)
{
  AffineOrInteger ret;
  if(x.is_integer && y.is_integer)
  {
    ret.is_integer = true;
    ret.integer = ::pow(x.integer, y.integer);
  }
  else
  {
    ret.is_integer = false;
    affine_t x_affine, y_affine;
    if(x.is_integer)x_affine = x.integer;
    else x_affine = x.affine_value;
    if(y.is_integer)y_affine = y.integer;
    else y_affine = y.affine_value;
    kv::interval<double> itv = to_interval(x_affine);
    double l = itv.lower(), u = itv.upper();
    if(u >= 0 && l >= 0)
    {
      HYDLA_LOGGER_DEBUG(x_affine);
      HYDLA_LOGGER_DEBUG(y_affine);
      HYDLA_LOGGER_DEBUG(x.affine_value);
      HYDLA_LOGGER_DEBUG(y.affine_value);
      //ret.affine_value = exp(y_affine * log(x_affine));
      ret.affine_value = exp(y.affine_value * log(x.affine_value));
      HYDLA_LOGGER_DEBUG(ret.affine_value);
    }
    else
    {
      if(!y.is_integer)
      {
          HYDLA_LOGGER_DEBUG("l: ", l, ", u: ", u);
          throw ApproximateException("noninteger power function for interval including zero");
      }
      if(u >= 0)
      {
        // include zero
        ret.affine_value = pow(x_affine, y.integer);
        HYDLA_LOGGER_DEBUG_VAR(ret.affine_value);
      }
      else
      {
        // pure negative interval
        if(y.integer % 2)
        {
          ret.affine_value = -exp(y_affine * log(-x_affine));
        }
        else
        {
          ret.affine_value = exp(y_affine * log(-x_affine));
        }
      }
    }
  }
  return ret;
}

void AffineTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Plus> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  accept(node->get_lhs());
  AffineOrInteger lhs = current_val_;
  accept(node->get_rhs());
  AffineOrInteger rhs = current_val_;
  current_val_ = lhs + rhs;
  HYDLA_LOGGER_NODE_VALUE;
  return;
}


void AffineTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Subtract> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  accept(node->get_lhs());
  AffineOrInteger lhs = current_val_;
  accept(node->get_rhs());
  AffineOrInteger rhs = current_val_;
  current_val_ = lhs - rhs;
  HYDLA_LOGGER_NODE_VALUE;
  return;
}


void AffineTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Times> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  accept(node->get_lhs());
  AffineOrInteger lhs = current_val_;
  accept(node->get_rhs());
  AffineOrInteger rhs = current_val_;
  current_val_ = lhs * rhs;
  HYDLA_LOGGER_NODE_VALUE;
  return;
}


void AffineTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Divide> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  accept(node->get_lhs());
  AffineOrInteger lhs = current_val_;
  accept(node->get_rhs());
  AffineOrInteger rhs = current_val_;
  current_val_ = lhs / rhs;
  HYDLA_LOGGER_NODE_VALUE;
  return;
}


AffineOrInteger AffineTreeVisitor::sqrt_affine(const AffineOrInteger &a)
{
  HYDLA_LOGGER_DEBUG_VAR(a);
  affine_t affine_value;
  if(a.is_integer)
  {
    affine_value = a.integer;
  }
  else
  {
    affine_value = a.affine_value;
  }
  affine_value = sqrt(affine_value);
  AffineOrInteger result_ai;
  result_ai.is_integer = false;
  result_ai.affine_value = affine_value;
  return result_ai;
}

void AffineTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Power> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  accept(node->get_lhs());
  AffineOrInteger lhs = current_val_;  
  // TODO: 文字列以外で判定する
  std::string rhs_str = get_infix_string(node->get_rhs());
  if(rhs_str == "1/2")
  {
    current_val_ = sqrt_affine(lhs);
  }
  else if(rhs_str == "2" && !lhs.is_integer)
  {
    current_val_.affine_value = square(lhs.affine_value);
    current_val_.is_integer = false;
  }
  else
  {
    accept(node->get_rhs());
    AffineOrInteger rhs = current_val_;
    current_val_ = pow(lhs, rhs);
  }
  HYDLA_LOGGER_NODE_VALUE;
}

void AffineTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Negative> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  accept(node->get_child());
  current_val_ = -current_val_;
  HYDLA_LOGGER_NODE_VALUE;
  return;
}


void AffineTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Positive> node)
{
  // do nothing
  return;
}

void AffineTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Pi> node)
{
  current_val_.affine_value = pi;
  current_val_.is_integer = false;
}

void AffineTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::E> node)
{
  current_val_.affine_value = e;
  current_val_.is_integer = false;
}


void AffineTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Number> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  std::string number_str = node->get_number();

  // try translation to int
  try{
    int integer = lexical_cast<int>(number_str);
    current_val_.is_integer = true;
    current_val_.integer = integer;
    HYDLA_LOGGER_NODE_VALUE;
    return;
  }catch(const bad_lexical_cast &e){
    HYDLA_LOGGER_DEBUG("name: ",
                 typeid(e).name(), "\n what: ", e.what());
  }

  //try approximation as double with upper rounding
  kv::interval<double> itv = kv::interval<double>(number_str);
  current_val_.affine_value = affine_t(itv);
  current_val_.is_integer = false;
  HYDLA_LOGGER_NODE_VALUE;
}


void AffineTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Float> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  current_val_.affine_value = affine_t(node->get_number());
  current_val_.is_integer = false;
  HYDLA_LOGGER_NODE_VALUE;
}

void AffineTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Function> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  std::string name = node->get_string();
  HYDLA_LOGGER_DEBUG(name);
  if(name == "log")
  {
    if(node->get_arguments_size() != 1)invalid_node(*node);
    accept(node->get_argument(0) );
    if(current_val_.is_integer)current_val_.affine_value = log(current_val_.integer);
    else current_val_.affine_value = log(current_val_.affine_value);
  }
  else
  {
    invalid_node(*node);
  }
  HYDLA_LOGGER_NODE_VALUE;
}

void AffineTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Parameter> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  current_val_.is_integer = false;
  current_val_.affine_value = affine_t();
  parameter_t param(node->get_name(),
                    node->get_differential_count(),
                    node->get_phase_id());
  parameter_idx_map_t::left_iterator it = parameter_idx_map_.left.find(param);
  int idx;
  if(it == parameter_idx_map_.left.end())
  {
    idx = ++affine_t::maxnum();
    parameter_idx_map_.insert(
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



void AffineTreeVisitor::visit(boost::shared_ptr<symbolic_expression::Variable> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  simulator::Variable variable(node->get_name(), differential_count);
  if(variable_map.find(variable) == variable_map.end())throw ApproximateException("unknown variable: " + variable.get_string() );
  if(!variable_map[variable].unique())throw ApproximateException("the value of a variable must be unique: "+ variable.get_string());
  accept(variable_map[variable].get_unique_value().get_node());
  HYDLA_LOGGER_NODE_VALUE;
  return;
}


void AffineTreeVisitor::visit(boost::shared_ptr<Differential> node)
{
  HYDLA_LOGGER_NODE_VISIT;
  differential_count++;
  // TODO: 変数以外の微分値は扱えないので、その判定もしたい。
  accept(node->get_child());
  differential_count--;
  return;
}

void AffineTreeVisitor::invalid_node(symbolic_expression::Node& node)
{
  throw ApproximateException("invalid node: " + node.get_string());
}


#define DEFINE_INVALID_NODE(NODE_NAME)                           \
void AffineTreeVisitor::visit(boost::shared_ptr<NODE_NAME> node) \
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

DEFINE_INVALID_NODE(SymbolicT)
DEFINE_INVALID_NODE(Infinity)
DEFINE_INVALID_NODE(True)
DEFINE_INVALID_NODE(False)


}
}