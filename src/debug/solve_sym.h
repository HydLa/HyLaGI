#pragma once

#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

namespace hydla { 
  namespace debug {
    class Solve_sym{
    public:
      Solve_sym(); 
      ~Solve_sym(); 
      static int add_equation(std::string name, std::string eq);
      static std::string solve_equation(std::string name, std::string eq);
      static std::string solve_inequalities(std::string name, std::string eq);
      static int add_var(std::vector<std::string> name_var_list);
      static void solve_main(std::vector<std::vector<std::vector<std::string>>> constraint_map, std::vector<std::vector<std::string>> v_map, std::vector<std::vector<std::string>> h_map);
      static int check_error(PyObject *pValue);
      static std::string conv_list(std::vector<std::string> list);
      static std::vector<std::string> mv_to_v(std::vector<std::vector<std::string>> v_map);
      static std::tuple<std::vector<std::vector<std::vector<std::string>>>, std::map<std::string, std::string>> add_equations(std::vector<std::vector<std::vector<std::string>>> constraint_map, std::string solve_var);
      static std::vector<std::string> make_eq_list(std::vector<std::vector<std::vector<std::string>>> constraint_map, std::vector<std::string> solve_module, bool add_ask, bool add_tell);
      static std::vector<std::vector<std::string>> make_cons_sets(std::vector<std::string> ask_eq_list);
      static bool check_duplication_ask(std::map<std::vector<std::string>, std::map<std::string, std::string>> each_var_val_map, std::map<std::string, std::string> var_val, std::vector<std::string> each_ask_cons_set);
      static std::map<std::string, std::string> make_val_map(std::string solve_eq_list, std::string solve_var, std::map<std::string, std::string> range_map, std::vector<std::string> eq_list, std::vector<std::string> add_var_list);



    };
  }
}
