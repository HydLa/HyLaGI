#ifndef _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_CREATOR_H_
#define _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_CREATOR_H_

#include <assert.h>
#include <vector>
#include <sstream>

#include <boost/shared_ptr.hpp>

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
  typedef std::vector<container_sptr>           container_stack_t;

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
    unnamed_module_num_ = 1;

    parse_tree->dispatch(this);
    assert(mod_set_stack_.size() == 1);
    return mod_set_stack_.back();
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
    if(container_name_.empty()) {
      std::stringstream s;
      container_name_ = s.str();
    }

    // create ModuleSet
    module_set_sptr mod_set(new ModuleSet(container_name_, node));
    container_name_.clear();

    // create Container
    container_sptr  container(new Container(mod_set));
    mod_set_stack_.push_back(container);
  }

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Weaker> node)
  {    
    container_name_.clear();

    // 左辺：弱い制約
    node->get_lhs()->accept(node->get_lhs(), this);
    container_sptr lhs(mod_set_stack_.back());
    mod_set_stack_.pop_back();

    // 右辺：強い制約
    node->get_rhs()->accept(node->get_rhs(), this);
    mod_set_stack_.back()->add_weak(*lhs);
  }

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parallel> node)
  {    
    container_name_.clear();

    // 左辺
    node->get_lhs()->accept(node->get_lhs(), this);
    container_sptr lhs(mod_set_stack_.back());
    mod_set_stack_.pop_back();

    // 右辺
    node->get_rhs()->accept(node->get_rhs(), this);
    mod_set_stack_.back()->add_parallel(*lhs);
  }

private:

  /// モジュール集合の集合を一時的に保存しておくスタック
  container_stack_t             mod_set_stack_;

  /// 登録される制約モジュールの名前
  std::string                   container_name_;

  /// 無名制約モジュールの通し番号
  int                           unnamed_module_num_;
};

} // namespace ch
} // namespace hydla

#endif // _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_CREATOR_H_
