#include "VariableReplacer.h"
#include "SymbolicValue.h"
#include "Logger.h"
#include "Dumpers.h"

using namespace std;
using namespace hydla::parse_tree;

namespace hydla {
namespace simulator {



VariableReplacer::VariableReplacer(const variable_map_t& map):variable_map(map)
{}

VariableReplacer::~VariableReplacer()
{}

void VariableReplacer::replace_value(value_t& val)
{
  differential_cnt = 0;
  replace_cnt = 0;
  val->accept(*this);
}


void VariableReplacer::visit(hydla::simulator::symbolic::SymbolicValue& val)
{
  new_child_.reset();
  accept(val.get_node());
  if(new_child_) val.set_node(new_child_);
}


void VariableReplacer::visit(boost::shared_ptr<hydla::parse_tree::Variable> node)
{
  string v_name = node->get_name();
  variable_map_t::const_iterator it = variable_map.begin();
  for(;it != variable_map.end(); it++)
    {
      if(it->first->get_name() == v_name && it->first->get_derivative_count() == differential_cnt)
	{
	  new_child_ = it->second->get_node()->clone();
	  replace_cnt++;
	  // upper_bound to avoid infinite loop (may be caused by circular reference)
	  if(replace_cnt >= variable_map.size())
	    {
	      HYDLA_LOGGER_LOCATION(ERROR)
              HYDLA_LOGGER_ERROR("variable_map:", variable_map);
	      assert(0);
	    }
	  replace_cnt--;

	  break;
	}
    }
}

void VariableReplacer::visit(boost::shared_ptr<hydla::parse_tree::Differential> node)
{
  differential_cnt++;
  accept(node->get_child());
  differential_cnt--;
}


#define DEFINE_DEFAULT_VISIT_ARBITRARY(NODE_NAME)        \
void VariableReplacer::visit(boost::shared_ptr<NODE_NAME> node) \
{                                                     \
  for(int i=0;i<node->get_arguments_size();i++){      \
    accept(node->get_argument(i));                    \
    if(new_child_) {                                  \
      node->set_argument((new_child_), i);            \
      new_child_.reset();                             \
    }                                                 \
  }                                                   \
}

#define DEFINE_DEFAULT_VISIT_BINARY(NODE_NAME)        \
void VariableReplacer::visit(boost::shared_ptr<NODE_NAME> node) \
{                                                     \
  dispatch_lhs(node);                                 \
  dispatch_rhs(node);                                 \
}

#define DEFINE_DEFAULT_VISIT_UNARY(NODE_NAME)        \
void VariableReplacer::visit(boost::shared_ptr<NODE_NAME> node) \
{ dispatch_child(node);}

#define DEFINE_DEFAULT_VISIT_FACTOR(NODE_NAME)        \
void VariableReplacer::visit(boost::shared_ptr<NODE_NAME> node){}


DEFINE_DEFAULT_VISIT_ARBITRARY(Function)
DEFINE_DEFAULT_VISIT_ARBITRARY(UnsupportedFunction)
DEFINE_DEFAULT_VISIT_ARBITRARY(ArbitraryNode)

DEFINE_DEFAULT_VISIT_UNARY(Negative)
DEFINE_DEFAULT_VISIT_UNARY(Positive)


DEFINE_DEFAULT_VISIT_BINARY(Plus)
DEFINE_DEFAULT_VISIT_BINARY(Subtract)
DEFINE_DEFAULT_VISIT_BINARY(Times)
DEFINE_DEFAULT_VISIT_BINARY(Divide)
DEFINE_DEFAULT_VISIT_BINARY(Power)

DEFINE_DEFAULT_VISIT_FACTOR(Pi)
DEFINE_DEFAULT_VISIT_FACTOR(E)
DEFINE_DEFAULT_VISIT_FACTOR(Parameter)
DEFINE_DEFAULT_VISIT_FACTOR(SymbolicT)
DEFINE_DEFAULT_VISIT_FACTOR(Number)
DEFINE_DEFAULT_VISIT_FACTOR(SVtimer)
DEFINE_DEFAULT_VISIT_FACTOR(Infinity)




} //namespace simulator
} //namespace hydla
