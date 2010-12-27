#include "MathematicaVCS.h"

#include <cassert>

#include "MathematicaVCSPoint.h"
#include "MathematicaVCSInterval.h"

using namespace hydla::vcs;

namespace hydla {
namespace vcs {
namespace mathematica {

void MathematicaVCS::change_mode(hydla::symbolic_simulator::Mode m, int approx_precision)
{
  mode_ = m;
  switch(m) {
    case hydla::symbolic_simulator::DiscreteMode:
      vcs_.reset(new MathematicaVCSPoint(&ml_));
      break;

    case hydla::symbolic_simulator::ContinuousMode:
      vcs_.reset(new MathematicaVCSInterval(&ml_, approx_precision));
      break;

    default:
      assert(0);//assert発見
  }
}



MathematicaVCS::MathematicaVCS(const hydla::symbolic_simulator::Opts &opts)
{
  
  HYDLA_LOGGER_DEBUG("#*** init mathlink ***");
  //std::cout << opts.mathlink << std::endl;

  //TODO: 例外を投げるようにする
  if(!ml_.init(opts.mathlink.c_str())) {
    std::cerr << "can not link" << std::endl;
    exit(-1);
  }

  // 出力する画面の横幅の設定
  ml_.MLPutFunction("SetOptions", 2);
  ml_.MLPutSymbol("$Output"); 
  ml_.MLPutFunction("Rule", 2);
  ml_.MLPutSymbol("PageWidth"); 
  ml_.MLPutSymbol("Infinity"); 
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // デバッグプリント
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optUseDebugPrint"); 
  ml_.MLPutSymbol(opts.debug_mode ? "True" : "False");
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // プロファイルモード
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optUseProfile"); 
  ml_.MLPutSymbol(opts.profile_mode ? "True" : "False");
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // 並列モード
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optParallel"); 
  ml_.MLPutSymbol(opts.parallel_mode ? "True" : "False");
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // 出力形式
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optOutputFormat"); 
  switch(opts.output_format) {
    case hydla::symbolic_simulator::fmtTFunction:
      ml_.MLPutSymbol("fmtTFunction");
      break;

    case hydla::symbolic_simulator::fmtNumeric:
    default:
      ml_.MLPutSymbol("fmtNumeric");
      break;
  }
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  ml_.MLPutFunction("ToExpression", 1);
  ml_.MLPutString(vcs_math_source());  
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();
}

MathematicaVCS::~MathematicaVCS()
{}

bool MathematicaVCS::reset()
{
  return vcs_->reset();
}

bool MathematicaVCS::reset(const variable_map_t& vm)
{
  return vcs_->reset(vm);
}

bool MathematicaVCS::create_variable_map(variable_map_t& vm)
{
  return vcs_->create_variable_map(vm);
}

VCSResult MathematicaVCS::add_constraint(const tells_t& collected_tells)
{
  return vcs_->add_constraint(collected_tells);
}
  
VCSResult MathematicaVCS::check_entailment(const ask_node_sptr& negative_ask)
{
  return vcs_->check_entailment(negative_ask);
}

VCSResult MathematicaVCS::integrate(
  integrate_result_t& integrate_result,
  const positive_asks_t& positive_asks,
  const negative_asks_t& negative_asks,
  const time_t& current_time,
  const time_t& max_time,
  const not_adopted_tells_list_t& not_adopted_tells_list)
{
  return vcs_->integrate(integrate_result, 
                         positive_asks, 
                         negative_asks, 
                         current_time, 
                         max_time,
                         not_adopted_tells_list);
}

// void MathematicaVCS::change_mode(Mode m)
// {
//   if(mode_ == m) {
//     vcs_->reset();
//   }
//   else {
//     mode_ = m;
//     switch(m) {
//       case DiscreteMode:
//         vcs_.reset(new MathematicaVCSPoint(ml_));
//         break;

//       case ContinuousMode:
//         vcs_.reset(new MathematicaVCSInterval(ml_));
//         break;

//       default:
//         assert(0);
//     }
//   }
// }


void MathematicaVCS::apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time){
  vcs_->apply_time_to_vm(in_vm, out_vm, time);
}


  //value_tを指定された精度で数値に変換する
std::string MathematicaVCS::get_real_val(const value_t &val, int precision){
  std::string ret;
  
  if(!val.is_undefined()) {
    ml_.put_function("ToString", 2);  
    ml_.put_function("N", 2);  
    ml_.put_function("ToExpression", 1);
    ml_.put_string(val.str);
    ml_.put_integer(precision);
    ml_.put_symbol("CForm");  
    ml_.skip_pkt_until(RETURNPKT);
    ret = ml_.get_string();
  }
  else {
    ret = "UNDEF";
  }
  return ret;
}

  //time_tを指定された精度で数値に変換する
std::string MathematicaVCS::get_real_val(const time_t &time, int precision){
  ml_.put_function("ToString", 2);  
  ml_.put_function("N", 2);  
  ml_.put_function("ToExpression", 1);
  ml_.put_string(time.get_string());
  ml_.put_integer(precision);
  ml_.put_symbol("CForm");
  
  ml_.skip_pkt_until(RETURNPKT);
  return  ml_.get_string();
}


bool MathematicaVCS::less_than(const time_t &lhs, const time_t &rhs)
{
  ml_.put_function("ToString", 1);  
  ml_.put_function("Less", 2);  
  ml_.put_function("ToExpression", 1);
  ml_.put_string(lhs.get_string());
  ml_.put_function("ToExpression", 1);
  ml_.put_string(rhs.get_string());
  
  ml_.skip_pkt_until(RETURNPKT);
  std::string ret = ml_.get_string();
  return  ret == "True";
}

void MathematicaVCS::simplify(time_t &time) 
{
  HYDLA_LOGGER_DEBUG("SymbolicTime::send_time : ", time);
  ml_.put_function("ToString", 1);
  ml_.put_function("FullForm", 1);
  ml_.put_function("Simplify", 1);
  ml_.put_function("ToExpression", 1);
  ml_.put_string(time.get_string());
  ml_.skip_pkt_until(RETURNPKT);
  time.set(ml_.get_string());
}



} // namespace mathematica
} // namespace vcs
} // namespace hydla 
