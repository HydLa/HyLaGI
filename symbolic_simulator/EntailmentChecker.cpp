#include "EntailmentChecker.h"
//#include "PacketChecker.h"
#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {


EntailmentChecker::EntailmentChecker(MathLink& ml, bool debug_mode) :
  ml_(ml),
  debug_mode_(debug_mode)
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
  const boost::shared_ptr<hydla::parse_tree::Ask>& negative_ask, 
  hydla::symbolic_simulator::ConstraintStore& constraint_store)
{

/*
  ml_.put_function("checkEntailment", 3);

  ml_.put_function("And", 2);
  ml_.put_function("GreaterEqual", 2);
  ml_.put_symbol("x");
  ml_.MLPutInteger(0);
  ml_.put_function("LessEqual", 2);
  ml_.put_symbol("x");
  ml_.MLPutInteger(5);

  ml_.put_function("List", 1);
  ml_.put_function("And", 2);
  ml_.put_function("GreaterEqual", 2);
  ml_.put_symbol("x");
  ml_.MLPutInteger(2);
  ml_.put_function("LessEqual", 2);
  ml_.put_symbol("x");
  ml_.MLPutInteger(3);

  ml_.put_function("List", 1);
  ml_.put_symbol("x");

  ml_.MLEndPacket();
*/
/*
  // checkEntailment[guard, store, vars]��n������
  ml_.put_function("checkEntailment", 3);


  // ask����̃K�[�h�̎��𓾂�Mathematica�ɓn��
  PacketSender ps(ml_, NP_POINT_PHASE);
  ps.visit(negative_ask);


  // ����X�g�A���玮�𓾂�Mathematica�ɓn��
  ps.put_cs(constraint_store);
*/
/*
  // tell����̏W������tells�𓾂�Mathematica�ɓn��
  int tells_size = collected_tells.size();
  ml_.put_function("List", tells_size);
  TellCollector::tells_t::iterator tells_it = collected_tells.begin();
  while(tells_it!=collected_tells.end())
  {
    ps.visit((*tells_it));
    tells_it++;
  }
*/
/*
  // vars��n��
  ml_.put_function("Join", 2);
  ps.put_vars();
  // ����X�g�A���ɏo������ϐ����n��
  ps.put_cs_vars(constraint_store);



  ml_.skip_pkt_until(RETURNPKT);
  
  int num;
  if(! ml_.MLGetInteger(&num)){
    std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
    throw MathLinkError("MLGetInteger", ml_.MLError());
  }
  if(debug_mode_) std::cout << "EntailmentChecker#num:" << num << std::endl;

  // Mathematica����1�iTrue��\���j���Ԃ��true���A0�iFalse��\���j���Ԃ��false��Ԃ�
  return num==1;
  */
  return true;
}


} //namespace symbolic_simulator
} // namespace hydla
