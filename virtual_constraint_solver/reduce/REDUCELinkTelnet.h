#ifndef _INCLUDED_HYDLA_VCS_REDUCE_LINK_TELNET_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_LINK_TELNET_H_

#include "REDUCELink.h"

#include "sexp/SExpAST.h"
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/error_code.hpp>
#include <sstream>
#include <stdexcept>

namespace hydla {
namespace vcs {
namespace reduce {

/**
 * REDUCEサーバとの接続クライアント、サーバ接続とstringの送受信を行う
 */
class REDUCELinkTelnet : public REDUCELink {
public:

  /**
   * localhost:1206 に接続する
   */
  REDUCELinkTelnet();
  ~REDUCELinkTelnet();

  /**
   * end_of_redeval_行まで文字列をgetlineする
   * skip_until_redevalを推奨
   * \return 0
   */
  int read_until_redeval();

  /**
   * end_of_redeval_行まで文字列をgetlineする
   * \return 0
   */
  int skip_until_redeval();

  /**
   * 受信した複数行のstringを結合して破損のないLisp式を戻す
   * \return REDUCEから受け取るS式
   */
  std::string get_s_expr();

  /**
   * 受信した複数行のstringを結合してSExpASTを戻す
   * \return REDUCEから受け取るS式をパースしたもの
   */
  const hydla::parser::SExpAST get_as_s_exp_parse_tree();

  /**
   * stringの送信
   * \param cmd REDUCEへ送信する文字列
   * \return 0
   */
  int send_string(const std::string cmd);

private:

  /**
   * getlineを行い、異常を見つけた場合throwする
   * \param cmd 呼び出し元の関数名, デバッグ用
   * \param line REDUCEへ送信する文字列
   * \return getlineの戻り値
   */
  std::istream& getline_with_throw(const std::string& cmd, std::string& line);

  boost::asio::ip::tcp::iostream s_;
  static const std::string end_of_redeval_;

};

} // namespace reduce
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REDUCE_LINK_TELNET_H_

