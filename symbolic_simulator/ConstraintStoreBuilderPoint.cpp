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
  // variable_map をもとに constraint_store をつくる

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
  // strには
  // "And[Equal[x,1]]"や"And[Equal[x,1],Equal[y,2],Equal[z,3]]"や
  // "And[Equal[x,1],Equal[Derivative[1][x],1],Equal[prev[x],1],Equal[prev[Derivative[2][x]],1]]"や
  // "And[]"？などが入る
  std::string vars_list = "";
  // vars_listには
  // "List[x]"や"List[x,y,z]"や"List[x,Derivative[1][x],prev[x],prev[Derivative[2][x]]"や
  // "List[]"などが入る

  str.append("And[");
  vars_list.append("List[");
  variable_map_t::variable_list_t::iterator it = variable_map.begin();
  while(true)
  {
    SymbolicVariable symbolic_variable = (*it).first;
    SymbolicValue symbolic_value = (*it).second;    
    std::string name = symbolic_variable.name;
    std::string value = symbolic_value.str;

    // SymbolicVariable側に関する文字列を作成
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

    // SymbolicValue側に関する文字列を作成
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

  // createVariableList[制約ストアの式, 制約ストアに出現する変数の一覧, {}]を送信
  ml_.put_function("createVariableList", 3);
  ml_.put_function("ToExpression", 1);
  ml_.put_string(constraint_store_.first.str);
  ml_.put_function("ToExpression", 1);
  ml_.put_string(constraint_store_.second.str);
  ml_.put_function("List", 0);

  ml_.skip_pkt_until(RETURNPKT);

  // 各変数名とその値（文字列）が返ってくるのでそれを変数表に入れる
  // List[pair[x,"1"]]やList[pair[x,"1"], pair[y,"2"], pair[z,"3"]]や
  // List[pair[x,"1"], pair[Derivative[1][x],"1"], pair[prev[x],"1"], pair[prev[Derivative[2][x],"1"]]]やList[]など

  // 最初はList関数が届くのでその要素数（pairの個数）を得る
  ml_.MLGetType(); // MLGetTypeしてからでないとMLGetArgCountでエラーになる（原因はよく分からない？）
  int funcarg;
  if(! ml_.MLGetArgCount(&funcarg)){
    std::cout << "#funcCase:MLGetArgCount:unable to get the number of arguments from ml" << std::endl;
    throw MathLinkError("MLGetArgCount", ml_.MLError());
  }

  ml_.MLGetNext(); // Listという関数名

  SymbolicVariable symbolic_variable;
  SymbolicValue symbolic_value;
  // Listに含まれる1つ1つのpairについて調べる
  for(int i = 0; i < funcarg; i++)
  {
    ml_.MLGetNext(); // pair関数が得られる
    ml_.MLGetNext(); // pairという関数名
//    symbolic_variable.previous = false;    
  A:
    switch(ml_.MLGetNext()) // pair[variable, value]のvariable側が得られる
    {
    case MLTKFUNC: // Derivative[number][]とprev[]
      switch(ml_.MLGetNext()) // Derivative[number]やprevという関数名
      {
      case MLTKFUNC: // Derivative[number]
        ml_.MLGetNext(); // Derivativeという関数名
        ml_.MLGetNext(); // number
        int n;
        if(! ml_.MLGetInteger(&n)){
          std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
          throw MathLinkError("MLGetInteger", ml_.MLError());
        }
        symbolic_variable.derivative_count = n;
        ml_.MLGetNext(); // 変数
        symbolic_variable.name = ml_.get_symbol();
        break;
      case MLTKSYM: // prev
//        symbolic_variable.previous = true;
        goto A; // prevの中身を調べる（通常変数の場合とDerivativeつきの場合とがある）
        break;
      default:
        ;
      }
      break;
    case MLTKSYM: // シンボル（記号）xとかyとか
      symbolic_variable.derivative_count = 0;
      symbolic_variable.name = ml_.get_symbol();
      break;
    default:
      ;
    }

    std::string str = ml_.get_string(); //pair[variable, value]のvalue側（文字列）が得られる
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
