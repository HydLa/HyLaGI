#include "Timer.h"
#include <iostream>
#include <sstream>


#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) 

namespace hydla{
  namespace timer{

    struct timezone 
    {
      long  tz_minuteswest; /* minutes W of Greenwich */
      int  tz_dsttime;     /* type of dst correction */
    };

     
    int gettimeofday(struct timeval *tv, struct timezone *tz)
    {
      FILETIME ft;
      unsigned __int64 tmpres = 0;
      static int tzflag;
     
      if (NULL != tv)
      {
        GetSystemTimeAsFileTime(&ft);
     
        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;
     
        /*converting file time to unix epoch*/
        tmpres -= DELTA_EPOCH_IN_MICROSECS; 
        tmpres /= 10;  /*convert into microseconds*/
        tv->tv_sec = (long)(tmpres / 1000000UL);
        tv->tv_usec = (long)(tmpres % 1000000UL);
      }
     
      if (NULL != tz)
      {
        if (!tzflag)
        {
          _tzset();
          tzflag++;
        }
        
        _get_timezone(&tz->tz_minuteswest);
        tz->tz_minuteswest /= 60;
        _get_daylight(& tz->tz_dsttime);
      }
     
      return 0;
    }


  }  //  namespace timer
}  //  namespace hydla

#endif

namespace hydla{
  namespace timer{
    
    Timer::Timer(){
      restart();
    }
    
    Timer::~Timer(){}

    void Timer::restart() {
      gettimeofday(&start_point_,NULL);
    }
    
    void Timer::reset(){
      restart();
      elapsed_time_ = 0;
    }

    void Timer::count_time(){
      elapsed_time_ += get_time();
    }

    std::string Timer::get_time_string(){
      std::stringstream sstr;
      sstr << std::fixed;
      sstr << std::setprecision(TIMER_PLACES) << elapsed_time_;
      return sstr.str();
    }

    elapsed_time_t Timer::get_time(){
      timeval temp;
      gettimeofday(&temp,NULL);
      return ( (temp.tv_sec - start_point_.tv_sec) + (temp.tv_usec - start_point_.tv_usec)*0.000001 );
    }
    
    void Timer::elapsed(){
      std::cout << std::fixed;
      std::cout << std::setprecision(TIMER_PLACES) << get_time() << " s" << std::endl;
      std::cout << resetiosflags(std::ios_base::floatfield);
    }

    void Timer::elapsed(std::string str){
      std::cout << str << " : ";
      elapsed();
    }
    
    bool Timer::is_zero(){
      if(elapsed_time_ == 0) return true;
      else return false;
    }

  }
}


