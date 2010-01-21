#include "RealPaverVCSInterval.h"

#include <sstream>

#include "RPConstraintSolver.h"
#include "Logger.h"
#include "rp_constraint.h"
#include "rp_constraint_ext.h"
#include "rp_container.h"
#include "rp_container_ext.h"

#include "../mathematica/PacketSender.h"
#include "../mathematica/PacketChecker.h"
#include <boost/lexical_cast.hpp>

using namespace hydla::vcs::mathematica;

namespace hydla {
namespace vcs {
namespace realpaver {

RealPaverVCSInterval::RealPaverVCSInterval(MathLink* ml) :
constraint_store_(),
  ml_(ml)
{}


RealPaverVCSInterval::~RealPaverVCSInterval()
{}

RealPaverBaseVCS* RealPaverVCSInterval::clone()
{
  RealPaverVCSInterval* vcs_ptr = new RealPaverVCSInterval(this->ml_);
  vcs_ptr->constraint_store_ = this->constraint_store_;
  return vcs_ptr;
}

/**
 * ����X�g�A�̏������������Ȃ�
 */
bool RealPaverVCSInterval::reset()
{
  this->constraint_store_ = ConstraintStoreInterval(); // ���ꂠ���Ă�̂��H
  return true;
}

/**
 * �^����ꂽ�ϐ��\�����ɁC����X�g�A�̏������������Ȃ�
 */
bool RealPaverVCSInterval::reset(const variable_map_t& variable_map)
{
  this->constraint_store_.build(variable_map);
  HYDLA_LOGGER_DEBUG("vcs:reset: new_constraint_store\n",
    this->constraint_store_);
  return true;
}

/**
 * ���݂̐���X�g�A����ϐ��\���쐬����
 */
bool RealPaverVCSInterval::create_variable_map(variable_map_t& variable_map)
{
  this->constraint_store_.build_variable_map(variable_map);
  return true;
}

/**
 * �����ǉ�����
 */
VCSResult RealPaverVCSInterval::add_constraint(const tells_t& collected_tells)
{
  // integrateExpr[cons, vars]��n������
  ml_->put_function("integrateExpr", 2);
  ml_->put_function("Join", 3);

  // tell����̏W������tells�𓾂�Mathematica�ɓn��
  ml_->put_function("List", collected_tells.size());
  PacketSender ps(*ml_);
  tells_t::const_iterator tells_it  = collected_tells.begin();
  tells_t::const_iterator tells_end = collected_tells.end();
  for(; (tells_it) != tells_end; ++tells_it) {
    ps.put_node((*tells_it)->get_child(), PacketSender::VA_Time, true);
  }
  // ����X�g�A���ɂ���tell������n��
  int cs_exprs_size = constraint_store_.nodes_.size();
  ml_->put_function("List", cs_exprs_size);
  tells_t::const_iterator cs_tells_it  = constraint_store_.nodes_.begin();
  tells_t::const_iterator cs_tells_end = constraint_store_.nodes_.end();
  for(; (cs_tells_it) != cs_tells_end; ++cs_tells_it) {
    ps.put_node((*cs_tells_it)->get_child(), PacketSender::VA_Time, true);
  }
  // �����l������n���i�K�v�Ȃ��̂̂݁j
  // tells�Ɛ���X�g�A����tell���񂻂ꂼ��Ɋւ��āA�o������ϐ��̍ő�����񐔖����܂ł��K�v
  // ��j�������x''�܂ŏo������i�ő�����񐔂�2�j�Ȃ�΁Ax[0]��x'[0]�܂ł̒l���K�v

  // �e�ϐ��Ɋւ��āA�ő�����񐔂�ێ�����悤�Ȕz�񂪂ق���
  PacketSender::max_diff_map_t max_diff_map;
  ps.create_max_diff_map(max_diff_map);

  // max_diff_map���ɂ���ϐ��Ɋւ��āAMathematica�ɓn��
  int max_diff_map_count = 0;
  PacketSender::max_diff_map_t::const_iterator max_diff_map_it = max_diff_map.begin();
  for( ;max_diff_map_it != max_diff_map.end(); ++max_diff_map_it)
  {
    for(int i=0; i< max_diff_map_it->second; ++i) // �����񐔂�1��ȏ�̂��̂̂ݕK�v
    {
      max_diff_map_count++;
    }
  }
  ml_->put_function("List", max_diff_map_count);
  max_diff_map_it = max_diff_map.begin();
  for(; max_diff_map_it != max_diff_map.end(); ++max_diff_map_it)
  {
    for(int i=0; i< max_diff_map_it->second; ++i)
    {
      ml_->put_function("Equal", 2);
      ps.put_var(
        boost::make_tuple(max_diff_map_it->first, 
                          i,
                          false),
        PacketSender::VA_Zero);
      // �ϐ��������ɁA�l�̃V���{���쐬
      std::string init_value_str = init_prefix;    
      init_value_str += boost::lexical_cast<std::string>(i);
      init_value_str += max_diff_map_it->first;
      ml_->put_symbol(init_value_str);
    }
  }

  // vars��n��
  //ps.put_vars(PacketSender::VA_Time);

  int vars_count = 0;
  max_diff_map_it = max_diff_map.begin();
  for(; max_diff_map_it != max_diff_map.end(); ++max_diff_map_it)
  {
    for(int j=0; j<= max_diff_map_it->second; ++j)
    {
      vars_count++;
    }
  }

  ml_->put_function("List", vars_count);
  max_diff_map_it = max_diff_map.begin();
  for(; max_diff_map_it != max_diff_map.end(); ++max_diff_map_it)
  {
    for(int j=0; j<= max_diff_map_it->second; ++j)
    {
      ps.put_var(
        boost::make_tuple(max_diff_map_it->first, 
                          j,
                          false),
        PacketSender::VA_Time);
    }
  }

/*
  PacketChecker pc(*ml_);
  pc.check();
*/

  ml_->skip_pkt_until(RETURNPKT);

  ml_->MLGetNext(); // List�֐�
  // ���ʂ̎󂯎��
  // {1, {{usrVarht, 1, 0}, {usrVarv, 0, initValue0v - 10 t}, {usrVarht, 0, initValue0ht + (initValue0v - 5 t) t}, {usrVarv, 1, 0}}}
  // �����ɂ��List�֐��̗v�f�����ς��
  // underconstraint�܂���overconstraint���Ɨv�f��1�A����ȊO�Ȃ�v�f��2�̂͂�
  int list_arg_count = ml_->get_arg_count();
  if(list_arg_count ==1){
    // under or over constraint
  }
  ml_->MLGetNext(); // List�Ƃ����֐���
  ml_->MLGetNext(); // List�̐擪�̗v�f�i����1�j
  ml_->MLGetNext(); // List�֐�
  int cons_count = ml_->get_arg_count();
  ml_->MLGetNext(); // List�Ƃ����֐���
  for(int k=0; k<cons_count; ++k)
  {
    ml_->MLGetNext(); // List�֐�
    ml_->MLGetNext(); // List�Ƃ����֐���
    ml_->MLGetNext(); // List�̐擪�v�f�i�ϐ����j
    std::string var_name = ml_->get_symbol();
    int derivative_count = ml_->get_integer();
    var_name.insert(PacketSender::var_prefix.length(), boost::lexical_cast<std::string>(derivative_count));
    std::string value_str = ml_->get_string();
    std::string cons_str = var_name + "=" + value_str;
    std::cout << "cons_str:" << cons_str << std::endl;
  }

  return VCSR_TRUE;
}

/**
 * ���݂̐���X�g�A����^����ask�����o�\���ǂ���
 */
VCSResult RealPaverVCSInterval::check_entailment(const ask_node_sptr& negative_ask)
{
  // �X�g�A���R�s�[
  // �K�[�h�����Ɣے�����(�ϐ��͂��ׂď����l�ϐ���)
  // solve(S & g) == empty -> FALSE
  // solve(S&ng0)==empty /\ solve(S&ng1)==empty /\ ... -> TRUE
  // ng�����݂��Ȃ�(g������)�ꍇ�CTRUE�ł͂Ȃ�
  // else -> UNKNOWN
/*
  // ����X�g�A���R�s�[
  ctr_set_t ctrs = this->constraint_store_.get_store_exprs_copy();
  var_name_map_t vars = this->constraint_store_.get_store_vars();
  // �K�[�h�����Ƃ��̔ے�����
  ctr_set_t g, ng;
  var_name_map_t prevs_in_g;
  GuardConstraintBuilder builder;
  builder.set_vars(vars);
  builder.create_guard_expr(negative_ask, g, ng, vars, prevs_in_g);
  // �m�F
  {
    rp_vector_variable vec = ConstraintSolver::create_rp_vector(vars);
    HYDLA_LOGGER_DEBUG("#**** vcs:check_entailment: guards ****");
    std::stringstream ss;
    ctr_set_t::iterator it = g.begin();
    while(it != g.end()){
      rp::dump_constraint(ss, *it, vec, 10);
      ss << "\n";
      it++;
    }
    HYDLA_LOGGER_DEBUG(ss.str());
    ss.str("");
    HYDLA_LOGGER_DEBUG("#**** vcs:check_entailment: not_guards ****");
    it = ng.begin();
    while(it != ng.end()){
      if(*it != NULL) rp::dump_constraint(ss, *it, vec, 10);
      ss << "\n";
      it++;
    }
    HYDLA_LOGGER_DEBUG(ss.str());
    ss.str("");
    HYDLA_LOGGER_DEBUG("#**** vcs:check_entailment: prevs_in_guards ****");
    var_name_map_t::iterator it2 = prevs_in_g.begin();
    while(it2 != prevs_in_g.end()){
      HYDLA_LOGGER_DEBUG((*it2).left);
      it2++;
    }
    rp_vector_destroy(&vec);
  }

  // ask������prev�ϐ��Ɋւ��鎮�ł���C������prev�ϐ��̒l��(-oo,+oo)�������ꍇ�ɂ�FALSE
  if(this->is_guard_about_undefined_prev(vars, ctrs, prevs_in_g)) {
    HYDLA_LOGGER_DEBUG("#*** vcs:check_entailment ==> FALSE(guard_about_undefined_prev) ***");
    RealPaverVCSInterval::clear_ctr_set(ctrs);
    RealPaverVCSInterval::clear_ctr_set(g);
    RealPaverVCSInterval::clear_ctr_set(ng);
    //this->finalize();
    return VCSR_FALSE;
  }

  // solve(S & g) == empty -> FALSE
  rp_box box;
  ctr_set_t ctr_and_g = ctrs;
  ctr_and_g.insert(g.begin(), g.end());
  if(!(ConstraintSolver::solve_hull(&box, vars, ctr_and_g))) {
    HYDLA_LOGGER_DEBUG("#*** vcs:chack_entailment: ==> FALSE ***");
    RealPaverVCSInterval::clear_ctr_set(ctrs);
    RealPaverVCSInterval::clear_ctr_set(g);
    RealPaverVCSInterval::clear_ctr_set(ng);
    //this->finalize();
    return VCSR_FALSE;
  }
  rp_box_destroy(&box);

  // solve(S&ng0)==empty /\ solve(S&ng1)==empty /\ ... -> TRUE
  // ng�����݂��Ȃ�(g������)�ꍇ�CTRUE�ł͂Ȃ�
  bool is_TRUE = true;
  if(ng.size() == 0) is_TRUE = false;
  for(ctr_set_t::iterator ctr_it=ng.begin();
    ctr_it!=ng.end(); ctr_it++) {
    ctr_set_t ctr_and_ng = ctrs;
    ctr_and_ng.insert(*ctr_it);
    if(ConstraintSolver::solve_hull(&box, vars, ctr_and_ng)) {
      is_TRUE = false;
      rp_box_destroy(&box);
    }
  }
  if(is_TRUE) {
    HYDLA_LOGGER_DEBUG("#*** vcs:check_entailment: ==> TRUE ***");
    RealPaverVCSInterval::clear_ctr_set(ctrs);
    RealPaverVCSInterval::clear_ctr_set(g);
    RealPaverVCSInterval::clear_ctr_set(ng);
    //this->finalize();
    return VCSR_TRUE;
  }

  // else -> UNKNOWN
  HYDLA_LOGGER_DEBUG("#*** vcs:check_entailment: ==> UNKNOWN ***");
  RealPaverVCSInterval::clear_ctr_set(ctrs);
  RealPaverVCSInterval::clear_ctr_set(g);
  RealPaverVCSInterval::clear_ctr_set(ng);
  //this->finalize();
  return VCSR_UNKNOWN;
*/
return VCSR_TRUE;
}

void RealPaverVCSInterval::clear_ctr_set(ctr_set_t& ctrs)
{
  ctrs.erase(static_cast<rp_constraint>(NULL));
  for(ctr_set_t::iterator it=ctrs.begin();
    it!=ctrs.end();it++) {
      rp_constraint c = *it;
      rp_constraint_destroy(&c);
  }
  ctrs.clear();
}
/*
bool RealPaverVCSInterval::is_guard_about_undefined_prev(const var_name_map_t& vars,
                                                      const std::set<rp_constraint>& ctrs,
                                                      const var_name_map_t& p_in_g)
{
  bool res = false;
  rp_box box;
  bool is_consistent_store_only = ConstraintSolver::solve_hull(&box, vars, ctrs);
  assert(is_consistent_store_only);
  for(var_name_map_t::right_const_iterator it=p_in_g.right.begin();
    it!=p_in_g.right.end(); it++) {
    int index = it->first;
    assert(index >= 0);
    if(rp_binf(rp_box_elem(box, index))==-RP_INFINITY
      && rp_bsup(rp_box_elem(box, index))==RP_INFINITY) res = true;
  }
  rp_box_destroy(&box);
  return res;
}
*/
/**
 * ask�̓��o��Ԃ��ω�����܂Őϕ��������Ȃ�
 */
VCSResult RealPaverVCSInterval::integrate(integrate_result_t& integrate_result,
                                  const positive_asks_t& positive_asks,
                                  const negative_asks_t& negative_asks,
                                  const time_t& current_time,
                                  const time_t& max_time)
{
  // nodes_��rp_constraint�ɂȂ���
  // ������ϐ��Ƃ��Ė��ɑg�ݍ���(rp_variable)
  // ����
  // �������Ȃ���VCSR_FALSE(�������Őς܂Ȃ������ł������񂾂���)
  // ������proof����Ă��邩�H
  //// proof�`�F�b�N�P�F������hull�������l�ϐ��̃h���C�������ׂĊ܂�ł��邩�H
  //// proof�`�F�b�N�Q�F(�\���o�ɂ��proof�����݂��邩�H)
  //// proof�`�F�b�N�Q���F�������ɂ��Ď����ȊO�̕ϐ���萔�ɕς���proof�������邩�H
  // ����Ă��遨�������������Ƀ\�[�g�Chull�������l�ϐ��h���C�������ׂĊ܂񂾎��_�Ŏc����̂Ă�
  //// �u���_�Łv���Ə����̂Ă����Ă��܂��\�����c�H -> ���������̂͂�邹�΂���
  // ����ĂȂ����S���ςށC����ask�𔲂��čēxintegrate���邽�߂�VCSR_UNKNOWN��Ԃ�
  return VCSR_TRUE;
}

/**
 * ������Ԃ̏o�͂������Ȃ�
 */
std::ostream& RealPaverVCSInterval::dump(std::ostream& s) const
{
  return s;
  //return this->constraint_store_.dump_cs(s);
}

void RealPaverVCSInterval::add_single_constraint(const node_sptr &constraint_node,
                                              const bool neg_expression)
{
  ConstraintBuilder builder;
  rp_constraint c;
  c = builder.build_constraint(constraint_node, neg_expression);
  var_name_map_t vars;
  vars.insert(builder.vars_begin(), builder.vars_end());
  //if(c) this->constraint_store_.add_constraint(c, vars);
}

std::ostream& operator<<(std::ostream& s, const RealPaverVCSInterval& vcs)
{
  return vcs.dump(s);
}


} // namespace realapver
} // namespace vcs
} // namespace hydla 
