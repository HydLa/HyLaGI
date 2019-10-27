#pragma once

#include "json_parser.h"
#include "ParseTree.h"
#include "picojson.h"

#include <iostream>
#include <algorithm>
#include <boost/algorithm/string.hpp>

using namespace hydla::debug;
using namespace std;
using namespace picojson;


namespace hydla{
  namespace debug{
    Json_p::Json_p(){}

    std::string next_m = "#";
    std::string ask_c = "$";
    std::string logical_and = "&";
    std::string logical_and_tell = "_";
    std::string definition_m = "@";


    std::tuple<std::vector<std::vector<std::vector<std::string>>>, std::vector<std::vector<std::string>>> Json_p::json_p(std::stringstream& sout, std::string prio){
      value jstr;
      std::string err = parse(jstr, sout);
      std::string serch_str = json_search(jstr, false, false);
      std::vector<std::vector<std::vector<std::string>>> v_ret;
      std::vector<std::vector<std::string>> h_ret;
      std::map<std::string, std::vector<std::string>> p_ret;
      v_ret = make_vector(serch_str);
      p_ret = make_prio_map(prio);
      h_ret = make_h_map(v_ret, p_ret);
      return std::forward_as_tuple(v_ret, h_ret);
    }

    /// 自分より強い制約のマップを作成
    std::map<std::string, std::vector<std::string>> Json_p::make_prio_map(std::string prio){
      std::map<std::string, std::vector<std::string>> ret;
      std::vector<std::string> split_prio, split_p;
      bool flag = false;

      boost::split(split_prio, prio, boost::is_any_of(";\n"), boost::token_compress_on);
      for(auto p : split_prio){
        if (p.find("->") != std::string::npos) {
          replace_all(p, " ", "");
          replace_all(p, "\"", "");
          boost::split(split_p, p, boost::is_any_of("->"), boost::token_compress_on);
          ret[split_p[1]].push_back(split_p[0]);
        }
      }
      while(1){ /// 直接階層が設定されていない制約も確認する
        flag = false;
        for(auto p : ret){
          for(auto p1 : p.second){
            for(auto p2 : ret[p1]){
              if(std::find(p.second.begin(), p.second.end(), p2) == p.second.end() and p2 != p.first){
                ret[p.first].push_back(p2);
                flag = true;
              }
            }
          }
        }
        if(!flag){ // 書き換えが発生しなかったら抜ける
          break;
        }
      }
      return ret;
    }

    std::vector<std::vector<std::string>> Json_p::make_h_map(std::vector<std::vector<std::vector<std::string>>> v_ret, std::map<std::string, std::vector<std::string>> p_ret){
      std::vector<std::vector<std::string>> ret, h_ret;
      std::vector<std::string> each_cons, h;
      std::string ask_flag; // []がつかない制約は0, それ以外でask制約があれば1, なければ2

      for(auto constraints : v_ret){
        each_cons.clear();
        ask_flag = "0";
        for(auto ask : constraints[1]){
          if(ask == "0"){
            ask_flag = "2";
            break;
          }else if(ask == "null"){
            continue;
          }else{
            ask_flag = "1";
            break;
          }
        }
        if(ask_flag != "0"){
          each_cons.push_back(ask_flag);
          each_cons.push_back(constraints[0][0]);
          ret.push_back(each_cons);
        }
      }
      for(auto p : ret){
        h.clear();
        h.push_back(p[0]);
        h.push_back(p[1]);
        for(auto p1 : p_ret[p[1]]){
          h.push_back(p1);
        }
        h_ret.push_back(h);
      }
      return h_ret;
    }

    std::vector<std::vector<std::vector<std::string>>> Json_p::make_vector(std::string serch_str){
      std::vector<std::vector<std::vector<std::string>>> ret;
      std::vector<std::vector<std::string>> each_m;
      std::vector<std::string> each_eq, each_eq_ask, each_eq_tell;
      std::vector<std::string> split_str, split_str2, split_str21, split_str3, split_str3_ask, split_str3_tell, cons_list;
      std::string cons;

      boost::split(split_str, serch_str, boost::is_any_of(next_m));
      split_str.pop_back();
      for(auto spl : split_str){
        each_eq.clear();
        each_eq_ask.clear();
        each_eq_tell.clear();
        each_m.clear();
        boost::split(split_str2, spl, boost::is_any_of(definition_m));
        cons = *split_str2.begin();
        if(std::find(cons_list.begin(), cons_list.end(), cons) != cons_list.end()){ // 登録済みなら飛ばす
          continue;
        }
        each_eq.push_back(cons);
        cons_list.push_back(cons);
        boost::split(split_str21, split_str2[1], boost::is_any_of(logical_and_tell)); /// 複数tell制約がある場合の対処(複数のask制約には対応していない)
        for(auto split_str22 : split_str21){
          boost::split(split_str3, split_str22, boost::is_any_of(ask_c));
          boost::split(split_str3_ask, *split_str3.begin(), boost::is_any_of(logical_and));
          boost::split(split_str3_tell, split_str3[1], boost::is_any_of(logical_and));
          for(auto spl_ask : split_str3_ask){
            each_eq_ask.push_back(spl_ask);
          }
          for(auto spl_tell : split_str3_tell){
            each_eq_tell.push_back(spl_tell);
          }
        }
        each_m.push_back(each_eq);
        each_m.push_back(each_eq_ask);
        each_m.push_back(each_eq_tell);
        ret.push_back(each_m);
      }
      return ret;
    }

    std::string Json_p::json_search(value jstr, bool always, bool ask){
      std::vector<std::string> lrhs = {"Parallel", "LogicalAnd", "Weaker", "Ask"};
      std::vector<std::string> cld = {"Constraint", "ConstraintCaller", "Always"};
      std::vector<std::string> ineq = {"Equal", "Less", "LessEqual", "Greater", "GreaterEqual"};
      std::string sol, ret, mo_name;
      if(jstr.get<object>()["type"].is<null>()){
      }else{
        std::string type_name = jstr.get<object>()["type"].get<std::string>();
        if(jstr.get<object>()["name"].is<null>()){
        }else{
          mo_name = jstr.get<object>()["name"].get<std::string>();
        }
        if(std::find(lrhs.begin(), lrhs.end(), type_name) != lrhs.end()){
          if(type_name == "Ask"){
            ask = true;
          }
          sol = json_search(jstr.get<object>()["lhs"], always, ask);
          ret = json_search(jstr.get<object>()["rhs"], always, ask);
          if(type_name == "Ask"){
            ret = sol + ask_c + ret;
          }else if(type_name == "LogicalAnd" && ask){
            ret = sol + logical_and + ret;
          }else if(type_name == "LogicalAnd"){
            ret = sol + logical_and_tell + ret;
          }else{
            ret = sol + ret;
          }
        }else if(std::find(cld.begin(), cld.end(), type_name) != cld.end()){
          if(type_name == "Always"){
            always = true;
          }
          ret = json_search(jstr.get<object>()["child"], always, ask);
          if(type_name == "ConstraintCaller"){
            ret = mo_name + definition_m + ret + next_m;
          }
        }else if(std::find(ineq.begin(), ineq.end(), type_name) != ineq.end()){
          if(always){
            ret = make_equation(jstr);
            if(!ask){
              ret = "0" + ask_c + ret;
            }
          }else{
            ret = "null" + ask_c + "null";
          }
        }
      }
      return ret;
    }

    std::string Json_p::make_equation(value jstr){
      std::vector<std::string> lrhs = {"Subtract", "Plus", "Divide", "Times", "Equal", "Less", "LessEqual", "Greater", "GreaterEqual", "Power"};
      std::vector<std::string> cld = {"Negative", "Previous", "Differential"};
      std::vector<std::string> solo = {"Variable", "Number", "SymbolicT"};
      std::string ret;

      std::string type_name = jstr.get<object>()["type"].get<std::string>();

      if(std::find(lrhs.begin(), lrhs.end(), type_name) != lrhs.end()){
        std::string lhs = make_equation(jstr.get<object>()["lhs"]);
        std::string rhs = make_equation(jstr.get<object>()["rhs"]);
        if(type_name == "Subtract"){
          ret = lhs + " - " + rhs;
        }else if(type_name == "Plus"){
          ret = lhs + " + " + rhs;
        }else if(type_name == "Divide"){
          ret = "Rational(" + lhs + "," + rhs + ")";
        }else if(type_name == "Times"){
          ret = lhs + " * " + rhs;
        }else if(type_name == "Equal"){
          ret = rhs + " - " + lhs;
        }else if(type_name == "Less"){
          //ret = rhs + " - " + lhs;
          ret = lhs + " - " + rhs + " < 0 ";
        }else if(type_name == "LessEqual"){
          //ret = rhs + " - " + lhs;
          ret = lhs + " - " + rhs + " <= 0 ";
        }else if(type_name == "Greater"){
          ret = rhs + " - " + lhs + " < 0 ";
        }else if(type_name == "GreaterEqual"){
          ret = rhs + " - " + lhs + " <= 0 ";
        }else if(type_name == "Power"){
          ret = lhs + " ** " + rhs;
        }
      }else if(std::find(cld.begin(), cld.end(), type_name) != cld.end()){
        ret = make_equation(jstr.get<object>()["child"]);
        if(type_name == "Previous"){
          ret = "P" + ret;
        }else if(type_name == "Differential"){
          ret = "D" + ret;
        }else if(type_name == "Negative"){
          ret = "-" + ret;
        }
      }else if(std::find(solo.begin(), solo.end(), type_name) != cld.end()){
        if(type_name == "Number"){
          ret = jstr.get<object>()["value"].get<std::string>();
        }else if(type_name == "Variable"){
          ret = jstr.get<object>()["name"].get<std::string>();
        }
      }
      return ret;
    }


    std::vector<std::vector<std::string>> Json_p::make_v_map(std::shared_ptr<hydla::parse_tree::ParseTree> parse_tree){
      std::vector<std::vector<std::string>> ret;
      std::vector<std::string> map_ret;

      for (auto entry : parse_tree->get_variable_map())
      {
        map_ret.push_back(entry.first);
        map_ret.push_back(std::to_string(entry.second));
        ret.push_back(map_ret);
        map_ret.clear();
      }
      return ret;
    }



  }
}
