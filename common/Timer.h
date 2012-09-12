#ifndef _INCLUDED_HYDLA_TIMER_H_
#define _INCLUDED_HYDLA_TIMER_H_

#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>
#include <stack>

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
    
    struct PhaseTime {
      elapsed_time_t phase_time;
      elapsed_time_t calculate_closure_time;

      boost::shared_ptr<PhaseTime> parent;
      std::vector< boost::shared_ptr<PhaseTime> > children;
      bool point_phase;
    };

    typedef boost::shared_ptr<PhaseTime> phase_time_sptr_t;
    typedef std::vector<phase_time_sptr_t> phase_time_sptrs_t;
    

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
       * タイマーを初期化する
       */      
      static void init_timer();

      /**
       * 呼ばれた時点での経過時刻を得る
       */
      elapsed_time_t get_time();

      /**
       * 呼ばれた時点での経過時刻を
       * pに対応する変数に加算する
       */
      void count_time(std::string p);


      /**
       * 呼ばれた時点でのタイマー初期化からの経過時刻を
       * 表示する
       */
      void elapsed();
      void elapsed(std::string str);

      /**
       * 各timeの値を出力する
       */
      static void output_time(bool nd_mode);

      /**
       * 新たなphase_time_sptr_tをstackにpushする
       */
      static void push_new_phase_time();

      /**
       * current_phase_time_を新しいphaseに更新する
       */
      static void update_phase_time();
    
      /**
       * シミュレーションしているphaseがpoint_phaseであることを設定する
       */  
      static void is_point_phase();

      /**
       * output_timeから呼び出される関数
       */
      static void output_phase_time_node(phase_time_sptr_t &node, std::vector<std::string> &result, int &case_num, int &phase_num, bool nd_mode);

      /**
       * 新しいphase_time_sptr_tを作り返す
       */
      static phase_time_sptr_t make_new_phase_time();

      /**
       * phase_timeに保存されている内容を得る
       */
      static std::string get_phase_time_output(phase_time_sptr_t &node);
      
    private:
      /**
       * 測定開始時の値
       */
      time_point_t start_point_;

      /**
       * phase_timeのroot
       */
      static phase_time_sptr_t phase_time_root_;

      /**
       * 次にシミュレーションするphase_time_sptr_tを入れておくスタック
       */
      static std::stack< phase_time_sptr_t > phase_time_stack_;

      /**
       * シミュレーション中のPhaseに対応するphase_time_sptr_t
       */
      static phase_time_sptr_t current_phase_time_;

    };
  }  //  namespace timer
}  //  namespace hydla

#endif  // _INCLUDED_HYDLA_TIMER_H_
