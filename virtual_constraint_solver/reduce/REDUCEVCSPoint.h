#ifndef _INCLUDED_HYDLA_VCS_REDUCE_VCS_POINT_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_VCS_POINT_H_

#include <ostream>

#include "REDUCELink.h"
#include "REDUCEVCSType.h"
#include "REDUCEStringSender.h"

#include "../../parser/SExpParser.h"

using namespace hydla::parser;

namespace hydla {
namespace vcs {
namespace reduce {

class REDUCEVCSPoint :
public virtual_constraint_solver_t
{
public:
  typedef SExpParser::const_tree_iter_t const_tree_iter_t;

  typedef std::set<REDUCEStringSender::var_info_t> constraint_store_vars_t;

  typedef std::pair<std::set<std::set<REDUCEValue> >, constraint_store_vars_t> constraint_store_t;
//  typedef std::pair<std::set<const_tree_iter_t>, constraint_store_vars_t> constraint_store_t;

  typedef std::pair<std::set<std::set<REDUCEValue> >,
      constraint_store_vars_t> parameter_store_t;

  //  MathematicaVCSPoint(MathLink* ml);
  REDUCEVCSPoint(REDUCELink* cl);

  virtual ~REDUCEVCSPoint();

  /**
   * ����X�g�A�̏������������Ȃ�
   */
  virtual bool reset();

  /**
   * �^����ꂽ�ϐ��\�����ɁC����X�g�A�̏������������Ȃ�
   */
  virtual bool reset(const variable_map_t& variable_map);


  /**
   * �^����ꂽ�ϐ��\�ƒ萔�\�����ɁC����X�g�A�̏������������Ȃ�
   */
  virtual bool reset(const variable_map_t& vm, const parameter_map_t& pm);

  /**
   * ���݂̐���X�g�A����ϐ��\���쐬����
   */
  virtual bool create_maps(create_result_t & create_result);

  /**
   * �����ǉ�����
   */
  virtual void add_constraint(const constraints_t& constraints);

  /**
   * ���݂̐���X�g�A����^����ask�����o�\���ǂ���
   */
  virtual VCSResult check_entailment(const ask_node_sptr& negative_ask, const appended_asks_t &appended_asks);

  /**
   * ����X�g�A�����������𔻒肷��D
   * �����Ő����n���ꂽ�ꍇ�͈ꎞ�I�ɐ���X�g�A�ɒǉ�����D
   */
  virtual VCSResult check_consistency();
  virtual VCSResult check_consistency(const constraints_t& constraints);

  /**
   * ask�̓��o��Ԃ��ω�����܂Őϕ��������Ȃ�
   */
  virtual VCSResult integrate(
      integrate_result_t& integrate_result,
      const constraints_t &constraints,
      const time_t& current_time,
      const time_t& max_time);

  /**
   * ������Ԃ̏o�͂������Ȃ�
   */
  std::ostream& dump(std::ostream& s) const;

private:
  typedef REDUCEStringSender::max_diff_map_t max_diff_map_t;

  void send_cs() const;
  void send_ps() const;
  void send_cs_vars() const;
  void send_pars() const;
  void receive_constraint_store(constraint_store_t& store);

  /**
   * check_consistency �̋��ʕ���
   */
  VCSResult check_consistency_sub();

  /**
   * ���A�����Ɋւ��鐧���������
   */
  void add_left_continuity_constraint(REDUCEStringSender& rss, max_diff_map_t& max_diff_map);

  /**
   * ����X�g�A��true�ł��邩�ǂ���
   */
//  bool cs_is_true()
//  {
//    return constraint_store_.first.size()==1 &&
//        (*constraint_store_.first.begin()).size()==1 &&
//        (*(*constraint_store_.first.begin()).begin()).get_string()=="True";
//  }


  mutable REDUCELink* cl_;
  max_diff_map_t max_diff_map_;          //���݂̐���X�g�A�ɏo�����钆�ōő�̔����񐔂��L�^���Ă����\�D
  constraint_store_t constraint_store_;
  parameter_store_t parameter_store_;
  constraints_t tmp_constraints_;  //�ꎞ�I�ɐ����ǉ�����Ώ�
  std::set<std::string> par_names_; //�ꎞ���̂�
  SExpParser sp_;
};

std::ostream& operator<<(std::ostream& s, const REDUCEVCSPoint& m);

} // namespace reduce
} // namespace simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REDUCE_VCS_POINT_H_
