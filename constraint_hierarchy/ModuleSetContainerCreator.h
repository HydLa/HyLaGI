#ifndef _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_CREATOR_H_
#define _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_CREATOR_H_

#include <assert.h>
#include <deque>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include "ParseTree.h"
#include "Node.h"
#include "TreeVisitor.h"

#include "ModuleSet.h"

namespace hydla {
namespace ch {



/**
 * Container型のモジュール集合の集合を表すクラスを構築するためのクラス
 */
template <class Container>
class ModuleSetContainerCreator : public hydla::parse_tree::TreeVisitor {
public:
  typedef boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree_sptr;
  typedef typename boost::shared_ptr<Container> container_sptr;
  typedef std::deque<container_sptr>            container_stack_t;
  typedef std::map<std::string, int>            mod_name_map_t;
 
 

  ModuleSetContainerCreator()
  {}

  virtual ~ModuleSetContainerCreator()
  {}

  /**
   * 与えられたパースツリーを元にモジュール集合の集合を表すクラスを構築する
   */
  container_sptr create(const parse_tree_sptr& parse_tree)
  {
    mod_set_stack_.clear();
    container_name_.clear();
    mod_name_map_.clear();
    constraint_level_ = 0;

    parse_tree->dispatch(this);
    assert(mod_set_stack_.size() <= 1);

    container_sptr ret;
    if(mod_set_stack_.size() == 1) {
      ret = mod_set_stack_.back();
    }
    else {
      ret.reset(new Container);
    }
    return ret;
  }

  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node)
  {
    container_name_ = node->get_name();
    accept(node->get_child());
  }

  virtual void visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node)
  {
    container_name_ = node->get_name();
    accept(node->get_child());
  }

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Constraint> node)
  {
    container_name_ += "$";
    container_name_ += boost::lexical_cast<std::string>(
                        mod_name_map_[container_name_]++);

    // create ModuleSet
    module_set_sptr mod_set(new ModuleSet(container_name_, node));
    container_name_.clear();

    // create Container
    container_sptr  container(new Container(mod_set));
    mod_set_stack_.push_back(container);
  }

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Weaker> node)
  {    
    constraint_level_++;
    container_name_.clear();

    // 左辺：弱い制約
    node->get_lhs()->accept(node->get_lhs(), this);
    container_sptr lhs(mod_set_stack_.back());
    mod_set_stack_.pop_back();

    constraint_level_--;

    // 右辺：強い制約
    node->get_rhs()->accept(node->get_rhs(), this);
    mod_set_stack_.back()->add_weak(*lhs);

  }

  
  /**
  * 並列合成「,」
   */
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parallel> node)
  {    
    container_name_.clear();

    // 左辺
    node->get_lhs()->accept(node->get_lhs(), this);
    container_sptr lhs(mod_set_stack_.back());
    mod_set_stack_.pop_back();

    // 右辺
    node->get_rhs()->accept(node->get_rhs(), this);
    // トップレベルではrequired制約扱いする
    if(constraint_level_ == 0) mod_set_stack_.back()->add_required_parallel(*lhs);
    else mod_set_stack_.back()->add_parallel(*lhs);
  }

private:

  /// モジュール集合の集合を一時的に保存しておくスタック
  container_stack_t mod_set_stack_;

  /// 登録される制約モジュールの名前
  std::string       container_name_;

  /**
   * 制約モジュールの管理
   * HydLaの制約モジュールは多重集合のため，
   * 同一名のモジュールも区別する必要がある
   */
  mod_name_map_t    mod_name_map_;

  /// 制約の優先順位に関するレベル・深さ（0が最優先, required）
  int constraint_level_;
};

} // namespace ch
} // namespace hydla

#endif // _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_CREATOR_H_
