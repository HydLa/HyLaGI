#include "MathematicaLink.h"
#include "LinkError.h"
#include "Simulator.h"
#include "TimeOutError.h"
#include "Utility.h"
#include "math_source.h"

namespace hydla {
namespace backend {
namespace mathematica {

const std::string MathematicaLink::par_prefix = "p";

MathematicaLink::MathematicaLink(const std::string &wstp_name,
                                 bool ignore_warnings,
                                 const std::string &simplify_time,
                                 const int simplify_level,
                                 const int dsolve_method, bool solve_over_reals)
    : env_(0), link_(0) {

  if ((env_ = WSInitialize(0)) == (WSENV)0)
    throw LinkError("math", "can not link", 0);
  int err;
  link_ = WSOpenString(env_, wstp_name.c_str(), &err);
  if (link_ == (WSLINK)0 || err != WSEOK)
    throw LinkError("math", "can not link", 1);
  if (!WSActivate(link_))
    throw LinkError("math", "can not link", 2);

  // 出力する画面の横幅の設定
  WSPutFunction("SetOptions", 2);
  WSPutSymbol("$Output");
  WSPutFunction("Rule", 2);
  WSPutSymbol("PageWidth");
  WSPutSymbol("Infinity");
  WSEndPacket();
  skip_pkt_until(RETURNPKT);
  WSNewPacket();

  // 警告無視
  WSPutFunction("Set", 2);
  WSPutSymbol("optIgnoreWarnings");
  WSPutSymbol(ignore_warnings ? "True" : "False");
  WSEndPacket();
  skip_pkt_until(RETURNPKT);
  WSNewPacket();

  WSPutFunction("Set", 2);
  WSPutSymbol("optTimeConstraint");
  WSPutFunction("ToExpression", 1);
  WSPutString(simplify_time.c_str());
  WSEndPacket();
  skip_pkt_until(RETURNPKT);
  WSNewPacket();

  WSPutFunction("Set", 2);
  WSPutSymbol("optSimplifyLevel");
  WSPutInteger(simplify_level);
  WSEndPacket();
  skip_pkt_until(RETURNPKT);
  WSNewPacket();

  WSPutFunction("Set", 2);
  WSPutSymbol("optDSolveMethod");
  WSPutInteger(dsolve_method);
  WSEndPacket();
  skip_pkt_until(RETURNPKT);
  WSNewPacket();

  WSPutFunction("Set", 2);
  WSPutSymbol("optSolveOverReals");
  WSPutSymbol(solve_over_reals ? "True" : "False");
  WSEndPacket();
  skip_pkt_until(RETURNPKT);
  WSNewPacket();

  WSPutFunction("ToExpression", 1);
  WSPutString(math_source());
  WSEndPacket();
  skip_pkt_until(RETURNPKT);
  WSNewPacket();

  on_next_ = false;
}

MathematicaLink::~MathematicaLink() { clean(); }

void MathematicaLink::reset() {}

void MathematicaLink::clean() {
  if (link_) {
    WSPutFunction("Exit", 0);
    WSEndPacket();
    WSClose(link_);
    link_ = 0;
  }
  if (env_) {
    WSDeinitialize(env_);
    env_ = 0;
  }
}

void MathematicaLink::skip_pkt_until(int pkt_name) {
  int p;
  while ((p = WSNextPacket()) && p != pkt_name) {
    WSNewPacket();
  }
}

bool MathematicaLink::receive_to_return_packet() {
  bool at_end = false;
  debug_print_.clear();
  input_print_.clear();
  // 結果を受け取る（受け取るまで待ち続ける）
  while (true) {
    int pkt = WSNextPacket();
    switch (pkt) {
    case RETURNPKT:
      WSGetType();
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
      if (input_print_.empty()) {
        input_print_ = str;
        if (trace)
          HYDLA_LOGGER_DEBUG("input: \n", str);
      } else {
        if (trace)
          HYDLA_LOGGER_DEBUG_INTERNAL("trace: ", str);
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
      WSNewPacket();
      put_string("a");

      // skip a text packet and an empty list
      WSNextPacket();
      WSNewPacket();
      WSNextPacket();
      WSNewPacket();
      throw LinkError("math", "receive menu packet (Did you interrupt?)", 0);
      break;
    default:
      throw LinkError("math", "receive unknownpkt", pkt);
      break;
    }
    if (at_end)
      break;
    WSNewPacket();
  }
  return true;
}

void MathematicaLink::_check() {
  int pkt;
  // 結果を受け取る（受け取るまで待ち続ける）
  std::cout << "in _check()" << std::endl;
  while (true) {
    if ((pkt = WSNextPacket()) == ILLEGALPKT) {
      std::cout << "illegal packet" << std::endl;
      break;
    }
    std::cout << "not illegal:" << pkt << std::endl;
    switch (pkt) {
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
    WSNewPacket();
    std::cout << "new packet" << std::endl;
  }
  std::cout << "while end" << std::endl;
  std::cout << std::endl;
}

void MathematicaLink::check() {
  // Returnパケット以降のチェック
  std::cout << "in check()" << std::endl;
  switch (WSGetType()) { // 現行オブジェクトの型を得る
  case WSTKSTR:          // 文字列
    strCase();
    break;
  case WSTKSYM: // シンボル（記号）
    symCase();
    break;
  case WSTKINT: // 整数
    intCase();
    break;
  case WSTKOLDINT: // 古いバージョンのMathlinkライブラリの整数
    std::cout << "oldint" << std::endl;
    break;
  case WSTKERR: // エラー
    std::cout << "err" << std::endl;
    break;
  case WSTKFUNC: // 合成関数
    funcCase();
    break;
  case WSTKREAL: // 近似実数
    std::cout << "real" << std::endl;
    break;
  case WSTKOLDREAL:
    std::cout << "oldreal" << std::endl;
    break;
  case WSTKOLDSTR:
    std::cout << "oldstr" << std::endl;
    break;
  case WSTKOLDSYM:
    std::cout << "oldsym" << std::endl;
    break;
  default:
    std::cout << "unknown_token" << std::endl;
  }
  std::cout << std::endl;
}

void MathematicaLink::strCase() {
  std::cout << "str" << std::endl;
  std::cout << "#string:\"" << get_string() << "\"" << std::endl;
}

void MathematicaLink::symCase() {
  std::cout << "symbol" << std::endl;
  std::cout << "#symname:" << get_string() << std::endl;
}

void MathematicaLink::intCase() {
  std::cout << "int" << std::endl;
  int n;
  if (!WSGetInteger(&n)) {
    std::cout << "WSGetInteger:unable to read the int from ml" << std::endl;
  }
  std::cout << "#n:" << n << std::endl;
}

void MathematicaLink::funcCase() {
  std::cout << "func" << std::endl;
  int funcarg;
  if (!WSGetArgCount(&funcarg)) {
    std::cout << "#funcCase:WSGetArgCount:unable to get the number of "
                 "arguments from ml"
              << std::endl;
  }
  std::cout << "#funcarg:" << funcarg << std::endl;
  switch (WSGetNext()) { //
  case WSTKFUNC: // 関数（Derivative[1][f]におけるDerivative[1]のように）
    std::cout << "#funcCase:case WSTKFUNC" << std::endl;
    funcCase();
    break;
  case WSTKSYM: // シンボル（記号）
    std::cout << "#funcCase:case WSTKSYM" << std::endl;
    {
      const char *s;
      WSGetSymbol(&s);
      std::cout << "#funcname:" << s << std::endl;
      WSReleaseString(s);
    }
    break;
  default:;
  }
  for (int i = 0; i < funcarg; i++) {
    switch (WSGetNext()) {
    case WSTKSTR:
      strCase();
      break;
    case WSTKSYM:
      symCase();
      break;
    case WSTKINT:
      intCase();
      break;
    case WSTKFUNC:
      funcCase();
      break;
    case WSTKOLDINT: // 古いバージョンのMathlinkライブラリの整数
      std::cout << "#funcCase:oldint" << std::endl;
      break;
    case WSTKERR: // エラー
      std::cout << "#funcCase:err" << std::endl;
      break;
    case WSTKREAL: // 近似実数
      std::cout << "#funcCase:real" << std::endl;
      break;
    case WSTKOLDREAL:
      std::cout << "#funcCase:oldreal" << std::endl;
      break;
    case WSTKOLDSTR:
      std::cout << "#funcCase:oldstr" << std::endl;
      break;
    case WSTKOLDSYM:
      std::cout << "#funcCase:oldsym" << std::endl;
      break;
    default:
      std::cout << "#funcCase:unknown_token" << std::endl;
    }
  }
}

void MathematicaLink::pre_send() {}

void MathematicaLink::pre_receive() {
  receive_to_return_packet();
  get_next();
  int ret_code = get_integer();
  if (ret_code == 0) {
    throw LinkError(backend_name(),
                    "input:\n" + get_input_print() + "\n\ntrace:\n" +
                        get_debug_print(),
                    0, "");
  }
  if (ret_code == -1) {
    throw timeout::TimeOutError("input:\n" + get_input_print() +
                                "\n\ntrace:\n" + get_debug_print());
  }
}

void MathematicaLink::get_function(std::string &name, int &cnt) {
  cnt = get_arg_count();
  name = get_symbol();
}

std::string MathematicaLink::get_symbol() {
  const char *s;
  if (!on_next_) {
    get_next();
  }
  if (!WSGetSymbol(&s)) {
    throw LinkError("math", "get_symbol", WSError(),
                    "input:\n" + input_print_ + "\n\ntrace:\n" + debug_print_);
  }
  std::string ret = s;

  WSReleaseSymbol(s);
  on_next_ = false;
  return ret;
}

std::string MathematicaLink::get_string() {
  const char *s;
  if (!on_next_) {
    get_next();
  }
  if (!WSGetString(&s)) {
    throw LinkError("math", "get_string", WSError(),
                    "input:\n" + input_print_ + "\n\ntrace:\n" + debug_print_);
  }
  std::string ret = s;
  WSReleaseString(s);
  on_next_ = false;
  return ret;
}

int MathematicaLink::get_integer() {
  int i;
  if (!on_next_) {
    get_next();
  }
  if (!WSGetInteger(&i)) {
    throw LinkError("math", "get_integer", WSError(),
                    "input:\n" + input_print_ + "\n\ntrace:\n" + debug_print_);
  }
  on_next_ = false;
  return i;
}

double MathematicaLink::get_double() {
  double d;
  if (!on_next_) {
    get_next();
  }
  if (!WSGetDouble(&d)) {
    throw LinkError("math", "get_double", WSError(),
                    "input:\n" + input_print_ + "\n\ntrace:\n" + debug_print_);
  }
  on_next_ = false;
  return d;
}

int MathematicaLink::get_arg_count() {
  int c;
  if (!on_next_) {
    get_next();
  }
  if (!WSGetArgCount(&c)) {
    throw LinkError("math", "get_arg_count", WSError(),
                    "input:\n" + input_print_ + "\n\ntrace:\n" + debug_print_);
  }
  on_next_ = false;
  return c;
}

void MathematicaLink::post_receive() { WSNewPacket(); }

MathematicaLink::DataType MathematicaLink::get_type() {
  int tk_type;
  if (!on_next_) {
    on_next_ = true;
    tk_type = WSGetNext();
  } else {
    tk_type = WSGetType();
  }
  switch (tk_type) {
  case WSTKFUNC:
    return DT_FUNC;
  case WSTKSYM:
    return DT_SYM;
  case WSTKSTR:
    return DT_STR;
  case WSTKINT:
    return DT_INT;
  default:
    throw LinkError("math", "get_type", WSError(),
                    "input:\n" + input_print_ + "\n\ntrace:\n" + debug_print_);
    return DT_NONE;
  }
}

MathematicaLink::DataType MathematicaLink::get_next() {
  int tk_type = WSGetNext();
  on_next_ = false;
  switch (tk_type) {
  case WSTKFUNC:
    return DT_FUNC;
  case WSTKSYM:
    return DT_SYM;
    break;
  case WSTKSTR:
    return DT_STR;
  case WSTKINT:
    return DT_INT;
  default:
    return DT_NONE;
  }
}

void MathematicaLink::put_parameter(const std::string &name, int diff_count,
                                    int id) {
  put_function(par_prefix.c_str(), 3);
  put_symbol(name.c_str());
  put_integer(diff_count);
  put_integer(id);
}

std::string MathematicaLink::get_token_name(int tk_type) {
  switch (tk_type) {
  case WSTKFUNC:
    return "WSTKFUNC";
  case WSTKSYM:
    return "WSTKSYM";
  case WSTKSTR:
    return "WSTKSTR";
  case WSTKINT:
    return "WSTKINT";
  default:
    return "unknown token";
  }
}

} // namespace mathematica
} // namespace backend
} // namespace hydla
