#include "MathLink.h"
#include "math_source.h"
#include "Simulator.h"
#include "PacketErrorHandler.h"

namespace hydla{
namespace backend{
namespace mathematica{


void MathLink::init(const hydla::simulator::Opts &opts)
{

  if((env_ = MLInitialize(0)) == (MLENV)0)throw LinkError("math", "can not link",0);
  int err;
  link_ = MLOpenString(env_, opts.mathlink.c_str(), &err);
  if(link_ == (MLINK)0 || err != MLEOK) throw LinkError("math", "can not link",1);
  if(!MLActivate(link_)) throw LinkError("math", "can not link",2);


  // 出力する画面の横幅の設定
  MLPutFunction("SetOptions", 2);
  MLPutSymbol("$Output"); 
  MLPutFunction("Rule", 2);
  MLPutSymbol("PageWidth"); 
  MLPutSymbol("Infinity"); 
  MLEndPacket();
  skip_pkt_until(RETURNPKT);
  MLNewPacket();

  // デバッグプリント
  MLPutFunction("Set", 2);
  MLPutSymbol("optUseDebugPrint"); 
  MLPutSymbol(opts.debug_mode ? "True" : "False");
  MLEndPacket();
  skip_pkt_until(RETURNPKT);
  MLNewPacket();

  // プロファイルモード
  MLPutFunction("Set", 2);
  MLPutSymbol("optUseProfile"); 
  MLPutSymbol(opts.profile_mode ? "True" : "False");
  MLEndPacket();
  skip_pkt_until(RETURNPKT);
  MLNewPacket();

  // 並列モード
  MLPutFunction("Set", 2);
  MLPutSymbol("optParallel"); 
  MLPutSymbol(opts.parallel_mode ? "True" : "False");
  MLEndPacket();
  skip_pkt_until(RETURNPKT);
  MLNewPacket();

  // 警告無視
  MLPutFunction("Set", 2);
  MLPutSymbol("optIgnoreWarnings"); 
  MLPutSymbol(opts.ignore_warnings ? "True" : "False");
  MLEndPacket();
  skip_pkt_until(RETURNPKT);
  MLNewPacket();

  // 最適化レベル
  MLPutFunction("Set",2);
  MLPutSymbol("optOptimizationLevel");
  MLPutInteger(opts.optimization_level);
  MLEndPacket();
  skip_pkt_until(RETURNPKT);
  MLNewPacket();
  
  // 近似モード
/*
  TODO: 一旦無効化している
  MLPutFunction("Set",2);
  MLPutSymbol("approxMode");
  if(opts.approx_mode == NO_APPROX)MLPutSymbol("none");
  else if(opts.approx_mode == NUMERIC_APPROX)MLPutSymbol("numeric");
  else if(opts.approx_mode == INTERVAL_APPROX)MLPutSymbol("interval");
  else assert(0);
  MLEndPacket();
  skip_pkt_until(RETURNPKT);
  MLNewPacket();
*/  
  
  MLPutFunction("Set",2);
  MLPutSymbol("approxThreshold");
  MLPutInteger(opts.approx_threshold);
  MLEndPacket();
  skip_pkt_until(RETURNPKT);
  MLNewPacket();
  
  
  MLPutFunction("Set",2);
  MLPutSymbol("approxPrecision");
  MLPutInteger(opts.approx_precision);
  MLEndPacket();
  skip_pkt_until(RETURNPKT);
  MLNewPacket();
  
  MLPutFunction("Set",2);
  MLPutSymbol("approxThresholdEx");
  MLPutInteger(opts.approx_threshold_ex);
  MLEndPacket();
  skip_pkt_until(RETURNPKT);
  MLNewPacket();

  
  // タイムアウト時間
  MLPutFunction("Set",2);
  MLPutSymbol("timeOutS"); 
  if(opts.timeout_calc > 0){
    MLPutInteger(opts.timeout_calc);
  }else{
    MLPutSymbol("Infinity");
  }
  MLEndPacket();
  skip_pkt_until(RETURNPKT);
  MLNewPacket();

  MLPutFunction("ToExpression", 1);
  MLPutString(math_source());  
  MLEndPacket();
  skip_pkt_until(RETURNPKT);
  MLNewPacket();
  

  //HydLaとMathematicaの関数名の対応関係を作っておく．
  typedef function_map_t::value_type value_t;
  function_map_.insert(value_t(function_t("sin", 1), function_t("Sin", 1)));
  function_map_.insert(value_t(function_t("sinh", 1), function_t("Sinh", 1)));
  function_map_.insert(value_t(function_t("Asin", 1), function_t("ArcSin", 1)));
  function_map_.insert(value_t(function_t("Asinh", 1), function_t("ArcSinh", 1)));
  function_map_.insert(value_t(function_t("cos", 1), function_t("Cos", 1)));
  function_map_.insert(value_t(function_t("cosh", 1), function_t("Cosh", 1)));
  function_map_.insert(value_t(function_t("Acos", 1), function_t("ArcCos", 1)));
  function_map_.insert(value_t(function_t("Acosh", 1), function_t("ArcCosh", 1)));
  function_map_.insert(value_t(function_t("tan", 1), function_t("Tan", 1)));
  function_map_.insert(value_t(function_t("tanh", 1), function_t("Tanh", 1)));
  function_map_.insert(value_t(function_t("Atan", 1), function_t("Arctan", 1)));
  function_map_.insert(value_t(function_t("Atanh", 1), function_t("ArcTanh", 1)));
  function_map_.insert(value_t(function_t("log", 2), function_t("Log", 2)));
  function_map_.insert(value_t(function_t("ln", 1), function_t("Log", 1)));


  HYDLA_LOGGER_FUNC_END(VCS);
}

bool MathLink::convert(const std::string &orig, const int &orig_cnt, const bool &hydla2back, std::string &ret, int &ret_cnt)
{
  if(hydla2back)
  {
    function_map_t::left_iterator it = function_map_.left.find(function_t(orig, orig_cnt));
  if(it != function_map_.left.end())
  {
    ret = it->second.first;
    ret_cnt = it->second.second;
    return true;
  }
  else
  {
    return false;
  }
}
else
{
  function_map_t::right_iterator it = function_map_.right.find(function_t(orig, orig_cnt));
  if(it != function_map_.right.end())
  {
    ret = it->second.first;
    ret_cnt = it->second.second;
    return true;
  }
  else
  {
    return false;
  }  
}
}



bool MathLink::receive(){
  HYDLA_LOGGER_FUNC_BEGIN(EXTERN);
  bool at_end = false;
  debug_print_.clear();
  input_print_.clear();
  // 結果を受け取る（受け取るまで待ち続ける）
  while(true){
    int pkt = MLNextPacket();
    switch(pkt){
    case RETURNPKT:
      HYDLA_LOGGER_EXTERN("%% Mathlink::receive returnpkt");
      MLGetType();
      at_end = true;
      break;
    case MESSAGEPKT: // Mathematica メッセージ識別子（symbol::string）
    {
      std::string str = get_string();
      HYDLA_LOGGER_EXTERN("%% Mathlink::receive messagepkt\n", str, "\n");
      break;
    }
    case TEXTPKT: // Print[]で生成されるようなMathematica からのテキスト出力
    {
      std::string str = get_string();
      HYDLA_LOGGER_EXTERN("%% Mathlink::receive textpkt\n", str, "\n");
      if(input_print_.empty()){
        input_print_ = str;
      }else{
        debug_print_ += str + "\n";
      }
      break;
    }
    case SYNTAXPKT:
      HYDLA_LOGGER_EXTERN("%% Mathlink::receive syntaxpkt\n");
      break;
    case INPUTNAMEPKT: // 次の入力に割り当てられる名前（通常 In[n]:=）
    {
      std::string str = get_string();
      HYDLA_LOGGER_EXTERN("%% Mathlink::receive inputnamepkt\n", str, "\n");
      break;
    }
    case ILLEGALPKT:
      HYDLA_LOGGER_EXTERN("%% Mathlink::receive illegalpkt\n");
      throw LinkError("math", "receive illegalpkt", 0);
      break;
    default:
      HYDLA_LOGGER_EXTERN("%% Mathlink::receive unknownpkt: ", pkt, "\n");
      throw LinkError("math", "receive unknownpkt", 0);
      break;
    }
    if(at_end)break;
    MLNewPacket();
  }
  HYDLA_LOGGER_FUNC_END(EXTERN);
  return true;
}


void MathLink::_check(){
  int pkt;
  // 結果を受け取る（受け取るまで待ち続ける）
  std::cout << "in _check()" << std::endl;
  while(true){
    if((pkt = MLNextPacket()) == ILLEGALPKT){
      std::cout << "illegal packet" << std::endl;
      break;
    }
    std::cout << "not illegal:" << pkt << std::endl;
    switch(pkt){
    case RETURNPKT:
      std::cout << "returnpkt" << std::endl;
      switch(MLGetType()){ // 現行オブジェクトの型を得る
      case MLTKSTR: // 文字列
        strCase();
        break;
      case MLTKSYM: // シンボル（記号）
        symCase();
        break;
      case MLTKINT: // 整数
        intCase();
        break;
      case MLTKOLDINT: // 古いバージョンのMathlinkライブラリの整数
        std::cout << "oldint" << std::endl;
        break;
      case MLTKERR: // エラー
        std::cout << "err" << std::endl;
        break;
      case MLTKFUNC: // 合成関数
        funcCase();
        break;
      case MLTKREAL: // 近似実数
        std::cout << "real" << std::endl;
        break;
      case MLTKOLDREAL:
        std::cout << "oldreal" << std::endl;
        break;
      case MLTKOLDSTR:
        std::cout <<"oldstr" << std::endl;
        break;
      case MLTKOLDSYM:
        std::cout << "oldsym" << std::endl;
        break;
      default:
        std::cout <<"unknown_token" << std::endl;
      }
      break;

    case MESSAGEPKT: // Mathematica メッセージ識別子（symbol::string）
      std::cout << "messagepkt" << std::endl;
      std::cout << "#message:" << get_string() << std::endl;
      break;
    case TEXTPKT: // Print[]で生成されるようなMathematica からのテキスト出力
      std::cout << "textpkt" << std::endl;
      std::cout << "#text:" << get_string() << std::endl;
      break;
    case SYNTAXPKT:
      std::cout << "syntaxpkt" << std::endl;
      break;
    case INPUTNAMEPKT: // 次の入力に割り当てられる名前（通常 In[n]:=）
      std::cout << "inputnamepkt" << std::endl;
      std::cout << "#inputname:" << get_string() << std::endl;
      break;
    default:
      std::cout << "unknown_outer" << std::endl;
    } 
    MLNewPacket();
    std::cout << "new packet" << std::endl;
  }
  std::cout << "while end" << std::endl;
  std::cout << std::endl;

}

void MathLink::check(){
  // Returnパケット以降のチェック
  std::cout << "in check()" << std::endl;
  switch(MLGetType()){ // 現行オブジェクトの型を得る
  case MLTKSTR: // 文字列
    strCase();
    break;
  case MLTKSYM: // シンボル（記号）
    symCase();
    break;
  case MLTKINT: // 整数
    intCase();
    break;
  case MLTKOLDINT: // 古いバージョンのMathlinkライブラリの整数
    std::cout << "oldint" << std::endl;
    break;
  case MLTKERR: // エラー
    std::cout << "err" << std::endl;
    break;
  case MLTKFUNC: // 合成関数
    funcCase();
    break;
  case MLTKREAL: // 近似実数
    std::cout << "real" << std::endl;
    break;
  case MLTKOLDREAL:
    std::cout << "oldreal" << std::endl;
    break;
  case MLTKOLDSTR:
    std::cout <<"oldstr" << std::endl;
    break;
  case MLTKOLDSYM:
    std::cout << "oldsym" << std::endl;
    break;
  default:
    std::cout <<"unknown_token" << std::endl;
  }
  std::cout << std::endl;
}

void MathLink::strCase(){
  std::cout << "str" << std::endl;
  std::cout << "#string:\"" << get_string() << "\"" << std::endl;
}

void MathLink::symCase(){
  std::cout << "symbol" << std::endl;
  std::cout << "#symname:" << get_string() << std::endl;
}

void MathLink::intCase(){
  std::cout << "int" << std::endl;
  int n;
  if(! MLGetInteger(&n)){
    std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
  }
  std::cout << "#n:" << n << std::endl;
}

void MathLink::funcCase(){
  std::cout << "func" << std::endl;
  int funcarg;
  if(! MLGetArgCount(&funcarg)){
    std::cout << "#funcCase:MLGetArgCount:unable to get the number of arguments from ml" << std::endl;
  }
  std::cout << "#funcarg:" << funcarg << std::endl;
  switch(MLGetNext()){ //
  case MLTKFUNC: // 関数（Derivative[1][f]におけるDerivative[1]のように）
    std::cout << "#funcCase:case MLTKFUNC" << std::endl;
    funcCase();
    break;
  case MLTKSYM: // シンボル（記号）
    std::cout << "#funcCase:case MLTKSYM" << std::endl;
    std::cout << "#funcname:" << get_symbol() << std::endl;
    break;
  default:
    ;
  }
  for(int i=0; i<funcarg; i++){
    switch (MLGetNext()){
    case MLTKSTR:
      strCase();
      break;
    case MLTKSYM:
      symCase();
      break;
    case MLTKINT:
      intCase();
      break;
    case MLTKFUNC:
      funcCase();
      break;
    case MLTKOLDINT: // 古いバージョンのMathlinkライブラリの整数
      std::cout << "#funcCase:oldint" << std::endl;
      break;
    case MLTKERR: // エラー
      std::cout << "#funcCase:err" << std::endl;
      break;
    case MLTKREAL: // 近似実数
      std::cout << "#funcCase:real" << std::endl;
      break;
    case MLTKOLDREAL:
      std::cout << "#funcCase:oldreal" << std::endl;
      break;
    case MLTKOLDSTR:
      std::cout << "#funcCase:oldstr" << std::endl;
      break;
    case MLTKOLDSYM:
      std::cout << "#funcCase:oldsym" << std::endl;
      break;
    default:
      std::cout << "#funcCase:unknown_token" << std::endl;      
    }
  }
}

void MathLink::pre_receive()
{
  receive();
  PacketErrorHandler::handle(this);
}

void MathLink::post_receive()
{
  MLNewPacket();
}

}
}
}
