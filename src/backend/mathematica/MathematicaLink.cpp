#include "MathematicaLink.h"
#include "math_source.h"
#include "Simulator.h"
#include "TimeOutError.h"
#include "Utility.h"
#include "LinkError.h"

namespace hydla{
namespace backend{
namespace mathematica{

const std::string MathematicaLink::par_prefix = "p";


MathematicaLink::MathematicaLink(const std::string &mathlink_name, bool ignore_warnings, const std::string& simplify_time, const int simplify_level) : env_(0), link_(0)
{

  if((env_ = MLInitialize(0)) == (MLENV)0)throw LinkError("math", "can not link",0);
  int err;
  link_ = MLOpenString(env_, mathlink_name.c_str(), &err);
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


  // 警告無視
  MLPutFunction("Set", 2);
  MLPutSymbol("optIgnoreWarnings"); 
  MLPutSymbol(ignore_warnings ? "True" : "False");
  MLEndPacket();
  skip_pkt_until(RETURNPKT);
  MLNewPacket();

  
  MLPutFunction("Set", 2);
  MLPutSymbol("optTimeConstraint");
  MLPutFunction("ToExpression", 1);
  MLPutString(simplify_time.c_str());
  MLEndPacket();
  skip_pkt_until(RETURNPKT);
  MLNewPacket();

  MLPutFunction("Set", 2);
  MLPutSymbol("optSimplifyLevel");
  MLPutInteger(simplify_level);
  MLEndPacket();
  skip_pkt_until(RETURNPKT);
  MLNewPacket();


  MLPutFunction("ToExpression", 1);
  MLPutString(math_source());  
  MLEndPacket();
  skip_pkt_until(RETURNPKT);
  MLNewPacket();
  
  
  typedef function_map_t::value_type f_value_t;
  //HydLaとMathematicaの関数名の対応関係を作っておく．
  function_map_.insert(f_value_t("sin", "Sin"));
  function_map_.insert(f_value_t("sinh", "Sinh"));
  function_map_.insert(f_value_t("Asin", "ArcSin"));
  function_map_.insert(f_value_t("Asinh","ArcSinh"));
  function_map_.insert(f_value_t("cos", "Cos"));
  function_map_.insert(f_value_t("cosh", "Cosh"));
  function_map_.insert(f_value_t("Acos", "ArcCos"));
  function_map_.insert(f_value_t("Acosh", "ArcCosh"));
  function_map_.insert(f_value_t("tan", "Tan"));
  function_map_.insert(f_value_t("tanh", "Tanh"));
  function_map_.insert(f_value_t("Atan", "ArcTan"));
  function_map_.insert(f_value_t("Atanh", "ArcTanh"));
  function_map_.insert(f_value_t("log", "Log"));
  function_map_.insert(f_value_t("ln", "Log"));
  on_next_ = false;
}

MathematicaLink::~MathematicaLink()
{
  clean();
}

void MathematicaLink::reset()
{
}

void MathematicaLink::clean()
{
  if(link_) {
    MLPutFunction("Exit", 0);
    MLEndPacket();
    MLClose(link_);
    link_ = 0;
  }
  if(env_)  {
    MLDeinitialize(env_);
    env_ = 0;
  }
}


void MathematicaLink::skip_pkt_until(int pkt_name)
{
  int p;
  while ((p = MLNextPacket()) && p != pkt_name) {
    MLNewPacket();
  }
}


bool MathematicaLink::receive_to_return_packet(){
  bool at_end = false;
  debug_print_.clear();
  input_print_.clear();
  // 結果を受け取る（受け取るまで待ち続ける）
  while(true){
    int pkt = MLNextPacket();
    switch(pkt){
    case RETURNPKT:
      MLGetType();
      on_next_ = true;
      at_end = true;
      break;
    case MESSAGEPKT: // Mathematica メッセージ識別子（symbol::string）
    {
      std::string str = get_string();
      break;
    }
    case TEXTPKT: // Print[]で生成されるようなMathematica からのテキスト出力
    {
      std::string str = get_string();
      str = utility::replace(str, "\\012", "\n");
      str = utility::replace(str, "\\011", "\t");
      if(input_print_.empty()){
        input_print_ = str;
        if(trace)HYDLA_LOGGER_DEBUG("input: \n", str);
      }else{
        if(trace)HYDLA_LOGGER_DEBUG("trace: ", str);
        debug_print_ += str + "\n";
      }
      break;
    }
    case CALLPKT:
      break;
    case SYNTAXPKT:
      break;
    case INPUTNAMEPKT: // 次の入力に割り当てられる名前（通常 In[n]:=）
    {
      std::string str = get_string();
      break;
    }
    case ILLEGALPKT:
      throw LinkError("math", "receive illegalpkt", 0);
      break;
    case MENUPKT:
      // should be in interrupt dialog
      MLNewPacket();
      put_string("a");

      //skip a text packet and an empty list
      MLNextPacket();
      MLNewPacket();
      MLNextPacket();
      MLNewPacket();
      throw LinkError("math", "receive menu packet (Did you interrupt?)", 0);
      break;
    default:
      throw LinkError("math", "receive unknownpkt", pkt);
      break;
    }
    if(at_end)break;
    MLNewPacket();
  }
  return true;
}


void MathematicaLink::_check(){
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
      check();
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

void MathematicaLink::check(){
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

void MathematicaLink::strCase(){
  std::cout << "str" << std::endl;
  std::cout << "#string:\"" << get_string() << "\"" << std::endl;
}

void MathematicaLink::symCase(){
  std::cout << "symbol" << std::endl;
  std::cout << "#symname:" << get_string() << std::endl;
}

void MathematicaLink::intCase(){
  std::cout << "int" << std::endl;
  int n;
  if(! MLGetInteger(&n)){
    std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
  }
  std::cout << "#n:" << n << std::endl;
}

void MathematicaLink::funcCase(){
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
    {
      const char* s;
      MLGetSymbol(&s);
      std::cout << "#funcname:" << s << std::endl;
      MLReleaseString(s);
    }
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

void MathematicaLink::pre_send()
{
}

void MathematicaLink::pre_receive()
{
  receive_to_return_packet();
  get_next();
  int ret_code = get_integer();
  if(ret_code == 0) {
    throw LinkError(backend_name(), "input:\n" + get_input_print() + "\n\ntrace:\n" + get_debug_print(), 0, "");
  }
  if(ret_code == -1) {
    throw timeout::TimeOutError("input:\n" + get_input_print() + "\n\ntrace:\n" + get_debug_print());
  }
}

  
void MathematicaLink::get_function(std::string &name, int &cnt)
{
  cnt = get_arg_count();
  name = get_symbol();
}

std::string MathematicaLink::get_symbol()
{
  const char *s;
  if(!on_next_)
  {
    get_next();
  }
  if(!MLGetSymbol(&s)){
    throw LinkError("math", "get_symbol", MLError(), "input:\n" + input_print_ + "\n\ntrace:\n" + debug_print_);
  }
  std::string ret = s;

  MLReleaseSymbol(s);
  on_next_ = false;
  return ret;
}
  
std::string MathematicaLink::get_string()
{
  const char*s;
  if(!on_next_)
  {
    get_next();
  }
  if(!MLGetString(&s)) {
    throw LinkError("math", "get_string", MLError(), "input:\n" + input_print_ + "\n\ntrace:\n" + debug_print_);
  }
  std::string ret = s;
  MLReleaseString(s);
  on_next_ = false;
  return ret; 
}
    
int MathematicaLink::get_integer()
{
  int i;
  if(!on_next_)
  {
    get_next();
  }
  if(!MLGetInteger(&i)) {
    throw LinkError("math", "get_integer", MLError(), "input:\n" + input_print_ + "\n\ntrace:\n" + debug_print_);
  }
  on_next_ = false;
  return i;
}

double MathematicaLink::get_double()
{
  double d;
  if(!on_next_)
  {
    get_next();
  }
  if(!MLGetDouble(&d)) {
    throw LinkError("math", "get_double", MLError(), "input:\n" + input_print_ + "\n\ntrace:\n" + debug_print_);
  }
  on_next_ = false;
  return d;
}

int MathematicaLink::get_arg_count()
{
  int c;
  if(!on_next_)
  {
    get_next(); 
  }
  if(!MLGetArgCount(&c)){
    throw LinkError("math", "get_arg_count", MLError(), "input:\n" + input_print_ + "\n\ntrace:\n" + debug_print_);
  }
  on_next_ = false;
  return c;
}

void MathematicaLink::post_receive()
{
  MLNewPacket();
}


MathematicaLink::DataType MathematicaLink::get_type(){
  int tk_type;
  if(!on_next_)
  {
    on_next_ = true;
    tk_type = MLGetNext();
  }
  else
  {
    tk_type = MLGetType();
  }
  switch(tk_type)
  {
  case MLTKFUNC:
    return DT_FUNC;
  case MLTKSYM:
    return DT_SYM;
  case MLTKSTR:
    return DT_STR;
  case MLTKINT:
    return DT_INT;
  default:
    throw LinkError("math", "get_type", MLError(), "input:\n" + input_print_ + "\n\ntrace:\n" + debug_print_);
    return DT_NONE;
  }
}

MathematicaLink::DataType MathematicaLink::get_next(){
  int tk_type = MLGetNext();
  on_next_ = false;
  switch(tk_type)
  {
  case MLTKFUNC:
    return DT_FUNC;
  case MLTKSYM:
    return DT_SYM;
    break;
  case MLTKSTR:
    return DT_STR;
  case MLTKINT:
    return DT_INT;
  default:
    return DT_NONE;
  }
}


void MathematicaLink::put_parameter(const std::string& name, int diff_count, int id)
{
  put_function(par_prefix.c_str(), 3);
  put_symbol(name.c_str());
  put_integer(diff_count);
  put_integer(id);
}


std::string MathematicaLink::get_token_name(int tk_type)
{
  switch(tk_type)
  {
  case MLTKFUNC:
    return "MLTKFUNC";
  case MLTKSYM:
    return "MLTKSYM";
  case MLTKSTR:
    return "MLTKSTR";
  case MLTKINT:
    return "MLTKINT";
  default:
    return "unknown token";
  }
}

}
}
}
