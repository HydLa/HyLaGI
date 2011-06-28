#ifndef _INCLUDED_HYDLA_VCS_REDUCE_VCS_POINT_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_VCS_POINT_H_

#include <ostream>

#include "REDUCELink.h"
//#include "mathlink_helper.h"

//TODO MathVCSType, PacketSender�̈ˑ�����������
#include "MathVCSType.h"
//#include "PacketSender.h"
#include "REDUCEStringSender.h"

#include "../../parser/SExpParser.h"

using namespace hydla::vcs::mathematica;
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

  typedef std::pair<const_tree_iter_t, constraint_store_vars_t> constraint_store_t;

  typedef std::pair<std::set<std::set<MathValue> >,
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
  virtual bool create_variable_map(variable_map_t& variable_map, parameter_map_t& parameter_map);

  /**
   * �����ǉ�����
   */
  virtual VCSResult add_constraint(const tells_t& collected_tells, const appended_asks_t &appended_asks);

  /**
   * ���݂̐���X�g�A����^����ask�����o�\���ǂ���
   */
  virtual VCSResult check_entailment(const ask_node_sptr& negative_ask, const appended_asks_t &appended_asks);

  /**
   * ask�̓��o��Ԃ��ω�����܂Őϕ��������Ȃ�
   */
  virtual VCSResult integrate(
      integrate_result_t& integrate_result,
      const positive_asks_t& positive_asks,
      const negative_asks_t& negative_asks,
      const time_t& current_time,
      const time_t& max_time,
      const not_adopted_tells_list_t& not_adopted_tells_list,
      const appended_asks_t& appended_asks);

  /**
   * ������Ԃ̏o�͂������Ȃ�
   */
  std::ostream& dump(std::ostream& s) const;

private:
  typedef std::map<std::string, int> max_diff_map_t;

  void send_cs() const;
  void send_ps() const;
  void send_cs_vars() const;
  void send_pars() const;

  /**
   * �ϐ��̍ő�����񐔂����Ƃ߂�
   */
  void create_max_diff_map(PacketSender& ps, max_diff_map_t& max_diff_map);

  /**
   * ���A�����Ɋւ��鐧���������
   */
  void add_left_continuity_constraint(PacketSender& ps, max_diff_map_t& max_diff_map);

  /**
   * ����X�g�A��true�ł��邩�ǂ���
   */
//  bool cs_is_true()
//  {
//    return constraint_store_.first.size()==1 &&
//        (*constraint_store_.first.begin()).size()==1 &&
//        (*(*constraint_store_.first.begin()).begin()).get_string()=="True";
//  }


  //  mutable MathLink* ml_;
  mutable REDUCELink* cl_;
  constraint_store_t constraint_store_;
  parameter_store_t parameter_store_;
  std::set<std::string> par_names_; //�ꎞ���̂�
  SExpParser sp_;
};

std::ostream& operator<<(std::ostream& s, const REDUCEVCSPoint& m);

} // namespace reduce
} // namespace simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REDUCE_VCS_POINT_H_
