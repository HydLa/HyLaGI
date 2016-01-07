#include "VariableReplacer.h"
#include "Logger.h"

using namespace std;
using namespace hydla::symbolic_expression;

namespace hydla {
namespace simulator {



VariableReplacer::VariableReplacer(const variable_map_t* map):variable_map(map)
{}

VariableReplacer::~VariableReplacer()
{}

void VariableReplacer::replace_node(symbolic_expression::node_sptr& node)
{
  differential_cnt = 0;
  replace_cnt = 0;
  new_child_.reset();
  accept(node);
  if(new_child_) node = new_child_;
}

void VariableReplacer::replace_variable_map(variable_map_t* vm)
{
  const variable_map_t *prev_vm = variable_map;
  variable_map = vm;
  for(auto &entry : *vm)
  {
    replace_range(entry.second);
  }
  variable_map = prev_vm;
}

void VariableReplacer::set_variable_map(const variable_map_t* map)
{
  variable_map = map;
}

void VariableReplacer::replace_value(value_t& val)
{
  symbolic_expression::node_sptr node = val.get_node();
  replace_node(node);
  val.set_node(node);
}

void VariableReplacer::replace_range(ValueRange &range)
{
  if(range.unique())
  {
    value_t val = range.get_unique_value();
    replace_value(val);
  }
  else
  {
    for(uint i = 0; i < range.get_lower_cnt(); i++)
    {
      value_t val = range.get_lower_bound(i).value;
      replace_value(val);
    }
    for(uint i = 0; i < range.get_upper_cnt(); i++)
    {
      value_t val = range.get_upper_bound(i).value;
      replace_value(val);
    }
  }
}

void VariableReplacer::visit(boost::shared_ptr<hydla::symbolic_expression::Variable> node)
{
  string v_name = node->get_name();
  for(auto it = variable_map->begin();it != variable_map->end(); it++)
    {
      if(it->first.get_name() == v_name && it->first.get_differential_count() == differential_cnt && it->second.unique())
      {
        //TODO: 値が範囲を持っている場合にも対応する
        node_sptr node = it->second.get_unique_value().get_node()->clone();
        VariableReplacer replacer(variable_map);
        replace_node(node);
        
        new_child_ = node;
        replace_cnt++;
        // upper_bound to avoid infinite loop (may be caused by circular reference)
        if(replace_cnt >= variable_map->size())
        {
          assert(0);
        }
        replace_cnt--;        
        break;
      }
    }
}

void VariableReplacer::visit(boost::shared_ptr<hydla::symbolic_expression::Differential> node)
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

DEFINE_DEFAULT_VISIT_UNARY(Negative)
DEFINE_DEFAULT_VISIT_UNARY(Positive)


DEFINE_DEFAULT_VISIT_BINARY(Plus)
DEFINE_DEFAULT_VISIT_BINARY(Subtract)
DEFINE_DEFAULT_VISIT_BINARY(Times)
DEFINE_DEFAULT_VISIT_BINARY(Divide)
DEFINE_DEFAULT_VISIT_BINARY(Power)

DEFINE_DEFAULT_VISIT_FACTOR(Pi)
DEFINE_DEFAULT_VISIT_FACTOR(E)
DEFINE_DEFAULT_VISIT_FACTOR(symbolic_expression::Parameter)
DEFINE_DEFAULT_VISIT_FACTOR(SymbolicT)
DEFINE_DEFAULT_VISIT_FACTOR(Number)
DEFINE_DEFAULT_VISIT_FACTOR(SVtimer)
DEFINE_DEFAULT_VISIT_FACTOR(Infinity)




} //namespace simulator
} //namespace hydla
