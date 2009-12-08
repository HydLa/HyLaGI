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


bool ConsistencyChecker::is_consistent(TellCollector::tells_t& collected_tells, ConstraintStore constraint_store)
{

/*
  ml_.MLPutFunction("isConsistent", 2);
  ml_.MLPutFunction("List", 3);
  ml_.MLPutFunction("Equal", 2);
  ml_.MLPutSymbol("x");
  ml_.MLPutSymbol("y");
  ml_.MLPutFunction("Equal", 2);
  ml_.MLPutSymbol("x");
  ml_.MLPutInteger(2);
  ml_.MLPutFunction("Equal", 2);
  ml_.MLPutSymbol("y");
  ml_.MLPutInteger(1);

  ml_.MLPutFunction("List", 2);
  ml_.MLPutSymbol("x");
  ml_.MLPutSymbol("y");
  ml_.MLEndPacket();
*/


  // isConsistent[expr, vars]��n������
  ml_.MLPutFunction("isConsistent", 2);


  ml_.MLPutFunction("Join", 2);
  // tell����̏W������expr�𓾂�Mathematica�ɓn��
  int tells_size = collected_tells.size();
  ml_.MLPutFunction("List", tells_size);
  TellCollector::tells_t::iterator tells_it = collected_tells.begin();
  PacketSender ps(ml_);
  while((tells_it) != collected_tells.end())
  {
    ps.visit((*tells_it));
    tells_it++;
  }

  // ����X�g�A�����expr�𓾂�Mathematica�ɓn��
  ps.put_cs(constraint_store);


  // vars��n��
  ps.put_vars();


/*
// �Ԃ��Ă���p�P�b�g�����
PacketChecker pc(ml_);
pc.check();
*/


  ml_.skip_pkt_until(RETURNPKT);
  // �����Ȃ������ꍇ��0���Ԃ�i����Ԃɖ���������Ƃ������Ɓj
  if(ml_.MLGetType() == MLTKINT)
  {
    std::cout << "ConsistencyChecker: false" << std::endl;
    return false;
  }
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
    std::cout << " ";
    const char* symname;
    ml_.MLGetNext(); // pair�֐���������
    ml_.MLGetNext(); // pair�Ƃ����֐���
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
        symbolic_variable.derivative_count = (unsigned int)n;
        ml_.MLGetNext(); // �ϐ�
        if(! ml_.MLGetSymbol(&symname)){
          std::cout << "MLGetSymbol:unable to read the symbol from ml" << std::endl;
          return false;
        }
        symbolic_variable.name = symname;
        std::cout << "Derivative[" << n << "][" << symname << "]";
        break;
      case MLTKSYM: // prev
//        symbolic_variable.previous = true;
        //std::cout << "prev[";
        goto A; // prev�̒��g�𒲂ׂ�i�ʏ�ϐ��̏ꍇ��Derivative���̏ꍇ�Ƃ�����j
        break;
      default:
        ;
      }
      break;
    case MLTKSYM: // �V���{���i�L���jx�Ƃ�y�Ƃ�
      if(! ml_.MLGetSymbol(&symname)){
        std::cout << "MLGetSymbol:unable to read the symbol from ml" << std::endl;
        return false;
      }
      symbolic_variable.name = symname;
      std::cout << symname;
      break;
    default:
      ;
    }
    ml_.MLReleaseSymbol(symname);

    std::cout << "=";
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
        std::cout << "Rational[" << numerator << "," << denominator << "]";
        break;
      case MLTKINT:
        symbolic_value.rational = false;
        if(! ml_.MLGetInteger(&numerator)){
          std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
          return false;
        }
        symbolic_value.numerator = numerator;        
        symbolic_value.denominator = 1;
        std::cout << numerator;
        break;
      default:
        ;
    }   
    //(constraint_store.variables).insert(std::make_pair(symbolic_variable, symbolic_value)); 
    //constraint_store.dump(std::cout);
    std::cout << std::endl;
  }

  std::cout << "--------------------------" << std::endl;
  std::cout << "ConsistencyChecker: true" << std::endl;

  return true;
}


} //namespace symbolic_simulator
} // namespace hydla
