#ifndef _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_H_
#define _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_H_

#include <boost/function.hpp>
#include <list>

#include "ModuleSet.h"

namespace hydla {
namespace ch {

class ModuleSetContainer {
public:

  typedef std::set<module_set_sptr> module_set_set_t;
  typedef std::vector<module_set_sptr> module_set_list_t;

  ModuleSetContainer() 
  {}
  ModuleSetContainer(module_set_sptr m);
  
  virtual ~ModuleSetContainer()
  {}

  /**
   * �W���̏W���̃_���v
   */
  virtual std::ostream& dump(std::ostream& s) const = 0;

  /**
   * �v�f���ő�ł��郂�W���[���W���𓾂�
   */
  virtual module_set_sptr get_max_module_set() const;

  /**
   * ���݂̒��ڃm�[�h�𓾂�
   */
  virtual module_set_sptr get_reverse_module_set() const;

  /**
   * ���݂̒��ڃm�[�h�𓾂�
   */
  virtual module_set_sptr get_module_set() const;
  
  /**
   * �T�����ׂ����W���[���W���̏W���𓾂�
   */
  module_set_list_t get_ms_to_visit() const;

  /**
   * ���݂̃��W���[���W��������⃂�W���[���W���̏W�������菜��
   */
  virtual bool eliminate_current_node();

  /**
   * ���ɒT�����ׂ����W���[���W���ɐi��
   * ���݂��Ȃ����false��Ԃ�.
   * �T���͏��������W���[���W������J�n����.
   */
  virtual bool reverse_go_next();
  
  /**
   * ���ɒT�����ׂ����W���[���W���ɐi��
   * ���݂��Ȃ����false��Ԃ��D
   */
  virtual bool go_next();
  
  module_set_list_t get_full_ms_list() const;
  
  /**
   * ���̃m�[�h��T���ς݂Ƃ��C�ȍ~�T�����Ȃ��悤�ɂ���
   */
  virtual void mark_r_current_node();

  /**
   * ���̃m�[�h��T���ς݂Ƃ��C�ȍ~�T�����Ȃ��悤�ɂ���
   */
  virtual void mark_current_node();

  /**
   * ���̃m�[�h�Ǝq�m�[�h���ȍ~�T�����Ȃ��悤�ɂ���
   */
  virtual void mark_nodes() = 0;
  
  /**
   * mark nodes which include given module_set
   */
  virtual void mark_nodes(const ModuleSet& ms);
  
  /**
   * �T�����ׂ����W���[���W���̏W�������������C���ڂ���W�����Ō�ɂ���.
   */
  virtual void reverse_reset();

  /**
   * �T�����ׂ����W���[���W���̏W�������������C���ڂ���W�����ŏ��ɖ߂��D
   */
  virtual void reset();
  
  
  /**
   * �^����ꂽ���W���[���W���̏W���͒T������K�v���������̂Ƃ��C
   * ���̏�ōŏ��ɒT�����ׂ����W���[���W���ɒ��ڂ���D
   */
  virtual void reset(const module_set_list_t &mss);
  
  /**
   * ���݂̃��W���[���W����
   * ��܂��郂�W���[���W����T���ς݂Ƃ���
   */
  virtual void mark_super_module_set();

  protected:
  module_set_list_t module_set_list_;
  module_set_list_t ms_to_visit_, r_ms_to_visit_;
};

std::ostream& operator<<(std::ostream& s, const ModuleSetContainer& m);

} // namespace ch
} // namespace hydla

#endif // _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_H_
