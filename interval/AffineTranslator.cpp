#include "AffineTranslator.h"
#include <boost/lexical_cast.hpp>
#include "TreeInfixPrinter.h"
#include <exception>
#include "Logger.h"

using namespace std;
using namespace hydla::parse_tree;
using namespace boost;

namespace hydla {
namespace interval {

AffineTranslator::AffineTranslator()
{}

AffineTranslator::~AffineTranslator()
{}

affine_t AffineTranslator::translate(node_sptr& node)
{
  accept(node);
  return current_val_;
}

void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::Plus> node)
{
  accept(node->get_lhs());
  affine_t lhs = current_val_;
  accept(node->get_rhs());
  affine_t rhs = current_val_;
  current_val_ = lhs + rhs;
  return;
}


void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::Subtract> node)
{
  accept(node->get_lhs());
  affine_t lhs = current_val_;
  accept(node->get_rhs());
  affine_t rhs = current_val_;
  current_val_ = lhs - rhs;
  return;
}


void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::Times> node)
{
  accept(node->get_lhs());
  affine_t lhs = current_val_;
  accept(node->get_rhs());
  affine_t rhs = current_val_;
  current_val_ = lhs * rhs;
  return;
}


void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::Divide> node)
{
  accept(node->get_lhs());
  affine_t lhs = current_val_;
  accept(node->get_rhs());
  affine_t rhs = current_val_;
  current_val_ = lhs / rhs;
  return;
}


void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::Power> node)
{
  string rhs_str = TreeInfixPrinter().get_infix_string(node->get_rhs());
  HYDLA_LOGGER(REST, rhs_str);
  if(rhs_str == "1/2")
  {
    accept(node->get_lhs());
    current_val_ = sqrt(current_val_);
  }
  else if(rhs_str == "2")
  {
    accept(node->get_lhs());
    current_val_ = square(current_val_);
  }
  else
  {
    //TODO: deal with general Power
    invalid_node(*node);
  }
}


void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::Negative> node)
{
  accept(node->get_child());
  current_val_ = -current_val_;
  return;
}


void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::Positive> node)
{
  // do nothing
  return;
}


void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::Pi> node)
{
  current_val_ = kv::constants<double>::pi();
}


void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::E> node)
{
  current_val_ = kv::constants<double>::e();
}


void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::Number> node)
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

void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::Function> node)
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

void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::Parameter> node)
{
  // TODO: do something
  return;
}


void AffineTranslator::invalid_node(parse_tree::Node& node)
{
  // TODO: throw exception specified for interval
  throw std::exception();
}


#define DEFINE_INVALID_NODE(NODE_NAME)        \
void AffineTranslator::visit(boost::shared_ptr<NODE_NAME> node) \
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
