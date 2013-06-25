#ifndef _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_

#include "../../parser/SExpParser.h"
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/error_code.hpp>
#include <sstream>
#include <stdexcept>

namespace hydla {
namespace vcs {
namespace reduce {

/**
 * REDUCE�T�[�o�Ƃ�socket�ʐM���̃G���[�A�܂���REDUCE���ɘ_���I�ȃG���[��\��
 */
class REDUCELinkError : public std::runtime_error {
public:
  REDUCELinkError(const std::string& msg, const std::string& line = "") :
    std::runtime_error(init(msg,line))
  {}

private:
  std::string init(const std::string& msg, const std::string& line)
  {
    std::stringstream s;
    std::string comma = (line!="") ? " : " : "";
    s << "reducelink error: " << msg << comma << line;
    return s.str();
  }
};

/**
 * REDUCE�T�[�o�Ƃ̐ڑ��N���C�A���g�A�T�[�o�ڑ���string�̑���M���s��
 */
class REDUCELink {
public:

  /**
   * localhost:1206 �ɐڑ�����
   */
  REDUCELink();
  ~REDUCELink();

  /**
   * end_of_redeval_�s�܂ŕ������getline����
   * skip_until_redeval�𐄏�
   * \return 0
   */
  int read_until_redeval();

  /**
   * end_of_redeval_�s�܂ŕ������getline����
   * \return 0
   */
  int skip_until_redeval();

  /**
   * ��M���������s��string���������Ĕj���̂Ȃ�Lisp����߂�
   * \return REDUCE����󂯎��S��
   */
  std::string get_s_expr();

  /**
   * ��M���������s��string����������SExpParser��߂�
   * \return REDUCE����󂯎��S�����p�[�X��������
   */
  const hydla::parser::SExpParser get_as_s_exp_parser();

  /**
   * string�̑��M
   * \param cmd REDUCE�֑��M���镶����
   * \return 0
   */
  int send_string(const std::string cmd);

  /**
   * getline���s���A�ُ���������ꍇthrow����
   * \param cmd �Ăяo�����̊֐���, �f�o�b�O�p
   * \param line REDUCE�֑��M���镶����
   * \return getline�̖߂�l
   */
  std::istream& getline_with_throw(const std::string& cmd, std::string& line);

  /**
   * �����񒆂Ɋ܂܂��query���J�E���g����
   * \param str ���ׂ镶����
   * \param query '(' �܂��� ')'
   * \return query�̃J�E���g��
   */
  int count_char(const std::string str, const char query) const;

private:
  boost::asio::ip::tcp::iostream s_;
  static const std::string end_of_redeval_;
};

} // namespace reduce
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REDUCE_LINK_H_
