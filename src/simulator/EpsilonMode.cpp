#include "PhaseSimulator.h"
#include "AskCollector.h"
#include "VariableFinder.h"
#include "Exceptions.h"
#include "Backend.h"
#include "PrevReplacer.h"

using namespace std;
using namespace hydla::simulator;
using namespace hydla::backend;

//n次近似
#define DIFFCNT 1
// #define _DEBUG_CUT_HIGH_ORDER
// #define _DEBUG_REDUCE_UNSUIT
// #define _DEBUG_ZERO_CASE

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

using namespace hydla::hierarchy;
using namespace hydla::simulator;
using namespace hydla::symbolic_expression;
using namespace hydla::logger;
using namespace hydla::timer;

using hydla::simulator::TellCollector;
using hydla::simulator::AskCollector;
using hydla::simulator::ContinuityMapMaker;
using hydla::simulator::IntervalPhase;
using hydla::simulator::PointPhase;
using hydla::simulator::VariableFinder;

variable_map_t hydla::simulator::cut_high_order_epsilon(Backend* backend_, phase_result_sptr_t& phase)
{
  variable_map_t vm_ret;
  bool have_eps = false;
#ifdef _DEBUG_CUT_HIGH_ORDER
  std::cout << "Cut High Order Epsilon Start;" << std::endl;
  if(phase->phase_type==0)
    std::cout << "PointPhase " << phase->id << std::endl;
  if(phase->phase_type==1)
    std::cout << "IntervalPhase " << phase->id << std::endl;
#endif
  for(parameter_map_t::iterator p_it = phase->parameter_map.begin(); p_it != phase->parameter_map.end(); p_it++)
  {
#ifdef _DEBUG_CUT_HIGH_ORDER
    std::cout << p_it->first << "\t: " << p_it->second << std::endl;
#endif
    std::string parameter_name = p_it->first.get_name();
    int parameter_differential_count = p_it->first.get_differential_count();
    if(parameter_name=="eps" && parameter_differential_count==0)
    {
#ifdef _DEBUG_CUT_HIGH_ORDER
      std::cout << "parameter eps Find!!"  << std::endl;
#endif
      int differential_times = DIFFCNT; // のこす次数
      have_eps = true;
      value_t time_ret;
      backend_->call("cutHighOrderVariable", 3, "vlnpi", "vl", &phase->current_time, &p_it->first, &differential_times, &time_ret);
      phase->current_time = time_ret;
     for(variable_map_t::iterator v_it = phase->variable_map.begin();v_it!=phase->variable_map.end();v_it++)
      {
        if(v_it->second.undefined())
        {
          vm_ret[v_it->first] = v_it->second;
        }
        else if(v_it->second.unique())
        {
          simulator::value_t ret;
          simulator::value_t val = v_it->second.get_unique();
          range_t& range = vm_ret[v_it->first];
#ifdef _DEBUG_CUT_HIGH_ORDER
          std::cout << v_it->first << "\t: (" << val;
#endif
          backend_->call("cutHighOrderVariable", 3, "vlnpi", "vl", &val, &p_it->first, &differential_times, &ret);
#ifdef _DEBUG_CUT_HIGH_ORDER
          std::cout << " -> " << ret << ")" << std::endl;
#endif
          range.set_unique(ret);
          // vm_ret[eps_v_it->first] = eps_ret;
        }
        else
        {
          range_t range = v_it->second;
          for(uint i = 0; i < range.get_lower_cnt(); i++)
          {
            ValueRange::bound_t bd = v_it->second.get_lower_bound(i);
            value_t val = bd.value;
            value_t ret;
            backend_->call("cutHighOrderVariable", 3, "vlnpi", "vl", &val, &p_it->first, &differential_times, &ret);
            range.set_lower_bound(ret, bd.include_bound);
          }
          for(uint i = 0; i < range.get_upper_cnt(); i++)
          {
            ValueRange::bound_t bd = v_it->second.get_upper_bound(i);
            value_t val = bd.value;
            value_t ret;
            backend_->call("cutHighOrderVariable", 3, "vlnpi", "vl", &val, &p_it->first, &differential_times, &ret);
            range.set_upper_bound(ret, bd.include_bound);
          }
          vm_ret[v_it->first] = range;
        }
      }
    }
  }
#ifdef _DEBUG_CUT_HIGH_ORDER
  std::cout << "Cut High Order Epsilon End;" << std::endl;
#endif
  if(have_eps)
    return vm_ret;
  else
    return phase->variable_map;
}

pp_time_result_t hydla::simulator::reduce_unsuitable_case(pp_time_result_t time_result, Backend* backend_, phase_result_sptr_t& phase)
{
#ifdef _DEBUG_REDUCE_UNSUIT
  std::cout << "Remove UnSuitable Cases Start;" << std::endl;
  if(phase->phase_type==0)
    std::cout << "PointPhase " << phase->id << std::endl;
  else
    std::cout << "IntervalPhase " << phase->id << std::endl;
  std::cout << "Next Phase Case Count\t: " << time_result.size() << std::endl;
#endif
  unsigned int time_it;
  pp_time_result_t eps_time_result;
  for(time_it=0;time_it < time_result.size();time_it++)
  {
#ifdef _DEBUG_REDUCE_UNSUIT
    std::cout << "Case \t: " << (time_it + 1) << std::endl;
#endif
    NextPhaseResult &candidate = time_result[time_it];
    bool isNG = false;
    for(parameter_map_t::iterator p_it = candidate.parameter_map.begin(); p_it != candidate.parameter_map.end(); p_it++)
    {
      std::string parameter_name = p_it->first.get_name();
      int parameter_differential_count = p_it->first.get_differential_count();
      if(parameter_name=="eps" && parameter_differential_count==0)
      {
#ifdef _DEBUG_REDUCE_UNSUIT
        std::cout << p_it->first << "\t: " << p_it->second << "\t->\t";
#endif
        symbolic_expression::node_sptr val;
        bool Ret;
        if(p_it->second.unique())
        {
          val = p_it->second.get_unique().get_node();
          val = symbolic_expression::node_sptr(new Times(val, p_it->second.get_unique().get_node()));
          backend_->call("isOverZero", 1, "en", "b", &val, &Ret);
          isNG = isNG || Ret;
        }
        else
        {
          if(p_it->second.get_lower_cnt())
          {
            val = p_it->second.get_lower_bound(0).value.get_node();
            backend_->call("isOverZero", 1, "en", "b", &val, &Ret);
            isNG = isNG || Ret;
          }
          if(p_it->second.get_upper_cnt())
          {
            val = symbolic_expression::node_sptr(new Number("-1"));
            val = symbolic_expression::node_sptr(new Times(val, p_it->second.get_upper_bound(0).value.get_node()));
            backend_->call("isOverZero", 1, "en", "b", &val, &Ret);
            isNG = isNG || Ret;
          }
        }
#ifdef _DEBUG_REDUCE_UNSUIT
        if(isNG)
        {
          std::cout<<"NG range"<<std::endl;
        }
        else
        {
          std::cout<<"OK"<<std::endl;
        }
#endif
      }
#ifdef _DEBUG_REDUCE_UNSUIT
      else
      {
        std::cout << p_it->first << "\t: " << p_it->second << std::endl;
      }
#endif
    }
    if(!isNG){
      eps_time_result.push_back(candidate);
    }
  }
#ifdef _DEBUG_REDUCE_UNSUIT
  std::cout << "######### remove NG Case & all "<< time_result.size() << " -> " << eps_time_result.size() << std::endl;
#endif
  if(time_result.size() != eps_time_result.size())
  {
    time_result.clear();
    for(time_it=0;time_it < eps_time_result.size();time_it++)
    {
      NextPhaseResult &eps_candidate = eps_time_result[time_it];
      time_result.push_back(eps_candidate);
#ifdef _DEBUG_REDUCE_UNSUIT
      std::cout << "NewCase\t: " << (time_it + 1) << std::endl;
      bool isNG = false;
      for(parameter_map_t::iterator p_it = eps_candidate.parameter_map.begin(); p_it != eps_candidate.parameter_map.end(); p_it++)
      {
        std::string parameter_name = p_it->first.get_name();
        int parameter_differential_count = p_it->first.get_differential_count();
        if(parameter_name=="eps" && parameter_differential_count==0)
        {
          std::cout << p_it->first << "\t: " << p_it->second << "\t->\t";
          symbolic_expression::node_sptr val;
          bool Ret;
          if(p_it->second.unique())
          {
            val = p_it->second.get_unique().get_node();
            val = symbolic_expression::node_sptr(new Times(val, p_it->second.get_unique().get_node()));
            backend_->call("isOverZero", 1, "en", "b", &val, &Ret);
            isNG = isNG || Ret;
          }
          else
          {
            if(p_it->second.get_lower_cnt())
            {
              val = p_it->second.get_lower_bound(0).value.get_node();
              backend_->call("isOverZero", 1, "en", "b", &val, &Ret);
              isNG = isNG || Ret;
            }
            if(p_it->second.get_upper_cnt())
            {
              val = symbolic_expression::node_sptr(new Number("-1"));
              val = symbolic_expression::node_sptr(new Times(val, p_it->second.get_upper_bound(0).value.get_node()));
              backend_->call("isOverZero", 1, "en", "b", &val, &Ret);
              isNG = isNG || Ret;
            }
          }
          if(isNG)
          {
            std::cout<<"NG range"<<std::endl;
          }
          else
          {
            std::cout<<"OK"<<std::endl;
          }
        }
      }
#endif
    }
  }
#ifdef _DEBUG_REDUCE_UNSUIT
  std::cout << "Remove UnSuitable Cases End;" << std::endl;
#endif
  //return time_result;
  return eps_time_result;
}

pp_time_result_t hydla::simulator::zero_case_expansion(pp_time_result_t time_result, Backend* backend_, phase_result_sptr_t& phase)
{
#ifdef _DEBUG_ZERO_CASE
  std::cout << "Zero Case Expand Start;" << std::endl;
  if(phase->phase_type==0)
    std::cout << "PointPhase " << phase->id << std::endl;
  else
    std::cout << "IntervalPhase " << phase->id << std::endl;
#endif
  unsigned int time_it;
  symbolic_expression::node_sptr max = symbolic_expression::node_sptr(new Number("0"));
  symbolic_expression::node_sptr min = symbolic_expression::node_sptr(new Number("0"));
  for(time_it=0;time_it < time_result.size();time_it++)
    {
#ifdef _DEBUG_ZERO_CASE
      std::cout << "Case \t: " << (time_it + 1) << std::endl;
#endif
      NextPhaseResult &candidate = time_result[time_it];
      for(parameter_map_t::iterator p_it = candidate.parameter_map.begin(); p_it != candidate.parameter_map.end(); p_it++)
        {
          std::string parameter_name = p_it->first.get_name();
          int parameter_differential_count = p_it->first.get_differential_count();
          if(parameter_name=="eps" && parameter_differential_count==0)
            {
#ifdef _DEBUG_ZERO_CASE
              std::cout << p_it->first << "\t: " << p_it->second << std::endl;
#endif
              symbolic_expression::node_sptr val;
              bool Ret;
              if(!p_it->second.unique())
                {
                  if(p_it->second.get_lower_cnt())
                    {
                      val = symbolic_expression::node_sptr(new Number("-1"));
                      val = symbolic_expression::node_sptr(new Times(val, p_it->second.get_lower_bound(0).value.get_node()));
                      val = symbolic_expression::node_sptr(new Plus(val, min));
                      //min - val;
                      backend_->call("isOverZero", 1, "en", "b", &val, &Ret);
                      if(Ret) min = p_it->second.get_lower_bound(0).value.get_node();
                    }
                  if(p_it->second.get_upper_cnt())
                    {
                      val = symbolic_expression::node_sptr(new Number("-1"));
                      val = symbolic_expression::node_sptr(new Times(val, max));
                      val = symbolic_expression::node_sptr(new Plus(val, p_it->second.get_upper_bound(0).value.get_node()));
                      //val - max;
                      backend_->call("isOverZero", 1, "en", "b", &val, &Ret);
                      if(Ret) max = p_it->second.get_upper_bound(0).value.get_node();
                    }
                }
            }
        }
    }
#ifdef _DEBUG_ZERO_CASE
  std::cout << "######### find parameter eps \nmax \t: "<< *max << "\nmin \t: " << *min << std::endl;
#endif

  for(time_it=0;time_it < time_result.size();time_it++)
    {
      NextPhaseResult &candidate = time_result[time_it];
      for(parameter_map_t::iterator p_it = candidate.parameter_map.begin(); p_it != candidate.parameter_map.end(); p_it++)
        {
          std::string parameter_name = p_it->first.get_name();
          int parameter_differential_count = p_it->first.get_differential_count();
          if(parameter_name=="eps" && parameter_differential_count==0)
            {
#ifdef _DEBUG_ZERO_CASE
              std::cout << p_it->first << "\t: " << p_it->second << "\t->\t";
#endif
              symbolic_expression::node_sptr val;
              bool Ret;
              if(p_it->second.unique())
                {
                  range_t range;

                  val = symbolic_expression::node_sptr(new Times(min, min));
                  backend_->call("isOverZero", 1, "en", "b", &val, &Ret);
                  if(Ret) range.set_lower_bound(min, false);

                  val = symbolic_expression::node_sptr(new Times(max, max));
                  backend_->call("isOverZero", 1, "en", "b", &val, &Ret);
                  if(Ret) range.set_upper_bound(max, false);

                  p_it->second = range;
                }

#ifdef _DEBUG_ZERO_CASE
              std::cout << p_it->second << std::endl;
#endif
            }
        }
    }
#ifdef _DEBUG_ZERO_CASE
  std::cout << "Zero Case Expand End;" << std::endl;
#endif
  return time_result;
}
