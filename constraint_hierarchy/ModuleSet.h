#ifndef _INCLUDED_HTDLA_CH_MODULE_SET_H_
#define _INCLUDED_HTDLA_CH_MODULE_SET_H_

#include <vector>
#include <string>
#include <ostream>

#include <boost/shared_ptr.hpp>

#include "Node.h"

namespace hydla {
namespace ch {

typedef boost::shared_ptr<class ModuleSet> module_set_sptr;

/**
 * ���W���[���̏W����\���N���X
 *
 */
class ModuleSet {
public:
  typedef std::pair<std::string,
                    hydla::parse_tree::node_sptr> module_t;
  typedef std::vector<module_t>                   module_list_t;

  typedef struct ModuleComp_ {
    bool operator()(module_t& a, module_t& b)
    {
      return a.first < b.first;
    }
  } ModuleComp;

  ModuleSet();
  ModuleSet(const std::string& name, hydla::parse_tree::node_sptr node);
  ModuleSet(ModuleSet& lhs, ModuleSet& rhs);

  ~ModuleSet();

  /**
   * �W��(���̃N���X)�̖��O
   */ 
  std::string get_name() const;

  /**
   * �W���̃p�[�X�c���[�̓��e�o��
   */
   std::string get_tree_dump() const;

  /**
   * ���̃N���X���m�̔�r
   * �܂܂�郂�W���[���������Ȃ��قǏ�����
   * ���W���[����������̎��͊܂܂�Ă��郂�W���[�����ɂ�蔻�f�������Ȃ�
   */ 
  int compare(ModuleSet& rhs) const;

  /**
   * �W���̊e���񃂃W���[���ɑ΂���TreeVisitor�̓K�p
   */ 
  void dispatch(hydla::parse_tree::TreeVisitor* visitor)
  {
    module_list_t::iterator it  = module_list_.begin();
    module_list_t::iterator end = module_list_.end();
    for(; it!=end; ++it) {
      (it->second)->accept(it->second, visitor);
    }
  }

private:
  module_list_t module_list_;
};

} // namespace ch
} // namespace hydla

#endif //_INCLUDED_HTDLA_CH_MODULE_SET_H_
