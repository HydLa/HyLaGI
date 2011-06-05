#ifndef _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/error_code.hpp>

#define END_TST "endtst___"
#define END_TST_OFFNAT "endtst___$"

namespace hydla {
namespace vcs {
namespace reduce {

/*
 * REDUCEサーバとの接続クライアント、サーバ接続とstringの送受信を行う
 */
class REDUCELink {

public:
  REDUCELink();

  ~REDUCELink();

  REDUCELink(const REDUCELink& old_cl);

  int read_until_redeval();
  int skip_until_redeval();


  std::string get_s_expr();

  /*
   * stringの送信
   */
  int send_string(std::string cmd);

  std::string get_line();


  int count_char(std::string str, char query);

  /*
   * 端末上で"solve({x+y=2,3x+2y=5},{x,y});" などと入力
   */
  void scanf_test();

  /*
     Goal 以下の3命令を送り計算を成功させ答えをrecv
     recvのwhileの抜ける条件を、line[0]=='('とする
     '('の数と')'の数を数え、それぞれが一致するテキストを返り値とする


    1: vars_:={y,yy,yyy,z,zz};
    2: expr_:={zz - yy = 0, yyy + 10 = 0, yy = 0, True, z - y = 0, y - 10 = 0};
    3: symbolic reval '(isconsistent vars_ expr_);
   */
  std::string isConsistent_test();


  /*
   * ";end;"を終端行と判定して、ファイル入力のログを読み飛ばす
   */
  void skip_until_file_end();

  void func_test(const char* filename);

private:
  boost::asio::ip::tcp::iostream s;
  boost::asio::io_service io_service;

};

} // namespace reduce
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_
