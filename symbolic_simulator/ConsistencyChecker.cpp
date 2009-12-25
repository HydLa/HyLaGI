#include "ConsistencyChecker.h"
//#include "PacketChecker.h"
#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {

ConsistencyChecker::ConsistencyChecker(MathLink& ml) :
  ml_(ml)
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
  PacketSender ps(ml_);
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
  ml_.put_function("ToExpression", 1);
  ml_.put_string(constraint_store.second.str);

/*
ml_.skip_pkt_until(RETURNPKT);
// �Ԃ��Ă���p�P�b�g�����
PacketChecker pc(ml_);
pc.check2();
*/

  ml_.skip_pkt_until(RETURNPKT);
  // �����Ȃ������ꍇ��0���Ԃ�i����Ԃɖ���������Ƃ������Ɓj
  if(ml_.MLGetType() == MLTKINT)
  {
    std::cout << "ConsistencyChecker: false" << std::endl;
    ml_.MLNewPacket();
    return false;
  }

  // �������ꍇ�͊e�ϐ����Ƃ��̒l���u������Łv�Ԃ��Ă���̂ł���𐧖�X�g�A�ɓ����
  // List["Equal[x,1]", "List[x]"]��List["And[Equal[x, 1], Equal[y, 2], Equal[z, 3]]", "List[x, y, z]"]��
  // List["And[Equal[x, 1], Equal[Derivative[1][x], 1], Equal[prev[x], 1], Equal[prev[Derivative[2][x]], 1]]",
  //      "List[x, Derivative[1][x], prev[x], prev[Derivative[2][x]]]"]  ��List["True", "List[]"]�Ȃ�
  std::cout << "---build constraint store---" << std::endl;

  ml_.MLGetNext(); // List�֐�
  ml_.MLGetNext(); // List�Ƃ����֐���
  std::string str = ml_.get_string();
  SymbolicValue symbolic_value;
  symbolic_value.str = str;
  constraint_store.first = symbolic_value;

  // �o������ϐ��̈ꗗ��������ŕԂ��Ă���̂ł���𐧖�X�g�A�ɓ����
  str = ml_.get_string();
  SymbolicValue vars_list;
  vars_list.str = str;
  constraint_store.second = vars_list;
  
/*
  // �������ꍇ�͊e�ϐ����Ƃ��̒l���Ԃ��Ă���̂ł���𐧖�X�g�A�ɓ����
  // List[pair[x,1]]��List[pair[x,1], pair[y,2], pair[z,3]]��
  // List[pair[x,1], pair[Derivative[1][x],1], pair[prev[x],1], pair[prev[Derivative[2][x],1]]]��List[]�Ȃ�
  std::cout << "---build constraint store---" << std::endl;

  // �ŏ���List�֐����͂��̂ł��̗v�f���ipair�̌��j�𓾂�
  int funcarg;
  if(! ml_.MLGetArgCount(&funcarg)){
    std::cout << "#funcCase:MLGetArgCount:unable to get the number of arguments from ml" << std::endl;
    return false;
  }

  ml_.MLGetNext(); // List�Ƃ����֐���

  SymbolicVariable symbolic_variable;
  SymbolicValue symbolic_value;
  // List�Ɋ܂܂��1��1��pair�ɂ��Ē��ׂ�
  for(int i = 0; i < funcarg; i++)
  {
    ml_.MLGetNext(); // pair�֐���������
    ml_.MLGetNext(); // pair�Ƃ����֐���
//    symbolic_variable.previous = false;    
  A:
    switch(ml_.MLGetNext()) // pair[variable, value]��variable����������
    {
    case MLTKFUNC: // Derivative[number][]��prev[]
      switch(ml_.MLGetNext()) // Derivative[number]��prev�Ƃ����֐���
      {
      case MLTKFUNC: // Derivative[number]
        ml_.MLGetNext(); // Derivative�Ƃ����֐���
        ml_.MLGetNext(); // number
        int n;
        if(! ml_.MLGetInteger(&n)){
          std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
          return false;
        }
        symbolic_variable.derivative_count = n;
        ml_.MLGetNext(); // �ϐ�
        symbolic_variable.name = ml_.get_symbol();
        break;
      case MLTKSYM: // prev
//        symbolic_variable.previous = true;
        goto A; // prev�̒��g�𒲂ׂ�i�ʏ�ϐ��̏ꍇ��Derivative���̏ꍇ�Ƃ�����j
        break;
      default:
        ;
      }
      break;
    case MLTKSYM: // �V���{���i�L���jx�Ƃ�y�Ƃ�
      symbolic_variable.derivative_count = 0;
      symbolic_variable.name = ml_.get_symbol();
      break;
    default:
      ;
    }

    int numerator;
    int denominator;
    switch(ml_.MLGetNext()) // pair[variable, value]��value����������
    {
      case MLTKFUNC: // Rational�֐�
        symbolic_value.rational = true;
        ml_.MLGetNext(); // Rational�Ƃ����֐���
        ml_.MLGetNext(); // ���q
        if(! ml_.MLGetInteger(&numerator)){
          std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
          return false;
        }
        symbolic_value.numerator = numerator;        
        ml_.MLGetNext(); // ����
        if(! ml_.MLGetInteger(&denominator)){
          std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
          return false;
        }
        symbolic_value.denominator = denominator;
        break;
      case MLTKINT:
        symbolic_value.rational = false;
        if(! ml_.MLGetInteger(&numerator)){
          std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
          return false;
        }
        symbolic_value.numerator = numerator;        
        symbolic_value.denominator = 1;
        break;
      default:
        ;
    }   
    constraint_store.set_variable(symbolic_variable, symbolic_value); 
  }
*/

  std::cout << constraint_store.first << std::endl;
  std::cout << "----------------------------" << std::endl;
  std::cout << "ConsistencyChecker: true" << std::endl;

  return true;
}


} //namespace symbolic_simulator
} // namespace hydla
