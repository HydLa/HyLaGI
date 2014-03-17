#include "Logger.h"
#include "REDUCELinkTelnet.h"
#include <boost/asio.hpp>
#include <fstream>
#include <iostream>
#include <string>

namespace hydla {
namespace backend {
namespace reduce {

const std::string REDUCELinkTelnet::end_of_redeval_ = "<redeval> end:";

REDUCELinkTelnet::REDUCELinkTelnet(const simulator::Opts &opts):REDUCELink(){
  //  s_.connect("localhost", "1206");
  s_.connect("127.0.0.1", "1206");
  if(!s_){ throw REDUCELinkError("fail to connect"); }
  init_opts(opts);
}

REDUCELinkTelnet::~REDUCELinkTelnet(){
  s_.close();
  if(!s_){ throw REDUCELinkError("fail to close"); }
}

int REDUCELinkTelnet::skip_until_redeval(){
  std::string line;
  while(getline_with_throw("skip_until_redeval", line)){
    HYDLA_LOGGER_DEBUG(line);
    if(line==end_of_redeval_){
      break;
    }
  }
  return 0;
}

std::string REDUCELinkTelnet::get_s_expr(){
  std::string line;
  getline_with_throw("get_s_expr", line);
  while(count_char(line, '(')!=count_char(line, ')')){
    std::string tmp;
    getline_with_throw("get_s_expr", tmp);
    // 次の行の文字列tmpを結合する前に、改行地点でスペースを入れておく
    line = line + " ";
    line = line + tmp;
  }

  HYDLA_LOGGER_DEBUG("get s_expr: ", line);
  return line;
}

const hydla::parser::SExpAST REDUCELinkTelnet::get_as_s_exp_parse_tree(){
  return hydla::parser::SExpAST(get_s_expr());
}

int REDUCELinkTelnet::send_string(const std::string cmd){
  s_ << cmd;
  if(!s_){ throw REDUCELinkError("fail to send_string"); }

  return 0;
}
std::istream& REDUCELinkTelnet::getline_with_throw(const std::string& cmd, std::string& line){
  std::istream& is = getline(s_, line);
  if(!s_){ throw REDUCELinkError(cmd, "fail to getline"); }

  if(line.find("***")!=std::string::npos){ // "***** 1 invalid as list"のような論理エラー出力の判定
    throw REDUCELinkError(cmd, "[ " + line + " ]");
  }else if(line.find("Declare")!=std::string::npos){ // 未定義の関数の入力の判定
    throw REDUCELinkError(cmd, "[ " + line + " ]");
  }

  return is;
}

} // namespace reduce
} // namespace backend
} // namespace hydla
