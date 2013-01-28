#ifndef _INCLUDED_HTDLA_CH_MODULE_SET_H_
#define _INCLUDED_HTDLA_CH_MODULE_SET_H_

#include <vector>
#include <string>
#include <ostream>
#include <algorithm>

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
  typedef module_list_t::const_iterator           module_list_const_iterator;

  struct ModuleComp {
    bool operator()(const module_t& a, const module_t& b) const
    {
      return a.first < b.first;
    }
  };

  ModuleSet();
  ModuleSet(const std::string& name, hydla::parse_tree::node_sptr node);
  ModuleSet(ModuleSet& lhs, ModuleSet& rhs);

  ~ModuleSet();

  /**
   * �W��(���̃N���X)�̖��O
   */ 
  std::string get_name() const;
  
  /**
   * ���l�L�@�ŏo��
   */ 
  std::string get_infix_string() const;
  
  /**
   * �W���̍ŏ��̗v�f
   */
  module_list_const_iterator begin() const 
  {
    return module_list_.begin();
  }

 /**
   * �W���̍Ō�̎��̗v�f
   */
  module_list_const_iterator end() const 
  {
   return module_list_.end();
  }

  /**
   * �W���̗v�f�̐�
   */
  size_t size() const 
  {
    return module_list_.size();
  }
  
  /**
   * find module
   */
  module_list_const_iterator find(const module_t& mod) const;
  
  
  /**
   * ���W���[����ǉ�
   */
  void add_module(const module_t& mod){module_list_.push_back(mod);}

  bool is_super_set(const ModuleSet& subset_mod) const
  {
    return std::includes(module_list_.begin(), 
                         module_list_.end(),
                         subset_mod.module_list_.begin(), 
                         subset_mod.module_list_.end(),
                         ModuleComp());
  }

  /**
   * �W���̃p�[�X�c���[�̓��e�o��
   */
  std::ostream& dump(std::ostream& s) const;


  /**
   * ���̃N���X���m�̔�r
   * �܂܂�郂�W���[���������Ȃ��قǏ�����
   * ���W���[����������̎��͊܂܂�Ă��郂�W���[�����ɂ�蔻�f�������Ȃ�
   */ 
  int compare(const ModuleSet& rhs) const;
  
  /**
   * return whether this module_set includes given module_set or not
   */
  bool including(const ModuleSet& ms) const;

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
  
  bool operator<(const ModuleSet& rhs) const{return (compare(rhs) < 0);}

private:
  module_list_t module_list_;
};

std::ostream& operator<<(std::ostream& s, const ModuleSet& m);



class ModuleSetComparator {
public:
  bool operator()(const module_set_sptr &lhs, 
                  const module_set_sptr &rhs) const
  {
    return lhs->compare(*rhs) > 0;
  }
};


} // namespace ch
} // namespace hydla

#endif //_INCLUDED_HTDLA_CH_MODULE_SET_H_
