#ifndef _INCLUDED_HYDLA_TIMER_H_
#define _INCLUDED_HYDLA_TIMER_H_

//#include <boost/shared_ptr.hpp>
//#include <vector>
#include <string>
//#include <stack>

#include <iomanip>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) 

#include < time.h >
#include <windows.h> //I've ommited this line.
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64



#else

#include <sys/time.h> /// Linux用？

#endif

#define TIMER_PLACES 6

namespace hydla{
  namespace timer{
    
    typedef struct timeval time_point_t;
    typedef double elapsed_time_t;

    class Timer
    {
    public:
      Timer();
      ~Timer();
      /**
       * 計測開始時刻を現在時刻に設定する
       */
      void restart();

      /**
       * 呼ばれた時点での経過時刻を得る
       */
      elapsed_time_t get_time();

      /**
       * elapsed_time_の値に経過時間を加算する
       */
      void count_time();

      /**
       * メンバ変数を全て初期化する
       */
      void reset();

      /**
       * 計測した時間を文字列で返す
       */
      std::string get_time_string();

      /**
       * 呼ばれた時点でのタイマー初期化からの経過時刻を
       * 表示する
       */
      void elapsed();
      void elapsed(std::string str);

      /**
       * 計測時間の値が0ならtrueを返す
       */
      bool is_zero();
      
    private:
      /**
       * 測定開始時の値
       */
      time_point_t start_point_;
      elapsed_time_t elapsed_time_;

    };
  }  //  namespace timer
}  //  namespace hydla

#endif  // _INCLUDED_HYDLA_TIMER_H_
