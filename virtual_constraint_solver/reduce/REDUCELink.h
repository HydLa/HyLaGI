#ifndef _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_

#include <stdexcept>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/error_code.hpp>

#define END_TST "endtst___"
#define END_TST_OFFNAT "endtst___$"

namespace hydla {
namespace vcs {
namespace reduce {

class REDUCELinkError : public std::runtime_error {
public:
  REDUCELinkError(const std::string& msg, const std::string& line = "") :
    std::runtime_error(init(msg,line))
  {}

private:
  std::string init(const std::string& msg, const std::string& line ="")
  {
    std::stringstream s;
    s << "reducelink error: " << msg << " : " << line;
    return s.str();
  }
};

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
   * ";end;"���I�[�s�Ɣ��肵�āA�t�@�C�����͂̃��O��ǂݔ�΂�
   */
  void skip_until_file_end();

private:
  boost::asio::ip::tcp::iostream s_;

};

} // namespace reduce
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_
