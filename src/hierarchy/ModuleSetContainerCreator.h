#pragma once

#include <assert.h>
#include <deque>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include "ParseTree.h"
#include "TreeInfixPrinter.h"
#include "Node.h"
#include "DefaultTreeVisitor.h"

#include "ModuleSet.h"

namespace hydla {
namespace hierarchy {



/**
 * Container型のモジュール集合の集合を表すクラスを構築するためのクラス
 */
template <class Container>
class ModuleSetContainerCreator : public hydla::symbolic_expression::DefaultTreeVisitor {
public:
  typedef boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree_sptr;
  typedef typename boost::shared_ptr<Container> container_sptr;
  typedef std::deque<container_sptr>            container_stack_t;
  typedef std::map<std::string, int>            mod_name_map_t;
  typedef std::set<ModuleSet>                   module_set_set_t;
 
 

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

    parse_tree->dispatch(this);
    assert(mod_set_stack_.size() <= 1);

    container_sptr ret;
    if(mod_set_stack_.size() == 1) {
      ret = mod_set_stack_.back();
    }
    else {
      ret.reset(new Container);
    }
    ret->generate_required_ms();
    return ret;
  }

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Plus> node)
  {
    accept(node->get_lhs());
    container_name_+="+";
    accept(node->get_rhs());
  }

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Subtract> node)
  {
    accept(node->get_lhs());
    container_name_+="-";
    accept(node->get_rhs());
  }

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Times> node)
  {
    accept(node->get_lhs());
    container_name_+="*";
    accept(node->get_rhs());
  }

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Divide> node)
  {
    accept(node->get_lhs());
    container_name_+="/";
    accept(node->get_rhs());
  }
  
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Power> node)
  {
    accept(node->get_lhs());
    container_name_+="^";
    accept(node->get_rhs());
  }

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Variable> node)
  {
    container_name_ += node->get_name();
  }

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Number> node)
  {
    container_name_ += node->get_number();
  }

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::ConstraintCaller> node)
  {
    container_name_ = node->get_name();
    int arg_size = node->actual_arg_size();
    if(arg_size) container_name_ += "("; 
    for(int i = 0; i < arg_size; i++){
      if(i) container_name_ += ",";
      accept(node->get_actual_arg(i));
    }
    if(arg_size) container_name_ += ")";
    accept(node->get_child());
  }
  
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::ProgramCaller> node)
  {
    container_name_ = node->get_name();
    int arg_size = node->actual_arg_size();
    if(arg_size) container_name_ += "("; 
    for(int i = 0; i < arg_size; i++){
      if(i) container_name_ += ",";
      accept(node->get_actual_arg(i));
    }
    if(arg_size) container_name_ += ")";
    accept(node->get_child());
  }

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Constraint> node)
  {
    if(container_name_ == ""){
      container_name_ = symbolic_expression::TreeInfixPrinter().get_infix_string(node->get_child());
      container_name_ += "$";
      container_name_ += boost::lexical_cast<std::string>(
                          mod_name_map_[container_name_]++); 
    }

    // create ModuleSet
    ModuleSet mod_set;
    for(auto ms : generated_mss_){
      if(ms.begin()->first == container_name_){
        mod_set = ms;
        break;
      }
    }
    if(mod_set.empty()){
      mod_set = ModuleSet(container_name_, node);
      generated_mss_.insert(mod_set);
    }
    container_name_.clear();

    // create Container
    container_sptr  container(new Container(mod_set));
    mod_set_stack_.push_back(container);
  }

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Weaker> node)
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

  
  /**
  * 並列合成「,」
   */
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Parallel> node)
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
  container_stack_t mod_set_stack_;

  /// 登録される制約モジュールの名前
  std::string       container_name_;

  /**
   * 制約モジュールの管理
   * HydLaの制約モジュールは多重集合のため，
   * 同一名のモジュールも区別する必要がある
   */
  mod_name_map_t    mod_name_map_;
  
  /**
   * ModuleSets which are generated before
   */
  module_set_set_t generated_mss_;
};

} // namespace hierarchy
} // namespace hydla
