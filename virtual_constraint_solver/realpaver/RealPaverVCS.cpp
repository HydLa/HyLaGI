#include "RealPaverVCS.h"

namespace hydla {
namespace vcs {
namespace realpaver {

RealPaverVCS::RealPaverVCS(Mode m)
{
  mode_ = m;
}

RealPaverVCS::RealPaverVCS(Mode m, MathLink* ml)
{
  mode_ = m;
}

RealPaverVCS::~RealPaverVCS()
{}

/**
 * 制約ストアの初期化をおこなう
 */
bool RealPaverVCS::reset()
{
  return vcs_->reset();
}

/**
 * 与えられた変数表を元に，制約ストアの初期化をおこなう
 */
bool RealPaverVCS::reset(const variable_map_t& vm)
{
  return vcs_->reset(vm);
}

/**
 * 現在の制約ストアから変数表を作成する
 */
bool RealPaverVCS::create_variable_map(variable_map_t& vm)
{
  return vcs_->create_variable_map(vm);
}

/**
 * 制約を追加する
 */
VCSResult RealPaverVCS::add_constraint(const tells_t& collected_tells)
{
  return vcs_->add_constraint(collected_tells);
}
  
/**
 * 現在の制約ストアから与えたaskが導出可能かどうか
 */
VCSResult RealPaverVCS::check_entailment(const ask_node_sptr& negative_ask)
{
  return vcs_->check_entailment(negative_ask);
}

/**
 * askの導出状態が変化するまで積分をおこなう
 */
bool RealPaverVCS::integrate(integrate_result_t& integrate_result,
                             const positive_asks_t& positive_asks,
                             const negative_asks_t& negative_asks,
                             const time_t& current_time,
                             const time_t& max_time)
{
  return vcs_->integrate(integrate_result, 
                         positive_asks, 
                         negative_asks, 
                         current_time, 
                         max_time);
}

/******************** realpaver only ********************/

void RealPaverVCS::add_single_constraint(const node_sptr& constraint_node,
                                         const bool neg_expression)
{
  return vcs_->add_single_constraint(constraint_node, neg_expression);
}

/******************** realpaver only ********************/

} // namespace realpaver
} // namespace vcs
} // namespace hydla 