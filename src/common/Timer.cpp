#include "Timer.h"
#include <iostream>
#include <sstream>

namespace hydla{
  namespace timer{

    Timer::Timer(){
      restart();
    }
    
    Timer::~Timer(){}

    void Timer::restart() {
      start_point_ = std::chrono::steady_clock::now();
    }

    std::chrono::nanoseconds Timer::get_time(){
      auto end_point = std::chrono::steady_clock::now();
      return std::chrono::duration_cast<std::chrono::nanoseconds>(end_point-start_point_);
    }

    unsigned int Timer::get_elapsed_h(){
      return std::chrono::duration_cast<std::chrono::hours>(get_time()).count();
    }
    unsigned int Timer::get_elapsed_m(){
      return std::chrono::duration_cast<std::chrono::minutes>(get_time()).count();
    }
    unsigned int Timer::get_elapsed_s(){
      return std::chrono::duration_cast<std::chrono::seconds>(get_time()).count();
    }
    unsigned int Timer::get_elapsed_ms(){
      return std::chrono::duration_cast<std::chrono::milliseconds>(get_time()).count();
    }
    unsigned int Timer::get_elapsed_us(){
      return std::chrono::duration_cast<std::chrono::microseconds>(get_time()).count();
    }
    unsigned int Timer::get_elapsed_ns(){
      return get_time().count();
    }

  }
}


