#ifndef _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_H_
#define _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_H_

#include <boost/function.hpp>
#include <set>

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
  virtual module_set_sptr get_module_set() const;
  
  /**
   * �T������K�v�̖������W���[���W���̏W���𓾂�
   */
  std::set<module_set_sptr> get_visited_module_sets() const;
  
  /**
   * ���ɒT�����ׂ����W���[���W���ɐi��
   * ���݂��Ȃ����false��Ԃ��D
   */
  virtual bool go_next();
  
  /**
   * ���̃m�[�h��T���ς݂Ƃ��C�ȍ~�T�����Ȃ��悤�ɂ���
   */
  virtual void mark_current_node();

  /**
   * ���̃m�[�h�Ǝq�m�[�h���ȍ~�T�����Ȃ��悤�ɂ���
   */
  virtual void mark_nodes() = 0;
  
  /**
   * �T�����ׂ����W���[���W���̏W�������������C���ڂ���W�����ŏ��ɖ߂��D
   */
  virtual void reset();
  
  
  /**
   * �^����ꂽ���W���[���W���̏W���͒T������K�v���������̂Ƃ��C
   * ���̏�ōŏ��ɒT�����ׂ����W���[���W���ɒ��ڂ���D
   */
  virtual void reset(const std::set<module_set_sptr> &mss);
  
  protected:
  module_set_list_t module_set_list_;
  module_set_set_t  visited_module_sets_;
  module_set_list_t::iterator current_module_set_;
};

std::ostream& operator<<(std::ostream& s, const ModuleSetContainer& m);

} // namespace ch
} // namespace hydla

#endif // _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_H_
