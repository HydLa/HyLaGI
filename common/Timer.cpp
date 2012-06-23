#include "Timer.h"
#include <iostream>


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
    
    std::vector<elapsed_time_t> Timer::pp_simulation_time_;
    std::vector<elapsed_time_t> Timer::pp_calculate_closure_time_;
    std::vector<elapsed_time_t> Timer::ip_simulation_time_;
    std::vector<elapsed_time_t> Timer::ip_calculate_closure_time_;
    
    Timer::Timer(){
      restart();
    }
    
    Timer::~Timer(){}

    void Timer::restart() {
      gettimeofday(&start_point_,NULL);
    }
    /*
    void Timer::push_next_case_time(){
      CaseTime newct;
      ct_.push_back(newct);
    }
    */
    void Timer::push_time(std::string p) {
      time_point_t temp;
      gettimeofday(&temp,NULL);
      elapsed_time_t t;
      t = (temp.tv_sec - start_point_.tv_sec) + (temp.tv_usec - start_point_.tv_usec)*0.000001;

      if(p == "PointPhase"){
	      pp_simulation_time_.push_back(t);
	      return;
      }
      
      if(p == "PP-CalculateClosure"){
	      pp_calculate_closure_time_.push_back(t);
	      return;
      }

      if(p == "IntervalPhase"){
	      ip_simulation_time_.push_back(t);
	      return;
      }
      
      if(p == "IP-CalculateClosure"){
	      ip_calculate_closure_time_.push_back(t);
      }

    }
    
    void Timer::elapsed(){
      time_point_t temp;
      gettimeofday(&temp,NULL);
      std::cout << (temp.tv_sec - start_point_.tv_sec) + (temp.tv_usec - start_point_.tv_usec)*0.000001 << " [sec]" << std::endl;
    }

    void Timer::output_time(){
      std::cout << "#---------Simulation Time---------" << std::endl << std::endl;
      std::vector<elapsed_time_t>::iterator pp_it = pp_simulation_time_.begin();
      std::vector<elapsed_time_t>::iterator pp_end = pp_simulation_time_.end();
      std::vector<elapsed_time_t>::iterator ip_it = ip_simulation_time_.begin();
      std::vector<elapsed_time_t>::iterator ip_end = ip_simulation_time_.end();
      std::vector<elapsed_time_t>::iterator pp_cc_it = pp_calculate_closure_time_.begin();
      std::vector<elapsed_time_t>::iterator pp_cc_end = pp_calculate_closure_time_.end();
      std::vector<elapsed_time_t>::iterator ip_cc_it = ip_calculate_closure_time_.begin();
      std::vector<elapsed_time_t>::iterator ip_cc_end = ip_calculate_closure_time_.end();

      for(int i = 0;; i++){

	      std::cout << "#---------" << i+1 << "---------" << std::endl;
	      std::cout << "---------PP---------" << std::endl;
	      if(ip_cc_it != ip_cc_end){
	        std::cout << "Calculate Closure Time : " << *pp_cc_it << " [sec]" << std::endl;
	        pp_cc_it++;
	      }
	      if(pp_it != pp_end){
	        std::cout << "Point Phase Time       : " << *pp_it << " [sec]" << std::endl << std::endl;
	        pp_it++;
	      }
	      std::cout << "---------IP---------" << std::endl;
	      if(ip_cc_it != ip_cc_end){
	        std::cout << "Calculate Closure Time : " << *ip_cc_it << " [sec]" << std::endl;
	        ip_cc_it++;
	      }
	      if(ip_it != ip_end){
	        std::cout << "Interval Phase Time    : " << *ip_it << " [sec]" << std::endl << std::endl;
	        ip_it++;
	      }

	      if(pp_cc_it == pp_cc_end && pp_it == pp_end && ip_cc_it == ip_cc_end && ip_it == ip_end){
	        break;
	      }

      }
    }

  }
}


