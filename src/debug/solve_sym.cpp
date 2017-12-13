#pragma once


#include "debug_main.h"
#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <boost/algorithm/string.hpp>


#include "IncrementalModuleSet.h"

using namespace hydla::debug;

namespace hydla{
  namespace debug{
    Solve_sym::Solve_sym(){}

    PyObject *pName, *pModule, *pFunc;
    PyObject *pArgs, *pValue;

    void Solve_sym::solve_main(std::vector<std::vector<std::vector<std::string>>> constraint_map, std::vector<std::vector<std::string>> v_map, std::vector<std::vector<std::string>> h_map)
    {
      std::string solve_ret;
      int error;
      char *a = "a";
      char **aa = &a;
      bool flag;
      std::string solve_var;
      std::vector<std::string> add_var_list, eq_list, ask_eq_list, true_eq_list;
      std::vector<std::vector<std::string>> each_ask_cons_sets;
      std::vector<std::vector<std::vector<std::string>>> equation_name_map;

      std::map<std::vector<std::string>, std::map<std::string, std::string>> each_var_val_map, ret_make_tell_map;///<{"INIT","FALL"},<x:1,y:1>}>
      std::map<std::string, std::string> range_map;
      std::vector<std::vector<std::vector<std::string>>> sym_prio_map; /// {{{"FALL"},{"x", ...},{"BOUNCE", ...}}, ...}

      add_var_list = mv_to_v(v_map);
      add_var_list.push_back("AAAvar");
      solve_var = conv_list(add_var_list);
      std::cout << solve_var << std::endl;

/// solve.py接続部分
      Py_InitializeEx(0);
      PyObject *sys = PyImport_ImportModule("sys");
      PyObject *path = PyObject_GetAttrString(sys, "path");
      PyList_Append(path, PyString_FromString("./src/debug"));  
      PyRun_SimpleString("import os, sys\n");

      PySys_SetArgv(0, aa); 
      pName = PyString_FromString("solve_sym");
      pModule = PyImport_Import(pName);
/// solve.py接続部分終わり


      if (pModule != NULL) {

        error = add_var(add_var_list);
        std::tie(equation_name_map, range_map, sym_prio_map) = add_equations(constraint_map, solve_var, h_map);
        for(auto p2 : sym_prio_map){
              std::cout << "hoge" << std::endl;
          for(auto p3 : p2){
            for(auto p4 : p3){
              std::cout << p4 << ", " << std::flush;
            }
              std::cout << "huga" << std::endl;
          }
        }

        /// 初期変数の登録(AAAeq:解なしかどうかの判定。AAAeq_true:不等式を満たさない時Falseを返させるため)
        error = add_equation("AAAeq", "AAAvar - 1");
        error = add_equation("AAAeq_true", "0");

        for(auto each_cons : h_map){
          if(each_cons[0] == "1"){
            ask_eq_list.push_back(each_cons[1]);
          }else if(each_cons[0] == "2"){
            true_eq_list.push_back(each_cons[1]);
          }
        }

        each_ask_cons_sets = make_cons_sets(ask_eq_list);
        /*for(auto p2 : each_ask_cons_sets){
          for(auto p3 : p2){
            std::cout << p3 << ", " << std::flush;
          }
              std::cout << "huga" << std::endl;
        }*/
        /// 前件の計算を行う
        each_var_val_map = make_ask_map(each_ask_cons_sets, true_eq_list, equation_name_map, range_map, add_var_list);

        /// 前件の計算結果の確認
        /*std::cout << "" << std::endl;
        std::cout << "ask solution" << std::endl;
        for(auto p : each_var_val_map){
          for(auto p2 : p.first){
            std::cout << p2 << ", " << std::flush;
          }
          std::cout << "hogeeee" << std::endl;
          for (auto entry : p.second){
            std::cout << entry.first << "<->" << entry.second << std::endl;
          }
        }*/

        /// 後件の計算を行う 
        std::cout << "" << std::endl;
        std::cout << "tell solution" << std::endl;
        eq_list = {};
        ret_make_tell_map = make_tell_map(each_var_val_map, equation_name_map, range_map, add_var_list, eq_list, sym_prio_map);
      }
      else {
        PyErr_Print();
        fprintf(stderr, "Failed to load \n");
      }
      std::cout << "" << std::endl;
      std::cout << "ask solution" << std::endl;
      for(auto p : each_var_val_map){
        flag = false;
        for(auto p2 : p.first){
          if(!flag){
            flag = true;
            std::cout << p2 << std::flush;
          }else{
            std::cout << ", " << p2 << std::flush;
          }
        }
        std::cout << "." << std::endl;
        for (auto entry : p.second){
          std::cout << "<" << entry.first << "," << entry.second << ">" << std::flush;
        }
        std::cout << "" << std::endl;
      }
      std::cout << "tell solution" << std::endl;
      for(auto p : ret_make_tell_map){
        flag = false;
        for(auto p2 : p.first){
          if(!flag){
            flag = true;
            std::cout << p2 << std::flush;
          }else{
            std::cout << ", " << p2 << std::flush;
          }
        }
        std::cout << "." << std::endl;
        for (auto entry : p.second){
          std::cout << "<" << entry.first << "," << entry.second << ">" << std::flush;
        }
        std::cout << "" << std::endl;
      }
      Py_Finalize();
    }

    /// 後件の計算
    std::map<std::vector<std::string>, std::map<std::string, std::string>> Solve_sym::make_tell_map(std::map<std::vector<std::string>, std::map<std::string, std::string>> each_var_val_map, std::vector<std::vector<std::vector<std::string>>> equation_name_map, 
      std::map<std::string, std::string> range_map, std::vector<std::string> add_var_list, std::vector<std::string> each_eq_list, std::vector<std::vector<std::vector<std::string>>> sym_prio_map){
      std::vector<std::string> add_eq_list, eq_list, re_eq_list, discrete_val, resolve_cons_set;
      std::map<std::vector<std::string>, std::map<std::string, std::string>> resolve_var_val_map;
      std::map<std::vector<std::string>, std::map<std::string, std::string>> ret, back_ret;

      for(auto var_val_map : each_var_val_map){
        eq_list = each_eq_list;
        add_eq_list = make_eq_list(equation_name_map, var_val_map.first, true, true);
        eq_list.insert(eq_list.end(), add_eq_list.begin(), add_eq_list.end());

          for(auto p2 : var_val_map.first){
            std::cout << p2 << ", " << std::flush;
          }
          std::cout << "end" << std::endl;
          for (auto entry : var_val_map.second){
            std::cout << entry.first << "<->" << entry.second << std::endl;
          }

        std::map<std::string, std::string> var_val_after; /// <x:1,y:1>

        /// make_val_map([<ask_0_BOUNCE, x<0>,...], [INIT, FALL,...], [x,y,Px,...]);
        var_val_after = make_val_map(range_map, eq_list, add_var_list);
        //if(var_val_after.empty()){ /// 解なしか暗黙の連続性に反する変数が存在する場合
        if(var_val_after.count("no_result")==1){
          std::cout << "No Solution" << std::endl;
          std::cout << var_val_after["no_result"] << std::endl;
          /// TODO:unsat coreを求めて、減らせる制約を減らして再計算する
          back_ret = find_unsat_core(var_val_after["no_result"], var_val_map.first, var_val_after);
          ret = connect_map(ret, back_ret);
          continue;
        }
        std::cout << "" << std::endl;
        std::cout << "check continue val" << std::endl;
        discrete_val = check_continue_val(var_val_map.first, var_val_map.second, var_val_after, add_var_list, equation_name_map, range_map);
        std::cout << "" << std::endl;
        if(!discrete_val.empty()){ /// 解なしか暗黙の連続性に反する変数が存在する場合
          std::cout << "conflict discrete" << std::endl;
          /// TODO:unsat coreを求めて、減らせる制約を減らして再計算する

          resolve_cons_set = make_resolve_cons_set(discrete_val, var_val_map.first, sym_prio_map);/// 制約を減らす
          resolve_var_val_map.clear();
          resolve_var_val_map[resolve_cons_set/*(ここに減らした制約集合を入れていく)*/] = var_val_map.second;
          re_eq_list = {};
          back_ret = make_tell_map(resolve_var_val_map, equation_name_map, range_map, add_var_list, re_eq_list, sym_prio_map);
          ret = connect_map(ret, back_ret);
          continue;
        }
        std::cout << "Find Solutions" << std::endl;
        for(auto p2 : var_val_map.first){
          std::cout << p2 << ", " << std::flush;
        }
        std::cout << "end" << std::endl;
        for(auto p2 : var_val_after){
          std::cout << "<" << p2.first << "," << p2.second << ">" << std::flush;
        }
        std::cout << "." << std::endl;
        std::cout << "check val map" << std::endl;
        back_ret = check_val_map(var_val_map.first, var_val_map.second, var_val_after, add_var_list, equation_name_map, range_map, sym_prio_map);
        ret = connect_map(ret, back_ret);
        std::cout << "" << std::endl;
      }
      return ret;
    }

    /// 2つの候補制約集合と解のマップを結合する
    std::map<std::vector<std::string>, std::map<std::string, std::string>> Solve_sym::connect_map(std::map<std::vector<std::string>, std::map<std::string, std::string>> base,std::map<std::vector<std::string>, std::map<std::string, std::string>> add){
      std::map<std::vector<std::string>, std::map<std::string, std::string>> ret;

      ret = base;
      for(auto a : add){
        ret.insert(std::make_pair(a.first, a.second));
      }
      return ret;
    }

    /// 矛盾する変数をもらって、制約階層に基づき制約を減らす
    std::vector<std::string> Solve_sym::make_resolve_cons_set(std::vector<std::string> discrete_val, std::vector<std::string> var_val_map_first, std::vector<std::vector<std::vector<std::string>>> sym_prio_map){
      std::vector<std::string> ret;
      std::vector<std::string> emp = {};
      bool flag;
      for(auto each_cons : var_val_map_first){ /// 候補制約集合に含まれる制約ごとに回す
        flag = false;
        for(auto conflict_val : discrete_val){ /// 矛盾する変数ごとに回す
          for(auto sym_prio : sym_prio_map){ /// {{{"FALL"},{"x", ...},{"BOUNCE", ...}}, ...}
            if(sym_prio[0][0] == each_cons){ /// sym_prioの制約名が候補制約集合内の制約と一致するなら
              flag = true;
              if(sym_prio[2] == emp){ /// each_consがrequiredなら
                ret.push_back(each_cons);
              }else{
                for(auto each_sym : sym_prio[1]){ /// sym_prioの変数ごとに回す
                  if(each_sym == conflict_val){ /// sym_prioの変数名が候補制約集合内の変数名と一致するなら
                    std::cout << conflict_val << " : " << each_cons << " < " << std::flush;
                    for(auto each_prio : sym_prio[2]){
                      std::cout << each_prio << ", " << std::flush;
                    }
                    std::cout << "end." << std::endl; /// 変数名：候補制約集合内の制約名 < 候補制約集合内の制約より優先度が高い制約名
                    break;
                  }
                }
              }
            }
          }
        }
        if(!flag){ /// 矛盾する変数に含まれないなら追加する
          ret.push_back(each_cons);
        }
      }
      /// TODO:無視できる変数が複数あるなら、どの制約を無視するか選択し、それ以外の制約を追加する
      /// TODO:無視した変数により無視されるようになってしまう制約を伝播的に削除していく
      /// 重複削除
      std::sort(ret.begin(), ret.end());
      ret.erase(std::unique(ret.begin(), ret.end()), ret.end() );
      //ret.push_back("FALL");
      return ret;
    }

    /// unsat coreとなる制約式を求める
    std::map<std::vector<std::string>, std::map<std::string, std::string>> Solve_sym::find_unsat_core(std::string error_m, std::vector<std::string> var_val_map_first, std::map<std::string, std::string> var_val_after){
      std::map<std::vector<std::string>, std::map<std::string, std::string>> ret;

      std::cout << "error_m" << std::endl;
      std::cout << error_m << std::endl;
      //if(error_m == "equality"){}
      ret[var_val_map_first] = var_val_after;
      return ret;
    }

    /// 得られた変数の解が連続するかどうかの確認(微分値の暗黙の連続性を確認するため)一つの変数において2つ以上の回数で離散変化が発生する場合のみその変数の配列を返す
    std::vector<std::string> Solve_sym::check_continue_val(std::vector<std::string> var_val_map_first,std::map<std::string, std::string> var_val_map_second, std::map<std::string, std::string> var_val_after, std::vector<std::string> add_var_list, std::vector<std::vector<std::vector<std::string>>> equation_name_map, std::map<std::string, std::string> range_map){
      std::vector<std::string> ret, sub_ret;
      std::string prev_name, val_name;

      for(auto var_map_after : var_val_after){ /// 値がないならそもそも暗黙の連続
        if(var_map_after.first == "AAAvar"){
          continue;
        }
        if(var_map_after.first[0] != 'P'){ /// prev変数以外を確認する
          std::cout << var_map_after.first << std::endl;
          prev_name = "P"+var_map_after.first;
          val_name = var_map_after.first;
          for (int i = 0; i < 5; ++i){ /// 使用している変数名を抽出する
           if(val_name[0] == 'D'){
              val_name = val_name.substr(1);
            }else{
              break;
            }
          }
          if(prev_name == var_map_after.second){ /// 対象の変数の値がprev値と一致するなら連続
            std::cout << "continue" << std::endl;
            std::cout << var_map_after.second << std::endl;
          }else{ /// それ以外の変数は不連続として扱う(TODO:prevの値によっては連続になることもありうるので、その辺の対応)
            std::cout << "discrete" << std::endl;
            std::cout << var_map_after.second << std::endl;
            std::cout << val_name << std::endl;
            if(std::find(sub_ret.begin(), sub_ret.end(), val_name) != sub_ret.end()){
              ret.push_back(val_name);
            }
            sub_ret.push_back(val_name);
          }
        }
      }
      std::sort(ret.begin(), ret.end());
      ret.erase(std::unique(ret.begin(), ret.end()), ret.end() );
      std::cout << "fin" << std::endl;
          /*for(auto p2 : ret){
            std::cout << p2 << ", " << std::flush;
          }
          std::cout << "end" << std::endl;*/
      /// no_solutionが見たい（TODO:本来は消す）
      //ret = {};
      return ret;
    }


    /// 得られたprev変数の解が一致するかどうかの確認(包含されている場合は分割を行う必要があるため)
    std::map<std::vector<std::string>, std::map<std::string, std::string>> Solve_sym::check_val_map(std::vector<std::string> var_val_map_first,std::map<std::string, std::string> var_val_map_second, std::map<std::string, std::string> var_val_after, std::vector<std::string> add_var_list, std::vector<std::vector<std::vector<std::string>>> equation_name_map, std::map<std::string, std::string> range_map, std::vector<std::vector<std::vector<std::string>>> sym_prio_map){
      std::string val_before;
      bool flag, undef_flag;
      std::vector<std::string> split_in_val, var_val_map_first_split, var_val_map_first_base;
      int error;
      std::map<std::vector<std::string>, std::map<std::string, std::string>> each_var_val_map, ret_make_tell_map, ret;

      var_val_map_first_base = var_val_map_first;
      for(auto var_map_after : var_val_after){
        if(var_map_after.first[0] == 'P'){ /// prev変数のみを確認する
          val_before = "";
          flag = false;
          undef_flag = false;
          std::cout << var_map_after.second << std::endl;
          /// 使用変数を求める
          boost::split(split_in_val, var_map_after.second, boost::is_any_of(" -/*"));
          for(auto each_in_val : split_in_val){ /// 式の要素ごとに回す /// tellの時の判定
            if(std::find(add_var_list.begin(), add_var_list.end(), each_in_val) != add_var_list.end()){ /// 要素が変数として認識できる場合
              if(each_in_val != var_map_after.first){ /// 確認しているprev変数以外の変数が出現した場合は値が定まらないから流す(?)(TODO:prevの変数が出たら一致しないって言ってもいいかも)
                undef_flag = true;
                break;
              }
            }
          }
          for(auto var_map : var_val_map_second){ /// askの時の判定
            if(var_map.first == var_map_after.first){
              std::cout << "hoge" << std::endl;
              std::cout << var_map.second << std::endl;
              val_before = var_map.second;
              flag = true;
              break;
            }
          }
          if(!undef_flag){
            if(flag){ /// askの時にprev変数が決まっている場合
              if(val_before == var_map_after.second){ /// askの時と同じprev値の時はそのままでいい
                continue;
              }/*else{ /// 包含判定する // これいらない気がする(包含しないことなんてない)
                std::cout << var_map_after.second << std::endl;
                std::cout << var_val_map_second[var_map_after.first] << std::endl;
              }*/
            }//else{ /// askの時に変数が決まっていない場合(-inf~infの範囲)
            /// askと違うprev値なら、tellで求めたprev値以外のprev値はどうなるのか確認する(名前をつける。どうにかする)
              //std::cout << "~Eq("+var_map_after.first+","+var_map_after.second+")" << std::endl;
            error = add_equation(var_map_after.first+"N"+var_map_after.second, "~Eq("+var_map_after.first+","+var_map_after.second+")"); /// TODO:範囲値が来た時にどうするか
            var_val_map_first_base.push_back(var_map_after.first+"Y"+var_map_after.second); /// PDyY0のような感じ。式を追加する必要があるなら追加するが、今の所必要なさそう
            var_val_map_first_split = var_val_map_first;
            var_val_map_first_split.push_back(var_map_after.first+"N"+var_map_after.second);
            each_var_val_map[var_val_map_first_split] = var_val_map_second;
            ret_make_tell_map = make_tell_map(each_var_val_map, equation_name_map, range_map, add_var_list, {var_map_after.first+"N"+var_map_after.second}, sym_prio_map);
            ret = connect_map(ret, ret_make_tell_map);
          }
        }
      }
      ret[var_val_map_first_base] = var_val_after;
      std::cout << "fin" << std::endl;
      return ret;
    }


    /// 前件の計算
    std::map<std::vector<std::string>, std::map<std::string, std::string>> Solve_sym::make_ask_map(std::vector<std::vector<std::string>> each_ask_cons_sets, std::vector<std::string> true_eq_list, std::vector<std::vector<std::vector<std::string>>> equation_name_map, std::map<std::string, std::string> range_map, std::vector<std::string> add_var_list){
      std::vector<std::string> eq_list;
      std::map<std::vector<std::string>, std::map<std::string, std::string>> ret;

      for(auto each_ask_cons_set : each_ask_cons_sets){
        each_ask_cons_set.insert(each_ask_cons_set.end(), true_eq_list.begin(), true_eq_list.end());
        eq_list = each_ask_cons_set;
        eq_list = make_eq_list(equation_name_map, eq_list, true, false);
        
        bool duplication_flag = false; /// すでにある候補制約集合と重複しているかどうかのフラグ
        std::map<std::string, std::string> var_val; /// <x:1,y:1>

        var_val = make_val_map(range_map, eq_list, add_var_list);
        //if(var_val.empty()){
        if(var_val.count("no_result")==1){
          std::cout << "empty" << std::endl;
          std::cout << var_val["no_result"] << std::endl;
          continue;
        }
        duplication_flag = check_duplication_ask(ret, var_val, each_ask_cons_set);
        if(!duplication_flag){
          std::sort(each_ask_cons_set.begin(), each_ask_cons_set.end());
          ret[each_ask_cons_set] = var_val;
        }
      }
      return ret;
    }


/// solve.pyからの出力をマップにパースする(TODO:と、言いつつ不等式部分の計算を行なっているので、この部分を関数化する)
    std::map<std::string, std::string> Solve_sym::make_val_map(std::map<std::string, std::string> range_map, std::vector<std::string> eq_list, std::vector<std::string> add_var_list){
      std::vector<std::string> split_str, split_val, add_var_list_c, split_in_val, split_solve_ret, split_each_solve_ret, eq_solve_ret, split_each_ret, split_e_each_ret;
      std::map<std::string, std::string> ret; /// <x:1,y:1>
      std::string subs, each_range_name, solve_ret, renge_eq, renge_eq_subs, renge_eq_o, mul_eq, each_eq_solve_ret_name, solve_var, solve_eq_list;
      bool flag, inf_flag, var_flag;
      bool rest_val = true;

      add_var_list_c = add_var_list;

      /// 等式の計算を行う(値が求まる変数全てに値が欲しいから繰り返し行う)
      while(rest_val){
        rest_val = false;
        solve_var = conv_list(add_var_list_c);
        solve_eq_list = conv_list(eq_list);
        //std::cout << solve_eq_list << std::endl;
        boost::algorithm::replace_all(solve_var, "[", "");
        boost::algorithm::replace_all(solve_var, "]", "");
        //std::cout << solve_var << std::endl;
        solve_ret = solve_equation(solve_eq_list, solve_var);
        std::cout << solve_ret << std::endl;
        /// []が帰ってくるならばその制約集合は解なし
        if(solve_ret == "[]" or solve_ret == "False"){
          ret.clear();
          ret["no_result"] = "equality";
          return ret;
        }

        /// いらない部分の消去(TODO:複数の解がある時はこれではダメ "}, {" でsplitして複数の候補制約集合に分割する(文字列はsplitできないので、一回特殊文字か何かに置き換える))
        boost::algorithm::replace_all(solve_ret, "[", "");
        boost::algorithm::replace_all(solve_ret, "]", "");
        /// 上は複数の解が存在する場合専用
        boost::algorithm::replace_all(solve_ret, "{", "");
        boost::algorithm::replace_all(solve_ret, "}", "");
        boost::algorithm::replace_all(solve_ret, " ", "");

        
        boost::split(split_str, solve_ret, boost::is_any_of(","));
        for(auto each_val : split_str){
          //std::cout << each_val << std::endl;
          boost::split(split_val, each_val, boost::is_any_of(":"));
          if(split_val[0] != "AAAvar"){
            //std::cout << split_val[0] << "->" << split_val[1] << std::endl;
            rest_val = true;
            ret[split_val[0]] = split_val[1];
            auto itr = std::find(add_var_list_c.begin(), add_var_list_c.end(), split_val[0]);
            if(itr != add_var_list_c.end()){
              add_var_list_c.erase(itr);
            }
          }
        }
      }
      /// 等式で解が全くもとまらないが、解なしではない場合にも対応する
      //if(ret.empty()){
        ret["AAAvar"] = "1";
      //}
      /// 不等式を解くために等式で求まった変数を代入する

      flag = false;
      subs += ".subs([";
      for(auto each_val : ret){
        if(flag){
          subs += ",";
        }
        flag = true;
        //std::cout << each_val.first << "->" << each_val.second << std::endl;
        subs += "(" + each_val.first + ", " + each_val.second + ")";
      }
      subs += "])";
      //std::cout << subs << std::endl;
      /// 不等式にsubsを代入して解く(等式と異なり、不等式は式を直接与えて計算する)
      for(auto each_range : range_map){
        each_range_name = each_range.first;
        if(std::find(eq_list.begin(), eq_list.end(), each_range_name) != eq_list.end()){
          //std::cout << each_range_name << std::endl;
          renge_eq = each_range.second;
          //std::cout << each_range.second << std::endl;
          /// 使用変数を求める
          boost::split(split_in_val, renge_eq, boost::is_any_of(" -"));
          for(auto each_in_val : split_in_val){ /// 不等式の要素ごとに回す
            if(std::find(add_var_list.begin(), add_var_list.end(), each_in_val) != add_var_list.end()){ /// 要素が変数として認識できる場合
              //std::cout << each_in_val << std::endl;
              renge_eq_subs = "(" + renge_eq + ")" + subs;
              /// ここで引数として与える変数に値が代入され、他の未知数が存在する時は呼び出しがうまくいかず解が帰ってこないが、現状はそのあとの処理としてうまく行っているので放置しておく
              solve_ret = solve_inequalities(renge_eq_subs, each_in_val); /// TODO:solve_inequalitiesの返しを"|"が最大優先度になるようにしたい
              //std::cout << "solve_ret" << std::endl;
              //std::cout << solve_ret << std::endl; /// (-oo < Py) & (Py < 1/2)こんなんが帰って来る(他にはEq(a, 1) & (((-oo < Py) & (Py < -1)) | ((1 < Py) & (Py < 2)))こんなんとか)
              /// 変数マップを作る(可能性としては複数の範囲、一つの範囲、一つの等式がありそう)
              if(solve_ret == "False"){ // その不等式内に答えが存在しない場合
                ret.clear();
                ret["no_result"] = renge_eq_subs;
                return ret;
              }else{
                boost::split(split_solve_ret, solve_ret, boost::is_any_of("|"));
                for(auto each_solve_ret : split_solve_ret){
                  boost::split(split_each_solve_ret, each_solve_ret, boost::is_any_of("&"));
                  for(auto each_split_each_solve_ret : split_each_solve_ret){
                    boost::split(eq_solve_ret, each_split_each_solve_ret, boost::is_any_of(" -()"));
                    inf_flag = false;
                    var_flag = false;
                    for(auto each_eq_solve_ret : eq_solve_ret){ /// 不等式の要素ごとに回す
                      if(each_eq_solve_ret == "oo"){ /// infを含む範囲はとりあえずいらない
                        inf_flag = true;
                        break;
                      }
                      if(std::find(add_var_list.begin(), add_var_list.end(), each_eq_solve_ret) != add_var_list.end()){ /// 要素が変数として認識できる場合
                        var_flag = true;
                        each_eq_solve_ret_name = each_eq_solve_ret;
                      }
                    }
                    if(!inf_flag and var_flag){
                      if(ret.count(each_eq_solve_ret_name) == 0){
                        ret[each_eq_solve_ret_name] = each_split_each_solve_ret;
                      }else{
                        ret[each_eq_solve_ret_name] = ret[each_eq_solve_ret_name] + " , " + each_split_each_solve_ret;
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
      for(auto each_ret : ret){
        if(each_ret.second.find(",") != std::string::npos){
          boost::split(split_each_ret, each_ret.second, boost::is_any_of(" "));
          mul_eq = "";
          for(auto e_split_each_ret : split_each_ret){
            if(e_split_each_ret.find("/") != std::string::npos){
              boost::split(split_e_each_ret, e_split_each_ret, boost::is_any_of("/"));
              mul_eq += "Retional(" + split_e_each_ret[0] + "," + split_e_each_ret[1] + ")";
            }else{
              mul_eq += e_split_each_ret;
            }
          }
          mul_eq = "[" + mul_eq + "]";
          solve_ret = solve_inequalities(mul_eq, each_ret.first);
          //std::cout << "solve_ret" << std::endl;
          //std::cout << solve_ret << std::endl;
          if(solve_ret == "False"){ // その不等式内に答えが存在しない場合
            ret.clear();
            ret["no_result"] = mul_eq;
            return ret;
          }else{
            boost::split(split_solve_ret, solve_ret, boost::is_any_of("|"));
            for(auto each_solve_ret : split_solve_ret){
              boost::split(split_each_solve_ret, each_solve_ret, boost::is_any_of("&"));
              for(auto each_split_each_solve_ret : split_each_solve_ret){
                boost::split(eq_solve_ret, each_split_each_solve_ret, boost::is_any_of(" -()"));
                inf_flag = false;
                var_flag = false;
                for(auto each_eq_solve_ret : eq_solve_ret){ /// 不等式の要素ごとに回す
                  if(each_eq_solve_ret == "oo"){ /// infを含む範囲はとりあえずいらない
                    inf_flag = true;
                    break;
                  }
                  if(std::find(add_var_list.begin(), add_var_list.end(), each_eq_solve_ret) != add_var_list.end()){ /// 要素が変数として認識できる場合
                    var_flag = true;
                    each_eq_solve_ret_name = each_eq_solve_ret;
                  }
                }
                if(!inf_flag and var_flag){
                  if(ret.count(each_eq_solve_ret_name) == 0){
                    ret[each_eq_solve_ret_name] = each_split_each_solve_ret;
                  }else{
                    ret[each_eq_solve_ret_name] = ret[each_eq_solve_ret_name] + " , " + each_split_each_solve_ret;
                  }
                }
              }
            }
          }
        }else{
          continue;
        }
      }
      return ret;
    }



///////////////// 式変形用関数 /////////////////////////

/// 配列をもらって、sympyに投げる時の形に変形する [a,b,c]のような感じ
    std::string Solve_sym::conv_list(std::vector<std::string> list){
      std::string list_str = "[";
      bool flag = false;
      for (auto e : list){
        if(flag){
          list_str += ",";
        }
        list_str += e;
        flag = true;
      }
      list_str += "]";
      return list_str;
    }

/// 変数と階数のマップをもらって、変数名の配列を返す
    std::vector<std::string> Solve_sym::mv_to_v(std::vector<std::vector<std::string>> v_map){
      std::vector<std::string> ret;
      for (auto e : v_map){
        int levels = std::stoi(e[1]);
        std::string differential = "";
        for(int i = 0; i <= levels; i++){
          ret.push_back(differential + e[0]);
          ret.push_back("P" + differential + e[0]);
          differential += "D";
        }
      }
      return ret;
    }

/// ガード付き制約名の配列をもらって、全組み合わせを返す
    std::vector<std::vector<std::string>> Solve_sym::make_cons_sets(std::vector<std::string> ask_eq_list){
      std::vector<std::vector<std::string>> ret, get_ret, c_get_ret;
      std::string name_cons;
      int sets_count = 0;

      if(ask_eq_list.size() == 0){
        ret = {{}};
      }else{
      name_cons = ask_eq_list[0];
      ask_eq_list.erase(ask_eq_list.begin());
        get_ret = make_cons_sets(ask_eq_list);
        c_get_ret = get_ret;
        for(auto eq_list : get_ret){
          get_ret[sets_count].push_back(name_cons);
          sets_count += 1;
        }
        get_ret.insert(get_ret.end(), c_get_ret.begin(), c_get_ret.end());
        ret = get_ret;
      }
      
      return ret;
    }

/// 制約集合の答えとして得られた結果がすでに登録されている結果と重複するかの判定
    bool Solve_sym::check_duplication_ask(std::map<std::vector<std::string>, std::map<std::string, std::string>> each_var_val_map, std::map<std::string, std::string> var_val, std::vector<std::string> each_ask_cons_set){
      bool ret;


      for(auto p : each_var_val_map){
        ret = true;
        /// 全く同じ変数表が得られているかの確認(TODO:一致ではなく包含判定に変更する)
        for (auto entry : var_val){
          if(p.second[entry.first] == entry.second){
            continue;
          }else{
            ret = false;
            break;
          }
        }
        if(ret){
          for (auto entry : p.second){
            if(var_val[entry.first] == entry.second){
              continue;
            }else{
              ret = false;
              break;
            }
          }
        }
        /// 制約集合が包含されているかの確認
        if(ret){
          for(auto p2 : each_ask_cons_set){
            if(std::find(p.first.begin(), p.first.end(), p2) != p.first.end()){
              continue;
            }else{
              ret = false;
              break;
            }
          }/* /// 逆向きに制約が包含されているか判定する
          if(!ret){
            ret = true;
            for(auto p2 : p.first){
              if(std::find(each_ask_cons_set.begin(), each_ask_cons_set.end(), p2) != each_ask_cons_set.end()){
                continue;
              }else{
                ret = false;
                break;
              }
            }
          }*/
        }
        if(ret){
          break;
        }
      }
      return ret;
    }


///////////////// python接続用関数 /////////////////////

/// 変数名の登録
    int Solve_sym::add_var(std::vector<std::string> name_var_list){
        int ret = 0;
        pFunc = PyObject_GetAttrString(pModule, "add_var");

        for(auto name_var : name_var_list){
          /* pFunc is a new reference */
          if (pFunc && PyCallable_Check(pFunc)) {
            pArgs = PyTuple_New(1);
            pValue = PyString_FromString(name_var.c_str());
            PyTuple_SetItem(pArgs, 0, pValue);
            pValue = PyObject_CallObject(pFunc, pArgs);
            Py_DECREF(pArgs);
            ret = check_error(pValue);
          }
          else {
            if (PyErr_Occurred())
              PyErr_Print();
            fprintf(stderr, "Cannot find function \n");
          }
        }
      return ret;
    }

/// 式の登録
    int Solve_sym::add_equation(std::string name, std::string eq){
        int ret = 0;
        pFunc = PyObject_GetAttrString(pModule, "add_equation");

        /* pFunc is a new reference */
        if (pFunc && PyCallable_Check(pFunc)) {
          pArgs = PyTuple_New(2);
          pValue = PyString_FromString(name.c_str());
          PyTuple_SetItem(pArgs, 0, pValue);
          pValue = PyString_FromString(eq.c_str());
          PyTuple_SetItem(pArgs, 1, pValue);
          pValue = PyObject_CallObject(pFunc, pArgs);
          Py_DECREF(pArgs);
          ret = check_error(pValue);
        }
        else {
          if (PyErr_Occurred())
            PyErr_Print();
          fprintf(stderr, "Cannot find function \n");
        }
      return ret;
    }

/// 制約式全体の登録(中でadd_equation()を呼ぶ) 同時に制約階層と使用変数の情報を作成する
    std::tuple<std::vector<std::vector<std::vector<std::string>>>, std::map<std::string, std::string>, std::vector<std::vector<std::vector<std::string>>>> Solve_sym::add_equations(std::vector<std::vector<std::vector<std::string>>> constraint_map, std::string solve_var, std::vector<std::vector<std::string>> h_map){
      int error;
      std::string eqation_name, cons_name;
      std::vector<std::string> each_name, each_ask, each_tell;
      std::vector<std::vector<std::string>> each_m;
      std::vector<std::vector<std::vector<std::string>>> ret;
      std::map<std::string, std::string> ret_range;
      std::vector<std::vector<std::vector<std::string>>> ret_sym_prio_map; /// {{{"FALL"},{"x", ...},{"BOUNCE", ...}}, ...}
      std::vector<std::vector<std::string>> sym_prio_map;
      std::vector<std::string> sym_, prio_, sym_ret;

      for(auto cons : constraint_map){
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
        for(auto ask : cons[1]){
          if(ask == "null"){
            continue;
          }
          eqation_name = "ask_"+std::to_string(count_ask)+"_"+cons_name;
          //std::cout << ask << std::endl;
          if(ask.find("<") == std::string::npos){
            error = add_equation(eqation_name, ask);
          }else{
            error = add_equation(eqation_name, "0");
            std::cout << "range : " << ask << std::endl;
            ret_range[eqation_name] = ask;
          }
          each_ask.push_back(eqation_name);
          count_ask += 1;
          sym_ret = get_sym(ask);
          sym_.insert(sym_.end(), sym_ret.begin(), sym_ret.end());
        }
        for(auto tell : cons[2]){
          if(tell == "null"){
            continue;
          }
          eqation_name = "tell_"+std::to_string(count_tell)+"_"+cons_name;
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
        sym_.erase(std::unique(sym_.begin(), sym_.end()), sym_.end() );
        for(auto h : h_map){
          if(h[1] == cons_name){
            for(int i = 2; i < h.size(); i++){
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

    /// 式から使用している変数を抽出する
    std::vector<std::string> Solve_sym::get_sym(std::string eq){
      std::vector<std::string> ret, split_eq;
      std::string val_name;

      boost::split(split_eq, eq, boost::is_any_of(" <>=+-*/()"), boost::token_compress_on);
      for(auto each_split_eq : split_eq){
        //if(var_map_after.first == "AAAvar"){
        //  continue;
        //}
        if(each_split_eq[0] != 'P' and each_split_eq != ""){ /// prev変数以外を確認する
          val_name = each_split_eq;
          for (int i = 0; i < 5; ++i){ /// 使用している変数名を抽出する
           if(val_name[0] == 'D'){
              val_name = val_name.substr(1);
            }else{
              break;
            }
          }
          if(islower(val_name[0]) == 0 ){
            continue;
          }
          ret.push_back(val_name);
        }
      }
      std::sort(ret.begin(), ret.end());
      ret.erase(std::unique(ret.begin(), ret.end()), ret.end() );
      return ret;
    }

/// 第２引数で与えられた制約モジュールによって作成されたpython内の式の名前をリストで返す(ask制約とtell制約の式の名前を返すかを第３、４引数で指定する)
    std::vector<std::string> Solve_sym::make_eq_list(std::vector<std::vector<std::vector<std::string>>> equation_name_map, std::vector<std::string> solve_module, bool add_ask, bool add_tell){
      std::string module_name, cons_name;
      std::vector<std::string> ret;

      for(auto equation_name : equation_name_map){
        module_name = equation_name[0][0];
        if(std::find(solve_module.begin(), solve_module.end(), module_name) == solve_module.end()){
          continue;
        }
        if(add_ask){
          for(auto ask : equation_name[1]){
            ret.push_back(ask);
          }
        }
        if(add_tell){
          for(auto tell : equation_name[2]){
            ret.push_back(tell);
          }
        }
      }
      /// AAAvar=1を返す式を追加する(本当に解なしか確認するため)
      ret.push_back("AAAeq");
      ret.push_back("AAAeq_true");
      return ret;
    }
/// ガード式を解き、第2引数の変数に対しての解を返す
    std::string Solve_sym::solve_equation(std::string name, std::string eq){
        std::string ret = "";
        int error = 0;
        pFunc = PyObject_GetAttrString(pModule, "solve_equation");

        /* pFunc is a new reference */
        if (pFunc && PyCallable_Check(pFunc)) {
          pArgs = PyTuple_New(2);
          pValue = PyString_FromString(name.c_str());
          PyTuple_SetItem(pArgs, 0, pValue);
          pValue = PyString_FromString(eq.c_str());
          PyTuple_SetItem(pArgs, 1, pValue);
          pValue = PyObject_CallObject(pFunc, pArgs);
          Py_DECREF(pArgs);
          error = check_error(pValue);
          if(error == 0){
            ret = PyString_AsString(pValue);
          }
        }
        else {
          if (PyErr_Occurred())
            PyErr_Print();
          fprintf(stderr, "Cannot find function \n");
        }
      return ret;
    }
/// 不等式を解き、第2引数の変数に対しての解を返す(現在はsolve_equationと同じ)
    std::string Solve_sym::solve_inequalities(std::string name, std::string eq){
        std::string ret = "";
        int error = 0;
        pFunc = PyObject_GetAttrString(pModule, "solve_inequalities");

        /* pFunc is a new reference */
        if (pFunc && PyCallable_Check(pFunc)) {
          pArgs = PyTuple_New(2);
          pValue = PyString_FromString(name.c_str());
          PyTuple_SetItem(pArgs, 0, pValue);
          pValue = PyString_FromString(eq.c_str());
          PyTuple_SetItem(pArgs, 1, pValue);
          pValue = PyObject_CallObject(pFunc, pArgs);
          Py_DECREF(pArgs);
          error = check_error(pValue);
          if(error == 0){
            ret = PyString_AsString(pValue);
          }
        }
        else {
          if (PyErr_Occurred()) PyErr_Print();
          fprintf(stderr, "Cannot find function \n");
        }
      return ret;
    }

/// python関数の呼び出しが成功したかどうかの判定
    int Solve_sym::check_error(PyObject *pValue){
      if (pValue != NULL) {
        //printf("Result of call: %s\n", PyString_AsString(pValue));
        return 0;
      }
      else {
        PyErr_Print();
        fprintf(stderr,"Call failed\n");
        return 1;
      }
    }
  }
}
