#include "PhaseSimulator.h"
#include "AskCollector.h"
#include "NonPrevSearcher.h"
#include "VariableFinder.h"
#include "Exceptions.h"
#include "Backend.h"
#include "PrevReplacer.h"

using namespace std;
using namespace hydla::simulator;
using namespace hydla::backend;

#include <iostream>
#include <fstream>
#include <boost/xpressive/xpressive.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include "Logger.h"
#include "Timer.h"

#include "PrevReplacer.h"
#include "TellCollector.h"
#include "AskCollector.h"
#include "VariableFinder.h"

#include "InitNodeRemover.h"
#include "MathematicaLink.h"
#include "REDUCELinkFactory.h"
#include "ContinuityMapMaker.h"

#include "PrevSearcher.h"
#include "NonPrevSearcher.h"

#include "Backend.h"
#include "Exceptions.h"
#include "AnalysisResultChecker.h"
#include "UnsatCoreFinder.h"
#include "AlwaysFinder.h"
#include "EpsilonMode.h"
#include "PhaseSimulator.h"

using namespace hydla::backend::mathematica;
using namespace hydla::backend::reduce;

using namespace boost;

using namespace hydla::ch;
using namespace hydla::simulator;
using namespace hydla::parse_tree;
using namespace hydla::logger;
using namespace hydla::timer;

using hydla::simulator::TellCollector;
using hydla::simulator::AskCollector;
using hydla::simulator::ContinuityMapMaker;
using hydla::simulator::IntervalPhase;
using hydla::simulator::PointPhase;
using hydla::simulator::VariableFinder;

variable_map_t hydla::simulator::cut_high_order_epsilon(backend_sptr_t backend_, phase_result_sptr_t phase)
{
  std::cout << "Cut High Order Epsilon Start;" << std::endl;
  variable_map_t eps_vm_ret;
  bool epsFind = false;
  if(phase->phase_type==0){std::cout << "PointPhase " << phase->id << std::endl;}
  if(phase->phase_type==1){std::cout << "IntervalPhase " << phase->id << std::endl;}

  for(parameter_map_t::iterator eps_p_it = phase->parameter_map.begin(); eps_p_it != phase->parameter_map.end(); eps_p_it++){
    std::cout << eps_p_it->first << "\t: " << eps_p_it->second << std::endl;
    std::string eps_parameter_name = eps_p_it->first.get_name();
    int eps_parameter_differential_count = eps_p_it->first.get_differential_count();
    if(eps_parameter_name=="eps" && eps_parameter_differential_count==0){
      std::cout << "parameter eps Find!!"  << std::endl;
      epsFind = true;
      variable_map_t& eps_vm_tmp = phase->variable_map;
      variable_map_t::iterator eps_v_it;
      simulator::value_t eps_ret;
      simulator::value_t eps_doit;
      int differential_times = 1; // のこす次数
      simulator::value_t eps_time = phase->current_time;
      simulator::value_t eps_time_ret;
      backend_->call("cutHighOrderVariable", 3, "vlnpi", "vl", &eps_time, &eps_p_it->first, &differential_times, &eps_time_ret);
      phase->current_time = eps_time_ret;
      for(eps_v_it = eps_vm_tmp.begin();eps_v_it!=eps_vm_tmp.end();eps_v_it++){
        if(eps_v_it->second.unique()){
          eps_doit = eps_v_it->second.get_unique();
          std::cout << eps_v_it->first << "\t: (" << eps_doit;
          backend_->call("cutHighOrderVariable", 3, "vlnpi", "vl", &eps_doit, &eps_p_it->first, &differential_times, &eps_ret);
          std::cout << " -> " << eps_ret << ")" << std::endl;
          eps_vm_ret[eps_v_it->first] = eps_ret;
        }
      }
    }
  }
  std::cout << "Cut High Order Epsilon End;" << std::endl;
  if(epsFind){return eps_vm_ret;}
  else {return phase->variable_map;}
}


pp_time_result_t hydla::simulator::reduce_unsuitable_case(pp_time_result_t time_result, backend_sptr_t backend_, phase_result_sptr_t phase)
{
  std::cout << "Remove UnSuitable Cases Start;" << std::endl;
  if(phase->phase_type==0){std::cout << "PointPhase " << phase->id << std::endl;}
  if(phase->phase_type==1){std::cout << "IntervalPhase " << phase->id << std::endl;}
  // 次のPointPhaseを求めた後に分岐するCase数が出る？
   std::cout << "Next Phase Case Count\t: " << time_result.size() << std::endl;
  unsigned int eps_time_it;
  pp_time_result_t eps_time_result;

  // 次のPointPhaseに向けて分岐するCaseを全て調べる
  for(eps_time_it=0;eps_time_it < time_result.size();eps_time_it++){
     std::cout << "Case \t: " << (eps_time_it + 1) << std::endl;
    NextPhaseResult &eps_candidate = time_result[eps_time_it];
    bool eps_isNG = false;

    // 変数表(parameter_map)の変数(parameter_t)を見ていく
    for(parameter_map_t::iterator eps_it = eps_candidate.parameter_map.begin(); eps_it != eps_candidate.parameter_map.end(); eps_it++){
      // 変数(parameter_t)の名前や微分回数をチェックする
      // 変数の名前がepsの時, 値が0の近傍か確認する
      std::string eps_parameter_name = eps_it->first.get_name();
      int eps_parameter_differential_count = eps_it->first.get_differential_count();
      if(eps_parameter_name=="eps" && eps_parameter_differential_count==0){
         std::cout << eps_it->first << "\t: " << eps_it->second << "\t->\t";

        node_sptr eps_val_node;
        bool eps_isRet;
        if(eps_it->second.unique()){
          eps_val_node = eps_it->second.get_unique().get_node();
          eps_val_node = node_sptr(new Times(eps_val_node, eps_it->second.get_unique().get_node()));
          // std::cout << "before is Over Zero \t: " << *eps_val_node << std::endl;
          backend_->call("isOverZero", 1, "en", "b", &eps_val_node, &eps_isRet);
          // std::cout << "after is Over Zero \t: " << eps_isRet << std::endl;
          eps_isNG = eps_isNG || eps_isRet;
        }else{
          if(eps_it->second.get_lower_cnt()){
            eps_val_node = eps_it->second.get_lower_bound(0).value.get_node();
            backend_->call("isOverZero", 1, "en", "b", &eps_val_node, &eps_isRet);
            eps_isNG = eps_isNG || eps_isRet;
          }
          if(eps_it->second.get_upper_cnt()){
            eps_val_node = node_sptr(new Number("-1"));
            eps_val_node = node_sptr(new Times(eps_val_node, eps_it->second.get_upper_bound(0).value.get_node()));
            backend_->call("isOverZero", 1, "en", "b", &eps_val_node, &eps_isRet);
            eps_isNG = eps_isNG || eps_isRet;
          }
        }
        if(eps_isNG){
          std::cout<<"NG range"<<std::endl;
        }else{
          std::cout<<"OK"<<std::endl;
        }
        }else{
          // eps以外の変数
          std::cout << eps_it->first << "\t: " << eps_it->second << std::endl;
      }
    }
    // 変数の中にNGなepsの範囲があった場合, 結果としない
    if(!eps_isNG){
      // eps_time_result[eps_time_result_it++] = eps_candidate;
      eps_time_result.push_back(eps_candidate);
    }
  }
   std::cout << "######### remove NG Case & all "<< time_result.size() << " -> " << eps_time_result.size() << std::endl;
  // 現在のcalculateNextPPの結果を削除して, 新しい結果ので置換する
  if(time_result.size() != eps_time_result.size()){
    time_result.clear();
    for(eps_time_it=0;eps_time_it < eps_time_result.size();eps_time_it++){
       std::cout << "NewCase\t: " << (eps_time_it + 1) << std::endl;
      NextPhaseResult &eps_candidate = eps_time_result[eps_time_it];
      time_result.push_back(eps_candidate);
      // 内容の確認
      bool eps_isNG = false;
      for(parameter_map_t::iterator eps_it = eps_candidate.parameter_map.begin(); eps_it != eps_candidate.parameter_map.end(); eps_it++){
        std::string eps_parameter_name = eps_it->first.get_name();
        int eps_parameter_differential_count = eps_it->first.get_differential_count();
        if(eps_parameter_name=="eps" && eps_parameter_differential_count==0){
           std::cout << eps_it->first << "\t: " << eps_it->second << "\t->\t";
          node_sptr eps_val_node;
          bool eps_isRet;
          if(eps_it->second.unique()){
            eps_val_node = eps_it->second.get_unique().get_node();
            eps_val_node = node_sptr(new Times(eps_val_node, eps_it->second.get_unique().get_node()));
            backend_->call("isOverZero", 1, "en", "b", &eps_val_node, &eps_isRet);
            eps_isNG = eps_isNG || eps_isRet;
          }else{
            if(eps_it->second.get_lower_cnt()){
              eps_val_node = eps_it->second.get_lower_bound(0).value.get_node();
              backend_->call("isOverZero", 1, "en", "b", &eps_val_node, &eps_isRet);
              eps_isNG = eps_isNG || eps_isRet;
            }
            if(eps_it->second.get_upper_cnt()){
              eps_val_node = node_sptr(new Number("-1"));
              eps_val_node = node_sptr(new Times(eps_val_node, eps_it->second.get_upper_bound(0).value.get_node()));
              backend_->call("isOverZero", 1, "en", "b", &eps_val_node, &eps_isRet);
              eps_isNG = eps_isNG || eps_isRet;
            }
          }
          if(eps_isNG){
            std::cout<<"NG range"<<std::endl;
            }else{
            std::cout<<"OK"<<std::endl;
          }
        }
      }
    }
  }
  std::cout << "Remove UnSuitable Cases End;" << std::endl;
  return time_result;
}
