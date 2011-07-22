#include "ParseTreeSemanticAnalyzer.h"

#include <assert.h>
#include <algorithm>
#include <boost/bind.hpp>

#include "ParseError.h"

using namespace hydla::parse_tree;
using namespace hydla::parse_error;

namespace hydla { 
namespace parser {


#define DEFINE_DEFAULT_VISIT_BINARY(NODE_NAME)        \
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<NODE_NAME> node) \
{                                                     \
  dispatch_lhs(node);                                 \
  dispatch_rhs(node);                                 \
}

#define DEFINE_DEFAULT_VISIT_UNARY(NODE_NAME)        \
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<NODE_NAME> node) \
{ dispatch_child(node);}

#define DEFINE_DEFAULT_VISIT_FACTOR(NODE_NAME)        \
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<NODE_NAME> node){}


ParseTreeSemanticAnalyzer::ParseTreeSemanticAnalyzer(
  DefinitionContainer<hydla::parse_tree::ConstraintDefinition>& constraint_definition,
  DefinitionContainer<hydla::parse_tree::ProgramDefinition>&    program_definition,
  hydla::parse_tree::ParseTree* parse_tree) :
    constraint_definition_(constraint_definition),
    program_definition_(program_definition),
    parse_tree_(parse_tree)
{}

ParseTreeSemanticAnalyzer::~ParseTreeSemanticAnalyzer()
{}

void ParseTreeSemanticAnalyzer::analyze(node_sptr& n/*, variable_map_t& variable_map*/)
{
  if(n) {
    State state;
    state.in_guard           = false;
    state.in_always          = false;
    state.in_constraint      = false;
    state.differential_count = 0;
    state_stack_.push(state);

  //  variable_map_ = &variable_map;

    accept(n);
    if(new_child_) n = new_child_;

    assert(state_stack_.size() == 1);
  }
}

// �����`
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<ConstraintDefinition> node)  
{
  assert(0);
}

// �v���O������`
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<ProgramDefinition> node)     
{
  assert(0);
}

node_sptr ParseTreeSemanticAnalyzer::apply_definition(
  const referenced_definition_t& def_type,
  boost::shared_ptr<hydla::parse_tree::Caller> caller, 
  boost::shared_ptr<Definition> definition)
{
  State& state = state_stack_.top();

  definition = boost::shared_static_cast<Definition>(definition->clone());
  
  //�z�Q�Ƃ̃`�F�b�N
  if (state.referenced_definition_list.find(def_type) != 
      state.referenced_definition_list.end()) {
    throw CircularReference(caller);
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
  new_state.referenced_definition_list.insert(def_type);

  // ��`�̎q�m�[�h�ɑ΂�preprocess�K�p
  state_stack_.push(new_state);
  dispatch_child(definition);
  state_stack_.pop();

  return definition->get_child();
}

// ����Ăяo��
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<ConstraintCaller> node)      
{
  referenced_definition_t deftype(
    std::make_pair(node->get_name(), 
                   node->actual_arg_size()));

  if(!node->get_child()) {
    // �����`����T��
    boost::shared_ptr<ConstraintDefinition> cons_def(
      constraint_definition_.get_definition(deftype));
    if(!cons_def) {
      throw UndefinedReference(node);
    }

    // ��`�̓W�J
    node->set_child( 
      apply_definition(deftype, node, cons_def));
  }
}

// �v���O�����Ăяo��
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<ProgramCaller> node)         
{
  referenced_definition_t deftype(
    std::make_pair(node->get_name(), 
                   node->actual_arg_size()));

  if(!node->get_child()) {
    boost::shared_ptr<Definition> defnode;

    // �����`����T��
    boost::shared_ptr<ConstraintDefinition> cons_def(
      constraint_definition_.get_definition(deftype));
    if(cons_def) {
      defnode = cons_def;
    } else {
      // �v���O������`����T��
      boost::shared_ptr<ProgramDefinition> prog_def(
        program_definition_.get_definition(deftype));
      if(prog_def) {
        defnode = prog_def;
      } else {
        throw UndefinedReference(node);
      }
    }

    // ��`�̓W�J
    node->set_child(
      apply_definition(deftype, node, defnode));
  }
}

// ����
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Constraint> node)            
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
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Ask> node)                   
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
DEFINE_DEFAULT_VISIT_UNARY(Tell)

// �Z�p�P�����Z�q
DEFINE_DEFAULT_VISIT_UNARY(Negative)
DEFINE_DEFAULT_VISIT_UNARY(Positive)


// ��r���Z�q
DEFINE_DEFAULT_VISIT_BINARY(Equal)
DEFINE_DEFAULT_VISIT_BINARY(UnEqual)
DEFINE_DEFAULT_VISIT_BINARY(Less)
DEFINE_DEFAULT_VISIT_BINARY(LessEqual)
DEFINE_DEFAULT_VISIT_BINARY(Greater)
DEFINE_DEFAULT_VISIT_BINARY(GreaterEqual)

// �Z�p�񍀉��Z�q
DEFINE_DEFAULT_VISIT_BINARY(Plus)
DEFINE_DEFAULT_VISIT_BINARY(Subtract)
DEFINE_DEFAULT_VISIT_BINARY(Times)
DEFINE_DEFAULT_VISIT_BINARY(Divide)
DEFINE_DEFAULT_VISIT_BINARY(Power)

// �_�����Z�q
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<LogicalAnd> node)
{
  if(!state_stack_.top().in_constraint) {
    throw InvalidConjunction(node->get_lhs(), 
                             node->get_rhs());
  }

  dispatch_lhs(node);
  dispatch_rhs(node); 
}

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<LogicalOr> node)
{
  if(!state_stack_.top().in_guard) {
    throw InvalidDisjunction(node->get_lhs(), 
                             node->get_rhs());
  }

  dispatch_lhs(node);
  dispatch_rhs(node); 
}

DEFINE_DEFAULT_VISIT_UNARY(Not)

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Weaker> node)
{
  if(state_stack_.top().in_constraint) {
    throw InvalidWeakComposition(node->get_lhs(), 
                                 node->get_rhs());
  }

  dispatch_lhs(node);
  dispatch_rhs(node); 

}

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Parallel> node)
{
  if(state_stack_.top().in_constraint) {
    throw InvalidParallelComposition(node->get_lhs(), 
                                     node->get_rhs());
  }

  dispatch_lhs(node);
  dispatch_rhs(node); 


}
  
// �������Z�q
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Always> node)
{
  State& state = state_stack_.top();

  // �K�[�h�̒��ɂ͂Ȃ�
  if(state.in_guard) {
    throw InvalidAlways(node->get_child());
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
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Differential> node)
{
  // �q�m�[�h���������ϐ��łȂ�������G���[
  if(!boost::dynamic_pointer_cast<Differential>(node->get_child()) &&
     !boost::dynamic_pointer_cast<Variable>(node->get_child())) {
       throw InvalidDifferential(node);
  }

  // �q�m�[�h�̒T��
  state_stack_.top().differential_count++;
  dispatch_child(node);
  state_stack_.top().differential_count--;
}

// ���Ɍ�
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Previous> node)
{  
  // �q�m�[�h���������ϐ��łȂ�������G���[
  if(!boost::dynamic_pointer_cast<Differential>(node->get_child()) &&
     !boost::dynamic_pointer_cast<Variable>(node->get_child())) {
       throw InvalidPrevious(node);
  }

  dispatch_child(node); 
}



// �O�p�֐�
DEFINE_DEFAULT_VISIT_UNARY(Sin)
DEFINE_DEFAULT_VISIT_UNARY(Cos)
DEFINE_DEFAULT_VISIT_UNARY(Tan)

// �t�O�p�֐�
DEFINE_DEFAULT_VISIT_UNARY(Asin)
DEFINE_DEFAULT_VISIT_UNARY(Acos)
DEFINE_DEFAULT_VISIT_UNARY(Atan)

// �~����
DEFINE_DEFAULT_VISIT_FACTOR(Pi)

// �ΐ�
DEFINE_DEFAULT_VISIT_BINARY(Log)
DEFINE_DEFAULT_VISIT_UNARY(Ln)

// ���R�ΐ��̒�
DEFINE_DEFAULT_VISIT_FACTOR(E)

// �C�ӂ̕�����
DEFINE_DEFAULT_VISIT_FACTOR(ArbitraryFactor)
DEFINE_DEFAULT_VISIT_UNARY(ArbitraryUnary)
DEFINE_DEFAULT_VISIT_BINARY(ArbitraryBinary)


// �ϐ�
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Variable> node)
{
  State& state = state_stack_.top();

  formal_arg_map_t::iterator it = 
    state.formal_arg_map.find(node->get_name());
  if(it != state.formal_arg_map.end()) {
    // ���g���������ł������ꍇ�A����������
    new_child_ = (*it).second->clone();
  } 
  else {
    // ���g���������ł������ꍇ
    parse_tree_->register_variable(node->get_name(), 
                                   state.differential_count);
  } 
}

// ����
DEFINE_DEFAULT_VISIT_FACTOR(Number)


} //namespace parser
} //namespace hydla
