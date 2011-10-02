//コンパイル: g++ -o main main.cpp -lboost_system-gcc43-mt-1_38
///
#include <iostream>
#include <fstream>

#include "REDUCELink.h"
#include "Logger.h"

#include <string>
#include <boost/asio.hpp>


using namespace std;
using namespace boost::asio;

namespace hydla {
namespace vcs {
namespace reduce {

REDUCELink::REDUCELink(){
  s_.connect("localhost", "1206");

  if(!s_){
	  HYDLA_LOGGER_ERROR("REDUCELink: fail to connect reduce server");
	  assert(0);
  }
}

REDUCELink::~REDUCELink(){
//  std::cout << "Begin REDUCELink::~REDUCELink()" << std::endl;
}

REDUCELink::REDUCELink(const REDUCELink& old_cl){
  std::cout << "Begin REDUCELink::REDUCELink(const REDUCELink& old_cl)" << std::endl;
  //  this.s_ =
}

int REDUCELink::read_until_redeval(){
//  std::cout << "Begin REDUCELink::read_until_redeval" << std::endl;
  std::string line;
  while(getline(s_, line)){
    cout << line << endl;
    if(line=="<redeval> end:") break;
  }
  return 0;
}

int REDUCELink::skip_until_redeval(){
//  std::cout << "Begin REDUCELink::skip_until_redeval" << std::endl;
  std::string line;
  while(getline(s_, line)){
    if(line=="<redeval> end:") break;
  }
  return 0;
}

std::string REDUCELink::get_s_expr(){
//  std::cout << "Begin REDUCELink::get_s_expr" << std::endl;
  std::string line;
  getline(s_, line);
  while(count_char(line, '(')!=count_char(line, ')')){
    std::string tmp;
    getline(s_, tmp);
    // 次の行の文字列tmpを結合する前に、改行地点でスペースを入れておく
    line = line + " ";
    line = line + tmp;
  }

  return line;
}

int REDUCELink::send_string(string cmd){
//  std::cout << "Begin REDUCELink::send_string" << std::endl;
  s_ << cmd;

  return 0;
}

std::string REDUCELink::get_line(){
  std::string line;
  getline(s_, line);

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
 * ファイル入力のログを読み飛ばす
 */
void REDUCELink::skip_until_file_end(){
  std::cout << "Begin REDUCELink::skip_until_file_end()" << std::endl;
  std::string line[3];
  while(true){
    getline(s_, line[0]);
    if(line[0]=="end;" && line[1]=="" && line[2]==";") break;
    line[2] = line[1]; line[1] = line[0];
  }
}


} // namespace reduce
} // namespace vcs
} // namespace hydla

