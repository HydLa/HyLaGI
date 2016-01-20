#include <float.h>
#include <queue>
#include <utility>
#include "IntervalNewton.h"
#include "IntervalTreeVisitor.h"
#include "Logger.h"
#include "HydLaError.h"

namespace hydla
{
namespace interval
{


const double INVALID_MID = -1;

// 「区間が等しい」を定義
bool itvd_equal(itvd x, itvd y)
{
  if(x.lower() == y.lower() && x.upper() == y.upper())
  {
    return true;
  }
  else
  {
    return false;
  }
}

// 二つの区間の共通部分を返す関数
// kvライブラリ内の関数 intersect では無理矢理共通部分を作っているが、今回の目的に合わない
// したがって、共通部分が無い場合は区間 [0,0] を返すようにした
itvd intersect_interval(itvd x, itvd y)
{
  double min, max;

  if(y.upper() < x.lower() || x.upper() < y.lower())
    return itvd(0.,0.);
  
  min = fmax(x.lower(), y.lower());
  max = fmin(x.upper(), y.upper());

  // たまにひっくり返るので、その対策
  if(min < max)
    return itvd(min, max);
  else
    return itvd(max, min);
}

// 存在証明
bool show_existence(itvd candidate, node_sptr exp, node_sptr dexp, parameter_map_t& phase_map_)
{
  itvd tmp;

  if(candidate == candidate.lower() && candidate == candidate.upper() &&
     candidate != itvd(0.,0.))
  {
    return true;
  }
  

  double max_width = width(candidate) * 2;
  double min_width = 0;
  while(true)
  {
    double w = (max_width + min_width)/ 2;
    if(max_width <= min_width || w == max_width || w == min_width) throw HYDLA_ERROR("show existence failed for exp:" + get_infix_string(exp) + " dexp: " + get_infix_string(dexp) + ". "); 
    tmp.lower() = candidate.lower() - w / 2.;
    tmp.upper() = candidate.upper() + w / 2.;
    std::cerr.precision(17);
    HYDLA_LOGGER_DEBUG_VAR(tmp);
  
    itvd n_x, div;
    bool parted;
    IntervalTreeVisitor visitor;
    itvd time_interval = itvd(mid(tmp));

    itvd f_result = visitor.get_interval_value(exp, &time_interval, &phase_map_);
    itvd d_result = visitor.get_interval_value(dexp, &tmp, &phase_map_);

    HYDLA_LOGGER_DEBUG_VAR(f_result);
    HYDLA_LOGGER_DEBUG_VAR(d_result);
  
    div = division_part1(f_result, d_result, parted);

    HYDLA_LOGGER_DEBUG("div : ", div);


    if(parted || in(0.,d_result))
    {
      max_width = (max_width + min_width) / 2;
      HYDLA_LOGGER_DEBUG("partition occured.");
      continue;
    }
  
    n_x = mid(tmp) - div;

    HYDLA_LOGGER_DEBUG("n_x : ", n_x);
  
    if(proper_subset(n_x, tmp))
    {
      return true;
    }
    else
    {
      HYDLA_LOGGER_DEBUG("proper_subset failed");
      min_width = (max_width + min_width) / 2;
      continue;
    }
  }
}

double calculate_mid_without_0(itvd current_interval, node_sptr exp, parameter_map_t& phase_map_)
{
  using namespace std;
  itvd f_result;
  queue<pair<double, double> > rate_queue;
  IntervalTreeVisitor visitor;
  rate_queue.push(make_pair(0, 1));

  const int STEP = 2;
  const double STOP_THRESHOLD = 0.01;
  // BFS  
  while(!rate_queue.empty())
  {
    double rate_inf = rate_queue.front().first;
    double rate_sup = rate_queue.front().second;
    HYDLA_LOGGER_DEBUG_VAR(rate_inf);
    HYDLA_LOGGER_DEBUG_VAR(rate_sup);
    rate_queue.pop();
    double rate =  (rate_inf + rate_sup)/2;
    double m = current_interval.lower() * (1-rate) + current_interval.upper() * rate;
    itvd m_itv = itvd(m);
    itvd f_result = visitor.get_interval_value(exp, &m_itv, &phase_map_);
    HYDLA_LOGGER_DEBUG_VAR(f_result);
    if(!in(0.,f_result))
    {
      return m;
    }else
    {
      if((rate_sup - rate_inf) / STEP < STOP_THRESHOLD)continue;
      double child_rate_inf = rate_inf;
      for(int i = 0; i < STEP; i++)
      {
        double child_rate_sup = rate_inf + (rate_sup - rate_inf) * (i+1) / STEP;
        rate_queue.push(make_pair(child_rate_inf, child_rate_sup));
        child_rate_inf = child_rate_sup;
      }
    }
  }
  return INVALID_MID;
}

itvd calculate_interval_newton(itvd init, node_sptr exp, node_sptr dexp, parameter_map_t& phase_map_)
{
  itv_stack_t candidate_stack;
  candidate_stack.push(init);
  return calculate_interval_newton(candidate_stack, exp, dexp, phase_map_);
}



itvd calculate_interval_newton(itv_stack_t &candidate_stack, node_sptr exp, node_sptr dexp, parameter_map_t& phase_map_)
{
  bool parted;
  itvd current_interval, prev_interval, div1, div2, nx, nx2, x2, f_result, d_result,m;
  itvd result_interval;

  std::cout.precision(17);
  std::cerr.precision(17);

  while(!candidate_stack.empty())
  {
    current_interval = candidate_stack.top();
    candidate_stack.pop();

    HYDLA_LOGGER_DEBUG("pop new interval: ", current_interval);
    for(int i=0;i<100;i++)
    {
      if(current_interval == itvd(0.,0.))
      {
        HYDLA_LOGGER_DEBUG("Wrong Interval");
        break;
      }
      prev_interval = current_interval;
      HYDLA_LOGGER_DEBUG_VAR(current_interval);
      IntervalTreeVisitor visitor;
      d_result = visitor.get_interval_value(dexp, &current_interval, &phase_map_);
      if(in(0., d_result))
      {
        double mid_val = calculate_mid_without_0(current_interval, exp, phase_map_);
        if(mid_val == INVALID_MID)
        {
          throw HYDLA_ERROR("No valid intermediate values are found in interval newton method.");
        }
        m = itvd(mid_val);
      }
      else
      {
        m = itvd(mid(current_interval));
      }
      f_result = visitor.get_interval_value(exp, &m, &phase_map_);

      HYDLA_LOGGER_DEBUG_VAR(f_result);
      HYDLA_LOGGER_DEBUG_VAR(d_result);

      nx = m - division_part1(f_result, d_result, parted);
      HYDLA_LOGGER_DEBUG_VAR(nx);
      HYDLA_LOGGER_DEBUG_VAR(prev_interval);
      current_interval = intersect_interval(prev_interval, nx);
      
      if(parted)
      {
        if(current_interval != itvd(0., 0.))
        {
          HYDLA_LOGGER_DEBUG("push candidate: ", current_interval);
          candidate_stack.push(current_interval);
        }
        nx2 = m - division_part2(f_result, d_result);
        HYDLA_LOGGER_DEBUG_VAR(nx2);
        current_interval = intersect_interval(prev_interval, nx2);
      }

      // stopping criteria
      if(itvd_equal(prev_interval, current_interval))
      {
        HYDLA_LOGGER_DEBUG_VAR(current_interval);
        HYDLA_LOGGER_DEBUG("Stopped at step ", i+1);
        break;
      }
    }
    if(!in(0., current_interval) && show_existence(current_interval, exp, dexp, phase_map_))
    {
      HYDLA_LOGGER_DEBUG("FIND");
      HYDLA_LOGGER_DEBUG_VAR(width(current_interval));
      HYDLA_LOGGER_DEBUG_VAR(current_interval);
      return current_interval;;
    }
  }
  return INVALID_ITV;
}


std::list<itvd> calculate_interval_newton_nd(itvd init, node_sptr exp, node_sptr dexp, parameter_map_t& phase_map_)
{
  bool parted;
  itvd current_interval, prev_interval, div1, div2, nx, nx2, x2, f_result, d_result,m;
  itv_stack_t candidate_stack;
  std::list<itvd> result_intervals;

  std::cout.precision(17);
  std::cerr.precision(17);

  candidate_stack.push(init);

  while(!candidate_stack.empty())
  {
    current_interval = candidate_stack.top();
    candidate_stack.pop();

    HYDLA_LOGGER_DEBUG("pop new interval: ", current_interval);
    for(int i=0;i<100;i++)
    {
      if(current_interval == itvd(0.,0.))
      {
        HYDLA_LOGGER_DEBUG("Wrong Interval");
        break;
      }
      prev_interval = current_interval;
      HYDLA_LOGGER_DEBUG_VAR(current_interval);
      IntervalTreeVisitor visitor;
      d_result = visitor.get_interval_value(dexp, &current_interval, &phase_map_);
      if(in(0., d_result))
      {
        double mid_val = calculate_mid_without_0(current_interval, exp, phase_map_);
        if(mid_val == INVALID_MID)
        {
          throw HYDLA_ERROR("No valid intermediate values are found in interval newton method.");
        }
        m = itvd(mid_val);
      }
      else
      {
        m = itvd(mid(current_interval));
      }
      f_result = visitor.get_interval_value(exp, &m, &phase_map_);

      HYDLA_LOGGER_DEBUG_VAR(f_result);
      HYDLA_LOGGER_DEBUG_VAR(d_result);
      

      nx = m - division_part1(f_result, d_result, parted);
      HYDLA_LOGGER_DEBUG_VAR(nx);
      HYDLA_LOGGER_DEBUG_VAR(prev_interval);
      current_interval = intersect_interval(prev_interval, nx);
      
      if(parted)
      {
        if(current_interval != itvd(0., 0.))
        {
          HYDLA_LOGGER_DEBUG("push candidate: ", current_interval);
          candidate_stack.push(current_interval);
        }
        nx2 = m - division_part2(f_result, d_result);
        HYDLA_LOGGER_DEBUG_VAR(nx2);
        current_interval = intersect_interval(prev_interval, nx2);
      }

      // stopping criteria
      if(itvd_equal(prev_interval, current_interval))
      {
        HYDLA_LOGGER_DEBUG_VAR(current_interval);
        HYDLA_LOGGER_DEBUG("Stopped at step ", i+1);
        break;
      }
    }
    if(!in(0., current_interval) && show_existence(current_interval, exp, dexp, phase_map_))
    {
      HYDLA_LOGGER_DEBUG("FIND");
      HYDLA_LOGGER_DEBUG_VAR(width(current_interval));
      HYDLA_LOGGER_DEBUG_VAR(current_interval);
      result_intervals.push_back(current_interval);
    }
  }

  return result_intervals;
}



}// namespcae interval
}// namespace hydla
