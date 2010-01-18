#include "RealPaverVCSPoint.h"

namespace hydla {
namespace vcs {
namespace realpaver {

RealPaverVCSPoint::RealPaverVCSPoint()
{}

RealPaverVCSPoint::~RealPaverVCSPoint()
{}

/**
 * 制約ストアの初期化をおこなう
 */
bool RealPaverVCSPoint::reset()
{
  return true;
}

/**
 * 与えられた変数表を元に，制約ストアの初期化をおこなう
 */
bool RealPaverVCSPoint::reset(const variable_map_t& variable_map)
{
  return true;
}

/**
 * 現在の制約ストアから変数表を作成する
 */
bool RealPaverVCSPoint::create_variable_map(variable_map_t& variable_map)
{
  return true;
}

/**
 * 制約を追加する
 */
VCSResult RealPaverVCSPoint::add_constraint(const tells_t& collected_tells)
{
  return VCSR_TRUE;
}
  
/**
 * 現在の制約ストアから与えたaskが導出可能かどうか
 */
VCSResult RealPaverVCSPoint::check_entailment(const ask_node_sptr& negative_ask)
{
  return VCSR_TRUE;
}

/**
 * askの導出状態が変化するまで積分をおこなう
 */
bool RealPaverVCSPoint::integrate(integrate_result_t& integrate_result,
                                  const positive_asks_t& positive_asks,
                                  const negative_asks_t& negative_asks,
                                  const time_t& current_time,
                                  const time_t& max_time)
{
  return true;
}

/**
 * 内部状態の出力をおこなう
 */
std::ostream& RealPaverVCSPoint::dump(std::ostream& s) const
{
  return s;
}

void RealPaverVCSPoint::add_single_constraint(const node_sptr &constraint_node,
                                              const bool neg_expression)
{
}

std::ostream& operator<<(std::ostream& s, const RealPaverVCSPoint& vcs)
{
  return vcs.dump(s);
}


} // namespace realapver
} // namespace vcs
} // namespace hydla 
