#include "PrevReplacer.h"
#include "SymbolicValue.h"
#include "Logger.h"
#include "Dumpers.h"

using namespace std;
using namespace hydla::parse_tree;

namespace hydla {
namespace simulator {

PrevReplacer::PrevReplacer(parameter_map_t& map, phase_result_sptr_t &phase, Simulator &simulator):parameter_map_(map), prev_phase_(phase), simulator_(simulator)
{}

PrevReplacer::~PrevReplacer()
{}

void PrevReplacer::replace_value(value_t& val)
{
  HYDLA_LOGGER_LOCATION(REST);
  val->accept(*this);
}


void PrevReplacer::visit(hydla::simulator::symbolic::SymbolicValue& val)
{
  replace(val.get_node());
  if(new_child_) val.set_node(new_child_);
}

void PrevReplacer::replace(node_sptr node)
{
  differential_cnt_ = 0;
  in_prev_ = false;
  new_child_.reset();
  accept(node);
}

void PrevReplacer::visit(boost::shared_ptr<hydla::parse_tree::Previous> node)
{
  assert(!in_prev_);
  in_prev_ = true;
  accept(node->get_child());
  if(parameter_node_)
  {
    new_child_ = parameter_node_;
    parameter_node_.reset();
  }
  in_prev_ = false;
}

void PrevReplacer::visit(boost::shared_ptr<hydla::parse_tree::Variable> node)
{
  HYDLA_LOGGER(REST, *node);
  string v_name = node->get_name();
  int diff_cnt = differential_cnt_;
  HYDLA_LOGGER_VAR(REST, v_name);
  HYDLA_LOGGER_VAR(REST, diff_cnt);
  parameter_node_ = node_sptr(new Parameter(v_name, diff_cnt, prev_phase_->id));
  parameter_t param(v_name, diff_cnt, prev_phase_->id);
  variable_t variable(v_name, diff_cnt);
  simulator_.introduce_parameter(variable, prev_phase_, prev_phase_->variable_map[variable]);
  parameter_map_[param] = prev_phase_->variable_map[variable];
  prev_phase_->parameter_map[param] = parameter_map_[param];
  prev_phase_->variable_map[variable] = value_t(new symbolic::SymbolicValue(parameter_node_));
}

void PrevReplacer::visit(boost::shared_ptr<hydla::parse_tree::Differential> node)
{
  differential_cnt_++;
  accept(node->get_child());
  differential_cnt_--;
}


#define DEFINE_DEFAULT_VISIT_ARBITRARY(NODE_NAME)        \
void PrevReplacer::visit(boost::shared_ptr<NODE_NAME> node) \
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
void PrevReplacer::visit(boost::shared_ptr<NODE_NAME> node) \
{                                                     \
  dispatch_lhs(node);                                 \
  dispatch_rhs(node);                                 \
}

#define DEFINE_DEFAULT_VISIT_UNARY(NODE_NAME)        \
void PrevReplacer::visit(boost::shared_ptr<NODE_NAME> node) \
{ dispatch_child(node);}

#define DEFINE_DEFAULT_VISIT_FACTOR(NODE_NAME)        \
void PrevReplacer::visit(boost::shared_ptr<NODE_NAME> node){}


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




}
}
