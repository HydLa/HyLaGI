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
    

    phase_time_sptr_t Timer::phase_time_root_;
    std::stack< phase_time_sptr_t > Timer::phase_time_stack_;

    phase_time_sptr_t Timer::current_phase_time_;

    
    Timer::Timer(){
      restart();
    }
    
    Timer::~Timer(){}

    void Timer::restart() {
      gettimeofday(&start_point_,NULL);
    }

    void Timer::init_timer(){
      phase_time_sptr_t temp(new PhaseTime);
      phase_time_root_ = temp;
      current_phase_time_ = phase_time_root_;
      push_new_phase_time();
    }

    elapsed_time_t Timer::get_time(){
      time_point_t temp;
      gettimeofday(&temp,NULL);
      return ( (temp.tv_sec - start_point_.tv_sec) + (temp.tv_usec - start_point_.tv_usec)*0.000001 );
    }

    void Timer::is_point_phase(){
      current_phase_time_->point_phase = true;
    }

    void Timer::count_time(std::string p) {
      if(p == "Phase"){
	current_phase_time_->phase_time += get_time();
	return;
      }
      if(p == "CalculateClosure"){
	current_phase_time_->calculate_closure_time += get_time();
	return;
      }
    }
    
    void Timer::elapsed(){
      std::cout << get_time() << " [sec]" << std::endl;
    }

    void Timer::elapsed(std::string str){
      std::cout << str << " : ";
      elapsed();
    }

    void Timer::output_time(bool nd_mode){
      if(phase_time_root_->children.size() == 0){
	std::cout << "No Result." << std::endl;
	return;
      }
      phase_time_sptrs_t::iterator it = phase_time_root_->children.begin(), end = phase_time_root_->children.end();
      int i=1, j=1;
      std::cout << "#---------Simulation Time---------" << std::endl;
      for(;it!=end;it++){
	std::vector<std::string> result;
	output_phase_time_node(*it, result, i, j, nd_mode);
      }
    }

    void Timer::output_phase_time_node(phase_time_sptr_t &node, std::vector<std::string> &result, int &case_num, int &phase_num, bool nd_mode){

      if(node->children.size() == 0){
    
	if(nd_mode){
	  std::cout << "#---------Case " << case_num++ << "---------" << std::endl;
	}
	std::vector<std::string>::const_iterator r_it = result.begin(), r_end = result.end();
	for(;r_it!=r_end;r_it++){
	  std::cout << *r_it;
	}
      
	std::cout << get_phase_time_output(node);
	std::cout << std::endl;
      }else{
	if(node->point_phase){
	  std::stringstream sstr;
	  sstr << "#---------" << phase_num++ << "---------\n";
	  result.push_back(sstr.str());
	}
	result.push_back(get_phase_time_output(node));
	phase_time_sptrs_t::iterator it = node->children.begin(), end = node->children.end();
	for(;it!=end;it++){
	  output_phase_time_node(*it, result, case_num, phase_num, nd_mode);
	}
	result.pop_back();
	if(node->point_phase){
	  result.pop_back();
	  phase_num--;
	}
      }
    }

    std::string Timer::get_phase_time_output(phase_time_sptr_t &node){
      std::stringstream sstr;
      if(node->point_phase)
	sstr << "#-------PP------\n";
      else
	sstr << "#-------IP------\n";

      sstr << "Calclate Closure Time : " << node->calculate_closure_time << " [sec]" << std::endl;
      sstr << "Phase Time            : " << node->phase_time << " [sec]" << std::endl << std::endl;
      return sstr.str();
    }

    phase_time_sptr_t Timer::make_new_phase_time(){
      phase_time_sptr_t new_phase_time(new PhaseTime);
      new_phase_time->parent = current_phase_time_;
      new_phase_time->phase_time = 0.0;
      new_phase_time->calculate_closure_time = 0.0;
      new_phase_time->point_phase = false;
      return new_phase_time;
    }

    void Timer::push_new_phase_time(){
      phase_time_stack_.push(make_new_phase_time());
    }

    void Timer::update_phase_time(){
      phase_time_sptr_t temp(phase_time_stack_.top());
      current_phase_time_ = temp;
      current_phase_time_->parent->children.push_back(current_phase_time_);
      phase_time_stack_.pop();
    }

  }
}


