#include "ConsistencyChecker.h"
//#include "PacketChecker.h"
#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {

ConsistencyChecker::ConsistencyChecker(MathLink& ml, bool debug_mode) :
  ml_(ml),
  debug_mode_(debug_mode)
{}

ConsistencyChecker::~ConsistencyChecker()
{}


bool ConsistencyChecker::is_consistent(tells_t& collected_tells, 
                                       ConstraintStore& constraint_store)
{

/*
  ml_.put_function("isConsistent", 2);
  ml_.put_function("List", 3);
  ml_.put_function("Equal", 2);
  ml_.put_symbol("x");
  ml_.put_symbol("y");
  ml_.put_function("Equal", 2);
  ml_.put_symbol("x");
  ml_.MLPutInteger(2);
  ml_.put_function("Equal", 2);
  ml_.put_symbol("y");
  ml_.MLPutInteger(1);

  ml_.put_function("List", 2);
  ml_.put_symbol("x");
  ml_.put_symbol("y");
  ml_.MLEndPacket();
*/

  // isConsistent[expr, vars]��n������
  ml_.put_function("isConsistent", 2);

  ml_.put_function("Join", 2);
  // tell����̏W������expr�𓾂�Mathematica�ɓn��
  int tells_size = collected_tells.size();
  ml_.put_function("List", tells_size);
  tells_t::iterator tells_it = collected_tells.begin();
  PacketSender ps(ml_, NP_POINT_PHASE);
  while((tells_it) != collected_tells.end())
  {
    ps.visit((*tells_it));
    tells_it++;
  }

  // ����X�g�A�����expr�𓾂�Mathematica�ɓn��
  ps.put_cs(constraint_store);


  // vars��n��
  ml_.put_function("Join", 2);
  ps.put_vars();
  // ����X�g�A���ɏo������ϐ����n��
  ps.put_cs_vars(constraint_store);


  // ���ʂ��󂯎��O�ɐ���X�g�A��������
  constraint_store.first.clear();
  constraint_store.second.clear();

/*
//ml_.skip_pkt_until(RETURNPKT);
// �Ԃ��Ă���p�P�b�g�����
PacketChecker pc(ml_);
pc.check();
*/

  ml_.skip_pkt_until(RETURNPKT);
  // �����Ȃ������ꍇ��0���Ԃ�i����Ԃɖ���������Ƃ������Ɓj
  if(ml_.MLGetType() == MLTKINT)
  {
    if(debug_mode_) std::cout << "ConsistencyChecker: false" << std::endl;
    ml_.MLNewPacket();
    return false;
  }

  // �������ꍇ�͉����u������Łv�Ԃ��Ă���̂ł���𐧖�X�g�A�ɓ����
  // List[List[List["Equal[x, 1]"], List["Equal[x, -1]"]], List[x]]��
  // List[List[List["Equal[x, 1]", "Equal[y, 1]"], List["Equal[x, -1]", "Equal[y, -1]"]], List[x, y, z]]��
  // List[List[List["Equal[x,1]", "Equal[Derivative[1][x],1]", "Equal[prev[x],1]", "Equal[prev[Derivative[2][x]],1]"]],
  // List[x, Derivative[1][x], prev[x], prev[Derivative[2][x]]]]  ��List[List["True"], List[]]�Ȃ�
  if(debug_mode_) {
    std::cout << "---build constraint store---" << std::endl;
  }

  ml_.MLGetNext(); // List�Ƃ����֐���
  ml_.MLGetNext(); // List�֐��iOr�Ō��΂ꂽ����\���Ă���j

  // List�֐��̗v�f���iOr�Ō��΂ꂽ���̌��j�𓾂�
  int or_size;
  if(! ml_.MLGetArgCount(&or_size)){
    std::cout << "#funcCase:MLGetArgCount:unable to get the number of arguments from ml" << std::endl;
    throw MathLinkError("MLGetArgCount", ml_.MLError());
    return false;
  }
  ml_.MLGetNext(); // List�Ƃ����֐���

  for(int i=0; i<or_size; i++)
  {
    ml_.MLGetNext(); // List�֐��iAnd�Ō��΂ꂽ����\���Ă���j

    // List�֐��̗v�f���iAnd�Ō��΂ꂽ���̌��j�𓾂�
    int and_size;
    if(! ml_.MLGetArgCount(&and_size)){
      std::cout << "#funcCase:MLGetArgCount:unable to get the number of arguments from ml" << std::endl;
      throw MathLinkError("MLGetArgCount", ml_.MLError());
      return false;
    }
    ml_.MLGetNext(); // List�Ƃ����֐���
    ml_.MLGetNext(); // List�̒��̐擪�v�f

    std::set<SymbolicValue> value_set;    
    for(int j=0; j<and_size; j++)
    {
      std::string str = ml_.get_string();
      SymbolicValue symbolic_value;
      symbolic_value.str = str;
      value_set.insert(symbolic_value);
    }
    constraint_store.first.insert(value_set);
  }


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
    switch(ml_.MLGetNext())
    {
      case MLTKFUNC: // Derivative[number][]
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
        ml_.MLGetNext(); // �ϐ�
        symbolic_variable.name = ml_.get_symbol();
        break;
      case MLTKSYM: // �V���{���i�L���jx�Ƃ�y�Ƃ�
        symbolic_variable.derivative_count = 0;
        symbolic_variable.name = ml_.get_symbol();
        break;
      default:
        ;
    }

    constraint_store.second.insert(symbolic_variable);
  }


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
    std::cout << "ConsistencyChecker: true" << std::endl;
  }

  return true;
}


} //namespace symbolic_simulator
} // namespace hydla
