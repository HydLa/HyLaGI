#include "AffineTransformer.h"
#include <boost/lexical_cast.hpp>
#include "TreeInfixPrinter.h"
#include <exception>
#include "Logger.h"

using namespace std;
using namespace hydla::parse_tree;
using namespace boost;
using namespace hydla::simulator;


namespace hydla {
namespace interval {

AffineTransformer* AffineTransformer::affine_translator_ = NULL;

AffineTransformer* AffineTransformer::get_instance()
{
  if(affine_translator_ == NULL)affine_translator_ = new AffineTransformer();
  return affine_translator_;
}

AffineTransformer::AffineTransformer()
{}

AffineTransformer::~AffineTransformer()
{}

void AffineTransformer::set_simulator(Simulator* simulator)
{
  simulator_ = simulator;
}

affine_t AffineTransformer::pow(affine_t x, affine_t y)
{
  return exp(y * log(x));
}


affine_t AffineTransformer::pow(affine_t x, int y)
{
/*
  if(x > 0)
  {
    return exp(y * log(x));
  }
  else
  {
    if(y % 2)
    {
      return -exp(y * log(-x));
    }
    else
    {
      return exp(y * log(-x));
    }
  }
*/
  return affine_t();
}

value_t AffineTransformer::transform(node_sptr& node, parameter_map_t &parameter_map)
{
  accept(node);
  value_t ret(current_val_.a(0));
  HYDLA_LOGGER(REST, current_val_);
  for(int i = 1; i < current_val_.a.size(); i++)
  {
    if(current_val_.a(i) == 0)continue;
    if(parameter_idx_map_.right.find(i)
       == parameter_idx_map_.right.end())
    {
      range_t range;
      range.set_lower_bound(value_t("-1"), true);
      range.set_upper_bound(value_t("1"), true);
      // a parameter whose differential count is -1 is regarded as a dummy variable
      parameter_t param = simulator_->introduce_parameter("affine", 0, i, range);
      parameter_idx_map_.insert(parameter_idx_t(param, i));
      parameter_map[param] = range_t(value_t(-1), value_t(1));
    }
    parameter_idx_map_t::right_iterator r_it = parameter_idx_map_.right.find(i);
    ret = ret + value_t(current_val_.a(i)) * value_t(r_it->second);
  }
  return ret;
}

void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Plus> node)
{
  accept(node->get_lhs());
  affine_t lhs = current_val_;
  accept(node->get_rhs());
  affine_t rhs = current_val_;
  current_val_ = lhs + rhs;
  HYDLA_LOGGER_VAR(INTERVAL, current_val_);
  return;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Subtract> node)
{
  accept(node->get_lhs());
  affine_t lhs = current_val_;
  accept(node->get_rhs());
  affine_t rhs = current_val_;
  current_val_ = lhs - rhs;
  HYDLA_LOGGER_VAR(INTERVAL, current_val_);
  return;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Times> node)
{
  accept(node->get_lhs());
  affine_t lhs = current_val_;
  accept(node->get_rhs());
  affine_t rhs = current_val_;
  current_val_ = lhs * rhs;
  HYDLA_LOGGER_VAR(INTERVAL, current_val_);
  return;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Divide> node)
{
  accept(node->get_lhs());
  affine_t lhs = current_val_;
  accept(node->get_rhs());
  affine_t rhs = current_val_;
  current_val_ = lhs / rhs;
  HYDLA_LOGGER_VAR(INTERVAL, current_val_);
  return;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Power> node)
{

  accept(node->get_lhs());
  affine_t lhs = current_val_;
  accept(node->get_rhs());
  affine_t rhs = current_val_;
  current_val_ = pow(lhs, rhs);
  HYDLA_LOGGER_VAR(INTERVAL, current_val_);
}


void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Negative> node)
{
  accept(node->get_child());
  current_val_ = -current_val_;
  HYDLA_LOGGER_VAR(INTERVAL, current_val_);
  return;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Positive> node)
{
  // do nothing
  return;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Pi> node)
{
  current_val_ = kv::constants<double>::pi();
}


void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::E> node)
{
  current_val_ = kv::constants<double>::e();
}


void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Number> node)
{
  double val;
  try{
    val = lexical_cast<double>(node->get_number());
  }catch(const std::exception &e){
    HYDLA_LOGGER(REST, "name: ",
                 typeid(e).name(), "\n what: ", e.what());
    invalid_node(*node);
  }
  current_val_ = affine_t(val);
}


void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Float> node)
{
  current_val_ = affine_t(node->get_number());
}

void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Function> node)
{
  string name = node->get_string();
  if(name == "ln")
  {
    if(node->get_arguments_size() != 1)invalid_node(*node);
    accept(node->get_argument(0) );
    current_val_ = log(current_val_);
  }
  else
  {
    invalid_node(*node);
  }
}

void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Parameter> node)
{
  current_val_ = affine_t();
  parameter_t param(node->get_name(),
                    node->get_derivative_count(),
                    node->get_phase_id());
  parameter_idx_map_t::left_iterator it = parameter_idx_map_.left.find(param);
  int idx;
  if(it == parameter_idx_map_.left.end())
  {
    idx = affine_t::maxnum();
    parameter_idx_map_.insert(
      parameter_idx_t(param, affine_t::maxnum()));
  }
  else
  {
    idx = it->second;
  }
  current_val_.a.resize(idx + 1);
  current_val_.a(idx) = 1;
  return;
}


void AffineTransformer::invalid_node(parse_tree::Node& node)
{
  // TODO: throw exception specified for interval
  throw std::exception();
}


#define DEFINE_INVALID_NODE(NODE_NAME)        \
void AffineTransformer::visit(boost::shared_ptr<NODE_NAME> node) \
{                                                     \
  invalid_node(*node);                                 \
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
DEFINE_INVALID_NODE(Differential)

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

DEFINE_INVALID_NODE(Variable)
DEFINE_INVALID_NODE(SymbolicT)
DEFINE_INVALID_NODE(Infinity)
DEFINE_INVALID_NODE(True)
DEFINE_INVALID_NODE(False)


}
}
