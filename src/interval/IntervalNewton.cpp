#include "IntervalNewton.h"
#include "IntervalTreeVisitor.h"

namespace hydla
{
namespace interval
{

void debug_print(std::string str, itvd x)
{
  std::cout << str << x << "\n";
}


// 「区間が等しい」を定義
bool itvd_eqal(itvd x, itvd y)
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
    
  tmp.lower() = candidate.lower() - 1.0 * pow(10, (int)log10(width(candidate)) - 1);            
  tmp.upper() = candidate.upper() + 1.0 * pow(10, (int)log10(width(candidate)) - 1);

  std::cout << "\n" << "width : " << width(candidate) << "\n";
  std::cout << "log(width) : " << log10(width(candidate)) << "\n";
  std::cout << "(int)log(width) : " << (int)log10(width(candidate)) << "\n";

  debug_print("tmp : ", tmp);
  
  itvd n_x, div;
  bool parted;
  IntervalTreeVisitor visitor;
  itvd time_interval = itvd(mid(tmp));


  itvd f_result = visitor.get_interval_value(exp, &time_interval, &phase_map_);
  itvd d_result = visitor.get_interval_value(dexp, &tmp, &phase_map_);
  
  div = division_part1(f_result, d_result, parted);

  debug_print("div : ", div);
  
  if(parted)
  {
    std::cout << "Parted False.\n\n";
    return false;
  }
  
  n_x = mid(tmp) - div;

  debug_print("n_x : ", n_x);
  
  if(proper_subset(n_x, tmp))
  {
    return true;
  }
  else
  {
    std::cout << "Not Subset.\n\n";
    return false;
  }
}

// 区間ニュートン法により近似解を求める関数
// 0以上の最小の解を求める
itvd calculate_interval_newton(itvd init, node_sptr exp, node_sptr dexp, parameter_map_t& phase_map_)
{
  bool parted;
  itvd initial_interval, current_value, prev_value, div1, div2, tmp1, tmp2, x1, x2;
  itvs *candidate_stack;

  // 暫定的な初期区間
  initial_interval = init;

  candidate_stack = new itvs();
  candidate_stack->push(initial_interval);
  
  while(!candidate_stack->empty())
  {
    current_value = candidate_stack->top();
    candidate_stack->pop();
    
    for(int i=0;i<100;i++)
    {
      itvd f_result;
      // if(i==0)
      // {
      //   IntervalTreeVisitor f_visitor(itvd(current_value.lower() + 0.1), phase_map_);
      //   f_result = f_visitor.get_interval_value(exp);
      // }
      // else 
      // {
      //   IntervalTreeVisitor f_visitor = IntervalTreeVisitor(itvd(mid(current_value)), phase_map_);
      //   f_result = f_visitor.get_interval_value(exp);
      // }

      std::cout.precision(17);
      
      itvd m = itvd(mid(current_value));

      IntervalTreeVisitor visitor;

      itvd time_interval = itvd(mid(current_value));
      itvd f_result = visitor.get_interval_value(exp, &time_interval, &phase_map_);
      itvd d_result = visitor.get_interval_value(dexp, &current_value, &phase_map_);

      std::cout << "f_result : " << f_result << "\n";
      std::cout << "d_result : " << d_result << "\n";
      
      if(in(0.,f_result) && in(0.,d_result))
      {
        time_interval = time_interval + 1./4.*width(current_value);
        f_result = visitor.get_interval_value(exp, &time_interval, &phase_map_);
        std::cout << "f_result2 : " << f_result << "\n";
      }
      
      prev_value = current_value;
      debug_print("prev_value : ", prev_value);
      div1 = division_part1(f_result, d_result, parted);
      debug_print("div1 : ", div1);
      tmp1 = time_interval - div1;
      debug_print("tmp1 : ", tmp1);
      x1 = intersect_interval(current_value, tmp1);
      debug_print("x1 : ", x1);
      if(parted)
      {
        div2 = division_part2(f_result, d_result);
        debug_print("div2 : ", div2);
        tmp2 = time_interval - div2;
        debug_print("tmp2 : ", tmp2);
        x2 = intersect_interval(current_value, tmp2);
        debug_print("x2 : ", x2);
        
        if(!(x1 == itvd(0.,0.)))
        {
          candidate_stack->push(x1);
          std::cout << "Push candidate!\n";
        }
        
        current_value = x2;
      }
      else
      {
        current_value = x1;
      }
    
      if(current_value == itvd(0.,0.))
      {
        std::cout << "Wrong Interval!\n";
        break;
      }

      // stopping criteria
      if(itvd_eqal(prev_value, current_value))
      {
        printf("Criteria prev == present.\n");
        printf("Stopping at %d times\n", i+1);
        break;
      }
    } // for
    if(current_value == itvd(0.,0.) || in(0., current_value))
    {
      continue;
    }
    
    if(show_existence(current_value, exp, dexp, phase_map_))
    {
      std::cout << "FIND!\n";
      printf("Width of answer : %lf\n", width(current_value));
      return current_value;
    }
    
  } // while

  // 解探索失敗
  return itvd(0.,0.);
}

}// namespcae interval
}// namespace hydla
