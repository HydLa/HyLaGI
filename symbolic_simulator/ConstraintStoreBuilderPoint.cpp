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

void ConstraintStoreBuilderPoint::build_constraint_store(variable_map_t variable_map)
{
  // variable_map �����Ƃ� constraint_store ������

  if(debug_mode_) std::cout << "------Variable map------" << std::endl;
  if(variable_map.size() == 0)
  {
    if(debug_mode_)
    {
      std::cout << "no Variables" << std::endl;
      std::cout << "-------------------------" << std::endl;
    }
    return;
  }
  if(debug_mode_)
  {
    std::cout << variable_map;
    std::cout << "------------------------" << std::endl;
  }

  if(debug_mode_) std::cout << "--build constraint store--" << std::endl;
  std::string str = "";
  // str�ɂ�
  // "And[Equal[x,1]]"��"And[Equal[x,1],Equal[y,2],Equal[z,3]]"��
  // "And[Equal[x,1],Equal[Derivative[1][x],1],Equal[prev[x],1],Equal[prev[Derivative[2][x]],1]]"��
  // "And[]"�H�Ȃǂ�����
  std::string vars_list = "";
  // vars_list�ɂ�
  // "List[x]"��"List[x,y,z]"��"List[x,Derivative[1][x],prev[x],prev[Derivative[2][x]]"��
  // "List[]"�Ȃǂ�����

  str.append("And[");
  vars_list.append("List[");
  variable_map_t::variable_list_t::iterator it = variable_map.begin();
  while(true)
  {
    SymbolicVariable symbolic_variable = (*it).first;
    SymbolicValue symbolic_value = (*it).second;    
    std::string name = symbolic_variable.name;
    std::string value = symbolic_value.str;

    // SymbolicVariable���Ɋւ��镶������쐬
    str += "Equal[";
    if(symbolic_variable.derivative_count > 0)
    {
      std::ostringstream derivative_count;
      derivative_count << symbolic_variable.derivative_count;
      str += "Derivative[";
      str += derivative_count.str();
      str += "][";
      str += name;
      str += "]";
      vars_list += "Derivative[";
      vars_list += derivative_count.str();
      vars_list += "][";
      vars_list += name;
      vars_list += "]";
    }
    else
    {
      str += name;
      vars_list += name;
    }

    str += ",";

    // SymbolicValue���Ɋւ��镶������쐬
    str += value;

    it++;
    if(it == variable_map.end()) break;
    str += "],";
    vars_list += ",";
  }

  str += "]]";
  vars_list += "]";

  constraint_store_.first.str = str;
  constraint_store_.second.str = vars_list;

  if(debug_mode_)
  {
    std::cout << constraint_store_.first << std::endl;
    std::cout << constraint_store_.second << std::endl;
    std::cout << "--------------------------" << std::endl;
  }
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

  return variable_map;
}

ConstraintStore& ConstraintStoreBuilderPoint::getcs()
{
  return this->constraint_store_;
}

} //namespace symbolic_simulator
} // namespace hydla
