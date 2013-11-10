#ifndef _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_

#include "sexp/SExpAST.h"
#include <sstream>
#include <stdexcept>

namespace hydla {
namespace vcs {
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
class REDUCELink {
public:

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
   * stringの送信
   * \param cmd REDUCEへ送信する文字列
   * \return 0
   */
  virtual int send_string(const std::string cmd) = 0;

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
};

} // namespace reduce
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_

