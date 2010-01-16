#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_MATHCONSTRAINTSTORE_POINT_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_MATHCONSTRAINTSTORE_POINT_H_

#include "mathlink_helper.h"
#include "Logger.h"
#include "MathValue.h"
#include "MathVariable.h"
#include "VariableMap.h"

#include "MathVCSType.h"

namespace hydla {
namespace vcs {
namespace mathematica {

struct MathConstraintStorePoint
{
  typedef virtual_constraint_solver_t::variable_map_t variable_map_t;
  typedef std::pair<std::set<std::set<MathValue> >, std::set<MathVariable> > store_t;
  store_t store;

  /**
   * 制約ストアの初期化をおこなう
   */
  bool reset()
  {
    store = store_t();
    return true;
  }

  /**
   * variable_map をもとに constraint_store をつくる
   */
  bool reset(const variable_map_t& variable_map)
  {
    HYDLA_LOGGER_DEBUG("----- Reset Constraint Store -----");

    if(variable_map.size() == 0)
    {
      HYDLA_LOGGER_DEBUG("no Variables");
      return true;
    }
    HYDLA_LOGGER_DEBUG("------Variable map------", variable_map);

    MathValue symbolic_value;    
    std::string value;
    MathVariable symbolic_variable;
    std::string variable_name;

    variable_map_t::variable_list_t::const_iterator it = variable_map.begin();

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

      // MathVariable側に関する文字列を作成
      str += "Equal[";
      str += "prev[";
      if(symbolic_variable.derivative_count > 0)
      {
        std::ostringstream derivative_count;
        derivative_count << symbolic_variable.derivative_count;
        str += "Derivative[";
        str += derivative_count.str();
        str += "][usrVar";
        str += variable_name;
        str += "]";
      }
      else
      {
        str += "usrVar";
        str += variable_name;
      }
      str += "]"; // prevの閉じ括弧

      str += ",";

      // MathValue側に関する文字列を作成
      str += value;
      str += "]"; // Equalの閉じ括弧

      MathValue new_symbolic_value;
      new_symbolic_value.str = str;
      std::set<MathValue> value_set;
      value_set.insert(new_symbolic_value);
      store.first.insert(value_set);


      // 制約ストア内の変数一覧を作成
//    std::string vars_name;
      symbolic_variable.name = "prev[usrVar" + variable_name + "]";
      store.second.insert(symbolic_variable);

      it++;
      while(it != variable_map.end())
      {
        symbolic_value = (*it).second;
        value = symbolic_value.str;
        if(value != "") break;
        it++;
      }
    }

//     if(debug_mode_)
//     {
//       std::set<std::set<MathValue> >::const_iterator or_cons_it;
//       std::set<MathValue>::const_iterator and_cons_it;
//       std::set<MathVariable>::const_iterator vars_it = store.second.begin();

//       or_cons_it = store.first.begin();
//       while((or_cons_it) != store.first.end())
//       {
//         and_cons_it = (*or_cons_it).begin();
//         while((and_cons_it) != (*or_cons_it).end())
//         {
//           std::cout << (*and_cons_it).str << " ";
//           and_cons_it++;
//         }
//         std::cout << std::endl;
//         or_cons_it++;
//       }

//       while((vars_it) != store.second.end())
//       {
//         std::cout << *(vars_it) << " ";
//         vars_it++;
//       }
//       std::cout << std::endl;
//       std::cout << "--------------------------" << std::endl;
//     }

    return true;
  }

    
  /**
   * constraint_store をもとに variable_map をつくる
   */
  bool create_variable_map(variable_map_t& variable_map) const
  {
    std::set<std::set<MathValue> >::const_iterator or_cons_it = store.first.begin();
    // Orでつながった制約のうち、最初の1つだけを採用することにする
    std::set<MathValue>::const_iterator and_cons_it = (*or_cons_it).begin();
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
        return false;
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
          return false;
        }
        std::string variable_derivative_count_str = variable_str.substr(variable_loc, bracket_loc-variable_loc);
        variable_derivative_count = std::atoi(variable_derivative_count_str.c_str());
        variable_loc = bracket_loc + 2; // "]["の長さ分
        bracket_loc = variable_str.find("]", variable_loc);
        if(bracket_loc == std::string::npos)
        {
          std::cout << "can't find bracket." << std::endl;
          return false;
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
        return false;
      }
      std::string value_str = cons_str.substr(comma_loc + 1, end_loc - (comma_loc + 1));

      MathVariable symbolic_variable;
      MathValue symbolic_value;
      symbolic_variable.name = variable_name;
      symbolic_variable.derivative_count = variable_derivative_count;
      symbolic_value.str = value_str;

      variable_map.set_variable(symbolic_variable, symbolic_value); 
      and_cons_it++;
    }
    // prev[]を除く処理は要らなさそう？
    return true;
  }

  void send_cs(MathLink* ml) const
  {
    HYDLA_LOGGER_DEBUG("---- Send Constraint Store -----");

    int or_cons_size = store.first.size();
    if(or_cons_size <= 0)
    {
      HYDLA_LOGGER_DEBUG("no Constraints");
      ml->put_function("List", 0);
      return;
    }

    std::set<std::set<MathValue> >::const_iterator or_cons_it;
    std::set<MathValue>::const_iterator and_cons_it;
//     or_cons_it = constraint_store.first.begin();
//     while((or_cons_it) != constraint_store.first.end())
//     {
//       and_cons_it = (*or_cons_it).begin();
//       while((and_cons_it) != (*or_cons_it).end())
//       {
//         std::cout << (*and_cons_it).str << " ";
//         and_cons_it++;
//       }
//       std::cout << std::endl;
//       or_cons_it++;
//     }

//     if(debug_mode_) {
//       std::cout << "----------------------------" << std::endl;
//     }

    ml->put_function("List", 1);
    ml->put_function("Or", or_cons_size);
    or_cons_it = store.first.begin();
    while((or_cons_it) != store.first.end())
    {
      int and_cons_size = (*or_cons_it).size();
      ml->put_function("And", and_cons_size);
      and_cons_it = (*or_cons_it).begin();
      while((and_cons_it) != (*or_cons_it).end())
      {
        ml->put_function("ToExpression", 1);
        std::string str = (*and_cons_it).str;
        ml->put_string(str);
        and_cons_it++;
      }
      or_cons_it++;
    }
  }

  void send_cs_vars(MathLink* ml) const
  {
    HYDLA_LOGGER_DEBUG("---- Send Constraint Store Vars -----");

    int vars_size = store.second.size();
    std::set<MathVariable>::const_iterator vars_it = store.second.begin();

    ml->put_function("List", vars_size);
    while((vars_it) != store.second.end())
    {
      if(int value = (*vars_it).derivative_count > 0)
      {
        ml->MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
        ml->MLPutArgCount(1);      // this 1 is for the 'f'
        ml->MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
        ml->MLPutArgCount(1);      // this 1 is for the '*number*'
        ml->put_symbol("Derivative");
        ml->MLPutInteger(value);
        ml->put_symbol((*vars_it).name);

        HYDLA_LOGGER_DEBUG("Derivative[", value, "][", (*vars_it).name, "]");
      }
      else
      {
        ml->put_symbol((*vars_it).name);
        
        HYDLA_LOGGER_DEBUG((*vars_it).name);
      }
      vars_it++;
    }
  }
};

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_POINT_H_

