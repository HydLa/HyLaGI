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
  //TCP/IP��REDUCE�Ƃ̃C���^�t�F�[�X
  //TODO: REDUCE���N�����ĂȂ��ȂǂŐڑ��ł��Ȃ��ꍇ�̕���


public:
  REDUCEClient();
  ~REDUCEClient();


  int send_to_reduce(std::string cmd);

  //TODO: recv���擾�o���Ȃ��ꍇ�̃^�C���A�E�g����
  //TODO: �I�������̎擾
  int recv_from_reduce();

  int count_char(std::string str, char query);

  /*
  * �[�����"solve({x+y=2,3x+2y=5},{x,y});" �ȂǂƓ���
  */
  void scanf_test();

/*
Goal �ȉ���3���߂𑗂�v�Z�𐬌�����������recv
     recv��while�̔�����������Aline[0]=='('�Ƃ���
     '('�̐���')'�̐��𐔂��A���ꂼ�ꂪ��v����e�L�X�g��Ԃ�l�Ƃ���


1: vars_:={y,yy,yyy,z,zz};
2: expr_:={zz - yy = 0, yyy + 10 = 0, yy = 0, True, z - y = 0, y - 10 = 0};
3: symbolic reval '(isconsistent vars_ expr_);
*/
  std::string isConsistent_test();


/*
  {";", "", "end;"} �̏��ɕ�������擾�������Ƃ��I������Ƃ��āA�I������܂�getline(s, line)������
  TODO: write "endoffile___"; ���t�@�C�����Ɏd���ݏI������Ƃ���
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
