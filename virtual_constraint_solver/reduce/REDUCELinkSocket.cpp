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

#define REDUCE_EOF "end:" // �I������

using namespace boost::asio::ip;

REDUCELinkSocket::REDUCELinkSocket(): socket_(io_service_), count_(0) {
  connect();
}

REDUCELinkSocket::~REDUCELinkSocket() {}

void REDUCELinkSocket::connect(){
  try {
    //���O����
    tcp::resolver resolver(io_service_);
    tcp::resolver::query query("localhost", "1206");

    //IPv4, IPv6��endpoint�擾
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    tcp::resolver::iterator end;

    //socket�̐ڑ�
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
      boost::array<char, 128> buf;  //boost::array�ւ̎�M
      boost::system::error_code error;

      size_t len = socket_.read_some(boost::asio::buffer(buf), error);

       std::cout << std::setw(3) << "[" << count_ << "] ";
       std::cout.write(buf.data(), len);
       std::cout << "[/]" << std::endl;

      std::string tmp = std::string(buf.data()).substr(0,len); //len�O��buf���o��

      //�ʐM�I��
      if(tmp.find(REDUCE_EOF)!=std::string::npos){
        //<s>��̉��s�����o��
        // std::cout << "[buf string]: " << tmp << std::endl;

        int index_eof = tmp.find(REDUCE_EOF);
        int index_feed_line = tmp.find('\n',index_eof+1); // ���s�ʒu��T��

        tmp = tmp.substr(index_feed_line+1);
        // std::cout << "[substr string]: " << tmp << std::endl;

        last_line_ = tmp;

        break; // �ڑ��ؒf
      }else if(error){
        throw boost::system::system_error(error); // ���炩�̃G���[
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
      boost::array<char, 128> buf;  //boost::array�ւ̎�M
      boost::system::error_code error;

      tl_.get_dtime(TimeLogger::begin_read_some);
      size_t len = socket_.read_some(boost::asio::buffer(buf), error);
      tl_.get_dtime(TimeLogger::end_read_some);

      std::string tmp = std::string(buf.data()).substr(0,len); //len�O��buf���o��

      tl_.get_dtime(TimeLogger::begin_if);
      //�ʐM�I��
      if(tmp.find(REDUCE_EOF)!=std::string::npos){
        //<s>��̉��s�����o��
        // std::cout << "[buf string]: " << tmp << std::endl;

        int index_eof = tmp.find(REDUCE_EOF);
        int index_feed_line = tmp.find('\n',index_eof+1); // ���s�ʒu��T��

        tmp = tmp.substr(index_feed_line+1);
        // std::cout << "[substr string]: " << tmp << std::endl;

        last_line_ = tmp;

        break; // �ڑ��ؒf
      }else if(error){
        throw boost::system::system_error(error); // ���炩�̃G���[
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

      // ���s�R�[�h���擾����܂�read
      while(line.find('\n')==std::string::npos){
        len = socket_.read_some(boost::asio::buffer(buf), error);
        line = line + std::string(buf.data()).substr(0,len); // �ԂɃX�y�[�X����Ă͂����Ȃ���
        if(error){
          throw boost::system::system_error(error); // ���炩�̃G���[
        }
      }
      // ���s�ȉ����폜����
      line = line.substr(0, (line.find('\n')+1));

      return line;
    }

    while(count_char(line, '(')!=count_char(line, ')') || line == ""){
      // std::cout << "['(' num]: " << count_char(line, '(') << "[')' num]:" << count_char(line, ')') << std::endl;

      len = socket_.read_some(boost::asio::buffer(buf), error);
      line = line + std::string(buf.data()).substr(0,len); // �ԂɃX�y�[�X����Ă͂����Ȃ���

      // std::cout << "[line]: " << line << std::endl;

      // std::cout << "[line string]: " << line << std::endl;

      // �ʐM�I������
      if(error){
        throw boost::system::system_error(error); // ���炩�̃G���[
      }
    }
    //  ���s�R�[�h��' '�ɒu��
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
    line = line.substr(line.find('('), (line.rfind(')')-line.find('(')+1)); // S���̌����J�b�g // �O���J�b�g

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

