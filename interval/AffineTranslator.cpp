#include "AffineTranslator.h"

using namespace std;
using namespace hydla::parse_tree;

namespace hydla {
namespace interval {

AffineTranslator::AffineTranslator()
{}

AffineTranslator::~AffineTranslator()
{}

void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::Plus> node)
{
}


void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::Subtract> node)
{
}


void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::Times> node)
{
}


void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::Divide> node)
{
}


void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::Power> node)
{
}


void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::Negative> node)
{
}


void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::Positive> node)
{
}


void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::Pi> node)
{
}


void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::E> node)
{
}


void AffineTranslator::visit(boost::shared_ptr<hydla::parse_tree::Function> node)
{
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
