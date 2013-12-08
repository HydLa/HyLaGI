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
 * モジュールの集合を表すクラス
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
   * 集合(このクラス)の名前
   */ 
  std::string get_name() const;
  
  /**
   * 中値記法で出力
   */ 
  std::string get_infix_string() const;
  
  /**
   * 集合の最初の要素
   */
  module_list_const_iterator begin() const 
  {
    return module_list_.begin();
  }

 /**
   * 集合の最後の次の要素
   */
  module_list_const_iterator end() const 
  {
   return module_list_.end();
  }

  /**
   * 集合の要素の数
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
   * モジュールを追加
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
   * 集合のパースツリーの内容出力
   */
  std::ostream& dump(std::ostream& s) const;


  /**
   * このクラス同士の比較
   * 含まれるモジュール数が少ないほど小さい
   * モジュール数が同一の時は含まれているモジュール名により判断をおこなう
   */ 
  int compare(const ModuleSet& rhs) const;
  
  /**
   * return whether this module_set includes given module_set or not
   */
  bool including(const ModuleSet& ms) const;

  /**
   * 集合の各制約モジュールに対してTreeVisitorの適用
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
std::ostream& operator<<(std::ostream &s, const ModuleSet::module_t& m);

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
