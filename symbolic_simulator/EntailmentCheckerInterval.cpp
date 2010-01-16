#include "EntailmentCheckerInterval.h"
//#include "PacketChecker.h"
#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {


EntailmentCheckerInterval::EntailmentCheckerInterval(MathLink& ml, bool debug_mode) :
  ml_(ml),
  debug_mode_(debug_mode)
{}

EntailmentCheckerInterval::~EntailmentCheckerInterval()
{}

/**
 * collected_tells����Anegative_asks����ask����̃K�[�h��������������邩�ǂ������ׂ�
 * @param negative_ask     �܂��W�J����Ă��Ȃ�ask����1��
 * @param collected_tells  tell����̃��X�g�i�W�J���ꂽask����́u=>�v�̉E�ӂ͂����ɒǉ������j
 * @return �`�F�b�N�̌��ʁA����ask���񂪓W�J���ꂽ���ǂ���
 */

bool EntailmentCheckerInterval::check_entailment(
  const boost::shared_ptr<hydla::parse_tree::Ask>& negative_ask, 
  hydla::symbolic_simulator::ConstraintStoreInterval& constraint_store)
{
/*
  // checkEntailment[guard, store, vars]��n������
  ml_.put_function("checkEntailment", 3);


  // ask����̃K�[�h�̎��𓾂�Mathematica�ɓn��
  PacketSender psi(ml_, NP_INTERVAL_PHASE);
  psi.visit(negative_ask);

  // ����X�g�A���玮store�𓾂�Mathematica�ɓn��
  psi.put_cs(constraint_store);

  // vars��n��
  ml_.put_function("Join", 2);
  psi.put_vars();
  // ����X�g�A���ɏo������ϐ����n��
  psi.put_cs_vars(constraint_store);

  ml_.skip_pkt_until(RETURNPKT);
  
  int num;
  if(! ml_.MLGetInteger(&num)){
    std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
    throw MathLinkError("MLGetInteger", ml_.MLError());
  }
  if(debug_mode_) std::cout << "EntailmentCheckerInterval#num:" << num << std::endl;

  // Mathematica����1�iTrue��\���j���Ԃ��true���A0�iFalse��\���j���Ԃ��false��Ԃ�
  return num==1;
  */
  return true;
}


} //namespace symbolic_simulator
} // namespace hydla
