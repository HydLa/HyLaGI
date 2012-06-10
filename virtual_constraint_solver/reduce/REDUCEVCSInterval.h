#ifndef _INCLUDED_HYDLA_VCS_REDUCE_VCS_INTERVAL_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_VCS_INTERVAL_H_

#include <map>
#include <set>
#include <vector>

#include "REDUCELink.h"
#include "REDUCEVCSType.h"
#include "REDUCEStringSender.h"

#include "../../parser/SExpParser.h"

using namespace hydla::parser;

namespace hydla {
namespace vcs {
namespace reduce {

class REDUCEVCSInterval : 
    public virtual_constraint_solver_t
{
public:
  typedef SExpParser::const_tree_iter_t const_tree_iter_t;

  struct ConstraintStore 
  {
    typedef std::map<REDUCEVariable, value_t>      init_vars_t;
    typedef hydla::simulator::constraints_t        constraints_t;

    init_vars_t   init_vars;
    constraints_t constraints;
  };
  
  typedef ConstraintStore constraint_store_t;


  /**
   * @param approx_precision �ߎ����鐸�x �l�����̏ꍇ�͋ߎ����s��Ȃ�
   */
  REDUCEVCSInterval(REDUCELink* cl, int approx_precision);

  virtual ~REDUCEVCSInterval();

  /**
   * ����X�g�A�����������𔻒肷��D
   * �����Ő����n���ꂽ�ꍇ�͈ꎞ�I�ɐ���X�g�A�ɒǉ�����D
   */
  virtual VCSResult check_consistency();
  virtual VCSResult check_consistency(const constraints_t& constraints);

  /**
   * ���݂̐���X�g�A����^����ask�����o�\���ǂ���
   */
  virtual VCSResult check_entailment(const node_sptr &node);

  /**
   * ask�̓��o��Ԃ��ω�����܂Őϕ��������Ȃ�
   */
  virtual VCSResult integrate(
    integrate_result_t& integrate_result,
    const constraints_t &constraints,
    const time_t& current_time,
    const time_t& max_time);  
  
  /**
   * �ϐ��\�ɑ΂��ė^����ꂽ������K�p����
   */
  virtual void apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time);

  /**
   * 
   */
  virtual void set_continuity(const continuity_map_t& continuity_map);

private:
  // check_consistency�̎�M����
  VCSResult check_consistency_receive();

  /**
   * �����l�����REDUCE�ɓn��
   */
  void send_init_cons(REDUCEStringSender& rss, const continuity_map_t& continuity_map);

  /**
   * ������o������ϐ��⏉���l����ƂƂ��ɑ��M����
   */
  void send_constraint(const constraints_t& constraints);

  /**
  * �����𑗐M����
  */
  void send_time(const time_t& time);

  /**
   * �ϐ��\�ɖ���`�̕ϐ���ǉ�����
   */
  void add_undefined_vars_to_vm(variable_map_t& vm);
  
  mutable REDUCELink* cl_;
  continuity_map_t continuity_map_;
  int approx_precision_;
};


} // namespace reduce
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_REDUCE_VCS_INTERVAL_H_
