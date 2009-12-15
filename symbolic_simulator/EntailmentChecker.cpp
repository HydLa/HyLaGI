#include "EntailmentChecker.h"
//#include "PacketChecker.h"
#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {


EntailmentChecker::EntailmentChecker(MathLink& ml) :
  ml_(ml)
{}

EntailmentChecker::~EntailmentChecker()
{}

/**
 * collected_tells����Anegative_asks����ask����̃K�[�h��������������邩�ǂ������ׂ�
 * @param negative_ask     �܂��W�J����Ă��Ȃ�ask����1��
 * @param collected_tells  tell����̃��X�g�i�W�J���ꂽask����́u=>�v�̉E�ӂ͂����ɒǉ������j
 * @return �`�F�b�N�̌��ʁA����ask���񂪓W�J���ꂽ���ǂ���
 */

bool EntailmentChecker::check_entailment(
  boost::shared_ptr<hydla::parse_tree::Ask> negative_ask, 
//hydla::simulator::TellCollector::tells_t& collected_tells)
  hydla::symbolic_simulator::ConstraintStore& constraint_store)
{

/*
  ml_.MLPutFunction("checkEntailment", 3);

  ml_.MLPutFunction("And", 2);
  ml_.MLPutFunction("GreaterEqual", 2);
  ml_.MLPutSymbol("x");
  ml_.MLPutInteger(0);
  ml_.MLPutFunction("LessEqual", 2);
  ml_.MLPutSymbol("x");
  ml_.MLPutInteger(5);

  ml_.MLPutFunction("List", 1);
  ml_.MLPutFunction("And", 2);
  ml_.MLPutFunction("GreaterEqual", 2);
  ml_.MLPutSymbol("x");
  ml_.MLPutInteger(2);
  ml_.MLPutFunction("LessEqual", 2);
  ml_.MLPutSymbol("x");
  ml_.MLPutInteger(3);

  ml_.MLPutFunction("List", 1);
  ml_.MLPutSymbol("x");

  ml_.MLEndPacket();
*/

  // checkEntailment[guard, tells, vars]��n������
  ml_.MLPutFunction("checkEntailment", 3);


  // ask����̃K�[�h�̎��𓾂�Mathematica�ɓn��
  PacketSender ps(ml_);
  ps.visit(negative_ask);


  // ����X�g�A�����expr�𓾂�Mathematica�ɓn��
  ps.put_cs(constraint_store);

/*
  // tell����̏W������tells�𓾂�Mathematica�ɓn��
  int tells_size = collected_tells.size();
  ml_.MLPutFunction("List", tells_size);
  TellCollector::tells_t::iterator tells_it = collected_tells.begin();
  while(tells_it!=collected_tells.end())
  {
    ps.visit((*tells_it));
    tells_it++;
  }
*/

  // vars��n��
  ps.put_vars();


/*
// �Ԃ��Ă���p�P�b�g�����
PacketChecker pc(ml_);
pc.check();
*/

  ml_.skip_pkt_until(RETURNPKT);
  
  int num;
  ml_.MLGetInteger(&num);
  std::cout << "EntailmentChecker#num:" << num << std::endl;
  
  // Mathematica����1�iTrue��\���j���Ԃ��true���A0�iFalse��\���j���Ԃ��false��Ԃ�
  return num==1;
}


} //namespace symbolic_simulator
} // namespace hydla
