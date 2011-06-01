///

#ifndef _INCLUDED_HYDLA_VCS_REDUCE_CLIENT_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_CLIENT_H_

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/error_code.hpp>

#define END_TST "endtst___"
#define END_TST_OFFNAT "endtst___$"

namespace hydla {
namespace vcs {
namespace reduce {

class REDUCEClient {
  //TCP/IPのREDUCEとのインタフェース
  //TODO: REDUCEが起動してないなどで接続できない場合の分岐


public:
  REDUCEClient();
  ~REDUCEClient();


  int send_to_reduce(std::string cmd);

  //TODO: recvが取得出来ない場合のタイムアウト処理
  //TODO: 終了条件の取得
  int recv_from_reduce();

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
  {";", "", "end;"} の順に文字列を取得したことを終了判定として、終了判定までgetline(s, line)をする
  TODO: write "endoffile___"; をファイル側に仕込み終了判定とする
*/
  void file_input_test();


  void func_test(const char* filename);

private:
  boost::asio::ip::tcp::iostream s;
  boost::asio::io_service io_service;
};

} // namespace reduce
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REDUCE_CLIENT_H_
