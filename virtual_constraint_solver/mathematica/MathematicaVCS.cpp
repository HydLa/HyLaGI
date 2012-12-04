#include "MathematicaVCS.h"

#include <cassert>

#include "MathematicaExpressionConverter.h"
#include "PacketSender.h"
#include "PacketErrorHandler.h"
#include "../SolveError.h"

using namespace hydla::vcs;
using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace vcs {
namespace mathematica {

void MathematicaVCS::change_mode(hydla::symbolic_simulator::Mode m, int approx_precision)
{
  mode_ = m;
  switch(m) {
    case hydla::symbolic_simulator::DiscreteMode:
      break;

    case hydla::symbolic_simulator::ContinuousMode:
      break;

    case hydla::symbolic_simulator::TestMode:
      break;

    default:
      assert(0);//assert発見
  }
}

MathematicaVCS::MathematicaVCS(const hydla::simulator::Opts &opts)
{
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCS::MathematicaVCS(Constructor) ***\n");
  //std::cout << opts.mathlink.c_str() << std::endl;

  if(!ml_.init(opts.mathlink.c_str())) {
    throw MathLinkError("can not link",0);
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

  // 最適化レベル
  ml_.MLPutFunction("Set",2);
  ml_.MLPutSymbol("optOptimizationLevel");
  ml_.MLPutInteger(opts.optimization_level);
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();
  
  // タイムアウト時間
  ml_.MLPutFunction("Set",2);
  ml_.MLPutSymbol("timeOutS"); 
  if(opts.timeout > 0){
    ml_.MLPutInteger(opts.timeout);
  }else{
    ml_.MLPutSymbol("Infinity");
  }
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  ml_.MLPutFunction("ToExpression", 1);
  ml_.MLPutString(vcs_math_source());  
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();
  
  MathematicaExpressionConverter::initialize();
  PacketSender::initialize();
  HYDLA_LOGGER_VCS("#*** End MathematicaVCS::MathematicaVCS(Constructor) ***\n");
}

MathematicaVCS::~MathematicaVCS()
{}


void MathematicaVCS::start_temporary(){
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCS::start_temporary ***\n");
  ml_.put_function("startTemporary", 0);
  ml_.receive();
  ml_.MLNewPacket();
  HYDLA_LOGGER_VCS("#*** END MathematicaVCS::start_temporary ***\n");
}

void MathematicaVCS::end_temporary(){
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCS::end_temporary ***\n");
  ml_.put_function("endTemporary", 0);
  ml_.receive();
  ml_.MLNewPacket();
  HYDLA_LOGGER_VCS("#*** END MathematicaVCS::end_temporary ***\n");
}

void MathematicaVCS::set_false_conditions(const node_sptr& constraint)
{
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCS::set_false_conditions ***\n");
  
  PacketSender::VariableArg arg = PacketSender::VA_Prev;

  HYDLA_LOGGER_VCS("%% setFalseConditions");
  ml_.put_function("setFalseConditions", 2);
  PacketSender ps(ml_);

  ps.put_node(constraint, arg);
  // varsを渡す
  ps.put_vars();

  ///////////////// 受信処理
  HYDLA_LOGGER_VCS( "--- receive ---");
  ml_.receive();
  ml_.MLNewPacket();

  HYDLA_LOGGER_VCS("\n#*** End MathematicaVCS::set_false_conditions ***\n");
  return;
}


bool MathematicaVCS::reset(const variable_map_t& variable_map, const parameter_map_t& parameter_map)
{
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCS::reset ***\n");

  is_temporary_ = false;
  
  ml_.MLNewPacket();

  ml_.put_function("resetConstraint", 0);
  
/////////////////// 受信処理
  // PacketChecker pc(ml_);
  // pc.check();
///////////////////
  ml_.receive();
  ml_.MLNewPacket();

  {
    PacketSender ps(ml_);
    ml_.put_function("addVariables", 1);
    ml_.put_function("List", variable_set_->size());
    for(variable_set_t::const_iterator it = variable_set_->begin(); it != variable_set_->end(); it++){
      ps.put_var(it->get_name(), it->get_derivative_count(), (mode_==hydla::symbolic_simulator::DiscreteMode || mode_==hydla::symbolic_simulator::TestMode)?PacketSender::VA_None:PacketSender::VA_Time);
    }
    
    ml_.receive();
    ml_.MLNewPacket();
  }

  {
    PacketSender ps(ml_);
    ml_.put_function("addParameterConstraint", 2);
    HYDLA_LOGGER_VCS("------Parameter map------\n", parameter_map);
    parameter_map_t::const_iterator it = 
      parameter_map.begin();
    int size=0;
    for(; it!=parameter_map.end(); ++it)
    {
      const value_range_t &range = it->second;
      if(range.is_unique()){
        size++;
      }else{
        if(range.get_lower_bound().value.get() && !range.get_lower_bound().value->is_undefined()){
          size++;
        }
        if(range.get_upper_bound().value.get() && !range.get_upper_bound().value->is_undefined()){
          size++;
        }
      }
    }
    
    ml_.put_function("And", size);
    it = parameter_map.begin();
    for(; it!=parameter_map.end(); ++it)
    {
      if(it->second.is_unique()){
        const value_t &value = it->second.get_lower_bound().value;
        parameter_t& param = *it->first;
        ml_.put_function("Equal", 2);
        ps.put_par(param.get_name(), param.get_derivative_count(), param.get_phase_id());
        ps.put_value(value, PacketSender::VA_Prev);
      }else{
        {
          const value_t &value = it->second.get_lower_bound().value;
          parameter_t& param = *it->first;
          if(value.get() && !value->is_undefined()){
            if(!it->second.get_lower_bound().include_bound){
              ml_.put_function("Greater", 2);
            }
            else{
              ml_.put_function("GreaterEqual", 2);
            }
            ps.put_par(param.get_name(), param.get_derivative_count(), param.get_phase_id());
            ps.put_value(value, PacketSender::VA_Prev);
          }
        }
        {
          
          const value_t &value = it->second.get_upper_bound().value;
          parameter_t& param = *it->first;
          if(value.get() && !value->is_undefined()){
            if(!it->second.get_upper_bound().include_bound){
              ml_.put_function("Less", 2);
            }
            else{
              ml_.put_function("LessEqual", 2);
            }
            ps.put_par(param.get_name(), param.get_derivative_count(), param.get_phase_id());
            ps.put_value(value, PacketSender::VA_Prev);
          }
        }
      }
    }
    ps.put_pars();
    ml_.receive();  
    ml_.MLNewPacket();
  }


  {
    PacketSender ps(ml_);
    ml_.put_function("addPrevConstraint", 2);
    HYDLA_LOGGER_VCS("------Variable map------\n", variable_map);
    variable_map_t::const_iterator it = 
      variable_map.begin();
    int size=0;
    for(; it!=variable_map.end(); ++it)
    {
      if(!it->second->is_undefined()){
        size++;
      }
    }
    ml_.put_function("And", size);
    it = variable_map.begin();
    for(; it!=variable_map.end(); ++it)
    {
      if(!it->second->is_undefined()){
        ml_.put_function("Equal", 2);
        ps.put_var(it->first->get_name(), it->first->get_derivative_count(), PacketSender::VA_Prev);
        ps.put_value(it->second, PacketSender::VA_Prev);
      }
    }
    ps.put_vars();
    ml_.receive(); 
    ml_.MLNewPacket(); 
  }

  HYDLA_LOGGER_VCS("#*** END MathematicaVCS::reset ***\n");
  return true;
}


MathematicaVCS::create_result_t MathematicaVCS::create_maps()
{
  
  HYDLA_LOGGER_VCS(
    "#*** Begin MathematicaVCS::create_maps ***\n");
    
/////////////////// 送信処理
  if(mode_==hydla::symbolic_simulator::DiscreteMode || mode_==hydla::symbolic_simulator::TestMode){
    ml_.put_function("createVariableMap", 0);
  }else{
    ml_.put_function("createVariableMapInterval", 0);
  }

/////////////////// 受信処理
  HYDLA_LOGGER_VCS("%% receive\n");
  ml_.receive();
  PacketErrorHandler::handle(&ml_);

  int token = ml_.get_type();
  
  create_result_t create_result;
  if(token == MLTKSYM){
    std::string name = ml_.get_symbol();
    assert(name == "underConstraint");
  }
  else{
    // List関数の要素数（式の個数）を得る
    int or_size = ml_.get_arg_count();
    HYDLA_LOGGER_VCS("or_size: ", or_size);
    ml_.get_next();// Listという関数名
    for(int or_it = 0; or_it < or_size; or_it++){
      ml_.get_next();
      variable_range_map_t map(original_range_map_);
      value_t symbolic_value;
      int and_size = ml_.get_arg_count();
      HYDLA_LOGGER_VCS("and_size: ", and_size);
      ml_.get_next();// Listという関数名
      for(int i = 0; i < and_size; i++)
      {
        //{{変数名，微分回数}, 関係演算子コード，数式}で来るはず

        ml_.get_next();
        ml_.get_next();// 関数であるということと，その引数の数
        ml_.get_next();// Listという関数名

        ml_.get_next();// 関数であるということと，その引数の数
        ml_.get_next();// Listという関数名
        
        std::string variable_name = ml_.get_symbol();
        HYDLA_LOGGER_VCS("%% name: ", variable_name);
        int variable_derivative_count;
        variable_derivative_count = ml_.get_integer();
        HYDLA_LOGGER_VCS("%% derivative_count: ", variable_derivative_count);
        // 関係演算子のコード
        int relop_code = ml_.get_integer();
        HYDLA_LOGGER_VCS("%% relop_code: ", relop_code);
        
        symbolic_value = MathematicaExpressionConverter::receive_and_make_symbolic_value(ml_);
        // TODO: ↓の一行消す
        if(variable_name == "t")continue;
        variable_t* variable_ptr = get_variable(variable_name.substr(6), variable_derivative_count);
        if(!variable_ptr){
          continue;
        }
        value_range_t tmp_range = map[variable_ptr];
        MathematicaExpressionConverter::set_range(symbolic_value, tmp_range, relop_code);
        if(symbolic_value->is_undefined()){
          throw SolveError("invalid value");
        }
        HYDLA_LOGGER_VCS("%% symbolic_value: ", *symbolic_value);
        map[variable_ptr] = tmp_range;
      }
      create_result.result_maps.push_back(map);
    }
  }
  
  ml_.MLNewPacket();
  
  
  for(unsigned int i=0; i < create_result.result_maps.size();i++){
    HYDLA_LOGGER_VCS("--- result map ", i, "/", create_result.result_maps.size(), "---\n");
    HYDLA_LOGGER_VCS(create_result.result_maps[i]);
  }
  HYDLA_LOGGER_VCS("#*** END MathematicaVCS::create_maps ***\n");
  return create_result;
}

void MathematicaVCS::add_constraint(const constraints_t& constraints)
{
  HYDLA_LOGGER_VCS(
    "#*** Begin MathematicaVCS::add_constraint ***\n");
  
  PacketSender::VariableArg arg = (mode_==hydla::symbolic_simulator::DiscreteMode || mode_== hydla::symbolic_simulator::TestMode)?PacketSender::VA_None:PacketSender::VA_Time;


  HYDLA_LOGGER_VCS("%% addConstraint");
  ml_.put_function("addConstraint", 2);
  PacketSender ps(ml_);

  /*
  for( constraints_t::const_iterator it = constraints.begin();it!=constraints.end();it++){
    HYDLA_LOGGER_VCS(**it);
  }
  */

  ps.put_nodes(constraints, arg);
  // varsを渡す
  ps.put_vars();


  ///////////////// 受信処理
  HYDLA_LOGGER_VCS( "--- receive ---");
  ml_.receive();
  ml_.MLNewPacket();

  HYDLA_LOGGER_VCS("\n#*** End MathematicaVCS::add_constraint ***\n");
  return;
}

void MathematicaVCS::add_constraint(const node_sptr& constraint)
{
  return add_constraint(constraints_t(1, constraint));
}

// TODO: add_guardなのかset_guardなのかとか，仕様とかをはっきりさせる
void MathematicaVCS::add_guard(const node_sptr& guard)
{
  HYDLA_LOGGER_VCS(
    "#*** Begin MathematicaVCS::add_guard ***\n");
  
  PacketSender::VariableArg arg = (mode_==hydla::symbolic_simulator::DiscreteMode || mode_==hydla::symbolic_simulator::TestMode)?PacketSender::VA_None:PacketSender::VA_Time;

  switch(mode_){
    case hydla::symbolic_simulator::DiscreteMode:
      ml_.put_function("addConstraint", 2);
      break;
    case hydla::symbolic_simulator::TestMode:
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

  HYDLA_LOGGER_VCS("\n#*** End MathematicaVCS::add_guard ***\n");
  return;
}

MathematicaVCS::TestResult MathematicaVCS::test_consistency(node_sptr& node)
{
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCS::test_consistency ***");

  ml_.put_function("testConsistency",0);

/////////////////// 受信処理
  HYDLA_LOGGER_VCS( "%%receive");
  ml_.receive();
  PacketErrorHandler::handle(&ml_);

  //まずListが来るはず
  ml_.get_next();
  
  int token = ml_.get_next();
  
  if(token == MLTKSYM){
    //TrueかFalseのはず
    std::string symbol = ml_.get_symbol();
    if(symbol == "True"){
      return TEST_TRUE;
    }else{
      return TEST_FALSE;
    }
  }else if(token == MLTKINT){
    HYDLA_LOGGER_VCS("illegal integer:", ml_.get_integer());
  }else{
    //更に二重リストが来るはず
    int map_size = ml_.get_arg_count();
    ml_.get_next();
    for(int i=0; i < map_size; i++){
      ml_.get_next();
      if(node == NULL) node = receive_condition_node();
      else node = node_sptr(new LogicalOr(node,receive_condition_node()));
    }
  }
  //終わりなのでパケットの最後尾までスキップ
  ml_.MLNewPacket();
  if(node != NULL){
    HYDLA_LOGGER_VCS("false_condition : ", *node);
  }
  
  HYDLA_LOGGER_VCS("#*** End MathematicaVCS::test_consistency ***");
  return TEST_UNKNOWN;
}

MathematicaVCS::check_consistency_result_t MathematicaVCS::check_consistency()
{
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCS::check_consistency ***");

  switch(mode_){ 
  case hydla::symbolic_simulator::DiscreteMode:
    ml_.put_function("checkConsistencyPoint", 0);
    break;
  case hydla::symbolic_simulator::TestMode:
    ml_.put_function("checkFalseConditions", 0);
    break;
  default:
    ml_.put_function("checkConsistencyInterval", 0);
    break;
  }
  
/////////////////// 受信処理
  HYDLA_LOGGER_VCS( "%%receive");
  ml_.receive();
  PacketErrorHandler::handle(&ml_);
  
  //まずListが来るはず
  ml_.get_next();
  
  int token = ml_.get_next();
  
  check_consistency_result_t ret;
  
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
  
  HYDLA_LOGGER_VCS("#*** End MathematicaVCS::check_consistency ***");
  return ret;
}

MathematicaVCS::PP_time_result_t MathematicaVCS::calculate_next_PP_time(
  const constraints_t& discrete_cause,
  const time_t& current_time,
  const time_t& max_time)
{
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCSInterval::calculate_next_PP_time ***");

////////////////// 送信処理
  PacketSender ps(ml_);
  
  // calculateNextPointPhaseTime[maxTime, discCause]を渡したい
  ml_.put_function("calculateNextPointPhaseTime", 2);


  // maxTimeを渡す
  time_t tmp_time(max_time->clone());
  *tmp_time -= *current_time;
  HYDLA_LOGGER_VCS("%% current time:", *current_time);
  HYDLA_LOGGER_VCS("%% send time:", *tmp_time);
  ps.put_value(tmp_time, PacketSender::VA_Time);

  //離散変化の条件を渡す
  ml_.put_function("List", discrete_cause.size());
  for(constraints_t::const_iterator it = discrete_cause.begin(); it != discrete_cause.end();it++){
    ps.put_node(*it, PacketSender::VA_Time);
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
  HYDLA_LOGGER_VCS("#*** End MathematicaVCSInterval::calculate_next_PP_time ***");
  return result;
}

node_sptr MathematicaVCS::receive_condition_node(){
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCS::receive_condition_map ***");
  int condition_size = ml_.get_arg_count(); //条件式の数
  HYDLA_LOGGER_VCS("%% map size:", condition_size);
  ml_.get_next();

  node_sptr ret;

  for(int cond_it = 0; cond_it < condition_size; cond_it++){
    // 最初，Listの引数の数(MLTKFUNC）
    ml_.get_next();
    ml_.get_next(); ml_.get_next(); // これでListの先頭要素まで来る
    ml_.get_next(); ml_.get_next(); // 先頭要素のprevを読み飛ばす

    std::string name = ml_.get_symbol();
    int derivative_count = ml_.get_integer();
     if(get_variable(name, derivative_count) == NULL){
      throw SolveError("some unknown form of result");
    }
    int relop_code = ml_.get_integer();
    HYDLA_LOGGER_VCS("%% returned relop_code: ", relop_code);
    node_sptr tmp_var = node_sptr(new Variable(name));
    for(int i = 0; i < derivative_count; i++){
      tmp_var = node_sptr(new Differential(tmp_var));
    }
    switch(relop_code){
      case 0: // Equal
        tmp_var = node_sptr(new Equal(tmp_var, MathematicaExpressionConverter::make_tree(ml_)));
        break;
      case 1: // Less
        tmp_var = node_sptr(new Less(tmp_var, MathematicaExpressionConverter::make_tree(ml_)));
        break;
      case 2: // Greater
        tmp_var = node_sptr(new Greater(tmp_var, MathematicaExpressionConverter::make_tree(ml_)));
        break;
      case 3: // LessEqual
        tmp_var = node_sptr(new LessEqual(tmp_var, MathematicaExpressionConverter::make_tree(ml_)));
        break;
      case 4: // GreaterEqual
        tmp_var = node_sptr(new GreaterEqual(tmp_var, MathematicaExpressionConverter::make_tree(ml_)));
        break;
      default:
        assert(0);
        break;
    }
    if(ret == NULL) ret = tmp_var; 
    else ret = node_sptr(new LogicalAnd(ret,tmp_var));
  }
  //  HYDLA_LOGGER_VCS("--- result map---\n", map);
  HYDLA_LOGGER_VCS("#*** End MathematicaVCS::receive_condition_map ***");
  return ret;
}

void MathematicaVCS::receive_parameter_map(parameter_map_t &map){
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCS::receive_parameter_map ***");
  int condition_size = ml_.get_arg_count(); //条件式の数
  HYDLA_LOGGER_VCS("%% map size:", condition_size);
  ml_.get_next();
  for(int cond_it = 0; cond_it < condition_size; cond_it++){
    // 最初，Listの引数の数(MLTKFUNC）
    ml_.get_next();
    ml_.get_next(); ml_.get_next(); // これでListの先頭要素まで来る
    ml_.get_next(); ml_.get_next(); // 先頭要素のparameterを読み飛ばす
    std::string name = ml_.get_symbol();
    int derivative_count = ml_.get_integer();
    int id = ml_.get_integer();
    parameter_t* tmp_param = get_parameter(name, derivative_count, id);
    if(tmp_param == NULL){
      throw SolveError("some unknown form of result");
    }
    value_range_t tmp_range = map[tmp_param];
    HYDLA_LOGGER_VCS("%% returned parameter_name: ", *tmp_param);
    int relop_code = ml_.get_integer();
    HYDLA_LOGGER_VCS("%% returned relop_code: ", relop_code);
    value_t tmp_value = MathematicaExpressionConverter::receive_and_make_symbolic_value(ml_);
    HYDLA_LOGGER_VCS("%% returned value: ", tmp_value->get_string());
    MathematicaExpressionConverter::set_range(tmp_value, tmp_range, relop_code);
    map[tmp_param] = tmp_range;
  }
  HYDLA_LOGGER_VCS("--- result map---\n", map);
  HYDLA_LOGGER_VCS("#*** End MathematicaVCS::receive_parameter_map ***");
}

void MathematicaVCS::apply_time_to_vm(const variable_map_t& in_vm, 
                                              variable_map_t& out_vm, 
                                              const time_t& time)
{
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCS::apply_time_to_vm ***");

  PacketSender ps(ml_);

  variable_map_t::const_iterator it  = in_vm.begin();
  variable_map_t::const_iterator end = in_vm.end();
  for(; it!=end; ++it) {
    HYDLA_LOGGER_VCS("variable : ", *(it->first));
    // 値
    value_t    value;
    if(!it->second->is_undefined()) {
      ml_.put_function("applyTime2Expr", 2);
      ps.put_value(it->second, PacketSender::VA_Time);
      ps.put_value(time, PacketSender::VA_None);

    ////////////////// 受信処理

      ml_.receive();
      PacketErrorHandler::handle(&ml_);
      
      value = value_t(MathematicaExpressionConverter::receive_and_make_symbolic_value(ml_));
      HYDLA_LOGGER_REST("value : ", value->get_string());
    }
    out_vm[it->first] = value;
  }
  ml_.MLNewPacket();
  HYDLA_LOGGER_VCS("#*** End MathematicaVCS::apply_time_to_vm ***");
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

/*
//value_tを指定された精度で数値に変換する
std::string MathematicaVCS::get_real_val(const value_t &val, int precision, simulator::OutputFormat opfmt){
  std::string ret;
  PacketSender ps(ml_);

  if(!val->is_undefined() && opfmt == fmtNInterval) {
    
    //precisionは2より大きいとする
    if(precision<2) precision=2;

    ml_.put_function("ToString", 2);
    ml_.put_function("Interval", 1);    
    ml_.put_function("N", 2);  
    ps.put_value(val, PacketSender::VA_None);
    ml_.put_integer(precision+5);     
    ml_.put_symbol("InputForm");
    ml_.skip_pkt_until(RETURNPKT);
    ret = ml_.get_string();

    //usrVarの場合はそのまま
    if(ret.find(PacketSender::var_prefix) != -1) 
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
    
  } else  if(!val->is_undefined()) {
    ml_.put_function("ToString", 2);

    //ml_.put_function("Interval",1);

    ml_.put_function("N", 2);  
    ps.put_value(val, PacketSender::VA_None);
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

bool MathematicaVCS::less_than(const time_t &lhs, const time_t &rhs)
{
  ml_.put_function("ToString", 1);  
  ml_.put_function("Less", 2);  
  PacketSender ps(ml_);
  ps.put_value(lhs, PacketSender::VA_None);
  ps.put_value(rhs, PacketSender::VA_None);
  
  ml_.receive();
  std::string ret = ml_.get_string();
  return  ret == "True";
}

void MathematicaVCS::simplify(time_t &time) 
{
  ml_.put_function("integerString", 1);
  ml_.put_function("Simplify", 1);
  PacketSender ps(ml_);
  ps.put_value(time, PacketSender::VA_None);
  ml_.receive();
  time = time_t(MathematicaExpressionConverter::receive_and_make_symbolic_value(ml_));
}


void MathematicaVCS::set_continuity(const std::string& name, const int& derivative_count)
{
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCS::set_continuity ***\n");
  PacketSender ps(ml_);

  ml_.put_function("addInitConstraint", 2);
  ml_.put_function("Equal", 2);

  ps.put_var(name, derivative_count, PacketSender::VA_Prev);
  if(mode_==hydla::symbolic_simulator::DiscreteMode || mode_==hydla::symbolic_simulator::TestMode){
    ps.put_var(name, derivative_count, PacketSender::VA_None);
    ps.put_vars();
  }
  else{
    ps.put_var(name, derivative_count, PacketSender::VA_Zero);
    ps.put_vars();
  }


  ///////////////// 受信処理
  HYDLA_LOGGER_VCS( "%%receive\n");
  ml_.receive();
  ml_.MLNewPacket();

  HYDLA_LOGGER_VCS("#*** End MathematicaVCS::set_init_value ***\n");
}


  //SymbolicValueの時間をずらす
hydla::vcs::SymbolicVirtualConstraintSolver::value_t MathematicaVCS::shift_expr_time(const value_t& val, const time_t& time){
  value_t tmp_val;
  ml_.put_function("exprTimeShift", 2);
  PacketSender ps(ml_);
  ps.put_value(val, PacketSender::VA_None);
  ps.put_value(time, PacketSender::VA_None);
  ml_.receive();
  PacketErrorHandler::handle(&ml_);

  tmp_val = value_t(MathematicaExpressionConverter::receive_and_make_symbolic_value(ml_));
  return  tmp_val;
}


} // namespace mathematica
} // namespace vcs
} // namespace hydla 
