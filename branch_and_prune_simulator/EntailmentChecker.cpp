#include "EntailmentChecker.h"

#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace bp_simulator {


EntailmentChecker::EntailmentChecker() :
  debug_mode_(false)
{}

EntailmentChecker::EntailmentChecker(bool debug_mode) :
  debug_mode_(debug_mode)
{}

EntailmentChecker::~EntailmentChecker()
{}

// Ask����
void EntailmentChecker::visit(boost::shared_ptr<Ask> node)
{
  // ask��������
  // guard_list(and�Ȃ���)
  // not_guard_list(or�Ȃ���)
  // �̗��������
  this->accept(node->get_guard());
}

// Tell���� �K�v�Ȃ��H
void EntailmentChecker::visit(boost::shared_ptr<Tell> node)
{
}

// ��r���Z�q
// �����ň�C�ɓ�̎������
void EntailmentChecker::visit(boost::shared_ptr<Equal> node)
{
}

void EntailmentChecker::visit(boost::shared_ptr<UnEqual> node)               
{
}

void EntailmentChecker::visit(boost::shared_ptr<Less> node)                  
{
}

void EntailmentChecker::visit(boost::shared_ptr<LessEqual> node)             
{
}

void EntailmentChecker::visit(boost::shared_ptr<Greater> node)               
{
}

void EntailmentChecker::visit(boost::shared_ptr<GreaterEqual> node)          
{
}

// �_�����Z�q
void EntailmentChecker::visit(boost::shared_ptr<LogicalAnd> node)            
{
}


/**
 * collected_tells����negative_ask�̃K�[�h������entail�����ǂ������ׂ�
 * TRUE�Ȃ�collected_tells��ask����̌㌏��ǉ�����
 * 
 * @param negative_ask �܂��W�J����Ă��Ȃ�ask����
 * @param collected_tells tell����̃��X�g�i�W�J���ꂽask����́u=>�v�̉E�ӂ������ɒǉ������j
 * @param constraint_store ����X�g�A
 * 
 * @return entail����邩�ǂ��� {TRUE, UNKNOWN, FALSE}
 */
Trivalent EntailmentChecker::check_entailment(
  const boost::shared_ptr<Ask>& negative_ask,
  tells_t& collected_tells,
  ConstraintStore& constraint_store)
{
  // constraint_store + collected_tells = ������ S
  // ask��������g��ng�����
  // solve(S & g) == empty -> FALSE
  // solve(S&ng0)==empty /\ solve(S&ng1)==empty /\ ... -> TRUE
  // else -> UNKNOWN
  return FALSE;
}

} //namespace bp_simulator
} // namespace hydla
