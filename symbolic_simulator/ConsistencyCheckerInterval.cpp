#include "ConsistencyCheckerInterval.h"
//#include "PacketChecker.h"
#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {

ConsistencyCheckerInterval::ConsistencyCheckerInterval(MathLink& ml, bool debug_mode) :
  ml_(ml),
  debug_mode_(debug_mode)
{}

ConsistencyCheckerInterval::~ConsistencyCheckerInterval()
{}


bool ConsistencyCheckerInterval::is_consistent(tells_t& collected_tells, 
                                               ConstraintStoreInterval& constraint_store)
{
  // isConsistentInterval[tells, store, tellsVars, storeVars]��n������
  ml_.put_function("isConsistentInterval", 4);


  // tell����̏W������tells�𓾂�Mathematica�ɓn��
  int tells_size = collected_tells.size();
  ml_.put_function("List", tells_size);
  tells_t::iterator tells_it = collected_tells.begin();
  PacketSenderInterval psi(ml_, debug_mode_);

  while((tells_it) != collected_tells.end())
  {
    psi.visit((*tells_it));
    tells_it++;
  }

  // ����X�g�Astore��Mathematica�ɓn��
  psi.put_cs(constraint_store);

  // tellsvars��n��
  psi.put_vars();

  // storevars��n��
  psi.put_cs_vars(constraint_store);

  // ���ʂ��󂯎��O�ɐ���X�g�A��������
  constraint_store.first.clear();  
  constraint_store.second.clear();  

/*
ml_.skip_pkt_until(RETURNPKT);
// �Ԃ��Ă���p�P�b�g�����
PacketChecker pc(ml_);
pc.check2();
*/

  ml_.skip_pkt_until(RETURNPKT);

  // �����Ȃ������ꍇ��0���Ԃ�i����Ԃɖ���������A�܂���over-constraint�Ƃ������Ɓj
  if(ml_.MLGetType() == MLTKINT)
  {
    if(debug_mode_) std::cout << "ConsistencyCheckerInterval: false" << std::endl;
    ml_.MLNewPacket();
    return false;
  }

  // List[���l, ����ꗗ, �ϐ��ꗗ]���Ԃ�
  // ���l�����͖��Ȃ���������1�Aunder-constraint���N���Ă����2���Ԃ�

  ml_.MLGetNext(); // List�Ƃ����֐���
  ml_.MLGetNext(); // List�̒��̐擪

  int n;
  if(! ml_.MLGetInteger(&n)){
    std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
    throw MathLinkError("MLGetInteger", ml_.MLError());
  }

  if(debug_mode_) {
    std::cout << "ConsistencyCheckerInterval: " << n  << std::endl;
    if(n==2) std::cout << "under-constraint" << std::endl;
    std::cout << "---build constraint store---" << std::endl;
  }

  ml_.MLGetNext(); // List�֐�

  // List�֐��̗v�f���i����̌��j�𓾂�
  int cons_size;
  if(! ml_.MLGetArgCount(&cons_size)){
    std::cout << "#funcCase:MLGetArgCount:unable to get the number of arguments from ml" << std::endl;
    throw MathLinkError("MLGetArgCount", ml_.MLError());
    return false;
  }
  ml_.MLGetNext(); // List�Ƃ����֐���
  ml_.MLGetNext(); // List�̒��̐擪�v�f

  std::set<SymbolicValue> value_set;    
  for(int i=0; i<cons_size; i++)
  {
    std::string str = ml_.get_string();
    SymbolicValue symbolic_value;
    symbolic_value.str = str;
    value_set.insert(symbolic_value);
  }
  constraint_store.first.insert(value_set);


  // �o������ϐ��̈ꗗ��������ŕԂ��Ă���̂ł���𐧖�X�g�A�ɓ����
  ml_.MLGetNext(); // List�֐�

  // List�֐��̗v�f���i�ϐ��ꗗ�Ɋ܂܂��ϐ��̌��j�𓾂�
  int vars_size;
  if(! ml_.MLGetArgCount(&vars_size)){
    std::cout << "#funcCase:MLGetArgCount:unable to get the number of arguments from ml" << std::endl;
    throw MathLinkError("MLGetArgCount", ml_.MLError());
    return false;
  }
  ml_.MLGetNext(); // List�Ƃ����֐���

  for(int k=0; k<vars_size; k++)
  {
    SymbolicVariable symbolic_variable;
    std::string sym;
    ml_.MLGetNext(); // Derivative[number][�ϐ���][]�܂���x[]�Ȃǂ̊֐�
    switch(ml_.MLGetNext()) // Derivative[number][�ϐ���]�܂���x�Ƃ����֐���
    {
      case MLTKFUNC:
        ml_.MLGetNext(); // Derivative[number]�Ƃ����֐���
        ml_.MLGetNext(); // Derivative�Ƃ����֐���
        ml_.MLGetNext(); // number
        int n;
        if(! ml_.MLGetInteger(&n)){
          std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
          throw MathLinkError("MLGetInteger", ml_.MLError());
          return false;
        }
        symbolic_variable.derivative_count = n;
        ml_.MLGetNext(); // �ϐ���
        symbolic_variable.name = ml_.get_symbol();
        ml_.MLGetNext(); // t
        break;
      case MLTKSYM:
        sym = ml_.get_symbol();
        symbolic_variable.derivative_count = 0;
        ml_.MLGetNext(); // t
        symbolic_variable.name = sym;
        break;
      default:
        ;
    }
    constraint_store.second.insert(symbolic_variable);
  }
  ml_.MLNewPacket(); // �G���[���p�B�G���[�̌����s��

  if(debug_mode_) {
    std::set<std::set<SymbolicValue> >::iterator or_cons_it;
    std::set<SymbolicValue>::iterator and_cons_it;
    or_cons_it = constraint_store.first.begin();
    while((or_cons_it) != constraint_store.first.end())
    {
      and_cons_it = (*or_cons_it).begin();
      while((and_cons_it) != (*or_cons_it).end())
      {
        std::cout << (*and_cons_it).str << " ";
        and_cons_it++;
      }
      std::cout << std::endl;
      or_cons_it++;
    }
    std::cout << "----------------------------" << std::endl;
  }

  return n >= 1;
}


} //namespace symbolic_simulator
} // namespace hydla
