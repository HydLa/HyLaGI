#include "RealPaverVCSPoint.h"

#include <sstream>

#include "RPConstraintSolver.h"
#include "Logger.h"
#include "rp_constraint.h"
#include "rp_constraint_ext.h"
#include "rp_container.h"
#include "rp_container_ext.h"

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
  this->constraint_store_ = ConstraintStore(); // ���ꂠ���Ă�̂��H
  return true;
}

/**
 * �^����ꂽ�ϐ��\�����ɁC����X�g�A�̏������������Ȃ�
 */
bool RealPaverVCSPoint::reset(const variable_map_t& variable_map)
{
  this->constraint_store_.build(variable_map);
  return true;
}

/**
 * ���݂̐���X�g�A����ϐ��\���쐬����
 */
bool RealPaverVCSPoint::create_variable_map(variable_map_t& variable_map)
{
  this->constraint_store_.build_variable_map(variable_map);
  return true;
}

/**
 * �����ǉ�����
 */
VCSResult RealPaverVCSPoint::add_constraint(const tells_t& collected_tells)
{
  typedef std::set<rp_constraint> ctr_set_t;
  var_name_map_t vars = this->constraint_store_.get_store_vars();
  ConstraintBuilder builder;
  builder.set_vars(vars);
  ctr_set_t ctrs, ctrs_copy;
  // tell�����rp_constraint��
  for(tells_t::const_iterator t_it = collected_tells.begin();
      t_it!=collected_tells.end(); t_it++) {
        ctrs.insert(builder.build_constraint_from_tell(*t_it));
  }
  vars.insert(builder.vars_begin(), builder.vars_end());
  // tell����݂̂��R�s�[���Ă���
  for(ctr_set_t::iterator it=ctrs.begin(); it!=ctrs.end(); it++) {
    rp_constraint c;
    rp_constraint_clone(&c, *it);
    ctrs_copy.insert(c);
  }
  // �X�g�A�̐����ǉ�
  ctr_set_t store_copy = this->constraint_store_.get_store_exprs_copy();
  ctrs.insert(store_copy.begin(), store_copy.end());
  // �m�F
  rp_vector_variable vec = ConstraintSolver::create_rp_vector(vars);
  HYDLA_LOGGER_DEBUG("#**** vcs:add_constraint: constraints expression ****");
  std::stringstream ss;
  for(ctr_set_t::iterator it=ctrs.begin(); it!=ctrs.end(); it++) {
    rp::dump_constraint(ss, *it, vec, 10);
    ss << "\n";
  }
  HYDLA_LOGGER_DEBUG(ss.str());
  // ����̉������݂��邩�ǂ����H
  rp_box b;
  bool res = ConstraintSolver::solve_hull(&b, vars, ctrs);
  if(res) {
    this->constraint_store_.add_constraint(ctrs_copy.begin(), ctrs_copy.end(), vars);
    HYDLA_LOGGER_DEBUG("#*** vcs:add_constraint ==> Consistent ***\n",
      "#**** vcs:add_constraint: new constraint_store ***\n",
      this->constraint_store_);
    rp_box_destroy(&b);
    for(std::set<rp_constraint>::iterator it=ctrs.begin();
      it!=ctrs.end(); it++) {
        rp_constraint c = *it;
        rp_constraint_destroy(&c);
    }
    return VCSR_TRUE;
  } else {
    HYDLA_LOGGER_DEBUG("#*** vcs:add_constraint ==> Inconsistent ***\n");
    for(std::set<rp_constraint>::iterator it=ctrs.begin();
      it!=ctrs.end(); it++) {
        rp_constraint c = *it;
        rp_constraint_destroy(&c);
    }
    return VCSR_FALSE;
  }
}

/**
 * ���݂̐���X�g�A����^����ask�����o�\���ǂ���
 */
VCSResult RealPaverVCSPoint::check_entailment(const ask_node_sptr& negative_ask)
{
  // ����X�g�A���R�s�[
  std::set<rp_constraint> ctrs = this->constraint_store_.get_store_exprs_copy();
  var_name_map_t vars = this->constraint_store_.get_store_vars();
  // �K�[�h�����Ƃ��̔ے�����
  std::set<rp_constraint> g, ng;
  GuardConstraintBuilder builder;
  builder.set_vars(vars);
  builder.create_guard_expr(negative_ask, g, ng, vars);

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
  return this->constraint_store_.dump_cs(s);
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
