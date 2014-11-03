#pragma once

#include <stack>
/* 柏木先生の区間演算用のライブラリ */
#include "kv/interval.hpp"
#include "kv/rdouble.hpp"
#include "kv/dd.hpp"
#include "kv/rdd.hpp"

#include "Node.h"

namespace hydla
{
namespace interval
{

typedef kv::dd dd;
typedef kv::interval<double> itvd;
// typedef kv::interval<dd> itvd;
typedef std::stack<itvd> itvs;

typedef hydla::symbolic_expression::node_sptr node_sptr;

bool itvd_eqal(itvd x, itvd y);

itvd intersect_interval(itvd x, itvd y);

bool show_existence(itvd x, node_sptr exp, node_sptr dexp);

itvd calculate_interval_newton(itvd init, node_sptr exp, node_sptr dexp);

} // namespace interval
} // namespace hydla
