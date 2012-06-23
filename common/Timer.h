#ifndef _INCLUDED_HYDLA_TIMER_H_
#define _INCLUDED_HYDLA_TIMER_H_

#include <vector>
#include <string>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) 

#include < time.h >
#include <windows.h> //I've ommited this line.
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64



#else

#include <sys/time.h> /// Linux用？

#endif

namespace hydla{
  namespace timer{
    
    typedef struct timeval time_point_t;
    typedef double elapsed_time_t;

    /*
    typedef struct CaseTime_ {
      std::vector<elapsed_time_t> pp_calculate_closure_time_;
      std::vector<elapsed_time_t> pp_simulation_time_;
      std::vector<elapsed_time_t> ip_calculate_closure_time_;
      std::vector<elapsed_time_t> ip_simulation_time_;
    } CaseTime;
    */

    class Timer
    {
    public:
      Timer();
      ~Timer();
      /**
       * タイマーを初期化する
       */
      void restart();
      
      
      /**
       * 呼ばれた時点でのタイマー初期化からの経過時刻を
       * pに対応するsimulation_time_ に追加する
       */
      void push_time(std::string p);


      /**
       * 呼ばれた時点でのタイマー初期化からの経過時刻を
       * 表示する
       */
      void elapsed();

      /**
       * 各timeの値を出力する
       */
      static void output_time();
      
      /**
       * CaseTimeのベクタに新たなCaseに対応する
       * CaseTime構造体を追加する
       * --nd用
       */
      //      static void push_next_case_time();

    private:
      /**
       * 測定開始時の値
       */
      time_point_t start_point_;

      /**
       * 1ケースの時間を保持する構造体
       */
      //      static std::vector<CaseTime> pt_;


      static std::vector<elapsed_time_t> pp_calculate_closure_time_;
      static std::vector<elapsed_time_t> pp_simulation_time_;
      static std::vector<elapsed_time_t> ip_calculate_closure_time_;
      static std::vector<elapsed_time_t> ip_simulation_time_;



    };
  }  //  namespace timer
}  //  namespace hydla

#endif  // _INCLUDED_HYDLA_TIMER_H_
