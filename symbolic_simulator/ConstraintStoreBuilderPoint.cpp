#include "ConstraintStoreBuilderPoint.h"
#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {


ConstraintStoreBuilderPoint::ConstraintStoreBuilderPoint(MathLink& ml, bool debug_mode) :
  ml_(ml),
  debug_mode_(debug_mode)
{
  constraint_store_.first.str = "True";
  constraint_store_.second.str = "{}";
}

ConstraintStoreBuilderPoint::~ConstraintStoreBuilderPoint()
{}

void ConstraintStoreBuilderPoint::build_constraint_store( /*variable_map_t variable_map */ )
{
  /* this->constraint_store_ = variable_map */ ;
}

variable_map_t ConstraintStoreBuilderPoint::build_variable_map()
{

  variable_map_t variable_map;

  // createVariableList[����X�g�A�̎�, ����X�g�A�ɏo������ϐ��̈ꗗ, {}]�𑗐M
  ml_.put_function("createVariableList", 3);
  ml_.put_function("ToExpression", 1);
  ml_.put_string(constraint_store_.first.str);
  ml_.put_function("ToExpression", 1);
  ml_.put_string(constraint_store_.second.str);
  ml_.put_function("List", 0);

  ml_.skip_pkt_until(RETURNPKT);

  // �e�ϐ����Ƃ��̒l�i������j���Ԃ��Ă���̂ł����ϐ��\�ɓ����
  // List[pair[x,"1"]]��List[pair[x,"1"], pair[y,"2"], pair[z,"3"]]��
  // List[pair[x,"1"], pair[Derivative[1][x],"1"], pair[prev[x],"1"], pair[prev[Derivative[2][x],"1"]]]��List[]�Ȃ�
  if(debug_mode_) std::cout << "--------Variable Map--------" << std::endl;

  // �ŏ���List�֐����͂��̂ł��̗v�f���ipair�̌��j�𓾂�
  ml_.MLGetType(); // MLGetType���Ă���łȂ���MLGetArgCount�ŃG���[�ɂȂ�i�����͂悭������Ȃ��H�j
  int funcarg;
  if(! ml_.MLGetArgCount(&funcarg)){
    std::cout << "#funcCase:MLGetArgCount:unable to get the number of arguments from ml" << std::endl;
    throw MathLinkError("MLGetArgCount", ml_.MLError());
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
          throw MathLinkError("MLGetInteger", ml_.MLError());
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

    std::string str = ml_.get_string(); //pair[variable, value]��value���i������j��������
    symbolic_value.str = str;

    variable_map.set_variable(symbolic_variable, symbolic_value); 
  }
  if(debug_mode_) {
    std::cout << variable_map;
    std::cout << "----------------------------" << std::endl;
  }
  return variable_map;
}

ConstraintStore& ConstraintStoreBuilderPoint::getcs()
{
  return this->constraint_store_;
}

} //namespace symbolic_simulator
} // namespace hydla
