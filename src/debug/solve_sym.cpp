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
      std::string solve_var, solve_eq_list;
      std::vector<std::string> add_var_list, eq_list, ask_eq_list, true_eq_list;
      std::vector<std::vector<std::string>> each_ask_cons_sets;
      std::vector<std::vector<std::vector<std::string>>> equation_name_map;

      std::map<std::vector<std::string>, std::map<std::string, std::string>> each_var_val_map;///<{"INIT","FALL"},<x:1,y:1>}>
      std::map<std::string, std::string> range_map;

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
        std::tie(equation_name_map, range_map) = add_equations(constraint_map, solve_var);

      for (auto entry : range_map)
      {
        std::cout << entry.first << std::endl;
        std::cout << entry.second << std::endl;
      }

        /// 初期変数の登録
        error = add_equation("AAAeq", "AAAvar - 1");

        for(auto each_cons : h_map){
          if(each_cons[0] == "1"){
            ask_eq_list.push_back(each_cons[1]);
          }else if(each_cons[0] == "2"){
            true_eq_list.push_back(each_cons[1]);
          }
        }

        each_ask_cons_sets = make_cons_sets(ask_eq_list);
        /// 前件の計算を行う
        for(auto each_ask_cons_set : each_ask_cons_sets){
          each_ask_cons_set.insert(each_ask_cons_set.end(), true_eq_list.begin(), true_eq_list.end());
          eq_list = each_ask_cons_set;
          eq_list = make_eq_list(equation_name_map, eq_list, true, false);

            /*for(auto p2 : eq_list){
              std::cout << p2 << std::endl;
            }*/

          solve_eq_list = conv_list(eq_list); /// [INIT,FALL...]
          
          bool duplication_flag = false;
          std::map<std::string, std::string> var_val; /// <x:1,y:1>

          var_val = make_val_map(solve_eq_list, solve_var, range_map, eq_list, add_var_list);
          if(var_val.empty()){
            continue;
          }
          duplication_flag = check_duplication_ask(each_var_val_map, var_val, each_ask_cons_set);
          if(!duplication_flag){ // 要素数が多い集合から計算するから重複する時は要素数が少ない方であるはず
            each_var_val_map[each_ask_cons_set] = var_val;
          }
        }
        
              std::cout << "hoge" << std::endl;
        for(auto p : each_var_val_map){
          for(auto p2 : p.first){
            std::cout << p2 << std::endl;
          }
          for (auto entry : p.second)
          {
            std::cout << entry.first << std::endl;
            std::cout << entry.second << std::endl;
          }std::cout << "huga" << std::endl;
        }
        /// 後件の計算を行う とりあえず一回置いておく
        
        for(auto var_val_map : each_var_val_map){
          eq_list = make_eq_list(equation_name_map, var_val_map.first, true, true);

            for(auto p2 : var_val_map.first){
              std::cout << p2 << std::endl;
            }

          std::map<std::string, std::string> var_val_after; /// <x:1,y:1>

          solve_eq_list = conv_list(eq_list);
          //solve_ret = solve_equation(solve_eq_list, solve_var);
          var_val_after = make_val_map(solve_eq_list, solve_var, range_map, eq_list, add_var_list);
          if(var_val_after.empty()){
          std::cout << "noooooooooooo" << std::endl;
            continue;
          }
          std::cout << "hugaaaaaaaa" << std::endl;
            for(auto p2 : var_val_after){
              std::cout << p2.first << "->" << p2.second << std::endl;
            }
          //std::cout << solve_ret << std::endl;
        }

      }
      else {
        PyErr_Print();
        fprintf(stderr, "Failed to load \n");
        //return 1;
      }
      Py_Finalize();
      //return 0;
    }




/// solve.pyからの出力をマップにパースする(TODO:と、言いつつ不等式部分の計算を行なっているので、この部分を関数化する)
    std::map<std::string, std::string> Solve_sym::make_val_map(std::string solve_eq_list, std::string solve_var, std::map<std::string, std::string> range_map, std::vector<std::string> eq_list, std::vector<std::string> add_var_list){
      std::vector<std::string> split_str, split_val, add_var_list_c, split_in_val, split_solve_ret, split_each_solve_ret, eq_solve_ret, split_each_ret, split_e_each_ret;
      std::map<std::string, std::string> ret; /// <x:1,y:1>
      std::string subs, each_range_name, solve_ret, renge_eq, renge_eq_subs, renge_eq_o, mul_eq;
      bool flag;
      bool rest_val = true;

      add_var_list_c = add_var_list;

      /// 等式の計算を行う(値が求まる変数全てに値が欲しいから繰り返し行う)
      while(rest_val){
        rest_val = false;
        solve_var = conv_list(add_var_list_c);
        solve_ret = solve_equation(solve_eq_list, solve_var);
        std::cout << solve_ret << std::endl;
        /// []が帰ってくるならばその制約集合は解なし
        if(solve_ret == "[]"){
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
      /// 不等式にsubsを代入して解く
      for(auto each_range : range_map){
        each_range_name = each_range.first;
        if(std::find(eq_list.begin(), eq_list.end(), each_range_name) != eq_list.end()){
          //std::cout << each_range_name << std::endl;
          renge_eq = each_range.second;
          std::cout << each_range.second << std::endl;
          /// 使用変数を求める
          boost::split(split_in_val, renge_eq, boost::is_any_of(" -"));
          for(auto each_in_val : split_in_val){ /// 不等式の要素ごとに回す
            if(std::find(add_var_list.begin(), add_var_list.end(), each_in_val) != add_var_list.end()){ /// 要素が変数として認識できる場合
              std::cout << each_in_val << std::endl;
              renge_eq_subs = "(" + renge_eq + ")" + subs;
              solve_ret = solve_inequalities(renge_eq_subs, each_in_val); /// TODO:solve_inequalitiesの返しを"|"が最大優先度になるようにしたい
              std::cout << "solve_ret" << std::endl;
              std::cout << solve_ret << std::endl; /// (-oo < Py) & (Py < 1/2)こんなんが帰って来る(他にはEq(a, 1) & (((-oo < Py) & (Py < -1)) | ((1 < Py) & (Py < 2)))こんなんとか)
              /// 変数マップを作る(可能性としては複数の範囲、一つの範囲、一つの等式がありそう)
              if(solve_ret == "False"){ // その不等式内に答えが存在しない場合
                ret.clear();
                return ret;
              }else{
                boost::split(split_solve_ret, solve_ret, boost::is_any_of("|"));
                for(auto each_solve_ret : split_solve_ret){
                  boost::split(split_each_solve_ret, each_solve_ret, boost::is_any_of("&"));
                  for(auto each_split_each_solve_ret : split_each_solve_ret){
                    boost::split(eq_solve_ret, each_split_each_solve_ret, boost::is_any_of(" -()"));
                    for(auto each_eq_solve_ret : eq_solve_ret){ /// 不等式の要素ごとに回す
                      if(std::find(add_var_list.begin(), add_var_list.end(), each_eq_solve_ret) != add_var_list.end()){ /// 要素が変数として認識できる場合
                        if(ret.count(each_eq_solve_ret) == 0){
                          ret[each_eq_solve_ret] = each_split_each_solve_ret;
                        }else{
                          ret[each_eq_solve_ret] = ret[each_eq_solve_ret] + " , " + each_split_each_solve_ret;
                        }
                        break;
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
            return ret;
          }else{
            boost::split(split_solve_ret, solve_ret, boost::is_any_of("|"));
            for(auto each_solve_ret : split_solve_ret){
              boost::split(split_each_solve_ret, each_solve_ret, boost::is_any_of("&"));
              for(auto each_split_each_solve_ret : split_each_solve_ret){
                boost::split(eq_solve_ret, each_split_each_solve_ret, boost::is_any_of(" -"));
                for(auto each_eq_solve_ret : eq_solve_ret){ /// 不等式の要素ごとに回す
                  if(std::find(add_var_list.begin(), add_var_list.end(), each_eq_solve_ret) != add_var_list.end()){ /// 要素が変数として認識できる場合
                    if(ret.count(each_eq_solve_ret) == 0){
                      ret[each_eq_solve_ret] = each_solve_ret;
                    }else{
                      ret[each_eq_solve_ret] = ret[each_eq_solve_ret] + " , " + each_split_each_solve_ret;
                    }
                    break;
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
          }
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

/// 制約式全体の登録(中でadd_equation()を呼ぶ)
    std::tuple<std::vector<std::vector<std::vector<std::string>>>, std::map<std::string, std::string>> Solve_sym::add_equations(std::vector<std::vector<std::vector<std::string>>> constraint_map, std::string solve_var){
      int error;
      std::string eqation_name, cons_name;
      std::vector<std::string> each_name, each_ask, each_tell;
      std::vector<std::vector<std::string>> each_m;
      std::vector<std::vector<std::vector<std::string>>> ret;
      std::map<std::string, std::string> ret_range;

      for(auto cons : constraint_map){
        each_name.clear();
        each_ask.clear();
        each_tell.clear();
        each_m.clear();
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
            ret_range[eqation_name] = ask;
          }
          each_ask.push_back(eqation_name);
          count_ask += 1;
        }
        for(auto tell : cons[2]){
          if(tell == "null"){
            continue;
          }
          eqation_name = "tell_"+std::to_string(count_tell)+"_"+cons_name;
          error = add_equation(eqation_name, tell);
          each_tell.push_back(eqation_name);
          count_tell += 1;
        }
        each_m.push_back(each_name);
        each_m.push_back(each_ask);
        each_m.push_back(each_tell);
        ret.push_back(each_m);
      }
      return std::forward_as_tuple(ret, ret_range);
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
/// 不等式を解き、第2引数の変数に対しての解を返す
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
          if (PyErr_Occurred())
            PyErr_Print();
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
