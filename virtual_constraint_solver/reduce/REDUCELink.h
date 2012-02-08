#ifndef _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_

#include <stdexcept>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/error_code.hpp>

namespace hydla {
namespace vcs {
namespace reduce {

/*
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

/*
 * REDUCEサーバとの接続クライアント、サーバ接続とstringの送受信を行う
 */
class REDUCELink {
public:
  REDUCELink();
  ~REDUCELink();

  /*
   * end_of_redeval_行まで文字列をgetlineする
   * skip_until_redevalを推奨
   */
  int read_until_redeval();
  /*
   * end_of_redeval_行まで文字列をgetlineする
   */
  int skip_until_redeval();
  /*
   * 受信した複数行のstringを結合して破損のないLisp式を作る
   */
  std::string get_s_expr();
  /*
   * stringの送信
   */
  int send_string(const std::string cmd);
  /*
   * getlineを行い、またこの時にs_.badや受信文字列がエラーを見つけた場合REDUCELinkErrorをthrowする
   */
  std::istream& getline_with_throw(const std::string& cmd, std::string& line);
  int count_char(const std::string str, const char query) const;

private:
  boost::asio::ip::tcp::iostream s_;
  static const std::string end_of_redeval_;
};

} // namespace reduce
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_
