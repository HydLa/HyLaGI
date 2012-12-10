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

  /**
   * 集合が矛盾する条件を加える
   */
  void add_false_conditions(const hydla::parse_tree::node_sptr& node)
  {
    if(false_conditions_ == NULL){
      false_conditions_ = hydla::parse_tree::node_sptr(node);
    }else{
      false_conditions_ = hydla::parse_tree::node_sptr(new hydla::parse_tree::LogicalOr(false_conditions_, node));
    }
  }

  /**
   * 集合が矛盾する条件を得る
   */
  hydla::parse_tree::node_sptr get_false_conditions(){
    return hydla::parse_tree::node_sptr(false_conditions_);
  }

private:
  module_list_t module_list_;
  hydla::parse_tree::node_sptr false_conditions_;
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
