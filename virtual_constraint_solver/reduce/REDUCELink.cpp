#include <iostream>
#include <fstream>
#include <string>

#include "REDUCELink.h"
#include "Logger.h"

#include <boost/asio.hpp>

using namespace boost::asio;

namespace hydla {
namespace vcs {
namespace reduce {

const std::string end_of_redeval_ = "<redeval> end:";

REDUCELink::REDUCELink(){
  s_.connect("localhost", "1206");
  if(!s_){
        throw REDUCELinkError("fail to connect");
// TODO        throw REDUCELinkError("fail to connect", s_.error().message());

  }
}

REDUCELink::~REDUCELink(){
  s_.close();
}

REDUCELink::REDUCELink(const REDUCELink& old_cl){
  std::cout << "Begin REDUCELink::REDUCELink(const REDUCELink& old_cl)" << std::endl;
}

int REDUCELink::read_until_redeval(){
  std::string line;
  while(getline(s_, line)){
    std::cout << line << std::endl;
    if(line==end_of_redeval_){
      break;
    }else if(line.substr(0,std::min((int)line.size(),3))=="***"){ // "***** 1 invalid as list"のようなエラー判定
      throw REDUCELinkError("read_until_redeval", line);
      break;
    }
  }
  return 0;
}

int REDUCELink::skip_until_redeval(){
  std::string line;
  while(getline(s_, line)){
    HYDLA_LOGGER_DEBUG(line);
    if(line==end_of_redeval_){
      break;
    }else if(line.substr(0,std::min((int)line.size(),3))=="***"){ // "***** 1 invalid as list"のようなエラー判定
      throw REDUCELinkError("skip_until_redeval", line);
      break;
    }
  }
  return 0;
}

std::string REDUCELink::get_s_expr(){
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

int REDUCELink::send_string(std::string cmd){
  s_ << cmd;

  return 0;
}

/*
 * string中に指定したcharの数を数える
 */
int REDUCELink::count_char(std::string str, char query){
  int count = 0;
  std::string::size_type i = 0;
  while(true){
    i = str.find(query, i);
    if(i==std::string::npos) break;

    count++; i++;
  }

  return count;
}

} // namespace reduce
} // namespace vcs
} // namespace hydla

