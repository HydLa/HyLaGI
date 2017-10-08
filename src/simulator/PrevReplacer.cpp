#include "PrevReplacer.h"
#include "VariableReplacer.h"
#include "Logger.h"
#include "Backend.h"
#include "HydLaError.h"

using namespace std;
using namespace hydla::symbolic_expression;

namespace hydla {
namespace simulator {

PrevReplacer::PrevReplacer(PhaseResult &phase, Simulator &simulator, backend::Backend *b, bool affine): prev_phase(phase), simulator(simulator), backend(b), affine_mode(affine)
{}

PrevReplacer::~PrevReplacer()
{}

bool PrevReplacer::replace_value(value_t& val)
{
  symbolic_expression::node_sptr node = val.get_node();
  replace_node(node);
  val.set_node(node);
  return replaced;
}

void PrevReplacer::replace_node(symbolic_expression::node_sptr &node)
{
  differential_cnt = 0;
  in_prev = false;
  replaced = false;
  new_child.reset();
  accept(node);
  if(new_child) node = new_child;
}

void PrevReplacer::visit(boost::shared_ptr<hydla::symbolic_expression::Previous> node)
{
  assert(!in_prev);
  in_prev = true;
  accept(node->get_child());
  in_prev = false;
}

void PrevReplacer::visit(boost::shared_ptr<hydla::symbolic_expression::Variable> node)
{
  if(in_prev)
  {
    HYDLA_LOGGER_DEBUG_VAR(*node);
    string v_name = node->get_name();
    int diff_cnt = differential_cnt;
    HYDLA_LOGGER_DEBUG_VAR(v_name);
    HYDLA_LOGGER_DEBUG_VAR(diff_cnt);
    variable_t variable(v_name, diff_cnt);
    ValueRange range = prev_phase.variable_map[variable];
    replaced = true;

    // replace variables in the range with their values
    VariableReplacer v_replacer(prev_phase.variable_map, true);
    v_replacer.replace_range(range);
    HYDLA_LOGGER_DEBUG_VAR(range);

    if(range.unique())
    {
      new_child = range.get_unique_value().get_node();
    }
    else
    {
      parameter_t param(v_name, diff_cnt, prev_phase.id);
      if(affine_mode)
      {
        //replace with affine form
        if(range.get_upper_cnt() != 1 || range.get_lower_cnt() != 1)
        {
          HYDLA_LOGGER_ERROR("invalid range for affine arithmetic mode, " + range.get_string());
        }
        hydla::backend::MidpointRadius mr;
        value_t lb = range.get_lower_bound(0).value;
        value_t ub = range.get_upper_bound(0).value;
        backend->call("intervalToMidpointRadius", false, 2, "vltvlt", "r", &lb, &ub, &mr);
        range.set_upper_bound(value_t("1"), true);
        range.set_lower_bound(value_t("-1"), true);
        value_t new_value(mr.midpoint + mr.radius * value_t(param));
        new_child = new_value.get_node();
      }
      else
      {
        new_child = symbolic_expression::node_sptr(new symbolic_expression::Parameter(v_name, diff_cnt, prev_phase.id));
      }

      if(!simulator.get_parameter_map().count(param))
      {
        prev_phase.variable_map[variable] = value_t(new_child);

        simulator.introduce_parameter(variable, prev_phase, range);
        ConstraintStore par_cons = range.create_range_constraint(node_sptr(new symbolic_expression::Parameter(param.get_name(), param.get_differential_count(), param.get_phase_id())));
        prev_phase.add_parameter_constraint(par_cons);
        parameter_constraint.add_constraint_store(par_cons);
      }
    }
  }
}

ConstraintStore PrevReplacer::get_parameter_constraint()const
{
  return parameter_constraint;
}

void PrevReplacer::visit(boost::shared_ptr<hydla::symbolic_expression::Differential> node)
{
  differential_cnt++;
  accept(node->get_child());
  differential_cnt--;
}


#define DEFINE_DEFAULT_VISIT_ARBITRARY(NODE_NAME)        \
void PrevReplacer::visit(boost::shared_ptr<NODE_NAME> node) \
{                                                     \
  for(int i = 0;i < node->get_arguments_size();i++){      \
    accept(node->get_argument(i));                    \
    if(new_child) {                                  \
      node->set_argument((new_child), i);            \
      new_child.reset();                             \
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
DEFINE_DEFAULT_VISIT_FACTOR(symbolic_expression::Parameter)
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
