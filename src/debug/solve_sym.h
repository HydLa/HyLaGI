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
      static std::tuple<std::vector<std::vector<std::vector<std::string>>>, std::map<std::string, std::string>, std::vector<std::vector<std::vector<std::string>>>> add_equations(std::vector<std::vector<std::vector<std::string>>> constraint_map, std::string solve_var, std::vector<std::vector<std::string>> h_map);
      static std::vector<std::string> make_eq_list(std::vector<std::vector<std::vector<std::string>>> constraint_map, std::vector<std::string> solve_module, bool add_ask, bool add_tell);
      static std::vector<std::vector<std::string>> make_cons_sets(std::vector<std::string> ask_eq_list);
      static bool check_duplication_ask(std::map<std::vector<std::string>, std::map<std::string, std::string>> each_var_val_map, std::map<std::string, std::string> var_val, std::vector<std::string> each_ask_cons_set);
      static std::map<std::string, std::string> make_val_map(std::map<std::string, std::string> range_map, std::vector<std::string> eq_list, std::vector<std::string> add_var_list);
      static std::map<std::vector<std::string>, std::map<std::string, std::string>>  check_val_map(std::vector<std::string> var_val_map_first,std::map<std::string, std::string> var_val_map_second, std::map<std::string, std::string> var_val_after, std::vector<std::string> add_var_list, std::vector<std::vector<std::vector<std::string>>> equation_name_map, std::map<std::string, std::string> range_map, std::vector<std::vector<std::vector<std::string>>> sym_prio_map);
      static std::vector<std::string> check_continue_val(std::vector<std::string> var_val_map_first,std::map<std::string, std::string> var_val_map_second, std::map<std::string, std::string> var_val_after, std::vector<std::string> add_var_list, std::vector<std::vector<std::vector<std::string>>> equation_name_map, std::map<std::string, std::string> range_map);
      static std::map<std::vector<std::string>, std::map<std::string, std::string>> make_ask_map(std::vector<std::vector<std::string>> each_ask_cons_sets, std::vector<std::string> true_eq_list, std::vector<std::vector<std::vector<std::string>>> equation_name_map, std::map<std::string, std::string> range_map, std::vector<std::string> add_var_list);
      static std::map<std::vector<std::string>, std::map<std::string, std::string>> make_tell_map(std::map<std::vector<std::string>, std::map<std::string, std::string>> each_var_val_map, std::vector<std::vector<std::vector<std::string>>> equation_name_map, std::map<std::string, std::string> range_map, std::vector<std::string> add_var_list, std::vector<std::string> eq_list, std::vector<std::vector<std::vector<std::string>>> sym_prio_map);
      static std::vector<std::string> resolve_without_unsat_core(std::vector<std::string> var_val_map_first, std::map<std::string, std::string> var_val_after, std::vector<std::vector<std::vector<std::string>>> sym_prio_map);
      static std::vector<std::string> get_sym(std::string eq);
      static std::vector<std::string> make_resolve_cons_set(std::vector<std::string> discrete_val, std::vector<std::string> var_val_map_first, std::vector<std::vector<std::vector<std::string>>> sym_prio_map);
      static std::map<std::vector<std::string>, std::map<std::string, std::string>> connect_map(std::map<std::vector<std::string>, std::map<std::string, std::string>> base,std::map<std::vector<std::string>, std::map<std::string, std::string>> add);
      static std::vector<std::string> decide_erase_cons(std::vector<std::string> pre_erase, std::vector<std::vector<std::vector<std::string>>> sym_prio_map);
      static std::map<std::string, std::string> find_unsat_core(std::string error_m, std::vector<std::string> eq_list, std::string solve_var, std::string renge_eq, std::string each_in_val);
      static std::string make_sub_list(std::vector<std::string> list);
      static bool find_inc_rel(std::string val_before, std::string var_map_after_second);


    };
  }
}
