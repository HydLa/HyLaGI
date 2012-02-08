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
  if(!s_){ throw REDUCELinkError("fail to connect"); }
}

REDUCELink::~REDUCELink(){
  s_.close();
  if(!s_){ throw REDUCELinkError("fail to close"); }
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
    HYDLA_LOGGER_EXTERN(line);
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
    // ���̍s�̕�����tmp����������O�ɁA���s�n�_�ŃX�y�[�X�����Ă���
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
  if(!s_){ throw REDUCELinkError(cmd, "fail to getline"); }

  if(line.find("***")!=std::string::npos){ // "***** 1 invalid as list"�̂悤�Ș_���G���[�o�͂̔���
    throw REDUCELinkError(cmd, "[ " + line + " ]");
  }

  return is;
}

/*
 * string���Ɏw�肵��char�̐��𐔂���
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

