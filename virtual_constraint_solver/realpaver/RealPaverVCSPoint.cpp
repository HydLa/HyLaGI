#include "RealPaverVCSPoint.h"

namespace hydla {
namespace vcs {
namespace realpaver {

RealPaverVCSPoint::RealPaverVCSPoint()
{}

RealPaverVCSPoint::~RealPaverVCSPoint()
{}

/**
 * ����X�g�A�̏������������Ȃ�
 */
bool RealPaverVCSPoint::reset()
{
  return true;
}

/**
 * �^����ꂽ�ϐ��\�����ɁC����X�g�A�̏������������Ȃ�
 */
bool RealPaverVCSPoint::reset(const variable_map_t& variable_map)
{
  return true;
}

/**
 * ���݂̐���X�g�A����ϐ��\���쐬����
 */
bool RealPaverVCSPoint::create_variable_map(variable_map_t& variable_map)
{
  return true;
}

/**
 * �����ǉ�����
 */
VCSResult RealPaverVCSPoint::add_constraint(const tells_t& collected_tells)
{
  return VCSR_TRUE;
}
  
/**
 * ���݂̐���X�g�A����^����ask�����o�\���ǂ���
 */
VCSResult RealPaverVCSPoint::check_entailment(const ask_node_sptr& negative_ask)
{
  return VCSR_TRUE;
}

/**
 * ask�̓��o��Ԃ��ω�����܂Őϕ��������Ȃ�
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
 * ������Ԃ̏o�͂������Ȃ�
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
