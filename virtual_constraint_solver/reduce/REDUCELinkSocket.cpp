#include <iostream>
#include <fstream>
#include <string>
#include <boost/asio.hpp>
#include <cassert>

#include "REDUCELinkSocket.h"
#include "Logger.h"


namespace hydla {
namespace vcs {
namespace reduce {

#define REDUCE_EOF "end:" // 終了条件

using namespace boost::asio::ip;

REDUCELinkSocket::REDUCELinkSocket(): socket_(io_service_), count_(0) {
  connect();
}

REDUCELinkSocket::~REDUCELinkSocket() {}

void REDUCELinkSocket::connect(){
  try {
    //名前解決
    tcp::resolver resolver(io_service_);
    tcp::resolver::query query("localhost", "1206");

    //IPv4, IPv6のendpoint取得
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    tcp::resolver::iterator end;

    //socketの接続
    boost::system::error_code error = boost::asio::error::host_not_found;
    while(error && endpoint_iterator!=end){
      socket_.close();
      socket_.connect(*endpoint_iterator++, error);
    }
    if(error){
      throw boost::system::system_error(error);
    }
  }catch(std::exception& e){
    std::cerr << e.what() << std::endl;
  }
}

int REDUCELinkSocket::send_string(std::string cmd){
  try {
    //ignored_error
    boost::system::error_code ignored_error;
    boost::asio::write(socket_, boost::asio::buffer(cmd), boost::asio::transfer_all(), ignored_error);
  }catch(std::exception& e){
    std::cerr << e.what() << std::endl;
  }

  return 0;
}

int REDUCELinkSocket::read_until_redeval(){
  try{
    for(;;){
      ++count_;
      boost::array<char, 128> buf;  //boost::arrayへの受信
      boost::system::error_code error;

      size_t len = socket_.read_some(boost::asio::buffer(buf), error);

       std::cout << std::setw(3) << "[" << count_ << "] ";
       std::cout.write(buf.data(), len);
       std::cout << "[/]" << std::endl;

      std::string tmp = std::string(buf.data()).substr(0,len); //len前のbuf取り出し

      //通信終了
      if(tmp.find(REDUCE_EOF)!=std::string::npos){
        //<s>後の改行を取り出す
        // std::cout << "[buf string]: " << tmp << std::endl;

        int index_eof = tmp.find(REDUCE_EOF);
        int index_feed_line = tmp.find('\n',index_eof+1); // 改行位置を探す

        tmp = tmp.substr(index_feed_line+1);
        // std::cout << "[substr string]: " << tmp << std::endl;

        last_line_ = tmp;

        break; // 接続切断
      }else if(error){
        throw boost::system::system_error(error); // 何らかのエラー
      }
    }
  }catch(std::exception& e){
    std::cerr << e.what() << std::endl;
  }

  return 0;
}

int REDUCELinkSocket::skip_until_redeval(){
  tl_.get_dtime(TimeLogger::begin);
  try{
    for(;;){
      ++count_;
      boost::array<char, 128> buf;  //boost::arrayへの受信
      boost::system::error_code error;

      tl_.get_dtime(TimeLogger::begin_read_some);
      size_t len = socket_.read_some(boost::asio::buffer(buf), error);
      tl_.get_dtime(TimeLogger::end_read_some);

      std::string tmp = std::string(buf.data()).substr(0,len); //len前のbuf取り出し

      tl_.get_dtime(TimeLogger::begin_if);
      //通信終了
      if(tmp.find(REDUCE_EOF)!=std::string::npos){
        //<s>後の改行を取り出す
        // std::cout << "[buf string]: " << tmp << std::endl;

        int index_eof = tmp.find(REDUCE_EOF);
        int index_feed_line = tmp.find('\n',index_eof+1); // 改行位置を探す

        tmp = tmp.substr(index_feed_line+1);
        // std::cout << "[substr string]: " << tmp << std::endl;

        last_line_ = tmp;

        break; // 接続切断
      }else if(error){
        throw boost::system::system_error(error); // 何らかのエラー
      }
      tl_.get_dtime(TimeLogger::end_if);
    }
  }catch(std::exception& e){
    std::cerr << e.what() << std::endl;
  }
  tl_.get_dtime(TimeLogger::end);

  return 0;
}
std::string REDUCELinkSocket::get_s_expr(){
  try{
    boost::array<char, 128> buf;
    boost::system::error_code error;
    size_t len;

    // std::cout << "[Begin REDUCELinkSocket::get_s_expr]" << std::endl;
    // std::cout << "[last_line_ string]: " << last_line_ << std::endl;
    std::string line = last_line_;
    ++count_;

    if(count_char(line, '(')==0){
      // std::cout << "[[atom case]]" << std::endl;

      // 改行コードを取得するまでread
      while(line.find('\n')==std::string::npos){
        len = socket_.read_some(boost::asio::buffer(buf), error);
        line = line + std::string(buf.data()).substr(0,len); // 間にスペース入れてはいけない所
        if(error){
          throw boost::system::system_error(error); // 何らかのエラー
        }
      }
      // 改行以下を削除する
      line = line.substr(0, (line.find('\n')+1));

      return line;
    }

    while(count_char(line, '(')!=count_char(line, ')') || line == ""){
      // std::cout << "['(' num]: " << count_char(line, '(') << "[')' num]:" << count_char(line, ')') << std::endl;

      len = socket_.read_some(boost::asio::buffer(buf), error);
      line = line + std::string(buf.data()).substr(0,len); // 間にスペース入れてはいけない所

      // std::cout << "[line]: " << line << std::endl;

      // std::cout << "[line string]: " << line << std::endl;

      // 通信終了条件
      if(error){
        throw boost::system::system_error(error); // 何らかのエラー
      }
    }
    //  改行コードを' 'に置換
    std::string::iterator it = line.begin();
    while(it!=line.end()){
      if(*it=='\n'){
        //      it = line.erase(it);
        *it = ' '; ++it;
      }else{
        ++it;
      }
    }
    // std::cout << "line.find('('): " << line.find('(') << ", (line.rfind(')')): " << (line.rfind(')')) << std::endl;
    // std::cout << "line.size(): " << line.size() << std::endl;
    line = line.substr(line.find('('), (line.rfind(')')-line.find('(')+1)); // S式の後ろをカット // 前もカット

    // std::cout << "[s_expr string]" << line << "[/]" << std::endl;

    return line;
  }catch(std::exception& e){
    std::cerr << e.what() << std::endl;
  }
}

int REDUCELinkSocket::count_char(std::string str, char query){
  int count = 0;
  std::string::size_type i = 0;
  while(true){
    i = str.find(query, i);
    if(i==std::string::npos) break;
    //    // std::cout << "i[" << count << "]: " << i << std::endl;

    count++; i++;
  }

  return count;
}

} // namespace reduce
} // namespace vcs
} // namespace hydla

