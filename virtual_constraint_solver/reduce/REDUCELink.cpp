//コンパイル: g++ -o main main.cpp -lboost_system-gcc43-mt-1_38
///
#include <iostream>
#include <fstream>

#include "REDUCELink.h"


#include <string>
#include <boost/asio.hpp>


using namespace std;
using namespace boost::asio;

namespace hydla {
namespace vcs {
namespace reduce {

REDUCELink::REDUCELink(){
//  std::cout << "Begin REDUCELink::REDUCELink()" << std::endl;
  /*
  host_not_foundなどのエラー検知はtcp::iostreamでは出来ないみたい
  TODO tcp::socketに変更してhost_not_found判定
  */
  s.connect("localhost", "1206");
}

REDUCELink::~REDUCELink(){
//  std::cout << "Begin REDUCELink::~REDUCELink()" << std::endl;
}

REDUCELink::REDUCELink(const REDUCELink& old_cl){
  std::cout << "Begin REDUCELink::REDUCELink(const REDUCELink& old_cl)" << std::endl;
  //  this.s =
}

int REDUCELink::read_until_redeval(){
//  std::cout << "Begin REDUCELink::read_until_redeval" << std::endl;
  std::string line;
  while(getline(s, line)){
    cout << line << endl;
    if(line=="<redeval> end:") break;
  }
  return 0;
}

int REDUCELink::skip_until_redeval(){
//  std::cout << "Begin REDUCELink::skip_until_redeval" << std::endl;
  std::string line;
  while(getline(s, line)){
    if(line=="<redeval> end:") break;
  }
  return 0;
}

std::string REDUCELink::get_s_expr(){
//  std::cout << "Begin REDUCELink::get_s_expr" << std::endl;
  std::string line;
  getline(s, line);
  while(count_char(line, '(')!=count_char(line, ')')){
    std::string tmp;
    getline(s, tmp);
    // 次の行の文字列tmpを結合する前に、改行地点でスペースを入れておく
    line = line + " ";
    line = line + tmp;
  }

  return line;
}

int REDUCELink::send_string(string cmd){
//  std::cout << "Begin REDUCELink::send_string" << std::endl;
  s << cmd;

  return 0;
}

std::string REDUCELink::get_line(){
  std::string line;
  getline(s, line);

  return line;
}

/*
 * string中に指定したcharの数を数える
 */
int REDUCELink::count_char(std::string str, char query){
  int count = 0;
  string::size_type i = 0;
  while(true){
    i = str.find(query, i);
    if(i==std::string::npos) break;
    //    std::cout << "i[" << count << "]: " << i << std::endl;

    count++; i++;
  }

  return count;
}

/*
 * 端末上で"solve({x+y=2,3x+2y=5},{x,y});" などと入力
 * 返り値が無い入力をsendすると停止する
 */
void REDUCELink::scanf_test(){
  cout << "---- scanf phase ----" << endl;
  string scaned;
  while(true){
    cout << "cmd: ";
    cin >> scaned;
    if(scaned=="exit") break;
    send_string(scaned);
    get_line();
  }
}

/*
1: vars_:={y,yy,yyy,z,zz};
2: expr_:={zz - yy = 0, yyy + 10 = 0, yy = 0, True, z - y = 0, y - 10 = 0};
3: symbolic reval '(isconsistent vars_ expr_);
 */
std::string REDUCELink::isConsistent_test(){
  cout << "---- isConsistent_test() phase ----" << endl;
  std::string cmd, line;
  cmd = "vars_:={y,yy,yyy,z,zz};";
  send_string(cmd);
  if(getline(s, line)) std::cout << line;

  cmd = "expr_:={zz - yy = 0, yyy + 10 = 0, yy = 0, True, z - y = 0, y - 10 = 0};";
  send_string(cmd);
  if(getline(s, line)) std::cout << line;

  cmd = "symbolic reval '(isconsistent vars_ expr_);";
  send_string(cmd);
  if(getline(s, line)) std::cout << line;

  // "(.*"のものをとりあえず終了条件
  while(line[0]!='(' && line[line.length()-1]!=')'){
    getline(s, line);
    std::cout << line << std::endl;
    //      std::cout << line[line.length()-1] << std::endl;
  }

  //  string中の '('を数える関数
  //  int i = count_char(line, '(');





  while(count_char(line, '(')!=count_char(line, ')')){
    std::string tmp;
    getline(s, tmp);
    line = line + tmp;
  }


  cout << "---- exit isConsistent_test() phase ----" << endl;
  return line;
}


/*
 * ファイル入力のログを読み飛ばす
 */
void REDUCELink::skip_until_file_end(){
  std::cout << "Begin REDUCELink::skip_until_file_end()" << std::endl;
  std::string line[3];
  while(true){
    getline(s, line[0]);
    if(line[0]=="end;" && line[1]=="" && line[2]==";") break;
    line[2] = line[1]; line[1] = line[0];
  }
}


void REDUCELink::func_test(const char* filename){
  std::cout << "<main> func_test: " << filename << std::endl;

  std::ifstream ifs(filename);
  std::string cmd;
  //送信部
  while(getline(ifs, cmd)){
    s << cmd;
  }

  std::string line[3];
  //受信部
  while(true){
    getline(s, line[0]);
    cout << line[0] << endl;
    if(line[0]==END_TST) break;
    if(line[0]==END_TST_OFFNAT) break;
    if(line[0]=="end;" && line[1]=="" && line[2]==";") break;
    line[2] = line[1];
    line[1] = line[0];
  }
}

} // namespace reduce
} // namespace vcs
} // namespace hydla

