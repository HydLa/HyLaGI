/*
 * REDUCELink_skelton.h
 *
 *  Created on: 2011/11/11
 *      Author: yysaki
 */

#ifndef _INCLUDED_HYDLA_VCS_REDUCE_LINK_SOCKET_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_LINK_SOCKET_H_

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/error_code.hpp>

namespace hydla {
namespace vcs {
namespace reduce {
class REDUCELinkSocket {
public:
  REDUCELinkSocket();
  ~REDUCELinkSocket();
//  REDUCELinkSocket(const REDUCELinkSocket& old_cl);

  void connect();
  /*
   * stringÇÃëóêM
   */
  int send_string(std::string cmd);

  int read_until_redeval();
  int skip_until_redeval();

  std::string get_s_expr();
  int count_char(std::string str, char query);

private:
  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::socket socket_;

  int count_;
  std::string last_line_;

};

} // namespace reduce
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REDUCE_LINK_SOCKET_H_
