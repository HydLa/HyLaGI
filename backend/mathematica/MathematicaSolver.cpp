/*
#include "MathLink.h"

#include <cassert>

#include "MathematicaExpressionConverter.h"
#include "PacketErrorHandler.h"
#include "Dumpers.h"

using namespace hydla::solver;
using namespace hydla::simulator;

namespace hydla {
namespace solver {
namespace mathematica {

*/


/*

void MathLink::linear_approx(const value_t& val, value_t& approxed_val, value_range_t& itv, int precision)
{
  PacketSender ps(ml_);
  VariableArg arg = VA_None;
  
  assert(!val->undefined());

  ml_.put_function("linearApprox", 2);
  ps.put_value(val, arg);
  ml_.put_integer(precision);

  ml_.receive();
  PacketErrorHandler::handle(&ml_);
  int length = ml_.get_arg_count();
  assert(length == 3 || length == 1);
  ml_.get_next();
  ml_.get_next();
  
  value_t tmp_value;
  approxed_val = MathematicaExpressionConverter::receive_and_make_symbolic_value(ml_);
  if(length == 3)
  {
    tmp_value = MathematicaExpressionConverter::receive_and_make_symbolic_value(ml_);
    itv.set_lower_bound(tmp_value, true);
    tmp_value = MathematicaExpressionConverter::receive_and_make_symbolic_value(ml_);
    itv.set_upper_bound(tmp_value, true);
  }
  ml_.MLNewPacket();
  return;
}


bool MathLink::approx_val(const value_t& val, value_range_t& range, bool force_approx)
{
  PacketSender ps(ml_);
  VariableArg arg = VA_None;
  if(val->undefined())
  {
    return false;
  }


  if(!force_approx)
  {
    ml_.put_function("approxValue", 1);
    ml_.put_function("List", 1);
    ps.put_value(val, arg);
  }
  else
  {
    ml_.put_function("approxValue", 2);
    ml_.put_function("List", 1);
    ps.put_value(val, arg);
    ml_.put_symbol("interval");
  }

  ml_.receive();
  PacketErrorHandler::handle(&ml_);
  int length = ml_.get_arg_count();
  ml_.get_next();
  ml_.get_next();
  int approxed = ml_.get_integer();
  if(approxed)
  {
    if(length == 2)
    {
      value_t tmp_value = MathematicaExpressionConverter::receive_and_make_symbolic_value(ml_);
      range.set_unique(tmp_value);
    }
    else if(length == 3)
    {
      value_t tmp_value = MathematicaExpressionConverter::receive_and_make_symbolic_value(ml_);
      range.set_lower_bound(tmp_value, true);
      tmp_value = MathematicaExpressionConverter::receive_and_make_symbolic_value(ml_);
      range.set_upper_bound(tmp_value, true);
    }
    else assert(0);
    ml_.MLNewPacket();
    return true;
  }
  else
  {
    ml_.MLNewPacket();
    return false;
  }
}
  
void MathLink::approx_vm(variable_map_t& vm)
{
  PacketSender ps(ml_);
  variable_map_t::iterator it = vm.begin();
  VariableArg arg = VA_None;
  for(; it!=vm.end(); ++it)
  {
      if(!it->second.undefined() && it->second.get_lower_cnt() > 0 && it -> second.get_upper_cnt() > 0)
    {
      ml_.put_function("approxValue", 1);
      if(it->second.unique())
      {
        ml_.put_function("List", 1);
        ps.put_value(it->second.get_unique(), arg);
      }
      else
      {
        ml_.put_function("List", 2);
        ps.put_value(it->second.get_lower_bound().value, arg);
        ps.put_value(it->second.get_upper_bound().value, arg);
      }
      ml_.receive();
      PacketErrorHandler::handle(&ml_);
      int length = ml_.get_arg_count();
      ml_.get_next();
      ml_.get_next();
      int approxed = ml_.get_integer();
      
      if(approxed)
      {
        if(length == 2)
        {
          value_t tmp_value = MathematicaExpressionConverter::receive_and_make_symbolic_value(ml_);
          it->second.set_unique(tmp_value);
        }
        else if(length == 3)
        {
          value_t tmp_value = MathematicaExpressionConverter::receive_and_make_symbolic_value(ml_);
          it->second.set_lower_bound(tmp_value, true);
          tmp_value = MathematicaExpressionConverter::receive_and_make_symbolic_value(ml_);
          it->second.set_upper_bound(tmp_value, true);
        }
        else assert(0);
        
        // break here to avoid too much approximation
        break;
        
      }
      
      ml_.MLNewPacket();
    }
  }
}


void MathLink::reset_constraint(const variable_map_t& vm, const bool& send_derivatives)
{
  PacketSender ps(ml_);
  
  ml_.put_function("resetConstraintForVariable", 0);
  ml_.receive();
  ml_.MLNewPacket();
  
  ml_.put_function("addConstraint", 2);
  HYDLA_LOGGER_VCS("------Variable map------\n", vm);
  variable_map_t::const_iterator it = 
    vm.begin();
  int size=0;
   VariableArg arg = (mode_==hydla::simulator::symbolic::DiscreteMode || mode_== hydla::simulator::symbolic::ConditionsMode)?VA_None:VA_Time;
  for(; it!=vm.end(); ++it)
  {
    if((send_derivatives || it->first->get_derivative_count() == 0) && !it->second.undefined()){
      size++;
    }
  }
  ml_.put_function("And", size);
  it = vm.begin();
  for(; it!=vm.end(); ++it)
  {
    if((send_derivatives || it->first->get_derivative_count() == 0) && !it->second.undefined()){
      ml_.put_function("Equal", 2);
      ps.put_var(it->first->get_name(), it->first->get_derivative_count(), arg);
      ps.put_value(it->second.get_unique(), arg);
    }
  }
  
  it = vm.begin();
  ml_.put_function("List", vm.size());
  for(; it!=vm.end(); ++it)
  {
    ps.put_var(it->first->get_name(), it->first->get_derivative_count(), arg);
  }
  
  ml_.receive(); 
  ml_.MLNewPacket();
v}

bool MathLink::reset_parameters(const parameter_map_t& pm)
{
  PacketSender ps(ml_);
  
  ml_.put_function("resetConstraintForParameter", 2);
  send_parameter_map(pm, ps);
  
  ml_.receive(); 
  ml_.MLNewPacket();

  return true;
}

// TODO: add_guardなのかset_guardなのかとか，仕様とかをはっきりさせる
void MathLink::add_guard(const node_sptr& guard)
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  
  VariableArg arg = (mode_==hydla::simulator::symbolic::DiscreteMode || mode_==hydla::simulator::symbolic::ConditionsMode)?VA_None:VA_Time;

  switch(mode_){
    case hydla::simulator::symbolic::DiscreteMode:
      ml_.put_function("addConstraint", 2);
      break;
    case hydla::simulator::symbolic::ConditionsMode:
      ml_.put_function("addGuard",2);
      break;
    default:
      ml_.put_function("setGuard", 2);
      break;
  }

  PacketSender ps(ml_);

  ps.put_node(guard, arg);
  // varsを渡す
  ps.put_vars();
  
  ///////////////// 受信処理
  HYDLA_LOGGER_VCS("%% receive");
  ml_.receive();
  ml_.MLNewPacket();

  HYDLA_LOGGER_FUNC_END(VCS);
  return;
}

  
/////////////////// 受信処理
  HYDLA_LOGGER_VCS( "%%receive");
  ml_.receive();
  PacketErrorHandler::handle(&ml_);
  
  //まずListが来るはず
  ml_.get_next();
  
  int token = ml_.get_next();
  
  CheckConsistencyResult ret;
  
  if(token == MLTKSYM){
    //TrueかFalseのはず
    std::string symbol = ml_.get_symbol();
    if(symbol == "True"){
      ret.true_parameter_maps.push_back(parameter_map_t());
    }else{
      ret.false_parameter_maps.push_back(parameter_map_t());
    }
  }else if(token == MLTKINT){
    HYDLA_LOGGER_VCS("illegal integer:", ml_.get_integer());
  }else{
    //更に二重リストが来るはず
    int map_size = ml_.get_arg_count();
    ml_.get_next();
    for(int i=0; i < map_size; i++){
      ml_.get_next();
      parameter_map_t tmp_map;
      receive_parameter_map(tmp_map);
      ret.true_parameter_maps.push_back(tmp_map);
    }
    
    token = ml_.get_next();
    if(token == MLTKSYM){
      //TrueかFalseのはず
      std::string symbol = ml_.get_symbol();
      if(symbol != "False"){
        assert(0);
      }
    }else{
      map_size = ml_.get_arg_count();
      ml_.get_next();    
      for(int i=0; i < map_size; i++){
        ml_.get_next();
        parameter_map_t tmp_map;
        receive_parameter_map(tmp_map);
        ret.false_parameter_maps.push_back(tmp_map);
      }
    }
  }
  //終わりなのでパケットの最後尾までスキップ
  ml_.MLNewPacket();
  
  HYDLA_LOGGER_FUNC_END(VCS);
  return ret;
}

MathLink::PP_time_result_t MathLink::calculate_next_PP_time(
  const constraints_t& discrete_cause,
  const time_t& current_time,
  const time_t& max_time)
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);

////////////////// 送信処理
  PacketSender ps(ml_);
  
  // calculateNextPointPhaseTime[maxTime, discCause]を渡したい
  ml_.put_function("calculateNextPointPhaseTime", 2);


  // maxTimeを渡す
  time_t tmp_time(max_time->clone());
  *tmp_time -= *current_time;
  HYDLA_LOGGER_VCS("%% current time:", *current_time);
  HYDLA_LOGGER_VCS("%% send time:", *tmp_time);
  ps.put_value(tmp_time, VA_Time);

  //離散変化の条件を渡す
  ml_.put_function("List", discrete_cause.size());
  for(constraints_t::const_iterator it = discrete_cause.begin(); it != discrete_cause.end();it++){
    ps.put_node(*it, VA_Time);
  }

////////////////// 受信処理

  ml_.receive();
  PacketErrorHandler::handle(&ml_);
  
  // 次のPPの時刻と，その場合の条件の組，更に終了時刻かどうかを得る
  HYDLA_LOGGER_VCS("%% receive next PP time");
  int next_time_size = ml_.get_arg_count();
  HYDLA_LOGGER_VCS("next_time_size: ", next_time_size);
  ml_.get_next();
  PP_time_result_t result;
  for(int time_it = 0; time_it < next_time_size; time_it++){
    PP_time_result_t::candidate_t candidate;
    ml_.get_next();
    ml_.get_next();ml_.get_next();
    // 時刻を受け取る
    candidate.time = MathematicaExpressionConverter::receive_and_make_symbolic_value(ml_);
    *candidate.time += *current_time;
    HYDLA_LOGGER_VCS("next_phase_time: ", candidate.time);
    ml_.get_next();
    // 条件を受け取る
    receive_parameter_map(candidate.parameter_map);
    ml_.get_next();
    
    // 終了時刻かどうかを受け取る
    candidate.is_max_time = (bool)(ml_.get_integer() != 0);
    HYDLA_LOGGER_VCS("is_max_time: ",  candidate.is_max_time);
    HYDLA_LOGGER_VCS("--- parameter map ---\n",  candidate.parameter_map);
    result.candidates.push_back(candidate);
  }

  ml_.MLNewPacket();
  HYDLA_LOGGER_FUNC_END(VCS);
  return result;
}


void MathLink::apply_time_to_vm(const variable_map_t& in_vm, 
                                              variable_map_t& out_vm, 
                                              const time_t& time)
{
  HYDLA_LOGGER_VCS("#*** Begin MathLink::apply_time_to_vm ***");

  PacketSender ps(ml_);

  variable_map_t::const_iterator it  = in_vm.begin();
  variable_map_t::const_iterator end = in_vm.end();
  for(; it!=end; ++it) {
    HYDLA_LOGGER_VCS("variable : ", *(it->first));
    // 値
    value_t    value;
    if(!it->second.undefined()) {
      ml_.put_function("applyTime2Expr", 2);
      ps.put_value(it->second.get_unique(), VA_Time);
      ps.put_value(time, VA_None);

    ////////////////// 受信処理

      ml_.receive();
      PacketErrorHandler::handle(&ml_);
      
      value = value_t(MathematicaExpressionConverter::receive_and_make_symbolic_value(ml_));
      HYDLA_LOGGER_REST("value : ", value->get_string());
    }
    out_vm[it->first] = value;
  }
  ml_.MLNewPacket();
  HYDLA_LOGGER_FUNC_END(VCS);
}


//丸めを行う関数 get_real_val()で使う
std::string upward(std::string str){
  if(str.at(str.length()-1) == '.'){
    str = upward(str.substr(0,str.length()-1)) + '.';
  }else if(str.at(str.length()-1) != '9'){
    str = str.substr(0,str.length()-1)+(char)(str.at(str.length()-1)+1);
  }else if(str == "9"){
    str = "10";   
  }else {
    str = upward(str.substr(0,str.length()-1)) + '0';
  }
  return str;
}
*/

/*
//value_tを指定された精度で数値に変換する
std::string MathLink::get_real_val(const value_t &val, int precision, simulator::OutputFormat opfmt){
  std::string ret;
  PacketSender ps(ml_);

  if(!val->undefined() && opfmt == fmtNInterval) {
    
    //precisionは2より大きいとする
    if(precision<2) precision=2;

    ml_.put_function("ToString", 2);
    ml_.put_function("Interval", 1);    
    ml_.put_function("N", 2);  
    ps.put_value(val, VA_None);
    ml_.put_integer(precision+5);     
    ml_.put_symbol("InputForm");
    ml_.skip_pkt_until(RETURNPKT);
    ret = ml_.get_string();

    //usrVarの場合はそのまま
    if(ret.find(var_prefix) != -1) 
      return ret.substr(ret.find("[")+1,ret.find("]")-ret.find("[")-1);

    //parameterがある場合の対応 ex:Interval[1.6666666666666667`5.*pa]   Interval[pa]（不完全）
    //TODO:Interval[-5.`5.*(-6.`5. + pa)]などへの対応 ./hydla examples/sawtooth_wave_param.hydla -m s -t 10
    std::string parameter = "";
    if(ret.find("{") == -1){
      parameter = ret.substr(ret.find("[")+1,ret.find("]")-ret.find("[")-1);
      if(parameter.find("`") == -1) {
        return parameter;
      }else{
        //処理できる形式に変形
        ret = "Interval[{"+parameter.substr(0,precision+5)+", "+parameter.substr(0,precision+5)+"}]";
        parameter = parameter.substr(parameter.find_last_of(".")+1);
      }
    }

    int loc;
    int pre = precision;
    char sign = 'p';
    std::string lower = "";
    std::string upper = "";
    //指定された精度に合わせる
    if(ret.find("-")!=-1) {
      ret.erase(ret.find("-"),1);
      if(ret.find("-")!=-1) {
        ret.erase(ret.find("-"),1);
        //負の区間 上端と下端のどちらを丸めるか
        sign = 'n';
      }else{
        //[負,正]の区間
        sign = 'c';
      }
    }
    //下端を取り出す
    if(ret.find("}")-ret.find("{")-2>precision*2){
      for(int i=ret.find("{")+1;i<ret.find(",")-1;i++){
        if(ret.at(i)!='0' &&  ret.at(i)!='.') {
          if(ret.substr(ret.find("{")+1,pre).find(".")!=-1) pre++;
          lower = ret.substr(ret.find("{")+1,pre);
          break;
        }else if(ret.at(i) == '0') pre++;
      }
      pre = precision;
      //上端を取り出す
      for(int i=ret.find(",")+2;i<ret.length();i++){
        if(ret.at(i)!='0' &&  ret.at(i)!='.') {
          if(ret.substr(ret.find(",")+2,pre).find(".")!=-1) pre++;
          upper = ret.substr(ret.find(",")+2,pre);
          break;
        }else if(ret.at(i) == '0') pre++;
      }

      //precision＋１桁目を下端は切り捨て、上端は切り上げ
      switch (sign) {
      case 'n' : lower = upward(lower); break;
      case 'p' : upper = upward(upper); break;
      case 'c' : lower = "-" + upward(lower); upper = upward(upper); break;
      }
    
    }else{
      //ex:[0,0]などの場合に対応させる
      lower = ret.substr(ret.find("{")+1,ret.find(",")-(ret.find("{")+1));
      upper = ret.substr(ret.find(",")+2,ret.find("}")-(ret.find(",")+2));
    }
    //化学表記法(ex:*10^6)の対応
    std::string lowex = "";
    std::string upex = "";
    if((loc=ret.find("*^")) != -1){
      lowex = ret.substr(loc,ret.find(",")-loc);
      loc=ret.find_last_of("*^");
      upex = ret.substr(loc-1,lowex.length());
    }
    loc=0;
    
    //一致している部分を省略して表示する
    //TODO:"[0.9999,1.000]"を"0.999[9,10]"のように表示させる
    while(true){
      if(lower.at(loc)!=upper.at(loc)) {
        ret = lower.substr(0,loc);
        lower = lower.substr(loc);
        upper = upper.substr(loc);

        if(lowex.compare(upex)==0){
          ret = ret + "[" + lower + "," + upper + "]" + lowex;
          break;
        }else ret = ret + "[" + lower + lowex + "," + upper + upex + "]";
        break;
      }
      loc++;
      //もしも全て一致している場合はどちらか表示(ありえないはず)
      if(loc > lower.length()-1){
        ret = lower;
        break;
      }else if(loc > upper.length()-1) {
        ret = upper;
        break;
      }
    }

    if(sign == 'n') ret = "-"+ret;
    ret = ret+parameter;
    
  } else  if(!val->undefined()) {
    ml_.put_function("ToString", 2);

    //ml_.put_function("Interval",1);

    ml_.put_function("N", 2);  
    ps.put_value(val, VA_None);
    ml_.put_integer(precision);
    ml_.put_symbol("CForm");
    //ml_.put_symbol("InputForm");
    ml_.receive();
    ret = ml_.get_string();
  }  else {
    ret = "UNDEF";
  }

  return ret;
}
*/


/**
 * HAConverter用:
 */
/*
// v1 in v2 => true
bool MathLink::check_include_bound(value_t v1, value_t v2, parameter_map_t pm1, parameter_map_t pm2)
{
  // parameter_mapを送信
  {
    PacketSender ps(ml_);
    ml_.put_function("addParameterConstraintNow", 2);
    send_parameter_map(pm1, ps);
    ml_.receive();  
    ml_.MLNewPacket();
  }
  {
    PacketSender ps(ml_);
    ml_.put_function("addParameterConstraintPast", 2);
    send_parameter_map(pm2, ps);
    ml_.receive();  
    ml_.MLNewPacket();
  }
   ml_.put_function("checkIncludeBound", 2);
    
  PacketSender ps(ml_);
    
  ps.put_value(v1, VA_None);
  ps.put_value(v2, VA_None);
  ml_.receive();
  
  PacketErrorHandler::handle(&ml_);
  value_t tmp_val = value_t(MathematicaExpressionConverter::receive_and_make_symbolic_value(ml_));
    
  ml_.MLNewPacket();

  HYDLA_LOGGER_HA("value: ", *tmp_val);
    
  if (tmp_val->get_string() == "1") {
    return true;
  }else{
    return false;
  }
}

*/
 /*
} // namespace mathematica
} // namespace solver
} // namespace hydla 
 */
