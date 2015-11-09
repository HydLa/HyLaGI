#include <float.h>
#include "IntervalNewton.h"
#include "IntervalTreeVisitor.h"
#include "Logger.h"

namespace hydla
{
namespace interval
{


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
  tmp.lower() = candidate.lower() - width(candidate) / 2.;
  tmp.upper() = candidate.upper() + width(candidate) / 2.;
  // tmp.lower() = nextafter(candidate.lower(), -100.0);
  // tmp.upper() = nextafter(candidate.upper(), 100.0);

  if(candidate == candidate.lower() && candidate == candidate.upper() &&
     candidate != itvd(0.,0.))
  {
    return true;
  }
  
  double w = width(candidate);
  while(true)
  {
    std::cout.precision(17);
    HYDLA_LOGGER_DEBUG_VAR(width(candidate));
    HYDLA_LOGGER_DEBUG_VAR(log10(width(candidate)));
    HYDLA_LOGGER_DEBUG_VAR((int)log10(width(candidate)));
    HYDLA_LOGGER_DEBUG_VAR(candidate);
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
    HYDLA_LOGGER_DEBUG("parted: ", parted);

    if(parted || in(0.,d_result))
    {
      if(w == w /2.) return false;
      w = w / 2.;
      tmp.lower() = candidate.lower() - w / 2.;
      tmp.upper() = candidate.upper() + w / 2.;
      HYDLA_LOGGER_DEBUG_VAR(tmp);
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
      HYDLA_LOGGER_DEBUG("Not Subset.");
      if(w == w /2.) return false;
      w = w / 2. * 3.;
      tmp.lower() = candidate.lower() - w / 2.;
      tmp.upper() = candidate.upper() + w / 2.;
    }
  }
}

// 区間ニュートン法により近似解を求める関数
// 0以上の最小の解を求める
itvd calculate_interval_newton(itvd init, node_sptr exp, node_sptr dexp, parameter_map_t& phase_map_)
{
  bool parted;
  itvd initial_interval, current_value, prev_value, div1, div2, tmp1, tmp2, x1, x2;
  itvs candidate_stack;

  // 暫定的な初期区間
  initial_interval = init;

  candidate_stack.push(initial_interval);
  
  while(!candidate_stack.empty())
  {
    current_value = candidate_stack.top();
    candidate_stack.pop();
    HYDLA_LOGGER_DEBUG("pop candidate: ", current_value);
    
    for(int i=0;i<100;i++)
    {
      std::cout.precision(17);
      
      IntervalTreeVisitor visitor;

      itvd time_interval = itvd(mid(current_value));
      itvd f_result = visitor.get_interval_value(exp, &time_interval, &phase_map_);
      itvd d_result = visitor.get_interval_value(dexp, &current_value, &phase_map_);

      HYDLA_LOGGER_DEBUG_VAR(f_result);
      HYDLA_LOGGER_DEBUG_VAR(d_result);
      
      if(in(0.,f_result) && in(0.,d_result))
      {
        time_interval = time_interval + 1./4.*width(current_value);
        f_result = visitor.get_interval_value(exp, &time_interval, &phase_map_);
        HYDLA_LOGGER_DEBUG_VAR(f_result);
      }
      
      prev_value = current_value;
      HYDLA_LOGGER_DEBUG("prev_value : ", prev_value);
      HYDLA_LOGGER_DEBUG_VAR(time_interval);
      div1 = division_part1(f_result, d_result, parted);
      HYDLA_LOGGER_DEBUG("div1 : ", div1);
      tmp1 = time_interval - div1;
      HYDLA_LOGGER_DEBUG("tmp1 : ", tmp1);
      x1 = intersect_interval(current_value, tmp1);
      HYDLA_LOGGER_DEBUG("x1 : ", x1);
      if(parted)
      {
        div2 = division_part2(f_result, d_result);
        HYDLA_LOGGER_DEBUG("div2 : ", div2);
        tmp2 = time_interval - div2;
        HYDLA_LOGGER_DEBUG("tmp2 : ", tmp2);
        x2 = intersect_interval(current_value, tmp2);
        HYDLA_LOGGER_DEBUG("x2 : ", x2);
        
        if(!(x1 == itvd(0.,0.)))
        {
          candidate_stack.push(x1);
          HYDLA_LOGGER_DEBUG("Push candidate :", x1);
        }
        
        current_value = x2;
      }
      else
      {
        current_value = x1;
      }
    
      if(current_value == itvd(0.,0.))
      {
        HYDLA_LOGGER_DEBUG("Wrong Interval");
        break;
      }

      // stopping criteria
      if(itvd_equal(prev_value, current_value))
      {
        HYDLA_LOGGER_DEBUG("Stopped at step ", i+1);
        break;
      }
    } // for
    if(current_value == itvd(0.,0.) || in(0., current_value))
    {
      continue;
    }
    
    if(show_existence(current_value, exp, dexp, phase_map_))
    {
      HYDLA_LOGGER_DEBUG("Width of answer : ", width(current_value));
      HYDLA_LOGGER_DEBUG("answer: ", current_value);
      return current_value;
    }
    
  } // while
  HYDLA_LOGGER_DEBUG("There is no answer");
  // 解探索失敗
  return itvd(0.,0.);
}


std::list<itvd> calculate_interval_newton_nd(itvd init, node_sptr exp, node_sptr dexp, parameter_map_t& phase_map_)
{
  int count_retry;
  bool parted, change_m;
  itvd current_interval, prev_interval, div1, div2, nx, nx2, x2, f_result, d_result,m;
  itvs candidate_stack;
  std::list<itvd> result_intervals;

  std::cout.precision(17);

  candidate_stack.push(init);
  
  while(!candidate_stack.empty())
  {
    current_interval = candidate_stack.top();
    candidate_stack.pop();

    HYDLA_LOGGER_DEBUG("pop new interval: ", current_interval);
    count_retry = 0;
    change_m = false;
    for(int i=0;i<100;i++)
    {
      if(current_interval == itvd(0.,0.))
      {
        HYDLA_LOGGER_DEBUG("Wrong Interval");
        break;
      }
      prev_interval = current_interval;
      IntervalTreeVisitor visitor;
      if(!change_m)
      {
        m = itvd(mid(current_interval));
        f_result = visitor.get_interval_value(exp, &m, &phase_map_);
        d_result = visitor.get_interval_value(dexp, &current_interval, &phase_map_);
        HYDLA_LOGGER_DEBUG_VAR(f_result);
        HYDLA_LOGGER_DEBUG_VAR(d_result);
      }
      
      if(in(0.,f_result) && in(0.,d_result) && count_retry < 100)
      {
        count_retry++;
        m = m + 1./pow(2.,(count_retry + 1)) * width(current_interval);
        f_result = visitor.get_interval_value(exp, &m, &phase_map_);
        HYDLA_LOGGER_DEBUG_VAR(m);
        HYDLA_LOGGER_DEBUG_VAR(f_result);
        i--;
        change_m = true;
        continue;
      }
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
      change_m = false;
      count_retry = 0;
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
