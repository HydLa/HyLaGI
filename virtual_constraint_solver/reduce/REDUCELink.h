#ifndef _INCLUDED_HYDLA_BACKEND_REDUCE_LINK_H_
#define _INCLUDED_HYDLA_BACKEND_REDUCE_LINK_H_

#include "sexp/SExpAST.h"
#include <sstream>
#include <stdexcept>
#include "Link.h"
#include "Simulator.h"
#include <stack>

namespace hydla {
namespace backend {
namespace reduce {

/**
 * REDUCEサーバ/コマンドとの通信時のエラー、またはREDUCE側の論理的なエラーを表す
 */
class REDUCELinkError : public std::runtime_error {
public:
  REDUCELinkError(const std::string& msg, const std::string& line = "") :
    std::runtime_error(init(msg,line))
  {}

private:
  std::string init(const std::string& msg, const std::string& line)
  {
    std::stringstream s;
    std::string comma = (line!="") ? " : " : "";
    s << "reducelink error: " << msg << comma << line;
    return s.str();
  }
};

/**
 * REDUCEサーバとの接続クライアント、サーバ接続とstringの送受信を行う
 * telnet経由とIPC経由の実装を束ねるインタフェース
 */
class REDUCELink: public Link {
public:
  
  REDUCELink(const simulator::Opts &opts);
  virtual ~REDUCELink(){}

  /**
   * end_of_redeval_行まで文字列をgetlineする
   * skip_until_redevalを推奨
   * \return 0
   */
  virtual int read_until_redeval() = 0;

  /**
   * end_of_redeval_行まで文字列をgetlineする
   * \return 0
   */
  virtual int skip_until_redeval() = 0;

  /**
   * 受信した複数行のstringを結合して破損のないLisp式を戻す
   * \return REDUCEから受け取るS式
   */
  virtual std::string get_s_expr() = 0;

  /**
   * 受信した複数行のstringを結合してSExpASTを戻す
   * \return REDUCEから受け取るS式をパースしたもの
   */
  virtual const hydla::parser::SExpAST  get_as_s_exp_parse_tree() = 0;


  /**
   * inherit from Link
   */
  void put_function(const char* s, int n);
  void put_symbol(const char* s);
  void put_number(const char* value);
  void put_string(const char* s);
  void put_integer(int i);
  
  void get_function(std::string &name, int &cnt);
  std::string get_symbol();
  std::string get_string();
  int get_integer();
  int get_arg_count();
  DataType get_type();
  std::string get_token_name(int tk_type);
  DataType get_next();
  void pre_send();
  void pre_receive();
  void post_receive();


  /**
   * only for REDUCELink
   */
  void post_put();

  // TODO: implementation
  std::string get_debug_print();
  std::string get_input_print();

  void check(){assert(0);}

  bool convert(const std::string& orig, int orig_cnt, bool hydla2back, std::string& ret, int& ret_cnt);

  inline std::string backend_name(){return "REDUCE";}

protected:

  /**
   * 文字列中に含まれるqueryをカウントする
   * \param str 調べる文字列
   * \param query '(' または ')'
   * \return queryのカウント数
   */
  int count_char(const std::string str, const char query) const {
    int count = 0;
    std::string::size_type i = 0;
    while(true){
      i = str.find(query, i);
      if(i==std::string::npos) break;

      count++; i++;
    }

    return count;
  }

  /**
   * send string to reduce practically
   */
  virtual void send_string_to_reduce(const char* str) = 0;

private:
  /// used for put_function
  std::stack<int> arg_cnt_stack_;
};

} // namespace reduce
} // namespace backend
} // namespace hydla

#endif // _INCLUDED_HYDLA_BACKEND_REDUCE_LINK_H_

