#ifndef _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_

#include "../../parser/SExpParser.h"
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/error_code.hpp>
#include <sstream>
#include <stdexcept>

namespace hydla {
namespace vcs {
namespace reduce {

/**
 * REDUCEサーバとのsocket通信時のエラー、またはREDUCE側に論理的なエラーを表す
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
 */
class REDUCELink {
public:

  /**
   * localhost:1206 に接続する
   */
  REDUCELink();
  ~REDUCELink();

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
   * 受信した複数行のstringを結合してSExpParserを戻す
   * \return REDUCEから受け取るS式をパースしたもの
   */
  const hydla::parser::SExpParser get_as_s_exp_parser();

  /**
   * stringの送信
   * \param cmd REDUCEへ送信する文字列
   * \return 0
   */
  int send_string(const std::string cmd);

  /**
   * getlineを行い、異常を見つけた場合throwする
   * \param cmd 呼び出し元の関数名, デバッグ用
   * \param line REDUCEへ送信する文字列
   * \return getlineの戻り値
   */
  std::istream& getline_with_throw(const std::string& cmd, std::string& line);

  /**
   * 文字列中に含まれるqueryをカウントする
   * \param str 調べる文字列
   * \param query '(' または ')'
   * \return queryのカウント数
   */
  int count_char(const std::string str, const char query) const;

private:
  boost::asio::ip::tcp::iostream s_;
  static const std::string end_of_redeval_;
};

} // namespace reduce
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_
