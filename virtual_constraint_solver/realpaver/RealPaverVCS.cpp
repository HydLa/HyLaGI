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
 * ����X�g�A�̏������������Ȃ�
 */
bool RealPaverVCS::reset()
{
  return vcs_->reset();
}

/**
 * �^����ꂽ�ϐ��\�����ɁC����X�g�A�̏������������Ȃ�
 */
bool RealPaverVCS::reset(const variable_map_t& vm)
{
  return vcs_->reset(vm);
}

/**
 * ���݂̐���X�g�A����ϐ��\���쐬����
 */
bool RealPaverVCS::create_variable_map(variable_map_t& vm)
{
  return vcs_->create_variable_map(vm);
}

/**
 * �����ǉ�����
 */
VCSResult RealPaverVCS::add_constraint(const tells_t& collected_tells)
{
  return vcs_->add_constraint(collected_tells);
}
  
/**
 * ���݂̐���X�g�A����^����ask�����o�\���ǂ���
 */
VCSResult RealPaverVCS::check_entailment(const ask_node_sptr& negative_ask)
{
  return vcs_->check_entailment(negative_ask);
}

/**
 * ask�̓��o��Ԃ��ω�����܂Őϕ��������Ȃ�
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