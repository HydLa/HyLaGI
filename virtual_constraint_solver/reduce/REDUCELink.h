#ifndef _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/error_code.hpp>

#define END_TST "endtst___"
#define END_TST_OFFNAT "endtst___$"

namespace hydla {
namespace vcs {
namespace reduce {

/*
 * REDUCE�T�[�o�Ƃ̐ڑ��N���C�A���g�A�T�[�o�ڑ���string�̑���M���s��
 */
class REDUCELink {

public:
  REDUCELink();

  ~REDUCELink();

  REDUCELink(const REDUCELink& old_cl);

  int read_until_redeval();
  int skip_until_redeval();


  std::string get_s_expr();

  /*
   * string�̑��M
   */
  int send_string(std::string cmd);

  std::string get_line();


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
   * ";end;"���I�[�s�Ɣ��肵�āA�t�@�C�����͂̃��O��ǂݔ�΂�
   */
  void skip_until_file_end();

  void func_test(const char* filename);

private:
  boost::asio::ip::tcp::iostream s;
  boost::asio::io_service io_service;

};

} // namespace reduce
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_
