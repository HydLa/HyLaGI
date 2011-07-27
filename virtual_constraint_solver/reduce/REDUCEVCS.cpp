#include "REDUCEVCS.h"

#include <cassert>

#include "REDUCEVCSPoint.h"
#include "REDUCEVCSInterval.h"
#include "REDUCEStringSender.h"
#include "SExpConverter.h"

using namespace hydla::vcs;

namespace hydla {
namespace vcs {
namespace reduce {

void REDUCEVCS::change_mode(hydla::symbolic_simulator::Mode m, int approx_precision)
{
  mode_ = m;
  switch(m) {
    case hydla::symbolic_simulator::DiscreteMode:
      vcs_.reset(new REDUCEVCSPoint(&cl_));
      break;

    case hydla::symbolic_simulator::ContinuousMode:
      vcs_.reset(new REDUCEVCSInterval(&cl_, approx_precision));
      break;

    default:
      assert(0);//assert発見
  }
}



REDUCEVCS::REDUCEVCS(const hydla::symbolic_simulator::Opts &opts, variable_map_t &vm)
{
 

  HYDLA_LOGGER_VCS("#*** send depend statements of variables ***");

  std::ostringstream depend_str;
  depend_str << "depend {";
  variable_map_t::variable_list_t::const_iterator it =
    vm.begin();
  variable_map_t::variable_list_t::const_iterator end =
    vm.end();
  bool first_element = true;
  for(; it!=end; ++it)
  {
    const REDUCEVariable& variable = it->first;
    // 微分回数が0のものだけdepend文を作成
    if(variable.derivative_count == 0){
      if(!first_element) depend_str << ",";
      depend_str << variable.name;
      first_element = false;
    }
  }
  depend_str << "},t;";

  HYDLA_LOGGER_VCS("depend_str: ", depend_str.str()); 
  cl_.send_string((depend_str.str()).c_str());


  // REDUCEの関数定義を送信
  cl_.send_string(vcs_reduce_source());
//  cl_.read_until_redeval();
  cl_.skip_until_redeval();

  SExpConverter::initialize();
}

REDUCEVCS::~REDUCEVCS()
{}

bool REDUCEVCS::reset()
{
  return vcs_->reset();
}

bool REDUCEVCS::reset(const variable_map_t& vm)
{
  return vcs_->reset(vm);
}

bool REDUCEVCS::reset(const variable_map_t& vm, const parameter_map_t& pm)
{
  return vcs_->reset(vm, pm);
}

bool REDUCEVCS::create_maps(create_result_t &create_result)
{
  return vcs_->create_maps(create_result);
}

void REDUCEVCS::add_constraint(const constraints_t& constraints)
{
  vcs_->add_constraint(constraints);
}
  
VCSResult REDUCEVCS::check_entailment(const ask_node_sptr& negative_ask, const appended_asks_t& appended_asks)
{
  return vcs_->check_entailment(negative_ask, appended_asks);
}

VCSResult REDUCEVCS::check_consistency()
{
  return vcs_->check_consistency();
}

VCSResult REDUCEVCS::check_consistency(const constraints_t& constraints)
{
  return vcs_->check_consistency(constraints);
}

VCSResult REDUCEVCS::integrate(
  integrate_result_t& integrate_result,
  const constraints_t &constraints,
  const time_t& current_time,
  const time_t& max_time)
{
  return vcs_->integrate(integrate_result, 
                         constraints, 
                         current_time, 
                         max_time);
}

void REDUCEVCS::apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time){
  vcs_->apply_time_to_vm(in_vm, out_vm, time);
}


  //value_tを指定された精度で数値に変換する
std::string REDUCEVCS::get_real_val(const value_t &val, int precision){
  std::string ret;
  REDUCEStringSender rss(cl_);

  if(!val.is_undefined()) {

    // getRealVal(value_, prec_)を渡したい

    cl_.send_string("value_:=");
    rss.put_node(val.get_node(), true);
    cl_.send_string(";");


    cl_.send_string("prec_:=");
    std::stringstream precision_str;
    precision_str << precision;
    cl_.send_string(precision_str.str());
    cl_.send_string(";");


    cl_.send_string("symbolic redeval '(getRealVal value_ prec_);");


//    cl_.read_until_redeval();
    cl_.skip_until_redeval();
    ret = cl_.get_s_expr();
  }
  else {
    ret = "UNDEF";
  }

  return ret;
}


bool REDUCEVCS::less_than(const time_t &lhs, const time_t &rhs)
{

  REDUCEStringSender rss(cl_);

  // checkLessThan(lhs_, rhs_)を渡したい

  cl_.send_string("lhs_:=");
  rss.put_node(lhs.get_node(), true);
  cl_.send_string(";");


  cl_.send_string("rhs_:=");
  rss.put_node(rhs.get_node(), true);
  cl_.send_string(";");


  cl_.send_string("symbolic redeval '(checkLessThan lhs_ rhs_);");


  ////////////////// 受信処理

  cl_.read_until_redeval();

  std::string ans = cl_.get_s_expr();
  HYDLA_LOGGER_VCS("check_less_than_ans: ", ans);
  return  ans == "\"RETTRUE___\"";
}


void REDUCEVCS::simplify(time_t &time) 
{
  HYDLA_LOGGER_DEBUG("SymbolicTime::send_time : ", time);
  REDUCEStringSender rss(cl_);

  // simplifyExpr(expr_)を渡したい

  cl_.send_string("expr_:=");
  rss.put_node(time.get_node(), true);
  cl_.send_string(";");


  cl_.send_string("symbolic redeval '(simplifyExpr expr_);");


  ////////////////// 受信処理

  cl_.read_until_redeval();

  std::string ans = cl_.get_s_expr();
  HYDLA_LOGGER_VCS("expr_time_shift_ans: ", ans);

  // S式パーサで読み取る
  SExpParser sp;
  sp.parse_main(ans.c_str());
  SExpParser::const_tree_iter_t time_it = sp.get_tree_iterator();
  SExpConverter sc;
  time = sc.convert_s_exp_to_symbolic_value(sp, time_it);
}



  //SymbolicValueの時間をずらす
hydla::vcs::SymbolicVirtualConstraintSolver::value_t REDUCEVCS::shift_expr_time(const value_t& val, const time_t& time){
  value_t tmp_val;
  REDUCEStringSender rss(cl_);

  // exprTimeShift(expr_, time_)を渡したい

  cl_.send_string("expr_:=");
  rss.put_node(val.get_node(), true);
  cl_.send_string(";");


  cl_.send_string("time_:=");
  rss.put_node(time.get_node(), true);
  cl_.send_string(";");


  cl_.send_string("symbolic redeval '(exprTimeShift expr_ time_);");


  ////////////////// 受信処理

  cl_.read_until_redeval();

  std::string ans = cl_.get_s_expr();
  HYDLA_LOGGER_VCS("expr_time_shift_ans: ", ans);

  // S式パーサで読み取る
  SExpParser sp;
  sp.parse_main(ans.c_str());
  SExpParser::const_tree_iter_t value_it = sp.get_tree_iterator();
  SExpConverter sc;
  tmp_val = sc.convert_s_exp_to_symbolic_value(sp, value_it);
  return  tmp_val;
}


} // namespace reduce
} // namespace vcs
} // namespace hydla 
