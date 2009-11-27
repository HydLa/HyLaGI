#include "PreprocessParseTree.h"

#include <assert.h>
#include <algorithm>
#include <boost/bind.hpp>

#include "ParseError.h"

using namespace hydla::parse_tree;
using namespace hydla::parse_error;

namespace hydla { 
namespace parser {

PreprocessParseTree::PreprocessParseTree()
{}

PreprocessParseTree::~PreprocessParseTree()
{}

void PreprocessParseTree::start(hydla::parse_tree::ParseTree *pt)
{
  constraint_def_map_ = pt->get_constraint_def_map();
  program_def_map_    = pt->get_program_def_map();

  State state;
  state.in_guard           = false;
  state.in_always          = false;
  state.in_constraint      = false;
  state.differential_count = 0;
  state_stack_.push(state);

  pt->dispatch(this);
  assert(state_stack_.size() == 1);
}

// �����`
void PreprocessParseTree::visit(boost::shared_ptr<ConstraintDefinition> node)  
{
  assert(0);
}

// �v���O������`
void PreprocessParseTree::visit(boost::shared_ptr<ProgramDefinition> node)     
{
  assert(0);
}

node_sptr PreprocessParseTree::apply_definition(difinition_type_t* def_type,
                                                Caller* caller, 
                                                Definition* definition)
{
  State& state = state_stack_.top();
  
  //�z�Q�Ƃ̃`�F�b�N
  if (state.referenced_definition_list.find(*def_type) != 
      state.referenced_definition_list.end()) {
    throw CircularReference(caller->to_string());
  }

  State new_state(state);
  new_state.formal_arg_map.clear();

  assert(definition->bound_variable_size() == caller->actual_arg_size());
  
  Definition::bound_variables_iterator 
    bv_it = definition->bound_variable_begin();
  Definition::bound_variables_iterator 
    bv_end = definition->bound_variable_end();
  Caller::actual_args_iterator         
    aa_it = caller->actual_arg_begin();
  for(; bv_it!=bv_end; ++bv_it, ++aa_it) {
    // �������ɑ΂�preprocess�K�p
    accept(*aa_it);
    if(new_child_) {
      *aa_it = new_child_;
      new_child_.reset();
    }

    // �������Ǝ������̑Ή��t��
    new_state.formal_arg_map.insert(make_pair(*bv_it, *aa_it));
  }

  // �z�Q�ƌ��o�p���X�g�ɓo�^
  new_state.referenced_definition_list.insert(*def_type);

  // ��`�̎q�m�[�h�ɑ΂�preprocess�K�p
  boost::shared_ptr<Definition> defnode(
    boost::shared_static_cast<Definition>(definition->clone()));
  state_stack_.push(new_state);
  dispatch_child(defnode);
  state_stack_.pop();

  return defnode->get_child();
}

// ����Ăяo��
void PreprocessParseTree::visit(boost::shared_ptr<ConstraintCaller> node)      
{
  difinition_type_t def_type(node->get_name(), node->actual_arg_size());

  // �����`����T��
  constraint_def_map_t::const_iterator cons_def = 
    constraint_def_map_->find(def_type);
  if(cons_def == constraint_def_map_->end()) {
    throw UndefinedReference(node->to_string());
  }

  // ��`�̓W�J
  node->set_child(apply_definition(&def_type, 
                                   node.get(), 
                                   cons_def->second.get()));
}

// �v���O�����Ăяo��
void PreprocessParseTree::visit(boost::shared_ptr<ProgramCaller> node)         
{
  difinition_type_t def_type(node->get_name(), node->actual_arg_size());
  Definition* defnode;

  // �����`����T��
  constraint_def_map_t::const_iterator 
    cons_it = constraint_def_map_->find(def_type);
  if(cons_it!=constraint_def_map_->end()) {
    defnode = (*cons_it).second.get();
  } else {
    // �v���O������`����T��
    program_def_map_t::const_iterator 
      prog_it = program_def_map_->find(def_type);
    if(prog_it!=program_def_map_->end()) {
      defnode = (*prog_it).second.get();
    } else {
      throw UndefinedReference(node->to_string());
    }
  }

  // ��`�̓W�J
  node->set_child(apply_definition(&def_type, 
                                   node.get(), 
                                   defnode));
}

// ����
void PreprocessParseTree::visit(boost::shared_ptr<Constraint> node)            
{
  State& state = state_stack_.top();

  // ���łɐ��񎮂̒��ł������ꍇ�͎������g����菜��
  if(state.in_constraint) {
    dispatch_child(node);
    new_child_ = node->get_child();
  } 
  else {
    state_stack_.push(state);
    state_stack_.top().in_constraint = true;
    dispatch_child(node);
    state_stack_.pop();
  }
}

// Ask����
void PreprocessParseTree::visit(boost::shared_ptr<Ask> node)                   
{
  State& state = state_stack_.top();

  // �K�[�h�m�[�h�̒T��
  state_stack_.push(state);
  state_stack_.top().in_guard = true;
  dispatch<Ask, &Ask::get_guard, &Ask::set_guard>(node.get());
  state_stack_.pop();

  // �q�m�[�h�̒T��
  state_stack_.push(state);
  state_stack_.top().in_always = false;
  dispatch<Ask, &Ask::get_child, &Ask::set_child>(node.get());
  state_stack_.pop();
}

// Tell����
void PreprocessParseTree::visit(boost::shared_ptr<Tell> node)                  
{
  dispatch_child(node);
}

// �Z�p�P�����Z�q
void PreprocessParseTree::visit(boost::shared_ptr<Negative> node)
{
  dispatch_child(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<Positive> node)
{
  dispatch_child(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<Equal> node)                 
{
  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<UnEqual> node)               
{
  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<Less> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<LessEqual> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<Greater> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<GreaterEqual> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<Plus> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<Subtract> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<Times> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<Divide> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<LogicalAnd> node)
{
  if(!state_stack_.top().in_constraint) {
    throw InvalidConjunction(node->get_lhs()->to_string(), 
                             node->get_rhs()->to_string());
  }

  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<LogicalOr> node)
{
  if(!state_stack_.top().in_guard) {
    throw InvalidDisjunction(node->get_lhs()->to_string(), 
                             node->get_rhs()->to_string());
  }

  dispatch_lhs(node);
  dispatch_rhs(node);
}  

void PreprocessParseTree::visit(boost::shared_ptr<Weaker> node)
{
  if(state_stack_.top().in_constraint) {
    throw InvalidWeakComposition(node->get_lhs()->to_string(), 
                                 node->get_rhs()->to_string());
  }

  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<Parallel> node)
{
  if(state_stack_.top().in_constraint) {
    throw InvalidParallelComposition(node->get_lhs()->to_string(), 
                                     node->get_rhs()->to_string());
  }

  dispatch_lhs(node);
  dispatch_rhs(node);
}
  
// �������Z�q
void PreprocessParseTree::visit(boost::shared_ptr<Always> node)
{
  State& state = state_stack_.top();

  // �K�[�h�̒��ɂ͂Ȃ�
  if(state.in_guard) {
    throw InvalidAlways(node->get_child()->to_string());
  }

  // �q�m�[�h�̒T��
  state_stack_.push(state);
  state_stack_.top().in_always = true;
  dispatch_child(node);
  state_stack_.pop();
    
  // ���ł�always������ł������ꍇ���̃m�[�h���͂���
  if(state.in_always) {
    new_child_ = node->get_child();
  }
}
  
// ����
void PreprocessParseTree::visit(boost::shared_ptr<Differential> node)
{
  State& state = state_stack_.top();

  // �q�m�[�h�̒T��
  state_stack_.push(state);
  state_stack_.top().differential_count++;
  dispatch_child(node);
  state_stack_.pop();
}

// ���Ɍ�
void PreprocessParseTree::visit(boost::shared_ptr<Previous> node)
{
  dispatch_child(node);
}
  
// �ϐ�
void PreprocessParseTree::visit(boost::shared_ptr<Variable> node)
{
  State& state = state_stack_.top();

  formal_arg_map_t::iterator it = 
    state.formal_arg_map.find(node->get_name());
  if(it != state.formal_arg_map.end()) {
    // ���g���������ł������ꍇ�A����������
    new_child_ = (*it).second;
  } 
  else {
    // ���g���������ł������ꍇ�A
    // ���g�̂��łɓo�^�ς݂̔����񐔂���
    // �傫��������ϐ��̃��X�g�ɓo�^����
    variable_map_t::iterator it = variable_map_.find(node->get_name());
    if(it == variable_map_.end() ||
       it->second < state.differential_count) 
    {
      variable_map_.insert(make_pair(node->get_name(), 
                                     state.differential_count));
    }
  }
}

// ����
void PreprocessParseTree::visit(boost::shared_ptr<Number> node)
{
  //do nothing
}


} //namespace parser
} //namespace hydla
