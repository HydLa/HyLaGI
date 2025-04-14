#include "solve_sym.h"

#include <cstdio>
#include <cstdlib>
#include <string>

#include <Python.h>

#include "IncrementalModuleSet.h"
#include "boost_algo_str.h"
#include "debug_main.h"

using namespace hydla::debug;

namespace hydla {
namespace debug {
Solve_sym::Solve_sym() {}

PyObject *pName, *pModule, *pFunc;
PyObject *pArgs, *pValue;

void Solve_sym::solve_main(
    std::vector<std::vector<std::vector<std::string>>> constraint_map,
    std::vector<std::vector<std::string>> v_map,
    std::vector<std::vector<std::string>> h_map) {
  std::string solve_ret;
  int error;
  const char *a = "a";
  wchar_t *wa = nullptr;
  mbstowcs(wa, a, 1);
  wchar_t **aa = &wa;
  std::string solve_var;
  std::vector<std::string> add_var_list, eq_list, ask_eq_list, true_eq_list;
  std::vector<std::vector<std::string>> each_ask_cons_sets;
  std::vector<std::vector<std::vector<std::string>>> equation_name_map;

  std::map<std::vector<std::string>, std::map<std::string, std::string>>
      each_var_val_map, ret_make_tell_map; ///<{"INIT","FALL"},<x:1,y:1>}>
  std::map<
      std::map<std::vector<std::string>, std::map<std::string, std::string>>,
      std::map<std::vector<std::string>, std::map<std::string, std::string>>>
      ret_make_tell_map_2; ///<{"INIT","FALL"},<x:1,y:1>}>
  std::map<std::string, std::string> range_map;
  std::vector<std::vector<std::vector<std::string>>>
      sym_prio_map; /// {{{"FALL"},{"x", ...},{"BOUNCE", ...}}, ...}

  add_var_list = mv_to_v(v_map);
  add_var_list.push_back("AAAvar");
  solve_var = conv_list(add_var_list);

  /// solve.py接続部分
  Py_InitializeEx(0);
  PyObject *sys = PyImport_ImportModule("sys");
  PyObject *path = PyObject_GetAttrString(sys, "path");
  PyList_Append(path, PyUnicode_FromString("./src/debug"));
  PyRun_SimpleString("import os, sys\n");

  PySys_SetArgv(0, aa);
  pName = PyUnicode_FromString("solve_sym");
  pModule = PyImport_Import(pName);
  /// solve.py接続部分終わり

  if (pModule != NULL) {

    error = add_var(add_var_list);
    std::tie(equation_name_map, range_map, sym_prio_map) =
        add_equations(constraint_map, solve_var, h_map);

    /// 初期変数の登録(AAAeq:解なしかどうかの判定。)
    error = add_equation("AAAeq", "AAAvar - 1");

    for (auto each_cons : h_map) {
      if (each_cons[0] == "1") {
        ask_eq_list.push_back(each_cons[1]);
      } else if (each_cons[0] == "2") {
        true_eq_list.push_back(each_cons[1]);
      }
    }

    each_ask_cons_sets = make_cons_sets(ask_eq_list);
    /// 前件の計算を行う
    each_var_val_map = make_ask_map(each_ask_cons_sets, true_eq_list,
                                    equation_name_map, range_map, add_var_list);

    /// 後件の計算を行う
    eq_list = {};
    ret_make_tell_map_2 =
        make_tell_map_over(each_var_val_map, equation_name_map, range_map,
                           add_var_list, eq_list, sym_prio_map);
  } else {
    PyErr_Print();
    fprintf(stderr, "Failed to load \n");
  }
  find_result(ret_make_tell_map_2, v_map);
  Py_Finalize();
}

/// 結果の表示
void Solve_sym::find_result(
    std::map<
        std::map<std::vector<std::string>, std::map<std::string, std::string>>,
        std::map<std::vector<std::string>, std::map<std::string, std::string>>>
        ret_make_tell_map_2,
    std::vector<std::vector<std::string>> v_map) {
  bool flag, OCflag;
  std::map<std::string, std::string> each_ret;
  std::vector<std::map<std::string, std::string>> ret;
  std::string ccs, prev_val, val_name, prev;
  std::vector<std::string> use_val;
  int ii;
  for (auto m : ret_make_tell_map_2) {
    for (auto p : m.first) {
      flag = false;
      for (auto p2 : p.first) {
        if (!flag) {
          flag = true;
        } else {
        }
      }
      prev_val = "";
      for (auto entry : p.second) {
        if (entry.first != "AAAvar") {
          if (entry.first[0] == 'P') {
            val_name = entry.first.substr(1);
            ii = 0;
            while (ii < 10) {
              if (val_name[0] == 'D') {
                val_name = val_name.substr(1) + "'";
              } else {
                break;
              }
            }
            prev_val += val_name + "=" + entry.second + ";";
          }
        }
      }
    }
    for (auto p : m.second) {
      flag = false;
      ccs = "";
      for (auto p2 : p.first) {
        if (!flag) {
          flag = true;
          ccs = p2;
        } else {
          ccs += ", " + p2;
        }
      }
      ccs += ".";
      each_ret.insert(std::make_pair("CCS", ccs));
      each_ret.insert(std::make_pair("prev_val", prev_val));
      OCflag = false;
      use_val = {};
      for (auto entry : p.second) {
        if (entry.first != "AAAvar") {
          // std::cout << "<" << entry.first << "," << entry.second << ">" <<
          // std::flush;
          val_name = entry.first;
          ii = 0;
          while (ii < 10) {
            if (val_name[0] == 'P') {
              val_name = val_name.substr(1);
            } else if (val_name[0] == 'D') {
              val_name = val_name.substr(1);
            } else {
              break;
            }
          }
          // std::cout << val_name << std::endl;
          use_val.push_back(val_name);
        }
        if (entry.first == "unsat_cons") {
          val_name = entry.second;
          prev = "-";
          ii = 0;
          while (ii < 10) {
            if (val_name[0] == 'P') {
              val_name = val_name.substr(1);
            } else if (val_name[0] == 'D') {
              val_name = val_name.substr(1) + "'";
            } else {
              val_name = prev + val_name;
              break;
            }
          }
          each_ret.insert(std::make_pair("Cause", val_name + " is unsat val."));
          each_ret.insert(std::make_pair("ME", "Over Constrained"));
          ret.push_back(each_ret);
          OCflag = true;
          break;
        }
      }
      if (!OCflag) {
        for (auto vvv : v_map) {
          for (auto uv : use_val) {
            if (vvv[0] == uv) {
              OCflag = true;
              break;
            }
          }
          if (!OCflag) {
            each_ret.insert(std::make_pair("Cause", vvv[0] + " is undefined"));
            each_ret.insert(std::make_pair("ME", "Under Constrained"));
            ret.push_back(each_ret);
            break;
          }
        }
      }
      each_ret.clear();
    }
  }
  for (auto r : ret) {
    for (auto rr : r) {
      std::cout << "<" << rr.first << "," << rr.second << ">" << std::flush;
    }
    std::cout << "" << std::endl;
    std::cout << "CCS -> " << r["CCS"] << std::endl;
    std::cout << "prev_val -> " << r["prev_val"] << std::endl;
    std::cout << "ME -> " << r["ME"] << std::endl;
    std::cout << r["Cause"] << std::endl;
    std::cout << "" << std::endl;
  }
}

/// 前件の計算
std::map<std::vector<std::string>, std::map<std::string, std::string>>
Solve_sym::make_ask_map(
    std::vector<std::vector<std::string>> each_ask_cons_sets,
    std::vector<std::string> true_eq_list,
    std::vector<std::vector<std::vector<std::string>>> equation_name_map,
    std::map<std::string, std::string> range_map,
    std::vector<std::string> add_var_list) {
  std::vector<std::string> eq_list;
  std::map<std::vector<std::string>, std::map<std::string, std::string>> ret;

  for (auto each_ask_cons_set : each_ask_cons_sets) {
    each_ask_cons_set.insert(each_ask_cons_set.end(), true_eq_list.begin(),
                             true_eq_list.end());
    eq_list = each_ask_cons_set;
    eq_list = make_eq_list(equation_name_map, eq_list, true, false);

    bool duplication_flag =
        false; /// すでにある候補制約集合と重複しているかどうかのフラグ
    std::map<std::string, std::string> var_val; /// <x:1,y:1>

    var_val = make_val_map(range_map, eq_list, add_var_list);
    if (var_val.count("no_result") == 1) {
      continue;
    }
    duplication_flag = check_duplication_ask(ret, var_val, each_ask_cons_set);
    if (!duplication_flag) {
      std::sort(each_ask_cons_set.begin(), each_ask_cons_set.end());
      ret[each_ask_cons_set] = var_val;
    }
  }
  return ret;
}

/// 後件の計算の際，前件の結果と合わせてマッピングするための関数
std::map<std::map<std::vector<std::string>, std::map<std::string, std::string>>,
         std::map<std::vector<std::string>, std::map<std::string, std::string>>>
Solve_sym::make_tell_map_over(
    std::map<std::vector<std::string>, std::map<std::string, std::string>>
        each_var_val_map,
    std::vector<std::vector<std::vector<std::string>>> equation_name_map,
    std::map<std::string, std::string> range_map,
    std::vector<std::string> add_var_list,
    std::vector<std::string> each_eq_list,
    std::vector<std::vector<std::vector<std::string>>> sym_prio_map) {
  std::map<
      std::map<std::vector<std::string>, std::map<std::string, std::string>>,
      std::map<std::vector<std::string>, std::map<std::string, std::string>>>
      ret;
  std::map<std::vector<std::string>, std::map<std::string, std::string>>
      back_ret, e_var_val_map;

  for (auto var_val_map : each_var_val_map) {
    e_var_val_map.insert(std::make_pair(var_val_map.first, var_val_map.second));
    back_ret = make_tell_map(e_var_val_map, equation_name_map, range_map,
                             add_var_list, each_eq_list, sym_prio_map);
    ret.insert(std::make_pair(e_var_val_map, back_ret));
    e_var_val_map.clear();
  }
  return ret;
}

/// 後件の計算
std::map<std::vector<std::string>, std::map<std::string, std::string>>
Solve_sym::make_tell_map(
    std::map<std::vector<std::string>, std::map<std::string, std::string>>
        each_var_val_map,
    std::vector<std::vector<std::vector<std::string>>> equation_name_map,
    std::map<std::string, std::string> range_map,
    std::vector<std::string> add_var_list,
    std::vector<std::string> each_eq_list,
    std::vector<std::vector<std::vector<std::string>>> sym_prio_map) {
  std::vector<std::string> add_eq_list, eq_list, re_eq_list, discrete_val,
      resolve_cons_set;
  std::map<std::vector<std::string>, std::map<std::string, std::string>>
      resolve_var_val_map;
  std::map<std::vector<std::string>, std::map<std::string, std::string>> ret,
      back_ret;
  std::map<std::string, std::string> no_sol_map;

  no_sol_map["no_solution"] = "noting";

  for (auto var_val_map : each_var_val_map) {
    eq_list = each_eq_list;
    add_eq_list =
        make_eq_list(equation_name_map, var_val_map.first, true, true);
    eq_list.insert(eq_list.end(), add_eq_list.begin(), add_eq_list.end());
    std::map<std::string, std::string> var_val_after; /// <x:1,y:1>

    /// make_val_map([<ask_0_BOUNCE, x<0>,...], [INIT, FALL,...], [x,y,Px,...]);
    var_val_after = make_val_map(range_map, eq_list, add_var_list);
    if (var_val_after.count("no_result") == 1) {
      /// TODO:unsat
      /// coreを求めて、減らせる制約を減らして再計算する(複数の式の矛盾から、原因の制約を導いて無視、再度解く)
      resolve_cons_set = resolve_without_unsat_core(
          var_val_map.first, var_val_after, sym_prio_map); /// 制約を減らす
      if (resolve_cons_set[0] !=
          "no_solution") { /// 制約を減らせた場合は再計算する
        resolve_var_val_map.clear();
        resolve_var_val_map
            [resolve_cons_set /*(ここに減らした制約集合を入れていく)*/] =
                var_val_map.second;
        re_eq_list = {};
        back_ret =
            make_tell_map(resolve_var_val_map, equation_name_map, range_map,
                          add_var_list, re_eq_list, sym_prio_map);
      } else {
        no_sol_map["no_result"] = var_val_after["no_result"];
        no_sol_map["unsat_cons"] = var_val_after["unsat_cons"];
        back_ret[var_val_map.first] = no_sol_map;
      }
      ret = connect_map(ret, back_ret);
      continue;
    }
    discrete_val =
        check_continue_val(var_val_map.first, var_val_map.second, var_val_after,
                           add_var_list, equation_name_map, range_map);
    if (!discrete_val
             .empty()) { /// 解なしか暗黙の連続性に反する変数が存在する場合
      resolve_cons_set = make_resolve_cons_set(discrete_val, var_val_map.first,
                                               sym_prio_map); /// 制約を減らす
      if (resolve_cons_set[0] !=
          "no_solution") { /// 制約を減らせた場合は再計算する
        resolve_var_val_map.clear();
        resolve_var_val_map
            [resolve_cons_set /*(ここに減らした制約集合を入れていく)*/] =
                var_val_map.second;
        re_eq_list = {};
        back_ret =
            make_tell_map(resolve_var_val_map, equation_name_map, range_map,
                          add_var_list, re_eq_list, sym_prio_map);
      } else {
        back_ret[var_val_map.first] = no_sol_map;
      }
      ret = connect_map(ret, back_ret);
      continue;
    }
    back_ret =
        check_val_map(var_val_map.first, var_val_map.second, var_val_after,
                      add_var_list, equation_name_map, range_map, sym_prio_map);
    ret = connect_map(ret, back_ret);
  }
  return ret;
}

/// solve.pyからの出力をマップにパースする(TODO:と、言いつつ全ての計算を行なっているので、この部分を関数化する)
std::map<std::string, std::string>
Solve_sym::make_val_map(std::map<std::string, std::string> range_map,
                        std::vector<std::string> eq_list,
                        std::vector<std::string> add_var_list) {
  std::vector<std::string> split_str, split_val, add_var_list_c, split_in_val,
      split_solve_ret, split_each_solve_ret, eq_solve_ret, split_each_ret,
      split_e_each_ret;
  std::map<std::string, std::string> ret; /// <x:1,y:1>
  std::string subs, each_range_name, solve_ret, renge_eq, renge_eq_subs,
      renge_eq_o, mul_eq, each_eq_solve_ret_name, solve_var, solve_eq_list;
  bool flag, inf_flag, var_flag, replace_flag;
  bool rest_val = true;

  add_var_list_c = add_var_list;

  /// 等式の計算を行う(値が求まる変数全てに値が欲しいから繰り返し行う)
  while (rest_val) {
    rest_val = false;
    solve_var = conv_list(add_var_list_c);
    solve_eq_list = conv_list(eq_list);
    replace_all(solve_var, "[", "");
    replace_all(solve_var, "]", "");
    solve_ret = solve_equation(solve_eq_list, solve_var);
    /// []が帰ってくるならばその制約集合は解なし
    if (solve_ret == "[]" or solve_ret == "False") {
      ret.clear();
      renge_eq = "";
      ret = find_unsat_core(
          "equality", eq_list, solve_var, renge_eq,
          ""); /// 全ての等式の中からunsat_coreとなる等式を求める
      ret["no_result"] = "equality";
      return ret;
    }

    /// いらない部分の消去(TODO:複数の解がある時はこれではダメ "}, {"
    /// でsplitして複数の候補制約集合に分割する(文字列はsplitできないので、一回特殊文字か何かに置き換える))
    replace_all(solve_ret, "[", "");
    replace_all(solve_ret, "]", "");
    /// 上は複数の解が存在する場合専用
    replace_all(solve_ret, "{", "");
    replace_all(solve_ret, "}", "");
    replace_all(solve_ret, " ", "");

    split(split_str, solve_ret, ",");
    for (auto each_val : split_str) {
      split(split_val, each_val, ":");
      if (split_val[0] != "AAAvar") {
        rest_val = true;
        ret[split_val[0]] = split_val[1];
        auto itr = std::find(add_var_list_c.begin(), add_var_list_c.end(),
                             split_val[0]);
        if (itr != add_var_list_c.end()) {
          add_var_list_c.erase(itr);
        }
      }
    }
  }
  /// 等式で解が全くもとまらないが、解なしではない場合にも対応する
  // if(ret.empty()){
  ret["AAAvar"] = "1";
  //}
  /// 不等式を解くために等式で求まった変数を代入する

  flag = false;
  subs += ".subs([";
  for (auto each_val : ret) {
    if (flag) {
      subs += ",";
    }
    flag = true;
    subs += "(" + each_val.first + ", " + each_val.second + ")";
  }
  subs += "])";
  /// 不等式にsubsを代入して解く(等式と異なり、不等式は式を直接与えて計算する)
  for (auto each_range : range_map) {
    each_range_name = each_range.first;
    if (std::find(eq_list.begin(), eq_list.end(), each_range_name) !=
        eq_list.end()) {
      renge_eq = each_range.second;
      /// 使用変数を求める
      split(split_in_val, renge_eq, " -(),");
      for (auto each_in_val : split_in_val) { /// 不等式の要素ごとに回す
        if (std::find(add_var_list.begin(), add_var_list.end(), each_in_val) !=
            add_var_list.end()) { /// 要素が変数として認識できる場合
          renge_eq_subs = "(" + renge_eq + ")" + subs;
          /// ここで引数として与える変数に値が代入され、他の未知数が存在する時は呼び出しがうまくいかず解が帰ってこないが、現状はそのあとの処理としてうまく行っているので放置しておく
          solve_ret = solve_inequalities(
              renge_eq_subs,
              each_in_val); /// TODO:solve_inequalitiesの返しを"|"が最大優先度になるようにしたい(ケース分岐に対応するため)
          /// 変数マップを作る(可能性としては複数の範囲、一つの範囲、一つの等式がありそう)
          if (solve_ret == "False") { // その不等式内に答えが存在しない場合
            ret.clear();
            ret = find_unsat_core("range", eq_list, solve_var, renge_eq_subs,
                                  each_in_val); /// range:不等式が矛盾する
            ret["no_result"] = "range";
            return ret;
          } else if (solve_ret == "True") { // 代入した値がそのまま正しい場合
            continue;
          } else {
            split(
                split_solve_ret, solve_ret,
                "|"); /// ケース分岐に対応するためだが、現在は全体的に対応できていない
            for (auto each_solve_ret : split_solve_ret) {
              split(split_each_solve_ret, each_solve_ret, "&");
              for (auto each_split_each_solve_ret : split_each_solve_ret) {
                split(eq_solve_ret, each_split_each_solve_ret, " -()");
                inf_flag = false;
                var_flag = false;
                for (auto each_eq_solve_ret :
                     eq_solve_ret) { /// 不等式の要素ごとに回す
                  if (each_eq_solve_ret ==
                      "oo") { /// infを含む範囲はとりあえずいらない
                    inf_flag = true;
                    break;
                  }
                  if (std::find(add_var_list.begin(), add_var_list.end(),
                                each_eq_solve_ret) !=
                      add_var_list.end()) { /// 要素が変数として認識できる場合
                    var_flag = true;
                    each_eq_solve_ret_name = each_eq_solve_ret;
                  }
                }
                if (!inf_flag and var_flag) {
                  if (ret.count(each_eq_solve_ret_name) == 0) {
                    ret[each_eq_solve_ret_name] = each_split_each_solve_ret;
                  } else {
                    ret[each_eq_solve_ret_name] = ret[each_eq_solve_ret_name] +
                                                  "," +
                                                  each_split_each_solve_ret;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  /// 不等式をといた結果として、複数解が存在する場合、それらの成立判定を行う
  for (auto each_ret : ret) {
    if (each_ret.second.find(",") !=
        std::string::npos) { /// 複数解が存在するかの判定
      split(split_each_ret, each_ret.second, " ");
      mul_eq = "";
      for (auto e_split_each_ret : split_each_ret) {
        if (e_split_each_ret.find("/") !=
            std::string::
                npos) { /// python2.7では分数の余りを切り捨ててしまうので、書き換える必要がある
          split(split_e_each_ret, e_split_each_ret, "/");
          mul_eq += "Retional(" + split_e_each_ret[0] + "," +
                    split_e_each_ret[1] + ")";
        } else {
          mul_eq += e_split_each_ret;
        }
      }
      mul_eq = "[" + mul_eq + "]";
      solve_ret = solve_equation(mul_eq, each_ret.first);
      if (solve_ret == "False") { // その不等式内に答えが存在しない場合
        ret.clear();
        ret = find_unsat_core("var", eq_list, solve_var, each_ret.first,
                              ""); /// var:その変数の式が矛盾する()
        ret["no_result"] = "var";
        return ret;
      } else {
        split(split_solve_ret, solve_ret, "|");
        for (auto each_solve_ret : split_solve_ret) {
          replace_flag = false;
          split(split_each_solve_ret, each_solve_ret, "&");
          for (auto each_split_each_solve_ret : split_each_solve_ret) {
            split(eq_solve_ret, each_split_each_solve_ret, " -()");
            inf_flag = false;
            var_flag = false;
            for (auto each_eq_solve_ret :
                 eq_solve_ret) { /// 不等式の要素ごとに回す
              if (each_eq_solve_ret ==
                  "oo") { /// infを含む範囲はとりあえずいらない
                inf_flag = true;
                break;
              }
              if (std::find(add_var_list.begin(), add_var_list.end(),
                            each_eq_solve_ret) !=
                  add_var_list.end()) { /// 要素が変数として認識できる場合
                var_flag = true;
                each_eq_solve_ret_name = each_eq_solve_ret;
              }
            }
            if (!inf_flag and var_flag) {
              if (!replace_flag) { /// すでに書き換わっている場合は後に付け足すだけ
                ret[each_eq_solve_ret_name] = each_split_each_solve_ret;
                replace_flag = true;
              } else {
                ret[each_eq_solve_ret_name] = ret[each_eq_solve_ret_name] +
                                              "," + each_split_each_solve_ret;
              }
            }
          }
        }
      }
    } else {
      continue;
    }
  }
  return ret;
}

/// 得られたprev変数の解が一致するかどうかの確認(包含されている場合は分割を行う必要があるため)
std::map<std::vector<std::string>, std::map<std::string, std::string>>
Solve_sym::check_val_map(
    std::vector<std::string> var_val_map_first,
    std::map<std::string, std::string> var_val_map_second,
    std::map<std::string, std::string> var_val_after,
    std::vector<std::string> add_var_list,
    std::vector<std::vector<std::vector<std::string>>> equation_name_map,
    std::map<std::string, std::string> range_map,
    std::vector<std::vector<std::vector<std::string>>> sym_prio_map) {
  std::string val_before, n_str, y_str;
  bool flag, undef_flag, ask_flag;
  std::vector<std::string> split_in_val, var_val_map_first_split,
      var_val_map_first_base;
  int error;
  std::map<std::vector<std::string>, std::map<std::string, std::string>>
      each_var_val_map, ret_make_tell_map, ret;

  var_val_map_first_base = var_val_map_first;
  /// そもそも、　元の候補制約集合からask制約が抜けている場合は、prev変数値は変わるはずなのだからその基準で分割はしない
  for (auto var_map_after : var_val_after) {
    if (var_map_after.first[0] == 'P') { /// prev変数のみを確認する
      val_before = "";
      flag = false;
      undef_flag = false;
      /// 使用変数を求める
      split(split_in_val, var_map_after.second, " -/*");
      for (auto each_in_val :
           split_in_val) { /// 式の要素ごとに回す /// tellの時の判定
        if (std::find(add_var_list.begin(), add_var_list.end(), each_in_val) !=
            add_var_list.end()) { /// 要素が変数として認識できる場合
          if (each_in_val !=
              var_map_after
                  .first) { /// 確認しているprev変数以外の変数が出現した場合は値が定まらないから流す(?)(TODO:prevの変数が出たら一致しないって言ってもいいかも)
            undef_flag = true;
            break;
          }
        }
      }
      for (auto var_map : var_val_map_second) { /// askの時の判定
        if (var_map.first == var_map_after.first) {
          val_before = var_map.second;
          flag = true;
          break;
        }
      }
      if (!undef_flag) {
        if (flag) { /// askの時にprev変数が決まっている場合
          if (val_before ==
              var_map_after
                  .second) { /// askの時と同じprev値の時はそのままでいい
            continue;
          }
          ask_flag = false;
          ask_flag = find_inc_rel(
              val_before,
              var_map_after
                  .second); /// TODO:複数値が存在するとき、比較している2つの式の集合が等しいか確認する
          if (ask_flag) {
            continue;
          }
        }
        /// askと違うprev値なら、tellで求めたprev値以外のprev値はどうなるのか確認する(名前をつける。どうにかする)
        n_str = var_map_after.first + "N" + var_map_after.second;
        replace_all(n_str, "(", "");
        replace_all(n_str, ")", "");
        replace_all(n_str, " ", "");
        replace_all(n_str, ">=", "igtoe");
        replace_all(n_str, "<=", "iltoe");
        replace_all(n_str, ">", "igt");
        replace_all(n_str, "<", "ilt");
        y_str = var_map_after.first + "Y" + var_map_after.second;
        replace_all(y_str, "(", "");
        replace_all(y_str, ")", "");
        replace_all(y_str, " ", "");
        replace_all(y_str, ">=", "igtoe");
        replace_all(y_str, "<=", "iltoe");
        replace_all(y_str, ">", "igt");
        replace_all(y_str, "<", "ilt");
        error = add_equation(
            n_str,
            "0"); /// TODO:範囲値が来た時にどうするか(特殊解の想定なので、範囲は存在しないと信じてる(今の所))
        range_map[n_str] =
            "~Eq(" + var_map_after.first + "," + var_map_after.second + ")";
        var_val_map_first_base.push_back(
            y_str); /// PDyY0のような感じ。式を追加する必要があるなら追加するが、今の所必要なさそう
        var_val_map_first_split = var_val_map_first;
        var_val_map_first_split.push_back(n_str);
        each_var_val_map[var_val_map_first_split] = var_val_map_second;
        ret_make_tell_map =
            make_tell_map(each_var_val_map, equation_name_map, range_map,
                          add_var_list, {n_str}, sym_prio_map);
        ret = connect_map(ret, ret_make_tell_map);
      }
    }
  }
  ret[var_val_map_first_base] = var_val_after;
  return ret;
}

///////////////// 制約階層から制約集合を変更する関連の処理を行う関数
////////////////////////////
/// unsat_coreの制約を制約階層に基づいて取り除いた制約集合を返す
std::vector<std::string> Solve_sym::resolve_without_unsat_core(
    std::vector<std::string> var_val_map_first,
    std::map<std::string, std::string> var_val_after,
    std::vector<std::vector<std::vector<std::string>>> sym_prio_map) {
  std::vector<std::string> ret;
  std::vector<std::string> pre_erase, split_str, split_str_, erase;
  std::vector<std::string> emp = {};
  std::string erase_cons, c_cons;

  if (var_val_after["no_result"] == "equality") {
    /// pre_eraseをもらって削除する制約を決める
    split(split_str, var_val_after["unsat_cons"], ",");
    for (auto cons : split_str) {
      split(split_str_, cons, "_");
      c_cons = split_str_[2];
      for (auto sym_prio :
           sym_prio_map) { /// {{{"FALL"},{"x", ...},{"BOUNCE", ...}}, ...}
        if (sym_prio[0][0] ==
            c_cons) { /// sym_prioの制約名が候補制約集合内の制約と一致するなら
          if (sym_prio[2] != emp) { /// each_consがrequiredなら
            pre_erase.push_back(c_cons);
          }
        }
      }
    }
    if (pre_erase.empty()) {
      ret.clear();
    } else {
      erase = decide_erase_cons(pre_erase, sym_prio_map);
      erase_cons = erase
          [0]; /// 複数帰って来たときは、どれか一つだけ消す(TODO:ケース分岐に対応するならここでも分岐させる必要がある)
      /// 削除する制約より優先度が低くない制約をretに追加する
      for (auto each_cons :
           var_val_map_first) { /// 候補制約集合に含まれる制約ごとに回す
        if (each_cons == erase_cons) {
          continue;
        } else {
          for (auto sym_prio :
               sym_prio_map) { /// {{{"FALL"},{"x", ...},{"BOUNCE", ...}}, ...}
            if (sym_prio[0][0] ==
                each_cons) { /// sym_prioの制約名が候補制約集合内の制約と一致するなら
              if (sym_prio[2] == emp) { /// each_consがrequiredなら
                ret.push_back(each_cons);
              } else {
                if (std::find(sym_prio[2].begin(), sym_prio[2].end(),
                              erase_cons) != sym_prio[2].end()) {
                } else {
                  ret.push_back(each_cons);
                }
              }
              break;
            }
          }
        }
      }
    }

    /// TODO:無視できる変数が複数あるなら、どの制約を無視するか選択し、それ以外の制約を追加する
    /// TODO:無視した変数により無視されるようになってしまう制約を伝播的に削除していく
    /// 重複削除
    std::sort(ret.begin(), ret.end());
    ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
    // ret.push_back("FALL");
    if (ret.empty() ||
        ret.size() ==
            var_val_map_first
                .size()) { /// 制約の削減が出来なかった場合は解なしを返す
      ret.clear();
      ret.push_back("no_solution");
    }
  } else if (var_val_after["no_result"] == "range") {
    split(split_str, var_val_after["unsat_cons"], ",");
    ret = make_resolve_cons_set(split_str, var_val_map_first,
                                sym_prio_map); /// 制約を減らす
  } else {
    ret.clear();
    ret.push_back("no_solution");
  }
  return ret;
}

/// 矛盾する変数をもらって、制約階層に基づき制約を減らす(TODO:制約階層に基づきってところが実装できていない(とりあえず循環構造を持っている制約階層を考えないで実装してみた))
std::vector<std::string> Solve_sym::make_resolve_cons_set(
    std::vector<std::string> discrete_val,
    std::vector<std::string> var_val_map_first,
    std::vector<std::vector<std::vector<std::string>>> sym_prio_map) {
  std::vector<std::string> ret, pre_erase, erase;
  std::vector<std::string> emp = {};
  std::string erase_cons;
  bool flag;

  for (auto each_cons :
       var_val_map_first) { /// 候補制約集合に含まれる制約ごとに回す
    for (auto conflict_val : discrete_val) { /// 矛盾する変数ごとに回す
      for (auto sym_prio :
           sym_prio_map) { /// {{{"FALL"},{"x", ...},{"BOUNCE", ...}}, ...}
        if (sym_prio[0][0] ==
            each_cons) { /// sym_prioの制約名が候補制約集合内の制約と一致するなら
          if (sym_prio[2] == emp) { /// each_consがrequiredなら
            // ret.push_back(each_cons);
          } else {
            for (auto each_sym : sym_prio[1]) { /// sym_prioの変数ごとに回す
              if (each_sym ==
                  conflict_val) { /// sym_prioの変数名が候補制約集合内の変数名と一致するなら
                pre_erase.push_back(each_cons);
                break;
              }
            }
          }
        }
      }
    }
  }
  /// 重複削除
  std::sort(pre_erase.begin(), pre_erase.end());
  pre_erase.erase(std::unique(pre_erase.begin(), pre_erase.end()),
                  pre_erase.end());
  if (!pre_erase.empty()) {
    /// pre_eraseをもらって削除する制約を決める
    erase = decide_erase_cons(pre_erase, sym_prio_map);
    erase_cons = erase
        [0]; /// 複数帰って来たときは、どれか一つだけ消す(TODO:ケース分岐に対応するならここでも分岐させる必要がある)
    /// 削除する制約より優先度が低くない制約をretに追加する
    for (auto each_cons :
         var_val_map_first) { /// 候補制約集合に含まれる制約ごとに回す
      if (each_cons == erase_cons) {
        continue;
      } else {
        flag = false;
        for (auto sym_prio :
             sym_prio_map) { /// {{{"FALL"},{"x", ...},{"BOUNCE", ...}}, ...}
          if (sym_prio[0][0] ==
              each_cons) { /// sym_prioの制約名が候補制約集合内の制約と一致するなら
            flag = true;
            if (sym_prio[2] == emp) { /// each_consがrequiredなら
              ret.push_back(each_cons);
            } else {
              if (std::find(sym_prio[2].begin(), sym_prio[2].end(),
                            erase_cons) != sym_prio[2].end()) {
              } else {
                ret.push_back(each_cons);
              }
            }
            break;
          }
        }
        if (!flag) {
          ret.push_back(each_cons);
        }
      }
    }
  }

  /// TODO:無視できる変数が複数あるなら、どの制約を無視するか選択し、それ以外の制約を追加する
  /// TODO:無視した変数により無視されるようになってしまう制約を伝播的に削除していく
  /// 重複削除
  std::sort(ret.begin(), ret.end());
  ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
  // ret.push_back("FALL");
  if (ret.empty() ||
      ret.size() ==
          var_val_map_first
              .size()) { /// 制約の削減が出来なかった場合は解なしを返す
    ret.clear();
    ret.push_back("no_solution");
  }
  return ret;
}

/// 削除する制約を決める
std::vector<std::string> Solve_sym::decide_erase_cons(
    std::vector<std::string> pre_erase,
    std::vector<std::vector<std::vector<std::string>>> sym_prio_map) {
  std::vector<std::string> ret;
  bool flag;

  for (auto cons : pre_erase) {
    flag = false;
    for (auto sym_prio :
         sym_prio_map) { /// {{{"FALL"},{"x", ...},{"BOUNCE", ...}}, ...}
      if (sym_prio[0][0] ==
          cons) { /// sym_prioの制約名が候補制約集合内の制約と一致するなら
        for (auto each_prio : sym_prio[2]) {
          if (std::find(pre_erase.begin(), pre_erase.end(), each_prio) !=
              pre_erase.end()) {
            flag = true;
            break;
          }
        }
      }
    }
    if (flag) {
      ret.push_back(cons);
    }
  }
  if (ret.empty()) {
    ret = pre_erase;
  } else if (ret.size() != 1) {
    ret = decide_erase_cons(ret, sym_prio_map);
  }
  return ret;
}

///////////////// unsat core(変数 or 制約)を見つける関数
////////////////////////////

/// unsat coreとなる制約式を求める(複数の式の矛盾から、原因の制約を導く)
std::map<std::string, std::string> Solve_sym::find_unsat_core(
    std::string error_m, std::vector<std::string> eq_list,
    std::string solve_var, std::string renge_eq, std::string each_in_val) {
  std::map<std::string, std::string> ret;
  std::vector<std::string> s_eq_list, d_eq_list, eq, eq_var, eq_split;
  std::string solve_eq_list, add_cons, solve_ret, v_eq;
  bool s_sat, d_sat;

  s_sat = true;
  s_eq_list.clear();
  if (error_m == "equality") {
    s_eq_list.push_back("AAAeq");
    while (s_sat) {
      d_eq_list = s_eq_list;
      d_sat = true;
      while (d_sat) {
        for (auto eq : eq_list) {
          if (std::find(d_eq_list.begin(), d_eq_list.end(), eq) ==
              d_eq_list.end()) {
            d_eq_list.push_back(eq);
            add_cons = eq;
            break;
          }
        }
        /// 追加した制約式を含めて解を求める
        solve_eq_list = conv_list(d_eq_list);
        solve_ret = solve_equation(solve_eq_list, solve_var);
        /// []が帰ってくるならばその制約集合は解なし
        if (solve_ret == "[]" or solve_ret == "False") {
          s_eq_list.push_back(add_cons);
          d_sat = false;
        }
      }
      /// 追加した制約式を含めて解を求める
      solve_eq_list = conv_list(s_eq_list);
      solve_ret = solve_equation(solve_eq_list, solve_var);
      /// []が帰ってくるならばその制約集合は解なし
      if (solve_ret == "[]" or solve_ret == "False") {
        for (auto unsat_cons : s_eq_list) {
          if (unsat_cons != "AAAeq") {
            if (ret["unsat_cons"] == "") {
              ret["unsat_cons"] = unsat_cons;
            } else {
              ret["unsat_cons"] = ret["unsat_cons"] + "," + unsat_cons;
            }
          }
        }
        s_sat = false;
      }
    }
  } else if (error_m == "range") {
    replace_all(renge_eq, ".subs", "?");
    replace_all(renge_eq, "),(", ")!(");
    replace_all(renge_eq, "([", "");
    replace_all(renge_eq, "])", "");
    split(eq, renge_eq, "?");
    v_eq = eq[1];
    split(eq_var, v_eq, "!");
    s_eq_list.push_back(eq[0]);
    while (s_sat) {
      d_eq_list = s_eq_list;
      d_sat = true;
      while (d_sat) {
        for (auto eq : eq_var) {
          if (std::find(d_eq_list.begin(), d_eq_list.end(), eq) ==
              d_eq_list.end()) {
            d_eq_list.push_back(eq);
            add_cons = eq;
            break;
          }
        }
        /// 追加した制約式を含めて解を求める
        solve_eq_list = make_sub_list(d_eq_list);
        solve_ret = solve_equation(solve_eq_list, each_in_val);
        /// []が帰ってくるならばその制約集合は解なし
        if (solve_ret == "[]" or solve_ret == "False") {
          s_eq_list.push_back(add_cons);
          d_sat = false;
        }
      }
      /// 追加した制約式を含めて解を求める
      // solve_eq_list = conv_list(s_eq_list);
      solve_eq_list = make_sub_list(s_eq_list);
      solve_ret = solve_equation(solve_eq_list, each_in_val);
      /// []が帰ってくるならばその制約集合は解なし
      if (solve_ret == "[]" or solve_ret == "False") {
        ret["unsat_cons"] = each_in_val;
        s_sat = false;
      }
    }
  }
  return ret;
}

/// 得られた変数の解が連続するかどうかの確認(微分値の暗黙の連続性を確認するため)一つの変数において2つ以上の階数で離散変化が発生する場合のみその変数の配列を返す
std::vector<std::string> Solve_sym::check_continue_val(
    std::vector<std::string> var_val_map_first,
    std::map<std::string, std::string> var_val_map_second,
    std::map<std::string, std::string> var_val_after,
    std::vector<std::string> add_var_list,
    std::vector<std::vector<std::vector<std::string>>> equation_name_map,
    std::map<std::string, std::string> range_map) {
  std::vector<std::string> ret, sub_ret;
  std::string prev_name, val_name;

  for (auto var_map_after : var_val_after) { /// 値がないならそもそも暗黙の連続
    if (var_map_after.first == "AAAvar") {
      continue;
    }
    if (var_map_after.first[0] != 'P') { /// prev変数以外を確認する
      prev_name = "P" + var_map_after.first;
      val_name = var_map_after.first;
      for (int i = 0; i < 5; ++i) { /// 使用している変数名を抽出する
        if (val_name[0] == 'D') {
          val_name = val_name.substr(1);
        } else {
          break;
        }
      }
      if (prev_name == var_map_after.second ||
          var_val_map_second[var_map_after.first] ==
              var_map_after
                  .second) { /// 対象の変数の値がprev値と一致する、またはaskの時の解と一致するなら連続
      } else { /// それ以外の変数は不連続として扱う(TODO:prevの値によっては連続になることもありうるので、その辺の対応)
        if (std::find(sub_ret.begin(), sub_ret.end(), val_name) !=
            sub_ret.end()) {
          ret.push_back(val_name);
        }
        sub_ret.push_back(val_name);
      }
    }
  }
  std::sort(ret.begin(), ret.end());
  ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
  return ret;
}

///////////////// 式変形用関数 /////////////////////////

/// 配列をもらって、sympyに投げる時の形に変形する [a,b,c]のような感じ
std::string Solve_sym::conv_list(std::vector<std::string> list) {
  std::string list_str = "[";
  bool flag = false;
  for (auto e : list) {
    if (flag) {
      list_str += ",";
    }
    list_str += e;
    flag = true;
  }
  list_str += "]";
  return list_str;
}
/// 配列をもらって、sympyに投げる時の代入式に変形する
/// (x<0).subs([(x,-1),...])のような感じ
std::string Solve_sym::make_sub_list(std::vector<std::string> list) {
  std::string list_str = "";
  bool flag = false;
  for (auto e : list) {
    if (list_str == "") {
      list_str += e + ".subs([";
    } else {
      if (flag) {
        list_str += ",";
      }
      list_str += e;
      flag = true;
    }
  }
  list_str += "])";
  return list_str;
}

/// 変数と階数のマップをもらって、変数名の配列を返す
std::vector<std::string>
Solve_sym::mv_to_v(std::vector<std::vector<std::string>> v_map) {
  std::vector<std::string> ret;
  for (auto e : v_map) {
    int levels = std::stoi(e[1]);
    std::string differential = "";
    for (int i = 0; i <= levels; i++) {
      ret.push_back(differential + e[0]);
      ret.push_back("P" + differential + e[0]);
      differential += "D";
    }
  }
  return ret;
}

/// ガード付き制約名の配列をもらって、全組み合わせを返す
std::vector<std::vector<std::string>>
Solve_sym::make_cons_sets(std::vector<std::string> ask_eq_list) {
  std::vector<std::vector<std::string>> ret, get_ret, c_get_ret;
  std::string name_cons;
  int sets_count = 0;

  if (ask_eq_list.size() == 0) {
    ret = {{}};
  } else {
    name_cons = ask_eq_list[0];
    ask_eq_list.erase(ask_eq_list.begin());
    get_ret = make_cons_sets(ask_eq_list);
    c_get_ret = get_ret;
    for (auto eq_list : get_ret) {
      get_ret[sets_count].push_back(name_cons);
      sets_count += 1;
    }
    get_ret.insert(get_ret.end(), c_get_ret.begin(), c_get_ret.end());
    ret = get_ret;
  }

  return ret;
}

/// 式から使用している変数を抽出する
std::vector<std::string> Solve_sym::get_sym(std::string eq) {
  std::vector<std::string> ret, split_eq;
  std::string val_name;

  split(split_eq, eq, "<>=+-*/()");
  for (auto each_split_eq : split_eq) {
    // if(var_map_after.first == "AAAvar"){
    //  continue;
    //}
    if (/*each_split_eq[0] != 'P' and */ each_split_eq !=
        "") { /// prev変数も見るよ
      val_name = each_split_eq;
      for (int i = 0; i < 5; ++i) { /// 使用している変数名を抽出する
        if (val_name[0] == 'D') {
          val_name = val_name.substr(1);
        } else {
          break;
        }
      }
      if (islower(val_name[0]) == 0 and val_name[0] != 'P') {
        continue;
      }
      ret.push_back(val_name);
      ret.push_back(each_split_eq);
    }
  }
  std::sort(ret.begin(), ret.end());
  ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
  return ret;
}

/// 第２引数で与えられた制約モジュールによって作成されたpython内の式の名前をリストで返す(ask制約とtell制約の式の名前を返すかを第３、４引数で指定する)
std::vector<std::string> Solve_sym::make_eq_list(
    std::vector<std::vector<std::vector<std::string>>> equation_name_map,
    std::vector<std::string> solve_module, bool add_ask, bool add_tell) {
  std::string module_name, cons_name;
  std::vector<std::string> ret;

  for (auto equation_name : equation_name_map) {
    module_name = equation_name[0][0];
    if (std::find(solve_module.begin(), solve_module.end(), module_name) ==
        solve_module.end()) {
      continue;
    }
    if (add_ask) {
      for (auto ask : equation_name[1]) {
        ret.push_back(ask);
      }
    }
    if (add_tell) {
      for (auto tell : equation_name[2]) {
        ret.push_back(tell);
      }
    }
  }
  /// AAAvar=1を返す式を追加する(本当に解なしか確認するため)
  ret.push_back("AAAeq");
  return ret;
}

/// 制約式全体の登録(中でadd_equation()を呼ぶ)
/// 同時に制約階層と使用変数の情報を作成して返す
std::tuple<std::vector<std::vector<std::vector<std::string>>>,
           std::map<std::string, std::string>,
           std::vector<std::vector<std::vector<std::string>>>>
Solve_sym::add_equations(
    std::vector<std::vector<std::vector<std::string>>> constraint_map,
    std::string solve_var, std::vector<std::vector<std::string>> h_map) {
  int error;
  std::string eqation_name, cons_name;
  std::vector<std::string> each_name, each_ask, each_tell;
  std::vector<std::vector<std::string>> each_m;
  std::vector<std::vector<std::vector<std::string>>> ret;
  std::map<std::string, std::string> ret_range;
  std::vector<std::vector<std::vector<std::string>>>
      ret_sym_prio_map; /// {{{"FALL"},{"x", ...},{"BOUNCE", ...}}, ...}
  std::vector<std::vector<std::string>> sym_prio_map;
  std::vector<std::string> sym_, prio_, sym_ret;

  for (auto cons : constraint_map) {
    each_name.clear();
    each_ask.clear();
    each_tell.clear();
    each_m.clear();
    prio_.clear();
    sym_.clear();
    sym_prio_map.clear();
    cons_name = cons[0][0];
    each_name.push_back(cons_name);
    int count_ask = 0;
    int count_tell = 0;
    for (auto ask : cons[1]) {
      if (ask == "null") {
        continue;
      }
      eqation_name = "ask_" + std::to_string(count_ask) + "_" + cons_name;
      if (ask.find("<") == std::string::npos) {
        error = add_equation(eqation_name, ask);
      } else {
        error = add_equation(eqation_name, "0");
        // std::cout << "range : " << ask << std::endl;
        ret_range[eqation_name] = ask;
      }
      each_ask.push_back(eqation_name);
      count_ask += 1;
      sym_ret = get_sym(ask);
      sym_.insert(sym_.end(), sym_ret.begin(), sym_ret.end());
    }
    for (auto tell : cons[2]) {
      if (tell == "null") {
        continue;
      }
      eqation_name = "tell_" + std::to_string(count_tell) + "_" + cons_name;
      error = add_equation(eqation_name, tell);
      each_tell.push_back(eqation_name);
      count_tell += 1;
      sym_ret = get_sym(tell);
      sym_.insert(sym_.end(), sym_ret.begin(), sym_ret.end());
    }
    each_m.push_back(each_name);
    each_m.push_back(each_ask);
    each_m.push_back(each_tell);
    ret.push_back(each_m);
    std::sort(sym_.begin(), sym_.end());
    sym_.erase(std::unique(sym_.begin(), sym_.end()), sym_.end());
    for (auto h : h_map) {
      if (h[1] == cons_name) {
        for (int i = 2; i < h.size(); i++) {
          prio_.push_back(h[i]);
        }
      }
    }
    sym_prio_map.push_back(each_name);
    sym_prio_map.push_back(sym_);
    sym_prio_map.push_back(prio_);
    ret_sym_prio_map.push_back(sym_prio_map);
  }
  return std::forward_as_tuple(ret, ret_range, ret_sym_prio_map);
}

/// 2つの候補制約集合と解のマップを結合する
std::map<std::vector<std::string>, std::map<std::string, std::string>>
Solve_sym::connect_map(
    std::map<std::vector<std::string>, std::map<std::string, std::string>> base,
    std::map<std::vector<std::string>, std::map<std::string, std::string>>
        add) {
  std::map<std::vector<std::string>, std::map<std::string, std::string>> ret;

  ret = base;
  for (auto a : add) {
    ret.insert(std::make_pair(a.first, a.second));
  }
  return ret;
}

///////////////// 判定用関数 /////////////////////

/// 与えられた同じ変数に対する2つの式が意味的に等しいかの判定を行う
bool Solve_sym::find_inc_rel(std::string val_before,
                             std::string var_map_after_second) {
  std::string ret_val_before, ret_var_map_after_second;

  var_map_after_second = "[" + val_before + "," + var_map_after_second + "]";
  val_before = "[" + val_before + "]";
  ret_val_before = solve_equation(val_before, "");
  ret_var_map_after_second = solve_equation(var_map_after_second, "");
  return (ret_val_before == ret_var_map_after_second);
}

/// 制約集合の答えとして得られた結果がすでに登録されている結果と重複するかの判定
bool Solve_sym::check_duplication_ask(
    std::map<std::vector<std::string>, std::map<std::string, std::string>>
        each_var_val_map,
    std::map<std::string, std::string> var_val,
    std::vector<std::string> each_ask_cons_set) {
  bool ret;

  for (auto p : each_var_val_map) {
    ret = true;
    /// 全く同じ変数表が得られているかの確認(TODO:一致ではなく包含判定に変更する)
    for (auto entry : var_val) {
      if (p.second[entry.first] == entry.second) {
        continue;
      } else {
        ret = false;
        break;
      }
    }
    if (ret) {
      for (auto entry : p.second) {
        if (var_val[entry.first] == entry.second) {
          continue;
        } else {
          ret = false;
          break;
        }
      }
    }
    /// 制約集合が包含されているかの確認
    if (ret) {
      for (auto p2 : each_ask_cons_set) {
        if (std::find(p.first.begin(), p.first.end(), p2) != p.first.end()) {
          continue;
        } else {
          ret = false;
          break;
        }
      } /* /// 逆向きに制約が包含されているか判定する
       if(!ret){
         ret = true;
         for(auto p2 : p.first){
           if(std::find(each_ask_cons_set.begin(), each_ask_cons_set.end(), p2)
       != each_ask_cons_set.end()){ continue; }else{ ret = false; break;
           }
         }
       }*/
    }
    if (ret) {
      break;
    }
  }
  return ret;
}

///////////////// python接続用関数 /////////////////////

/// 変数名の登録
int Solve_sym::add_var(std::vector<std::string> name_var_list) {
  int ret = 0;
  pFunc = PyObject_GetAttrString(pModule, "add_var");

  for (auto name_var : name_var_list) {
    /* pFunc is a new reference */
    if (pFunc && PyCallable_Check(pFunc)) {
      pArgs = PyTuple_New(1);
      pValue = PyUnicode_FromString(name_var.c_str());
      PyTuple_SetItem(pArgs, 0, pValue);
      pValue = PyObject_CallObject(pFunc, pArgs);
      Py_DECREF(pArgs);
      ret = check_error(pValue);
    } else {
      if (PyErr_Occurred())
        PyErr_Print();
      fprintf(stderr, "Cannot find function \n");
    }
  }
  return ret;
}

/// 式の登録
int Solve_sym::add_equation(std::string name, std::string eq) {
  int ret = 0;
  pFunc = PyObject_GetAttrString(pModule, "add_equation");

  /* pFunc is a new reference */
  if (pFunc && PyCallable_Check(pFunc)) {
    pArgs = PyTuple_New(2);
    pValue = PyUnicode_FromString(name.c_str());
    PyTuple_SetItem(pArgs, 0, pValue);
    pValue = PyUnicode_FromString(eq.c_str());
    PyTuple_SetItem(pArgs, 1, pValue);
    pValue = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);
    ret = check_error(pValue);
  } else {
    if (PyErr_Occurred())
      PyErr_Print();
    fprintf(stderr, "Cannot find function \n");
  }
  return ret;
}

/// ガード式を解き、第2引数の変数に対しての解を返す
std::string Solve_sym::solve_equation(std::string name, std::string eq) {
  std::string ret = "";
  int error = 0;
  pFunc = PyObject_GetAttrString(pModule, "solve_equation");

  /* pFunc is a new reference */
  if (pFunc && PyCallable_Check(pFunc)) {
    pArgs = PyTuple_New(2);
    pValue = PyUnicode_FromString(name.c_str());
    PyTuple_SetItem(pArgs, 0, pValue);
    pValue = PyUnicode_FromString(eq.c_str());
    PyTuple_SetItem(pArgs, 1, pValue);
    pValue = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);
    error = check_error(pValue);
    if (error == 0) {
      ret = PyUnicode_AsUTF8(pValue);
    }
  } else {
    if (PyErr_Occurred())
      PyErr_Print();
    fprintf(stderr, "Cannot find function \n");
  }
  return ret;
}
/// 不等式を解き、第2引数の変数に対しての解を返す(現在はsolve_equationと同じだが、返しを"|"が最大優先度になるようにしたい(ケース分岐に対応するため))
std::string Solve_sym::solve_inequalities(std::string name, std::string eq) {
  std::string ret = "";
  int error = 0;
  pFunc = PyObject_GetAttrString(pModule, "solve_inequalities");

  /* pFunc is a new reference */
  if (pFunc && PyCallable_Check(pFunc)) {
    pArgs = PyTuple_New(2);
    pValue = PyUnicode_FromString(name.c_str());
    PyTuple_SetItem(pArgs, 0, pValue);
    pValue = PyUnicode_FromString(eq.c_str());
    PyTuple_SetItem(pArgs, 1, pValue);
    pValue = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);
    error = check_error(pValue);
    if (error == 0) {
      ret = PyUnicode_AsUTF8(pValue);
    }
  } else {
    if (PyErr_Occurred())
      PyErr_Print();
    fprintf(stderr, "Cannot find function \n");
  }
  return ret;
}

/// python関数の呼び出しが成功したかどうかの判定
int Solve_sym::check_error(PyObject *pValue) {
  if (pValue != NULL) {
    // printf("Result of call: %s\n", PyUnicode_AsUTF8(pValue));
    return 0;
  } else {
    PyErr_Print();
    fprintf(stderr, "Call failed\n");
    return 1;
  }
}
} // namespace debug
} // namespace hydla
