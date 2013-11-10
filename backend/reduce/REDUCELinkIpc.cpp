#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) 
#else

#include "Logger.h"
#include "REDUCELinkIpc.h"
#include <cassert>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace hydla {
namespace backend {
namespace reduce {

const std::string REDUCELinkIpc::end_of_redeval_ = "<redeval> end:";


REDUCELinkIpc::REDUCELinkIpc(const simulator::Opts &opts):REDUCELink(){
  initProcess();
  init_opts(opts);
} 
 
REDUCELinkIpc::~REDUCELinkIpc(){
  put_string(";bye;");
  // waitpid(pid_, NULL, 0);
}

int REDUCELinkIpc::read_until_redeval(){
  std::string line = getline_with_throw("read_until_redeval");
  while(line.find(end_of_redeval_) == std::string::npos){
    std::cout << line << std::endl;
    line = getline_with_throw("read_until_redeval");
  }
  return 0;
}

int REDUCELinkIpc::skip_until_redeval(){
  std::string line = getline_with_throw("skip_until_redeval");
  while(line.find(end_of_redeval_) == std::string::npos){
    HYDLA_LOGGER_EXTERN(line);
    line = getline_with_throw("skip_until_redeval");
  }
  return 0; 
}

std::string REDUCELinkIpc::get_s_expr(){
  std::string line = getline_with_throw("get_s_expr");
  while(count_char(line, '(')!=count_char(line, ')')){
    std::string tmp = getline_with_throw("get_s_expr");
    // 次の行の文字列tmpを結合する前に、改行地点でスペースを入れておく
    line += " ";
    line += tmp;
  }

  HYDLA_LOGGER_BACKEND("get s_expr: ", line);
  return line;
}

const hydla::parser::SExpParseTree REDUCELinkIpc::get_as_s_exp_parse_tree(){
  return hydla::parser::SExpParseTree(get_s_expr());
}

void REDUCELinkIpc::send_string_to_reduce(const char *cmd, int len){
  if(write(writefd_, cmd, len + 1) == -1){
    throw REDUCELinkError("send_string_to_reduce()", "write");
  }
  if(write(writefd_, "\n", 1) == -1){
    throw REDUCELinkError("send_string_to_reduce()", "write");
  }
}

void REDUCELinkIpc::send_string_to_reduce(const char *cmd){
  send_string_to_reduce(cmd, strlen(cmd));
}

std::string REDUCELinkIpc::getline_with_throw(const std::string& cmd){
  std::string line = getline();

  if(line.find("***")!=std::string::npos){ // "***** 1 invalid as list"のような論理エラー出力の判定
    throw REDUCELinkError(cmd, "[ " + line + " ]");
  }else if(line.find("Declare")!=std::string::npos){ // 未定義の関数の入力の判定
    throw REDUCELinkError(cmd, "[ " + line + " ]");
  }

  return line;
}

// prev_line_には前回getlineした時に最後に取得したbuffが代入されているものと仮定する
std::string REDUCELinkIpc::getline(){
  std::string null_line = "";

  // prev_line_の処理
  const size_t prev_pos = prev_line_.find('\n');
  if(prev_pos != std::string::npos){
    std::string ret = prev_line_.substr(0, (int)prev_pos);
    prev_line_ = prev_line_.substr(prev_pos + 1);
    return ret;
  }

  std::string ret = prev_line_;
  ssize_t n;
  char buff[MAXLINE];
  // 新しくパイプからread
  do{
    n = read(readfd_, buff, MAXLINE);
    if(n < 0) throw REDUCELinkError("getline()", "fail to read");

    const int buff_pos = std::find(buff, buff+n, '\n') - buff;
    if(buff_pos < n){ // buff内に'\n'が存在する場合
      prev_line_ = (buff_pos + 1 != n) ? 
        null_line.append(buff, buff_pos + 1, n - buff_pos - 1) :
        "";
      return ret.append(buff, buff_pos); // '\n'は除く
    }

    ret += null_line.append(buff, n);
  }while(n == MAXLINE);

  prev_line_ = "";
  return ret; 
}


void REDUCELinkIpc::initProcess(){
  int pipe1[2]; //client => server
  int pipe2[2]; //server => client

  if(pipe(pipe1) == -1) throw REDUCELinkError("initProcess()", "fail to pipe");
  if(pipe(pipe2) == -1) throw REDUCELinkError("initProcess()", "fail to pipe");

  pid_ = fork();
  if(pid_ < 0) throw REDUCELinkError("initProcess()", "fail to fork");

  if(pid_ == 0){ 
    // child
    close(pipe1[1]);
    close(pipe2[0]);

    // stdin
    dup2(pipe1[0], STDIN_FILENO);
    close(pipe1[0]);

    // stdout
    dup2(pipe2[1], STDOUT_FILENO);
    close(pipe2[1]);
    // reduceプロセスを実行する
    if(execlp("reduce", "reduce", NULL) == -1) throw REDUCELinkError("initProcess()", "fail to execlp");
  }

  // parent
  close(pipe1[0]);
  close(pipe2[1]);

  readfd_ = pipe2[0];
  writefd_ = pipe1[1];
}

} // namespace reduce
} // namespace backend
} // namespace hydla

#endif // defined(_MSC_VER) || defined(_MSC_EXTENSIONS) 

