#include "REDUCEVCSInterval.h"

#include <string>
#include <cassert>
#include <boost/foreach.hpp>

#include "Logger.h"
#include "PacketChecker.h"
#include "PacketErrorHandler.h"

using namespace hydla::vcs;
using namespace hydla::logger;

namespace hydla {
namespace vcs {
namespace reduce {

namespace {

struct MaxDiffMapDumper
{
  template<typename T>
  MaxDiffMapDumper(T it, T end)
  {
    for(; it!=end; ++it) {
      s << "name: " << it->first
        << "  diff: " << it->second
        << "\n";      
    }
  }

  std::stringstream s;
};

}


REDUCEVCSInterval::REDUCEVCSInterval(REDUCELink* cl, int approx_precision) :
  cl_(cl),
  approx_precision_(approx_precision)
{
}

REDUCEVCSInterval::~REDUCEVCSInterval()
{}

bool REDUCEVCSInterval::reset()
{
  constraint_store_ = constraint_store_t();
  return true;
}

bool REDUCEVCSInterval::reset(const variable_map_t& variable_map)
{

  HYDLA_LOGGER_VCS_SUMMARY("#*** Reset Constraint Store ***");
  if(variable_map.size() == 0)
  {
    HYDLA_LOGGER_VCS_SUMMARY("no Variables");
    return true;
  }
  HYDLA_LOGGER_VCS_SUMMARY("------Variable map------\n", 
                     variable_map);

  variable_map_t::variable_list_t::const_iterator it  = variable_map.begin();
  variable_map_t::variable_list_t::const_iterator end = variable_map.end();
  for(; it!=end; ++it) {
    if(!it->second.is_undefined()) {
      constraint_store_.init_vars.insert(
        std::make_pair(it->first, it->second));
    }

    else {
      value_t value;
      constraint_store_.init_vars.insert(
        std::make_pair(it->first, value));
    }

    // �����l����Ɋւ���max_diff_map�ɒǉ�
    constraint_store_t::init_vars_max_diff_map_t::iterator ivmd_it = 
      constraint_store_.init_vars_max_diff_map.find(it->first.name);
    if(ivmd_it==constraint_store_.init_vars_max_diff_map.end()) 
    {
      constraint_store_.init_vars_max_diff_map.insert(
        std::make_pair(it->first.name, it->first.derivative_count));
    }
    else if(it->first.derivative_count > ivmd_it->second)
    {
      ivmd_it->second = it->first.derivative_count;
    }

  }

  HYDLA_LOGGER_VCS(constraint_store_);

  return true;
}



bool REDUCEVCSInterval::reset(const variable_map_t& variable_map,  const parameter_map_t& parameter_map)
{
  if(!reset(variable_map)){
    return false;
  }


  if(parameter_map.size() == 0)
  {
    HYDLA_LOGGER_VCS_SUMMARY("no Parameters");
    return true;
  }
  HYDLA_LOGGER_VCS_SUMMARY("------Parameter map------\n", 
                     parameter_map);

  parameter_map_=parameter_map;

  return true;
}



void REDUCEVCSInterval::create_max_diff_map(
  REDUCEStringSender& rss, max_diff_map_t& max_diff_map)
{
  PacketSender::vars_const_iterator vars_it  = rss.vars_begin();
  PacketSender::vars_const_iterator vars_end = rss.vars_end();
  for(; vars_it!=vars_end; ++vars_it) {
    std::string name(vars_it->get<0>());
    int derivative_count = vars_it->get<1>();

    max_diff_map_t::iterator it = max_diff_map.find(name);
    if(it==max_diff_map.end()) {
      max_diff_map.insert(
        std::make_pair(name, derivative_count));
    }
    else if(it->second < derivative_count) {
      it->second = derivative_count;
    }    
  }

  HYDLA_LOGGER_VCS(
    "-- max diff map --\n",
    MaxDiffMapDumper(max_diff_map.begin(), 
                     max_diff_map.end()).s.str());

}

void REDUCEVCSInterval::send_init_cons(
  REDUCEStringSender& rss, 
  const max_diff_map_t& max_diff_map,
  bool use_approx)
{
  HYDLA_LOGGER_VCS("---- Begin REDUCEVCSInterval::send_init_cons ----");
    
  constraint_store_t::init_vars_t::const_iterator init_vars_it;
  constraint_store_t::init_vars_t::const_iterator init_vars_end;
  init_vars_it  = constraint_store_.init_vars.begin();
  init_vars_end = constraint_store_.init_vars.end();

  cl_->send_string("{");
  bool first_element = true;
  for(; init_vars_it!=init_vars_end; ++init_vars_it) {
    max_diff_map_t::const_iterator md_it = 
      max_diff_map.find(init_vars_it->first.name);

    // �����l����̂����A�W�߂�tell����ɏo������ۂ̍ő�����񐔂��������������񐔂̂��̂̂ݑ���
    if(md_it!=max_diff_map.end() &&
       md_it->second  > init_vars_it->first.derivative_count) 
    {
      if(!first_element) cl_->send_string(",");
      cl_->send_string("{t=0, ");
      // �ϐ���
      rss.put_var(
        boost::make_tuple(init_vars_it->first.name, 
                          init_vars_it->first.derivative_count, 
                          false));

      cl_->send_string("=");
  
      // �l
      if(use_approx && approx_precision_ > 0) {
        // �ߎ����đ��M
        // TODO:���Ƃ�����
//        ml_->put_function("approxExpr", 2);
//        ml_->put_integer(approx_precision_);
      }
      rss.put_node(init_vars_it->second.get_node(), false);
      cl_->send_string("}");
      first_element = false;
    }
  }
  cl_->send_string("}");

}



void REDUCEVCSInterval::send_parameter_cons() const{
  HYDLA_LOGGER_VCS("---- Begin REDUCEVCSInterval::send_parameter_cons ----");


  parameter_map_t::const_iterator par_it = parameter_map_.begin();
  parameter_map_t::const_iterator par_end = parameter_map_.end();
  int para_size=0;
  for(; par_it!=par_end; ++par_it) {
    value_range_t::or_vector::const_iterator or_it = par_it->second.or_begin(), or_end = par_it->second.or_end();
    for(;or_it != or_end; or_it++){
      para_size += or_it->size();
    }
  }

  HYDLA_LOGGER_VCS("parameter_cons_count: ", para_size);
  HYDLA_LOGGER_VCS_SUMMARY("------Parameter map------\n", 
                     parameter_map_);

  // TODO: �����Ƒ���
  cl_->send_string("{}");
}


void REDUCEVCSInterval::send_pars() const{
  HYDLA_LOGGER_VCS("---- Begin REDUCEVCSInterval::send_pars ----");


  parameter_map_t::const_iterator par_it = parameter_map_.begin();
  parameter_map_t::const_iterator par_end = parameter_map_.end();

  // TODO: �����Ƒ���
  cl_->send_string("{}");
}



void REDUCEVCSInterval::send_vars(
  REDUCEStringSender& rss, const max_diff_map_t& max_diff_map)
{
  HYDLA_LOGGER_VCS("---- REDUCEVCSInterval::send_vars ----");

  assert(0);

}

VCSResult REDUCEVCSInterval::add_constraint(const tells_t& collected_tells, const appended_asks_t& appended_asks)
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSInterval::add_constraint ***");

  REDUCEStringSender rss(*cl_);


//////////////////// ���M����

  // expr_��n���icollected_tells�Aappended_asks�Aconstraint_store�Aparameter_cons�Ainit_cons��5���琬��j
  cl_->send_string("expr_:=append(append(append(append(");

  // tell����̏W������tells�𓾂�REDUCE�ɓn��
  cl_->send_string("{");
  tells_t::const_iterator tells_it  = collected_tells.begin();
  tells_t::const_iterator tells_end = collected_tells.end();
  for(; (tells_it) != tells_end; ++tells_it) {
    rss.put_node((*tells_it)->get_child(), true);
  }
  cl_->send_string("},");  

  // appended_asks����K�[�h�����𓾂�REDUCE�ɓn��
  cl_->send_string("{");
  appended_asks_t::const_iterator append_it  = appended_asks.begin();
  appended_asks_t::const_iterator append_end = appended_asks.end();
  for(; append_it!=append_end; ++append_it) {
    HYDLA_LOGGER_VCS("put node (guard): ", *(append_it->ask->get_guard()),
                     "  entailed:", append_it->entailed);
    rss.put_node(append_it->ask->get_guard(), true, append_it->entailed);
  }
  cl_->send_string("}),");

  // ����X�g�Aconstraints��REDUCE�ɓn��
  send_cs(rss);
  cl_->send_string("),");

  // �p�����^����parameter_cons��REDUCE�ɓn��
  send_parameter_cons();
  cl_->send_string("),");

  // max_diff_map�����ƂɁA�����l����init_cons��n��
  max_diff_map_t max_diff_map;
  // �ϐ��̍ő�����񐔂����Ƃ߂�
  create_max_diff_map(rss, max_diff_map);
  // �����l����̑��M
  send_init_cons(rss, max_diff_map, false);
  cl_->send_string(");");


  // vars��n��
  cl_->send_string("vars_:=");
  rss.put_vars(true);


  cl_->send_string("symbolic redeval '(isconsistentinterval expr_ vars_);");


////////// ��M����
  HYDLA_LOGGER_EXTERN("--- receive  ---");

  cl_->read_until_redeval();

  std::string ans = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("add_constraint_ans: ",
                   ans);
  
  VCSResult result;






  assert(0);

  return result;
}
  
VCSResult REDUCEVCSInterval::check_entailment(const ask_node_sptr& negative_ask, const appended_asks_t& appended_asks)
{
  HYDLA_LOGGER_VCS_SUMMARY(
    "#*** REDUCEVCSInterval::check_entailment ***\n", 
    "ask: ", *negative_ask);

  REDUCEStringSender rss(*cl_);


//////////////////// ���M����

  // guard_����
  // ask����̃K�[�h�̎��𓾂�REDUCE�ɓn��
  cl_->send_string("guard_:=");
  rss.put_node(negative_ask->get_guard(), true);

  
  // store_��n���iconstraint_store�Aappended_asks�Ainit_cons�Aparameter_store��4���琬��j
  cl_->send_string("store_:=append(append(append(");

  // ����X�g�Aconstraints��REDUCE�ɓn��
  send_cs(rss);
  cl_->send_string(",");

  // appended_asks����K�[�h�����𓾂�Mathematica�ɓn��
  cl_->send_string("{");
  appended_asks_t::const_iterator append_it  = appended_asks.begin();
  appended_asks_t::const_iterator append_end = appended_asks.end();
  for(; append_it!=append_end; ++append_it) {
    HYDLA_LOGGER_VCS("put node (guard): ", *(append_it->ask->get_guard()),
                     "  entailed:", append_it->entailed);
    rss.put_node(append_it->ask->get_guard(), true, append_it->entailed);
  }
  cl_->send_string("}),");

  // max_diff_map�����ƂɁA�����l����init_cons��n��
  max_diff_map_t max_diff_map;
  // �ϐ��̍ő�����񐔂����Ƃ߂�
  create_max_diff_map(rss, max_diff_map);
  // �����l����̑��M
  send_init_cons(rss, max_diff_map, false);
  cl_->send_string("),");

  // parameter_store��n��
  send_parameter_cons();
  cl_->send_string(");");


  // �ϐ��̃��X�gvars_��n��
  cl_->send_string("vars_:=");
  send_vars(rss, max_diff_map);
  cl_->send_string(";");


  // �L���萔�̃��X�gpars_��n��
  cl_->send_string("pars_:=");
  send_pars();
  cl_->send_string(";");


  cl_->send_string("symbolic redeval '(checkentailmentinterval guard_ store_ vars_ pars_);");


/////////////////// ��M����
  HYDLA_LOGGER_VCS( "--- receive ---");

  cl_->read_until_redeval();

  std::string ans = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("check_entailment_ans: ",
                   ans);

  VCSResult result;
    

  


  assert(0);

  return result;
}

void REDUCEVCSInterval::send_ask_guards(
  REDUCEStringSender& rss, 
  const hydla::simulator::ask_set_t& asks) const
{
  // {�K�[�h�̎��Aask��ID}�̃��X�g�`���ő��M����

  assert(0);

}

VCSResult REDUCEVCSInterval::integrate(
  integrate_result_t& integrate_result,
  const positive_asks_t& positive_asks,
  const negative_asks_t& negative_asks,
  const time_t& current_time,
  const time_t& max_time,
  const not_adopted_tells_list_t& not_adopted_tells_list,
  const appended_asks_t& appended_asks)
{
  HYDLA_LOGGER_VCS("#*** REDUCEVCSInterval::integrate ***");

  HYDLA_LOGGER_VCS(constraint_store_);

////////////////// ���M����
  REDUCEStringSender rss(*cl_);
  


////////////////// ��M����

//   PacketChecker pc(*cl_);
//   pc.check();


////////////////// ��M�I��

  assert(0);

  return VCSR_TRUE;
}

void REDUCEVCSInterval::send_time(const time_t& time){
  HYDLA_LOGGER_VCS("SymbolicTime::send_time : ", time);
  REDUCEStringSender rss(*cl_);

  assert(0);

}

void REDUCEVCSInterval::apply_time_to_vm(const variable_map_t& in_vm, 
                                              variable_map_t& out_vm, 
                                              const time_t& time)
{
  HYDLA_LOGGER_VCS("--- apply_time_to_vm ---");

  REDUCEStringSender rss(*cl_);

  assert(0);

}

void REDUCEVCSInterval::add_undefined_vars_to_vm(variable_map_t& vm)
{
  HYDLA_LOGGER_VCS("--- add undefined vars to vm ---");  

  // �ϐ��\�ɓo�^����Ă���ϐ����ꗗ
  HYDLA_LOGGER_VCS("-- variable_name_list --");
  std::set<REDUCEVariable> variable_name_list;
  variable_map_t::const_iterator vm_it = vm.begin();
  variable_map_t::const_iterator vm_end = vm.end();
  for(; vm_it!=vm_end; ++vm_it){
    variable_name_list.insert(vm_it->first);
    HYDLA_LOGGER_VCS(vm_it->first);
  }

  constraint_store_t::init_vars_t::const_iterator init_vars_it;
  constraint_store_t::init_vars_t::const_iterator init_vars_end;
  init_vars_it  = constraint_store_.init_vars.begin();
  init_vars_end = constraint_store_.init_vars.end();
  HYDLA_LOGGER_VCS("-- search undefined variable --");
  // �����l����ϐ��̂����A�ϐ��\�ɓo�^����Ă���ϐ����ꗗ���ɂȂ����̂𒊏o�H
  for(; init_vars_it!=init_vars_end; ++init_vars_it) {
    variable_t variable = init_vars_it->first;
    std::set<REDUCEVariable>::const_iterator vlist_it = variable_name_list.find(variable);
    if(vlist_it==variable_name_list.end()){      
      value_t value;
      HYDLA_LOGGER_VCS("variable : ", variable);
      HYDLA_LOGGER_VCS("value : ", value);
      vm.set_variable(variable, value);
    }
  }
}

void REDUCEVCSInterval::send_not_adopted_tells(REDUCEStringSender& rss, const not_adopted_tells_list_t& na_tells_list) const
{
  // {tell�̎��Atell��ID}�̃��X�g�̃��X�g�`���ő��M����

  assert(0);

}

void REDUCEVCSInterval::send_cs(REDUCEStringSender& rss) const
{
  HYDLA_LOGGER_VCS(
    "---- Send Constraint Store -----\n",
    "cons size: ", constraint_store_.constraints.size());

  assert(0);

}

void REDUCEVCSInterval::send_cs_vars() const
{
  HYDLA_LOGGER_VCS("---- Send Constraint Store Vars -----");
}

std::ostream& REDUCEVCSInterval::dump(std::ostream& s) const
{
  s << "#*** Dump REDUCEVCSInterval ***\n"
    << "--- constraint store ---\n";
/*
   std::set<MathVariable>::const_iterator vars_it = 
     constraint_store_.second.begin();
   std::set<std::set<MathValue> >::const_iterator or_cons_it = 
     constraint_store_.first.begin();
   while((or_cons_it) != constraint_store_.first.end())
   {
     std::set<MathValue>::const_iterator and_cons_it = 
       (*or_cons_it).begin();
     while((and_cons_it) != (*or_cons_it).end())
     {
       s << (*and_cons_it).str << " ";
       and_cons_it++;
     }
     s << "\n";
     or_cons_it++;
   }

   while((vars_it) != constraint_store_.second.end())
   {
     s << *(vars_it) << " ";
     vars_it++;
   }*/

  return s;
}

std::ostream& operator<<(std::ostream& s, 
                         const REDUCEVCSInterval::constraint_store_t& c)
{
  s << "---- REDUCEVCSInterval::consraint_store_t ----\n"
    << "-- init vars --\n";

  BOOST_FOREACH(
    const REDUCEVCSInterval::constraint_store_t::init_vars_t::value_type& i, 
    c.init_vars)
  {
    s << "variable: " << i.first
      << "   value: " << i.second
      << "\n";
  }

  s << "-- constraints --\n";
  BOOST_FOREACH(
    const REDUCEVCSInterval::constraint_store_t::constraints_t::value_type& i, 
    c.constraints)
  {
    s << *i;
  }

  s << "-- init vars max diff map --\n";
  BOOST_FOREACH(
    const REDUCEVCSInterval::constraint_store_t::init_vars_max_diff_map_t::value_type& i, 
    c.init_vars_max_diff_map)
  {
    s << "name: " << i.first
      << "  diff: " << i.second
      << "\n";
  }
  
  return s;
}


} // namespace reduce
} // namespace simulator
} // namespace hydla 

