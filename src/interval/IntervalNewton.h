#pragma once

#include <stack>
#include <vector>
#include "kv/interval.hpp"
#include "kv/rdouble.hpp"
#include "kv/dd.hpp"
#include "kv/rdd.hpp"

#include "Node.h"
#include "Parameter.h"
#include "PhaseResult.h"

namespace hydla
{
namespace interval
{

typedef kv::dd dd;
typedef kv::interval<double> itvd;
// typedef kv::interval<dd> itvd;
typedef std::stack<itvd, std::vector<itvd>> itv_stack_t;

typedef hydla::symbolic_expression::node_sptr node_sptr;
typedef simulator::parameter_map_t            parameter_map_t;

const itvd INVALID_ITV = itvd(0, 0);

bool itvd_eqal(itvd x, itvd y);

itvd intersect_interval(itvd x, itvd y);

bool show_existence(itvd x, node_sptr exp, node_sptr dexp);

itvd calculate_interval_newton(itvd init, node_sptr exp, node_sptr dexp, parameter_map_t& phase_map_, bool discard_itv_0);
itvd calculate_interval_newton(itv_stack_t &candidate_stack, node_sptr exp, node_sptr dexp, parameter_map_t& phase_map_, bool discard_itv_0);

std::list<itvd> calculate_interval_newton_nd(itvd init, node_sptr exp, node_sptr dexp, parameter_map_t& phase_map_);

} // namespace interval
} // namespace hydla
