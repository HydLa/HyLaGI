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
 * Container�^�̃��W���[���W���̏W����\���N���X���\�z���邽�߂̃N���X
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
   * �^����ꂽ�p�[�X�c���[�����Ƀ��W���[���W���̏W����\���N���X���\�z����
   */
  container_sptr create(const parse_tree_sptr& parse_tree)
  {
    mod_set_stack_.clear();
    container_name_.clear();
    mod_name_map_.clear();
    in_weaker_ = false;

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
    in_weaker_ = true;
    container_name_.clear();

    // ���ӁF�ア����
    node->get_lhs()->accept(node->get_lhs(), this);
    container_sptr lhs(mod_set_stack_.back());
    mod_set_stack_.pop_back();

    // �E�ӁF��������
    node->get_rhs()->accept(node->get_rhs(), this);
    mod_set_stack_.back()->add_weak(*lhs);

    in_weaker_ = false;
  }

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parallel> node)
  {    
    container_name_.clear();

    // ����
    node->get_lhs()->accept(node->get_lhs(), this);
    container_sptr lhs(mod_set_stack_.back());
    mod_set_stack_.pop_back();

    // �E��
    node->get_rhs()->accept(node->get_rhs(), this);
    if(in_weaker_) mod_set_stack_.back()->add_parallel(*lhs);
    else mod_set_stack_.back()->add_required_parallel(*lhs);
  }

private:

  /// ���W���[���W���̏W�����ꎞ�I�ɕۑ����Ă����X�^�b�N
  container_stack_t mod_set_stack_;

  /// �o�^����鐧�񃂃W���[���̖��O
  std::string       container_name_;

  /**
   * ���񃂃W���[���̊Ǘ�
   * HydLa�̐��񃂃W���[���͑��d�W���̂��߁C
   * ���ꖼ�̃��W���[������ʂ���K�v������
   */
  mod_name_map_t    mod_name_map_;

  /// Weaker�m�[�h�̉��ɂ��邩�ǂ���
  bool in_weaker_;
};

} // namespace ch
} // namespace hydla

#endif // _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_CREATOR_H_
