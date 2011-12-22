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

const std::string REDUCELink::end_of_redeval_ = "<redeval> end:";

REDUCELink::REDUCELink(){
  s_.connect("localhost", "1206");
  // TODO        throw REDUCELinkError("fail to connect", s_.error().message());
  if(!s_){ throw REDUCELinkError("fail to connect"); }
}

REDUCELink::~REDUCELink(){
  s_.close();
}

REDUCELink::REDUCELink(const REDUCELink& old_cl){
  std::cout << "Begin REDUCELink::REDUCELink(const REDUCELink& old_cl)" << std::endl;
}

int REDUCELink::read_until_redeval(){
  std::string line;
  while(getline_with_throw("read_until_redeval", line)){
    std::cout << line << std::endl;
    if(line==end_of_redeval_){
      break;
    }
  }
  return 0;
}

int REDUCELink::skip_until_redeval(){
  std::string line;
  while(getline_with_throw("skip_until_redeval", line)){
    HYDLA_LOGGER_DEBUG(line);
    if(line==end_of_redeval_){
      break;
    }
  }
  return 0;
}

std::string REDUCELink::get_s_expr(){
  std::string line;
  getline_with_throw("get_s_expr", line);
  while(count_char(line, '(')!=count_char(line, ')')){
    std::string tmp;
    getline_with_throw("get_s_expr", tmp);
    // 次の行の文字列tmpを結合する前に、改行地点でスペースを入れておく
    line = line + " ";
    line = line + tmp;
  }

  return line;
}

int REDUCELink::send_string(const std::string cmd){
  s_ << cmd;
  if(!s_){ throw REDUCELinkError("fail to send_string"); }

  return 0;
}
std::istream& REDUCELink::getline_with_throw(const std::string& cmd, std::string& line){
  std::istream& is = getline(s_, line);

  if(!s_){
    throw REDUCELinkError("fail to　" + cmd);
  }else if(line.substr(0,std::min((int)line.size(),3))=="***"){ // "***** 1 invalid as list"のような論理エラー出力の判定
    throw REDUCELinkError(cmd, line);
  }

  return is;
}

/*
 * string中に指定したcharの数を数える
 */
int REDUCELink::count_char(const std::string str, const char query) const {
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

