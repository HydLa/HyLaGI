#include <iostream>

#include "Backend.h"
#include "EpsilonMode.h"

using namespace hydla::symbolic_expression;
using namespace hydla::backend;
// #define _DEBUG_CUT_HIGH_ORDER
// #define _DEBUG_REDUCE_UNSUIT

variable_map_t hydla::simulator::cut_high_order_epsilon(Backend* backend_, phase_result_sptr_t& phase)
{
  variable_map_t vm_ret;
  bool have_eps = false;
#ifdef _DEBUG_CUT_HIGH_ORDER
  std::cout << "Cut High Order Epsilon Start;" << std::endl;
  if(phase->phase_type==0)
    std::cout << "POINT_PHASE " << phase->id << std::endl;
  if(phase->phase_type==1)
    std::cout << "INTERVAL_PHASE " << phase->id << std::endl;
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
      int differential_times = 1; // のこす次数
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
          simulator::value_t val = v_it->second.get_unique_value();
          range_t& range = vm_ret[v_it->first];
#ifdef _DEBUG_CUT_HIGH_ORDER
          std::cout << v_it->first << "\t: (" << val;
#endif
          backend_->call("cutHighOrderVariable", 3, "vlnpi", "vl", &val, &p_it->first, &differential_times, &ret);
#ifdef _DEBUG_CUT_HIGH_ORDER
          std::cout << " -> " << ret << ")" << std::endl;
#endif
          range.set_unique_value(ret);
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

hydla::simulator::pp_time_result_t hydla::simulator::reduce_unsuitable_case(pp_time_result_t time_result, Backend* backend_, phase_result_sptr_t& phase)
{
#ifdef _DEBUG_REDUCE_UNSUIT
  std::cout << "Remove UnSuitable Cases Start;" << std::endl;
  if(phase->phase_type==0)
    std::cout << "POINT_PHASE " << phase->id << std::endl;
  else
    std::cout << "INTERVAL_PHASE " << phase->id << std::endl;
  std::cout << "Next Phase Case Count\t: " << time_result.size() << std::endl;
#endif

  pp_time_result_t eps_time_result;
/* TODO: implement
  unsigned int time_it;
  for(time_it=0;time_it < time_result.size();time_it++)
  {
#ifdef _DEBUG_REDUCE_UNSUIT
    std::cout << "Case \t: " << (time_it + 1) << std::endl;
#endif
    DCCandidate &candidate = time_result[time_it];
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
          val = p_it->second.get_unique_value().get_node();
          val = symbolic_expression::node_sptr(new Times(val, p_it->second.get_unique_value().get_node()));
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
      DCCandidate &eps_candidate = eps_time_result[time_it];
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
            val = p_it->second.get_unique_value().get_node();
            val = symbolic_expression::node_sptr(new Times(val, p_it->second.get_unique_value().get_node()));
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
*/
  return time_result;
}
