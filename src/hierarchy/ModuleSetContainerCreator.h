#pragma once

#include <assert.h>
#include <deque>
#include <map>
#include <queue>

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include "ParseTree.h"
#include "TreeInfixPrinter.h"
#include "Node.h"
#include "DefaultTreeVisitor.h"
#include "NodeReplacer.h"
#include "ListBoundVariableUnifier.h"

#include "ModuleSet.h"

namespace hydla {
namespace hierarchy {

#define MAKE_NEW_CONTAINER \
  container_name_ = symbolic_expression::TreeInfixPrinter().get_infix_string(node); \
  ModuleSet mod_set; \
  for(auto ms : generated_mss_){ \
    if(ms.begin()->first == container_name_){ \
      mod_set = ms; \
      break; \
    } \
  } \
  if(mod_set.empty()){ \
    mod_set = ModuleSet(container_name_, node); \
    generated_mss_.insert(mod_set); \
  } \
  container_name_.clear(); \
  if(conditions_) \
  { \
    container_sptr container(new Container(mod_set, conditions_)); \
    mod_set_stack_.push_back(container); \
  } \
  else \
  { \
    container_sptr container(new Container(mod_set)); \
    mod_set_stack_.push_back(container); \
  }
  


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
    ret->init();
    return ret;
  }

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::EachElement> node)
  {
    num_of_bound_variables_++;
    boost::shared_ptr<hydla::symbolic_expression::Variable> variable;
    variable = boost::dynamic_pointer_cast<hydla::symbolic_expression::Variable>(node->get_lhs());
    assert((variable));
    bound_variables_list_.push_back(std::make_pair(variable, hydla::symbolic_expression::node_sptr(new hydla::symbolic_expression::Variable("BV"+std::to_string(num_of_bound_variables_)))));
  }

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::ProgramList> node)
  {
    std::queue<hydla::symbolic_expression::node_sptr> program_queue;
    for(int i = 0; i < node->get_arguments_size(); i++)
    {
      program_queue.push(node->get_argument(i));
    }
    while(program_queue.size()>1)
    {
      std::queue<hydla::symbolic_expression::node_sptr> tmp_queue;
      while(!program_queue.empty())
      {
        hydla::symbolic_expression::node_sptr tmp = program_queue.front();
        program_queue.pop();
        if(!program_queue.empty())
        {
          tmp_queue.push(hydla::symbolic_expression::node_sptr(new hydla::symbolic_expression::Parallel(tmp, program_queue.front())));
          program_queue.pop();
        }
        else
        {
          tmp_queue.push(tmp);
        }
      }
      program_queue = tmp_queue;
    }
    assert(program_queue.size() == 1);
    accept(program_queue.front());
  }
  
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::ConditionalProgramList> node)
  {
    bound_variables_list_.clear();
    for(int i = 0; i < node->get_arguments_size(); i++) accept(node->get_argument(i));
    hydla::symbolic_expression::NodeReplacer nr;
    for(auto bv : bound_variables_list_)
    {
      nr.set_source(bv.first);
      nr.set_dest(bv.second);
      hydla::symbolic_expression::node_sptr replace = node->clone();
      nr.replace(replace);
      node = boost::dynamic_pointer_cast<hydla::symbolic_expression::ConditionalProgramList>(replace);
    }
    conditions_.reset();
    for(int i = 0; i < node->get_arguments_size(); i++)
    {
      if(i) conditions_ = symbolic_expression::node_sptr(new symbolic_expression::LogicalAnd(conditions_, node->get_argument(i)->clone()));
      else conditions_ = node->get_argument(i)->clone();
    }
    accept(node->get_program());
    conditions_.reset();
    bound_variables_list_.clear();
  }
  
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::ProgramListElement> node)
  {
    MAKE_NEW_CONTAINER
  }
  
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::ConstraintCaller> node)
  {
    MAKE_NEW_CONTAINER
  }
  
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::ProgramCaller> node)
  {
    accept(node->get_child());
  }

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Constraint> node)
  {
    MAKE_NEW_CONTAINER
  }

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Weaker> node)
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
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Parallel> node)
  {    
    container_name_.clear();

    // 左辺
    node->get_lhs()->accept(node->get_lhs(), this);
    container_sptr lhs(mod_set_stack_.back());
    mod_set_stack_.pop_back();

    // 右辺
    node->get_rhs()->accept(node->get_rhs(), this);
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
  
  /**
   * ModuleSets which are generated before
   */
  module_set_set_t generated_mss_;

  /**
   * flag representating processing in caller
   */
  bool in_caller_ = false;

  int constraint_level_ = 0;

  int num_of_bound_variables_ = 0;

  std::vector<std::pair<hydla::symbolic_expression::node_sptr,hydla::symbolic_expression::node_sptr> > bound_variables_list_;

  hydla::symbolic_expression::node_sptr conditions_;
};

} // namespace hierarchy
} // namespace hydla
