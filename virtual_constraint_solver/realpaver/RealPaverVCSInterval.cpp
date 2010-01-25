#include "RealPaverVCSInterval.h"

#include <sstream>
#include <algorithm>
#include <map>

#include "RPConstraintSolver.h"
#include "Logger.h"
#include "realpaver.h"
#include "rp_constraint_ext.h"
#include "rp_container_ext.h"
#include "rp_problem_ext.h"

#include "../mathematica/PacketSender.h"
#include "../mathematica/PacketChecker.h"
#include <boost/lexical_cast.hpp>

#undef min
#undef max

using namespace hydla::vcs::mathematica;
using namespace hydla::simulator;

namespace hydla {
namespace vcs {
namespace realpaver {

/**
 * �\�[�g�p��r�֐�(next_phase_state_t)
 */
struct TimeBoxCompare {
  int index_;
  TimeBoxCompare(int index):index_(index){}
  template<typename T>
  bool operator()(const T& lhs, const T& rhs) {
    return rp_binf(rp_box_elem(lhs, index_)) < rp_binf(rp_box_elem(rhs, index_));
  }
};

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
  HYDLA_LOGGER_DEBUG("#** vcs:add_constraint: use MathLink to integrate expression **");
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
        boost::make_tuple(max_diff_map_it->first, i, false),
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
  for(; max_diff_map_it != max_diff_map.end(); ++max_diff_map_it) {
    for(int j=0; j<= max_diff_map_it->second; ++j) vars_count++;
  }

  ml_->put_function("List", vars_count);
  max_diff_map_it = max_diff_map.begin();
  for(; max_diff_map_it != max_diff_map.end(); ++max_diff_map_it) {
    for(int j=0; j<= max_diff_map_it->second; ++j) {
      ps.put_var(
        boost::make_tuple(max_diff_map_it->first, j, false),
        PacketSender::VA_Time);
    }
  }

  ml_->skip_pkt_until(RETURNPKT);

  ml_->MLGetNext(); // List�֐�
  // ���ʂ̎󂯎��
  // {1, {{usrVarht, 1, 0}, {usrVarv, 0, initValue0v - 10 t}, {usrVarht, 0, initValue0ht + (initValue0v - 5 t) t}, {usrVarv, 1, 0}}}
  // �����ɂ��List�֐��̗v�f�����ς��
  // underconstraint�܂���overconstraint���Ɨv�f��1�A����ȊO�Ȃ�v�f��2�̂͂�
  int list_arg_count = ml_->get_arg_count();
  if(list_arg_count ==1){
    // under or over constraint
    HYDLA_LOGGER_DEBUG("#** vcs:add_constraint: cannot integrate expression ==> SOLVER ERROR **");
    return VCSR_SOLVER_ERROR;
  }
  ml_->MLGetNext(); // List�Ƃ����֐���
  ml_->MLGetNext(); // List�̐擪�̗v�f�i����1�j
  ml_->MLGetNext(); // List�֐�
  int cons_count = ml_->get_arg_count();
  ml_->MLGetNext(); // List�Ƃ����֐���

  // rp_constraint�쐬�C�ێ��p
  ctr_set_t ctrs, ctrs_copy;
  rp_vector_variable vec = ConstraintSolver::create_rp_vector(this->constraint_store_.get_store_vars());
  rp_table_symbol ts;
  rp_table_symbol_create(&ts);
  rp_vector_destroy(&rp_table_symbol_vars(ts));
  rp_table_symbol_vars(ts) = vec;

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
    // rp_constraint���쐬
    rp_constraint c;
    // TODO: cons_str����[]��啶����e^�Ȃǂ�K�؂ɕύX����΂����ƍL���p�[�Y�\
    if(!rp_parse_constraint_string(&c, const_cast<char *>(cons_str.c_str()), ts)){
      // TODO: ��������������̕K�v�����邩��
      HYDLA_LOGGER_DEBUG("#** vcs:add_constraint: cannot translate into rp_constraint ==> SOLVER ERROR");
      rp_table_symbol_destroy(&ts);
      return VCSR_SOLVER_ERROR;
    }
    ctrs.insert(c);
  }
  rp_table_symbol_destroy(&ts);
  // �R�s�[���Ă���
  for(ctr_set_t::iterator it=ctrs.begin(); it!=ctrs.end(); it++) {
    rp_constraint c;
    rp_constraint_clone(&c, *it);
    ctrs_copy.insert(c);
  }

  // consistency���`�F�b�N
  ctr_set_t store_copy = this->constraint_store_.get_store_exprs_copy();
  ctrs.insert(store_copy.begin(), store_copy.end());
  // �m�F
  // TODO: get_store_vars�ɑ��݂��Ȃ��ϐ������Ɏg���Ă���\���H
  vec = ConstraintSolver::create_rp_vector(this->constraint_store_.get_store_vars());
  HYDLA_LOGGER_DEBUG("#**** vcs:add_constraint: constraints expression ****");
  std::stringstream ss;
  for(ctr_set_t::iterator it=ctrs.begin(); it!=ctrs.end(); it++) {
    rp::dump_constraint(ss, *it, vec, 10); ss << "\n";
  }
  rp_vector_destroy(&vec);
  HYDLA_LOGGER_DEBUG(ss.str());
  // ����̉������݂��邩�ǂ����H
  rp_box b;
  bool res = ConstraintSolver::solve_hull(&b, this->constraint_store_.get_store_vars(), ctrs);
  if(res) {
    // consistent�Ȃ�C�X�g�A��tell�m�[�h��ǉ�
    HYDLA_LOGGER_DEBUG("#*** vcs:add_constraint ==> Consistent ***\n");
    this->constraint_store_.nodes_.insert(this->constraint_store_.nodes_.end(),
      collected_tells.begin(), collected_tells.end());
    this->constraint_store_.clear_non_init_constraint();
    this->constraint_store_.set_non_init_constraint(ctrs_copy);
    rp_box_destroy(&b);
    RealPaverVCSInterval::clear_ctr_set(ctrs);
    return VCSR_TRUE;
  } else {
    // in-consistent�Ȃ�C�������Ȃ�
    HYDLA_LOGGER_DEBUG("#*** vcs:add_constraint ==> Inconsistent ***\n");
    RealPaverVCSInterval::clear_ctr_set(ctrs);
    RealPaverVCSInterval::clear_ctr_set(ctrs_copy);
    return VCSR_FALSE;
  }
}

/**
 * ���݂̐���X�g�A����^����ask�����o�\���ǂ���
 */
VCSResult RealPaverVCSInterval::check_entailment(const ask_node_sptr& negative_ask)
{
  // ����X�g�A���R�s�[
  ctr_set_t ctrs = this->constraint_store_.get_store_exprs_copy();
  var_name_map_t vars = this->constraint_store_.get_store_vars();
  // �K�[�h�����Ƃ��̔ے�����
  ctr_set_t g, ng;
  var_name_map_t prevs_in_g;
  GuardConstraintBuilder builder;
  builder.set_vars(vars);
  // �K�[�h���̑S�Ă̕ϐ��͏����l�ϐ��ł���(6�����ڂ�true�Ő���)
  builder.create_guard_expr(negative_ask, g, ng, vars, prevs_in_g, true);
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

  // solve(S & g) == empty -> FALSE
  rp_box box;
  ctr_set_t ctr_and_g = ctrs;
  ctr_and_g.insert(g.begin(), g.end());
  if(!(ConstraintSolver::solve_hull(&box, vars, ctr_and_g))) {
    HYDLA_LOGGER_DEBUG("#*** vcs:chack_entailment: ==> FALSE ***");
    RealPaverVCSInterval::clear_ctr_set(ctrs);
    RealPaverVCSInterval::clear_ctr_set(g);
    RealPaverVCSInterval::clear_ctr_set(ng);
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
    return VCSR_TRUE;
  }

  // else -> UNKNOWN�����CIP��UNKOWN�͋N���Ȃ��͂��Ȃ̂�SOLVER ERROR
  // TODO: �u�͂��v�����C�����ɂ͋N����D�O��PP����̒l�ȊO�̈����p�����d�v�ȋC������
  HYDLA_LOGGER_DEBUG("#*** vcs:check_entailment: ==> SOLVER ERROR(UNKNOWN) ***");
  RealPaverVCSInterval::clear_ctr_set(ctrs);
  RealPaverVCSInterval::clear_ctr_set(g);
  RealPaverVCSInterval::clear_ctr_set(ng);
  return VCSR_SOLVER_ERROR;
}

void RealPaverVCSInterval::clear_ctr_set(ctr_set_t& ctrs)
{
  ctrs.erase(static_cast<rp_constraint>(NULL));
  for(ctr_set_t::iterator it=ctrs.begin();
    it!=ctrs.end();it++) {
      rp_constraint c = *it;
      if(c) rp_constraint_destroy(&c);
  }
  ctrs.clear();
}

/**
 * ask�̓��o��Ԃ��ω�����܂Őϕ��������Ȃ�
 */
VCSResult RealPaverVCSInterval::integrate(integrate_result_t& integrate_result,
                                  const positive_asks_t& positive_asks,
                                  const negative_asks_t& negative_asks,
                                  const time_t& current_time,
                                  const time_t& max_time)
{
  typedef virtual_constraint_solver_t::IntegrateResult::next_phase_state_t next_phase_state_t;
  typedef var_name_map_t::value_type vars_type_t;

  // �V�����ϐ�tt = t + t0(���_��IP�̊J�n�����ł���{����(?)�����ϐ�)��
  // �ϐ� tsum = tt + te(���o�ߎ��ԁCRP�Ɍv�Z���Ă��炤) ��p��
  // �萔t0(����܂ł̌o�ߎ��ԋ�Ԃ̕�)�Cte(�o�ߎ��ԋ�ԂƓ����l)
  // �����ł͕ϐ������p��
  var_name_map_t vars = this->constraint_store_.get_store_vars();
  std::string timevar_names[2] = {"tt", "tsum"};
  for(int i=0; i<2; ++i) {
    unsigned int size = vars.size();
    var_property vp(0, false);
    vars.insert(vars_type_t(timevar_names[i], size, vp));
  }

  double min_time = RP_INFINITY;
  std::vector<rp_box> results;
  ask_sptr ask_node;
  AskState ask_state;

  // �S�Ă�ask�ɂ��Čv�Z����
  // �܂���positive_asks����
  positive_asks_t::const_iterator pit, pend = positive_asks.end();
  for(pit=positive_asks.begin(); pit!=pend; ++pit) {
    std::vector<rp_box> tmp_results;
    // ���������֐�
    this->find_next_point_phase_states(tmp_results, *pit, vars,
      current_time, max_time, Positive2Negative);
    // �������Ȃ�������continue
    if(tmp_results.size() == 0) continue;
    // �������Ƀ\�[�g���čŏ��l�𓾂�
    std::sort(tmp_results.begin(), tmp_results.end(), TimeBoxCompare(vars.left.at("tsum")));
    // �����̎����̍ŏ��l���v�Z���Cmin_time��菬����������X�V����D
    if(rp_binf(rp_box_elem(tmp_results.at(0), vars.left.at("tt"))) < min_time) { // TODO: ���S�ɓ������̏ꍇ�́H
      for(std::vector<rp_box>::iterator vit=results.begin(); vit!=results.end(); ++vit) {
        rp_box b = *vit;
        rp_box_destroy(&b);
      }
      results.clear();
      results = tmp_results;
      ask_node = *pit;
      ask_state = Positive2Negative;
    }
  }
  // ����negative_asks
  negative_asks_t::const_iterator nit, nend = negative_asks.end();
  for(nit=negative_asks.begin(); nit!=nend; ++nit) {
    std::vector<rp_box> tmp_results;
    integrate_result_t tmp_result;
    rp_box tmp_box=NULL;
    // ���������֐�
    this->find_next_point_phase_states(tmp_results, *nit, vars,
      current_time, max_time, Negative2Positive);
    // �������Ȃ�������continue
    if(tmp_results.size()==0) continue;
    // �������Ƀ\�[�g���čŏ��l�𓾂�
    std::sort(tmp_results.begin(), tmp_results.end(), TimeBoxCompare(vars.left.at("tsum")));
    // �����̎����̍ŏ��l���v�Z���Cmin_time��菬����������X�V����D
    if(rp_binf(rp_box_elem(tmp_results.at(0), vars.left.at("tt"))) < min_time) { // TODO: ���S�ɓ������̏ꍇ�́H
      for(std::vector<rp_box>::iterator vit=results.begin(); vit!=results.end(); ++vit) {
        rp_box b = *vit;
        rp_box_destroy(&b);
      }
      results.clear();
      results = tmp_results;
      ask_node = *nit;
      ask_state = Negative2Positive;
    }
  }

  // �f�o�b�O�m�F
  //for(std::vector<rp_box>::iterator vit=results.begin(); vit!=results.end(); ++vit) {
  //  rp_box_display_simple_nl(*vit);
  //}

  // �������Ȃ���VCSR_FALSE(�������Őς܂Ȃ������ł������񂾂���)
  if(results.size() == 0) return VCSR_FALSE;

  // ������proof����Ă��邩�H
  //// proof�`�F�b�N�P�F������hull�������l�ϐ��̃h���C�������ׂĊ܂�ł��邩�H
  //// box_hull�̂��������l�̃h���C���ɂ��Ē��ׂ�
  //// init_ctr������solve_hull����box�Ə����l�ϐ�������ׂĈ�v����΂n�j
  bool guarantee = false;
  double g_time_sup;
  rp_box init_hull, result_hull;
  ctr_set_t init_ctr = this->constraint_store_.get_store_exprs_copy();
  ConstraintSolver::solve_hull(&init_hull, vars, init_ctr, 0.5);
  clear_ctr_set(init_ctr);
  rp_box_clone(&result_hull, results.at(0));
  // ������hull���v�Z����init_hull�ƈ�v���邩�m���߂�
  for(std::vector<rp_box>::iterator vit=results.begin(); vit!=results.end(); ++vit) {
    rp_box_merge(result_hull, *vit);
    bool hull_g = true;
    for(var_name_map_t::right_iterator vnm_it=vars.right.begin();
      vnm_it!=vars.right.end(); ++vnm_it) {
        if(vnm_it->info.prev_flag) {
          if(!rp_interval_equal(
            rp_box_elem(result_hull, vnm_it->first),
            rp_box_elem(init_hull, vnm_it->first))) {
              hull_g = false;
          }
        }
    }
    if(hull_g && !guarantee) { // ���鎞����proof�`�F�b�N�P�𖞂�����
      guarantee = true;
      g_time_sup = rp_bsup(rp_box_elem(*vit, vars.left.at("tsum")));
    }
    if(guarantee) { // ���ł�proof�`�F�b�N�𖞂�����
      if(rp_bsup(rp_box_elem(*vit, vars.left.at("tsum"))) > g_time_sup) {
        // proof�`�F�b�N�𖞂��������������box�ł��� => �c��폜
        for(std::vector<rp_box>::iterator vit2=vit; vit2!=results.end(); ++vit2) {
          rp_box b = *vit2;
          rp_box_destroy(&b);
        }
        results.erase(vit, results.end());
        break;
      }
    }
  }
  rp_box_destroy(&init_hull);
  rp_box_destroy(&result_hull);
  
  // �ςނׂ����ʂ�����ꂽ
  // ����Ă��遨�������������Ƀ\�[�g�Chull�������l�ϐ��h���C�������ׂĊ܂񂾎��_�Ŏc����̂Ă�
  // TODO: ����`�ȃK�[�h�������Ǝ�������hull������Ă����ƃ_���ȂƂ������肻���I
  // ����ĂȂ����S���ςށC����ask�𔲂��čēxintegrate���邽�߂�VCSR_UNKNOWN��Ԃ�
  if(guarantee) {
    HYDLA_LOGGER_DEBUG("#*** vcs:integrate: ==> GUARANTEED");

  } else {
    HYDLA_LOGGER_DEBUG("#*** vcs:integrate: ==> NOT GUARANTEED");
  }
  // results��time, vm�ɕϊ����đS���ς�
  for(std::vector<rp_box>::iterator vit=results.begin(); vit!=results.end(); ++vit) {
    next_phase_state_t nps;
    nps.is_max_time = false;
    // �܂��Ctsum��time_t�֒���
    rp_interval tsumi;
    rp_interval_copy(tsumi, rp_box_elem(*vit, vars.left.at("tsum")));
    nps.time.inf_ = rp_binf(tsumi);
    nps.time.sup_ = rp_bsup(tsumi);
    // �ϐ���variable_map�ɒ���
    // �����l�ϐ�������Ă���
    for(var_name_map_t::right_const_iterator vnm_it=vars.right.begin();
      vnm_it!=vars.right.end(); ++vnm_it) {
        if(vnm_it->info.prev_flag) continue; // �����l�ϐ��͒����Ȃ�
        std::string name(vnm_it->second);
        if(name=="t" || name=="tt" || name=="tsum") continue; //�����ϐ��͒����Ȃ�
        RPVariable bp_variable;
        name.erase(0, var_prefix.length());
        std::string dc_str(boost::lexical_cast<std::string>(vnm_it->info.derivative_count));
        name.erase(0, dc_str.length());
        bp_variable.derivative_count = vnm_it->info.derivative_count;
        bp_variable.name = name;
        RPValue bp_value(rp_binf(rp_box_elem(*vit, vnm_it->first)),
          rp_bsup(rp_box_elem(*vit, vnm_it->first)));
        nps.variable_map.set_variable(bp_variable, bp_value);
    }
    HYDLA_LOGGER_DEBUG("#*** vcs:integrate: one of result ***\n",
      "time <=> ", nps.time, "\n",
      nps.variable_map);
    integrate_result.states.push_back(nps);
  }
  integrate_result.changed_asks.push_back(
    std::make_pair(ask_state, ask_node->get_id()));

  // ��n��
  for(std::vector<rp_box>::iterator vit=results.begin(); vit!=results.end(); ++vit) {
    rp_box b = *vit;
    rp_box_destroy(&b);
  }

  if(guarantee) return VCSR_TRUE;
  else          return VCSR_UNKNOWN;

  //// TODO: proof�`�F�b�N�Q�F(�\���o�ɂ��proof�����݂��邩�H)
  //// TODO: proof�`�F�b�N�Q���F�������ɂ��Ď����ȊO�̕ϐ���萔�ɕς���proof�������邩�H
}

/**
 * ask����ɑ΂��āC���̃K�[�h������(������|�������Ȃ�)�悤�ɂȂ�
 * ��Ԃ����߂�integrate_result�֓����
 */
void RealPaverVCSInterval::find_next_point_phase_states(
  std::vector<rp_box>& results,
  const ask_sptr ask,
  var_name_map_t vars,
  const time_t& current_time,
  const time_t& max_time,
  hydla::simulator::AskState state)
{
  typedef std::pair<double, double> interval_t;
  typedef std::map<std::string, interval_t> dom_map_t;

  // ask����K�[�h����(�܂��͔ے�)������
  EqualConstraintBuilder builder;
  ctr_set_t guards;
  bool is_n2p = (state == Negative2Positive);
  builder.create_expr(ask, guards, vars, is_n2p);
  // �m�F
  std::stringstream ss;
  rp_vector_variable vec = ConstraintSolver::create_rp_vector(vars);
  for(ctr_set_t::iterator it=guards.begin(); it!=guards.end(); ++it) {
    rp_constraint c = *it;
    if(!c) assert(false); // �����Ȃ��I����̉ۑ�
    rp::dump_constraint(ss, c, vec);
    ss << "\n";
  }
  rp_vector_destroy(&vec);
  HYDLA_LOGGER_DEBUG("#*** vcs:integrate:find_naxt_pp_states: ***\n",
    " **** guards ****\n",
    ss.str());

  // �萔t0(����܂ł̌o�ߎ��ԋ�Ԃ̕�)�Cte(�o�ߎ��ԋ�ԂƓ����l)
  // ��p��
  if(is_n2p) {
    // N2P -> �K�[�h�����𐧖�ɉ�����
    // �萔t0��te��p��
    rp_problem problem;
    rp_problem_create(&problem, "find_next_pp");
    rp_constant t0, te;
    rp_interval t0i, tei;
    rp_interval_set(t0i, 0.0, current_time.width()); // TODO: width�ɂ�錅�����H
    rp_interval_set(tei, current_time.inf_, current_time.sup_);
    rp_constant_create(&t0, "t0", t0i);
    rp_constant_create(&te, "te", tei);
    rp_vector_insert(rp_problem_nums(problem), t0);
    rp_vector_insert(rp_problem_nums(problem), te);
    // �ϐ��̃h���C����p��
    // ��{�I�ɂ�(-oo, +oo)
    // �V�����ϐ�tt = t + t0(���_��IP�̊J�n�����ł���{����(?)�����ϐ�)��
    // �ϐ� tsum = tt + te(���o�ߎ��ԁCRP�Ɍv�Z���Ă��炤) ��p��
    // tt��[0, max_time.sup - current_time.inf]
    // t��[0, +oo), tsum��[0, +oo)
    // TODO: tt����Ȃ��ˁH
    //TODO: ���x�́H
    dom_map_t dom_map;
    dom_map.insert(dom_map_t::value_type("tt", interval_t(0, max_time.sup_ - current_time.inf_)));
    dom_map.insert(dom_map_t::value_type("t", interval_t(0, RP_INFINITY)));
    dom_map.insert(dom_map_t::value_type("tsum", interval_t(0, RP_INFINITY)));
    rp_vector_destroy(&rp_problem_vars(problem));
    rp_problem_vars(problem) = ConstraintSolver::create_rp_vector(vars, dom_map, 0.1);
    // ������R�s�[
    ctr_set_t ctrs = this->constraint_store_.get_store_exprs_copy();
    ctr_set_t non_init_ctrs = this->constraint_store_.get_store_non_init_constraint_copy();
    ctrs.insert(non_init_ctrs.begin(), non_init_ctrs.end());
    ctrs.insert(guards.begin(), guards.end());
    // �V�������� t = tt - t0�� tsum = tt + te��������
    rp_constraint c;
    rp_parse_constraint_string(&c, "t=tt-t0", rp_problem_symb(problem));
    ctrs.insert(c);
    rp_parse_constraint_string(&c, "tsum=tt+te", rp_problem_symb(problem));
    ctrs.insert(c);
    // ���ɐ����o�^
    for(ctr_set_t::iterator cit=ctrs.begin(); cit!=ctrs.end(); ++cit) {
      if(*cit==NULL) continue; // NULL�|�C���^�̓X�L�b�v
      rp_vector_insert(rp_problem_ctrs(problem), *cit);
      for(int i=0; i<rp_constraint_arity(*cit); i++) {
        ++rp_variable_constrained(rp_problem_var(problem, rp_constraint_var(*cit, i)));
      }
    }
    rp_problem_set_initial_box(problem);
    HYDLA_LOGGER_DEBUG("#*** vcs:integrate:find_naxt_pp_states: ***\n",
      "#**** problem to solve ****\n",
      problem);
    // ����
    rp_selector* select;
    rp_new(select,rp_selector_roundrobin,(&problem));
    rp_splitter* split;
    rp_new(split,rp_splitter_mixed,(&problem));
    // TODO: prover�g��
    rp_bpsolver solver(&problem,10,select,split);
    rp_box sol;
    // ����variable_map�ɒ�����result�֓���Ă���
    virtual_constraint_solver_t::IntegrateResult::next_phase_state_t nps;
    nps.is_max_time = true; // �����o�Ȃ������ꍇ�Ctrue
    while((sol=solver.compute_next())!=NULL) {
      // tt��0���܂ނ��͉̂�����Ȃ� -> �̂Ă�
      if(rp_interval_contains(rp_box_elem(sol, vars.left.at("tt")), 0.0)) continue;
      nps.is_max_time = false; // TODO: �����ɂ͈Ⴄ�\�����c�H
      // box���N���[������vector�ɓ����
      rp_box sol_clone;
      rp_box_clone(&sol_clone, sol);
      results.push_back(sol_clone);
      //// �܂��Ctsum��time_t�֒���
      //rp_interval tsumi;
      //rp_interval_copy(tsumi, rp_box_elem(sol, vars.left.at("tsum")));
      //nps.time.inf_ = rp_binf(tsumi);
      //nps.time.sup_ = rp_bsup(tsumi);
      //// �ϐ���variable_map�ɒ���
      //// �����l�ϐ�������Ă���
      //var_name_map_t::right_const_iterator vnm_it;
      //for(vnm_it=vars.right.begin(); vnm_it!=vars.right.end(); ++vnm_it) {
      //  std::string name(vnm_it->second);
      //  if(vnm_it->info.prev_flag) {
      //    name.erase(0, init_prefix.length());
      //  } else {
      //    if(name=="t" || name=="tt" || name=="tsum") continue; //�����ϐ��͒����Ȃ�
      //    RPVariable bp_variable;
      //    name.erase(0, var_prefix.length());
      //  }
      //  std::string dc_str(boost::lexical_cast<std::string>(vnm_it->info.derivative_count));
      //  name.erase(0, dc_str.length());
      //  bp_variable.derivative_count = vnm_it->info.derivative_count;
      //  bp_variable.name = name;
      //  RPValue bp_value(rp_binf(rp_box_elem(sol, vnm_it->first)),
      //    rp_bsup(rp_box_elem(sol, vnm_it->first)));
      //  nps.variable_map.set_variable(bp_variable, bp_value);
      //}
      //HYDLA_LOGGER_DEBUG("#*** vcs:integrate:find_naxt_pp_states: ***\n",
      //  "#**** one of result ****\n",
      //  "time <=> ", nps.time, "\n",
      //  nps.variable_map);
      //integrate_result.states.push_back(nps);
    } // while
    rp_problem_destroy(&problem);
  } else {
    // P2N -> �K�[�h�����̔ے��������������������čŏ����������
    //�@TODO: �߂����������I 
  }
  return;
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
