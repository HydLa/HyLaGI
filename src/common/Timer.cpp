#include "Timer.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace hydla{
namespace timer{

using namespace std;

Timer::Timer(){
  restart();
}
    
Timer::~Timer(){}

void Timer::restart() {
  start_point_ = chrono::steady_clock::now();
}

string Timer::get_time_string() const
{
  double time_double;
  time_double = get_elapsed_us() / 1000000.0;
  stringstream sstr;
  sstr << fixed;
  sstr << setprecision(6) << time_double << " s" ;
  return sstr.str();
}

chrono::nanoseconds Timer::get_time() const{
  auto end_point = chrono::steady_clock::now();
  return chrono::duration_cast<chrono::nanoseconds>(end_point-start_point_);
}

unsigned int Timer::get_elapsed_h() const{
  return chrono::duration_cast<chrono::hours>(get_time()).count();
}
unsigned int Timer::get_elapsed_m() const{
  return chrono::duration_cast<chrono::minutes>(get_time()).count();
}
unsigned int Timer::get_elapsed_s() const{
  return chrono::duration_cast<chrono::seconds>(get_time()).count();
}
unsigned int Timer::get_elapsed_ms() const{
  return chrono::duration_cast<chrono::milliseconds>(get_time()).count();
}
unsigned int Timer::get_elapsed_us() const{
  return chrono::duration_cast<chrono::microseconds>(get_time()).count();
}
unsigned int Timer::get_elapsed_ns() const{
  return get_time().count();
}

}
}


