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
   * �����ǉ�����D
   */
  virtual void add_constraint(const constraints_t& constraints);
  
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
   * 
   */
  virtual void set_continuity(const continuity_map_t& continuity_map);

private:
  typedef REDUCEStringSender::max_diff_map_t max_diff_map_t;

  /**
   * ���A�����Ɋւ��鐧���������
   */
  void add_left_continuity_constraint(const continuity_map_t& continuity_map, REDUCEStringSender &rss);

  /**
   * ������o������ϐ��⍶�A������ƂƂ��ɑ��M����
   */
  void send_constraint(const constraints_t& constraints);

  /**
   * check_consistency �̎�M����
   */
  VCSResult check_consistency_receive();

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
  continuity_map_t continuity_map_;
  max_diff_map_t max_diff_map_;          //���݂̐���X�g�A�ɏo�����钆�ōő�̔����񐔂��L�^���Ă����\�D
  constraints_t tmp_constraints_;  //�ꎞ�I�ɐ����ǉ�����Ώ�
  std::set<std::string> par_names_; //�ꎞ���̂�
  SExpParser sp_;
};

} // namespace reduce
} // namespace simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REDUCE_VCS_POINT_H_
