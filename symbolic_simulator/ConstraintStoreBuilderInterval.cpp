#include "ConstraintStoreBuilderInterval.h"
#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {


ConstraintStoreBuilderInterval::ConstraintStoreBuilderInterval(bool debug_mode) :
  debug_mode_(debug_mode)
{}

ConstraintStoreBuilderInterval::~ConstraintStoreBuilderInterval()
{}

void ConstraintStoreBuilderInterval::build_constraint_store(const variable_map_t& variable_map)
{
/*
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
    std::cout << "--build constraint store--" << std::endl;
  }

  SymbolicValue symbolic_value;
  std::string value;
  SymbolicVariable symbolic_variable;
  std::string variable_name;

  variable_map_t::variable_list_t::const_iterator it = variable_map.begin();

  std::set<SymbolicValue> value_set;
  while(it != variable_map.end())
  {
    symbolic_value = (*it).second;    
    value = symbolic_value.str;
    if(value != "") break;
    it++;
  }

  while(it != variable_map.end())
  {
    symbolic_variable = (*it).first;
    symbolic_value = (*it).second;    
    variable_name = symbolic_variable.name;
    value = symbolic_value.str;

    std::string str = "";

    // SymbolicVariable側に関する文字列を作成
    str += "Equal[";
    if(symbolic_variable.derivative_count > 0)
    {
      std::ostringstream derivative_count;
      derivative_count << symbolic_variable.derivative_count;
      str += "Derivative[";
      str += derivative_count.str();
      str += "][usrVar";
      str += variable_name;
      str += "][0]";
    }
    else
    {
      str += "usrVar";
      str += variable_name;
      str += "[0]";
    }

    str += ",";

    // SymbolicValue側に関する文字列を作成
    str += value;
    str += "]"; // Equalの閉じ括弧

    SymbolicValue new_symbolic_value;
    new_symbolic_value.str = str;
    value_set.insert(new_symbolic_value);


    // 制約ストア内の変数一覧を作成
    symbolic_variable.name = "usrVar" + variable_name;
    constraint_store_.second.insert(symbolic_variable);

    it++;
    while(it != variable_map.end())
    {
      symbolic_value = (*it).second;
      value = symbolic_value.str;
      if(value != "") break;
      it++;
    }
  }
  constraint_store_.first.insert(value_set);


  if(debug_mode_)
  {
    std::set<std::set<SymbolicValue> >::iterator or_cons_it;
    std::set<SymbolicValue>::iterator and_cons_it;
    std::set<SymbolicVariable>::iterator vars_it = constraint_store_.second.begin();

    or_cons_it = constraint_store_.first.begin();
    while((or_cons_it) != constraint_store_.first.end())
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

    while((vars_it) != constraint_store_.second.end())
    {
      std::cout << *(vars_it) << " ";
      vars_it++;
    }
    std::cout << std::endl;
    std::cout << "--------------------------" << std::endl;
  }
*/
}

void ConstraintStoreBuilderInterval::build_variable_map(variable_map_t& variable_map)
{
/*
  // constraint_store をもとに variable_map をつくる

  std::set<std::set<SymbolicValue> >::iterator or_cons_it = constraint_store_.first.begin();
  // Orでつながった制約のうち、最初の1つだけを採用することにする
  std::set<SymbolicValue>::iterator and_cons_it = (*or_cons_it).begin();
  while((and_cons_it) != (*or_cons_it).end())
  {
    std::string cons_str = (*and_cons_it).str;
    // cons_strは"Equal[usrVarx,2]"や"Equal[Derivative[1][usrVary],3]"など

    unsigned int loc = cons_str.find("Equal[", 0);
    loc += 6; // 文字列"Equal["の長さ分
    unsigned int comma_loc = cons_str.find(",", loc);
    if(comma_loc == std::string::npos)
    {
      std::cout << "can't find comma." << std::endl;
      return;
    }
    std::string variable_str = cons_str.substr(loc, comma_loc-loc);
    // variable_strは"usrVarx"や"Derivative[1][usrVarx]"など

    // nameとderivative_countへの分離
    std::string variable_name;
    int variable_derivative_count;
    unsigned int variable_loc = variable_str.find("Derivative[", 0);
    if(variable_loc != std::string::npos)
    {
      variable_loc += 11; // "Derivative["の長さ分
      unsigned int bracket_loc = variable_str.find("][", variable_loc);
      if(bracket_loc == std::string::npos)
      {
        std::cout << "can't find bracket." << std::endl;
        return;
      }
      std::string variable_derivative_count_str = variable_str.substr(variable_loc, bracket_loc-variable_loc);
      variable_derivative_count = std::atoi(variable_derivative_count_str.c_str());
      variable_loc = bracket_loc + 2; // "]["の長さ分
      bracket_loc = variable_str.find("]", variable_loc);
      if(bracket_loc == std::string::npos)
      {
        std::cout << "can't find bracket." << std::endl;
        return;
      }
      variable_loc += 6; // "usrVar"の長さ分
      variable_name =  variable_str.substr(variable_loc, bracket_loc-variable_loc);
    }
    else
    {
      variable_name =  variable_str.substr(6); // "usrVar"の長さ分
      variable_derivative_count = 0;
    }

    // 値の取得
    int str_size = cons_str.size();
    unsigned int end_loc = cons_str.rfind("]", str_size-1);

    if(end_loc == std::string::npos)
    {
      std::cout << "can't find bracket." << std::endl;
      return;
    }
    std::string value_str = cons_str.substr(comma_loc + 1, end_loc - (comma_loc + 1));

    SymbolicVariable symbolic_variable;
    SymbolicValue symbolic_value;
    symbolic_variable.name = variable_name;
    symbolic_variable.derivative_count = variable_derivative_count;
    symbolic_value.str = value_str;

    variable_map.set_variable(symbolic_variable, symbolic_value); 
    and_cons_it++;
  }

  // [t]を除く処理は要らなさそう？
  */
}

} //namespace symbolic_simulator
} // namespace hydla
