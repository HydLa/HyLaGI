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
//  hydla::simulator::TellCollector::tells_t& collected_tells,
  hydla::symbolic_simulator::ConstraintStore& constraint_store)
{

  // checkEntailment[guard, Join[tells, store], vars]��n������
  ml_.put_function("checkEntailment", 3);


  // ask����̃K�[�h�̎��𓾂�Mathematica�ɓn��
  PacketSenderInterval psi(ml_, debug_mode_);
  psi.visit(negative_ask);

/*
  ml_.put_function("Join", 2);
  // tell����̏W������tells�𓾂�Mathematica�ɓn��
  int tells_size = collected_tells.size();
  ml_.put_function("List", tells_size);
  TellCollector::tells_t::iterator tells_it = collected_tells.begin();
  while(tells_it!=collected_tells.end())
  {
    psi.visit((*tells_it));
    tells_it++;
  }
*/

  // ����X�g�A�������store�𓾂�Mathematica�ɓn��
  psi.put_cs(constraint_store);

  // vars��n��
  ml_.put_function("Join", 2);
  psi.put_vars();
  ml_.put_function("ToExpression", 1);
  ml_.put_string(constraint_store.second.str);

/*
// �Ԃ��Ă���p�P�b�g�����
PacketChecker pc(ml_);
pc.check();
*/

  ml_.skip_pkt_until(RETURNPKT);
  
  int num;
  ml_.MLGetInteger(&num);
  if(debug_mode_) std::cout << "EntailmentCheckerInterval#num:" << num << std::endl;

  // Mathematica����1�iTrue��\���j���Ԃ��true���A0�iFalse��\���j���Ԃ��false��Ԃ�
  return num==1;
}


} //namespace symbolic_simulator
} // namespace hydla
