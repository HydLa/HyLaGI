#include "PrevReplacer.h"
#include "VariableReplacer.h"
#include "Logger.h"
#include "Backend.h"

using namespace std;
using namespace hydla::parse_tree;

namespace hydla {
namespace simulator {

PrevReplacer::PrevReplacer(parameter_map_t& map, phase_result_sptr_t &phase, Simulator &simulator, bool approx):parameter_map_(map), prev_phase_(phase), simulator_(simulator), approx_(approx)
{}

PrevReplacer::~PrevReplacer()
{}

void PrevReplacer::replace_value(value_t& val)
{
  node_sptr node = val.get_node();
  replace_node(node);
  val.set_node(node);
}

void PrevReplacer::replace_node(node_sptr &node)
{
  differential_cnt_ = 0;
  in_prev_ = false;
  new_child_.reset();
  accept(node);
  if(new_child_) node = new_child_;
}

void PrevReplacer::visit(boost::shared_ptr<hydla::parse_tree::Previous> node)
{
  assert(!in_prev_);
  in_prev_ = true;
  accept(node->get_child());
  in_prev_ = false;
}

void PrevReplacer::visit(boost::shared_ptr<hydla::parse_tree::Variable> node)
{
  if(in_prev_)
  {
    HYDLA_LOGGER_DEBUG_VAR(*node);
    string v_name = node->get_name();
    int diff_cnt = differential_cnt_;
    HYDLA_LOGGER_DEBUG_VAR(v_name);
    HYDLA_LOGGER_DEBUG_VAR(diff_cnt);
    variable_t variable(v_name, diff_cnt);
    ValueRange range = prev_phase_->variable_map[variable];
  
    // replace variables in the range with their values
    VariableReplacer v_replacer(prev_phase_->variable_map);
    v_replacer.replace_range(range);

    if(range.unique())
    {
      new_child_ = range.get_unique().get_node();
    }
    else
    {
      new_child_ = node_sptr(new parse_tree::Parameter(v_name, diff_cnt, prev_phase_->id));
      parameter_t param(v_name, diff_cnt, prev_phase_->id);

      if(!parameter_map_.count(param))
      {
        if(approx_)
        {
          hydla::backend::MidpointRadius mr;
          value_t lb = range.get_lower_bound().value;
          value_t ub = range.get_upper_bound().value;
          simulator_.backend->call("intervalToMidpointRadius", 2, "vltvlt", "r", &lb, &ub, &mr);
          HYDLA_LOGGER_DEBUG("");
          range.set_upper_bound(value_t("1"), range.get_upper_bound().include_bound);
          range.set_lower_bound(value_t("-1"), range.get_lower_bound().include_bound);
          value_t new_value(mr.midpoint + mr.radius * value_t(new_child_));
          prev_phase_->variable_map[variable] = new_value;
          new_child_ = new_value.get_node();
        }
        else
        {
          prev_phase_->variable_map[variable] = value_t(new_child_);
        }

        simulator_.introduce_parameter(variable, prev_phase_, range);
        parameter_map_[param] = range;
        prev_phase_->parameter_map[param] = parameter_map_[param];
      }
    }
  }
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
  for(int i = 0;i < node->get_arguments_size();i++){      \
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

DEFINE_DEFAULT_VISIT_UNARY(Negative)
DEFINE_DEFAULT_VISIT_UNARY(Positive)

DEFINE_DEFAULT_VISIT_UNARY(ConstraintDefinition)
DEFINE_DEFAULT_VISIT_UNARY(ProgramDefinition)
DEFINE_DEFAULT_VISIT_UNARY(ConstraintCaller)
DEFINE_DEFAULT_VISIT_UNARY(ProgramCaller)
DEFINE_DEFAULT_VISIT_UNARY(Constraint)

DEFINE_DEFAULT_VISIT_BINARY(Ask)
DEFINE_DEFAULT_VISIT_UNARY(Tell)

DEFINE_DEFAULT_VISIT_BINARY(Plus)
DEFINE_DEFAULT_VISIT_BINARY(Subtract)
DEFINE_DEFAULT_VISIT_BINARY(Times)
DEFINE_DEFAULT_VISIT_BINARY(Divide)
DEFINE_DEFAULT_VISIT_BINARY(Power)

DEFINE_DEFAULT_VISIT_BINARY(Less)
DEFINE_DEFAULT_VISIT_BINARY(LessEqual)
DEFINE_DEFAULT_VISIT_BINARY(Greater)
DEFINE_DEFAULT_VISIT_BINARY(GreaterEqual)
DEFINE_DEFAULT_VISIT_BINARY(Equal)
DEFINE_DEFAULT_VISIT_BINARY(UnEqual)

DEFINE_DEFAULT_VISIT_BINARY(LogicalOr)
DEFINE_DEFAULT_VISIT_BINARY(LogicalAnd)

DEFINE_DEFAULT_VISIT_BINARY(Weaker)
DEFINE_DEFAULT_VISIT_BINARY(Parallel)

DEFINE_DEFAULT_VISIT_UNARY(Always)

DEFINE_DEFAULT_VISIT_FACTOR(Float)
DEFINE_DEFAULT_VISIT_FACTOR(True)
DEFINE_DEFAULT_VISIT_FACTOR(False)


DEFINE_DEFAULT_VISIT_FACTOR(Pi)
DEFINE_DEFAULT_VISIT_FACTOR(E)
DEFINE_DEFAULT_VISIT_FACTOR(parse_tree::Parameter)
DEFINE_DEFAULT_VISIT_FACTOR(SymbolicT)
DEFINE_DEFAULT_VISIT_FACTOR(Number)
DEFINE_DEFAULT_VISIT_FACTOR(SVtimer)
DEFINE_DEFAULT_VISIT_FACTOR(Infinity)

DEFINE_DEFAULT_VISIT_FACTOR(Print)
DEFINE_DEFAULT_VISIT_FACTOR(PrintIP)
DEFINE_DEFAULT_VISIT_FACTOR(PrintPP)
DEFINE_DEFAULT_VISIT_FACTOR(Scan)
DEFINE_DEFAULT_VISIT_FACTOR(Exit)
DEFINE_DEFAULT_VISIT_FACTOR(Abort)







}
}
