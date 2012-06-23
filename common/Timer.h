#ifndef _INCLUDED_HYDLA_TIMER_H_
#define _INCLUDED_HYDLA_TIMER_H_

#include <vector>
#include <string>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) 

#include < time.h >
#include <windows.h> //I've ommited this line.
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64



#else

#include <sys/time.h> /// Linux�p�H

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
       * �^�C�}�[������������
       */
      void restart();
      
      
      /**
       * �Ă΂ꂽ���_�ł̃^�C�}�[����������̌o�ߎ�����
       * p�ɑΉ�����simulation_time_ �ɒǉ�����
       */
      void push_time(std::string p);


      /**
       * �Ă΂ꂽ���_�ł̃^�C�}�[����������̌o�ߎ�����
       * �\������
       */
      void elapsed();

      /**
       * �etime�̒l���o�͂���
       */
      static void output_time();
      
      /**
       * CaseTime�̃x�N�^�ɐV����Case�ɑΉ�����
       * CaseTime�\���̂�ǉ�����
       * --nd�p
       */
      //      static void push_next_case_time();

    private:
      /**
       * ����J�n���̒l
       */
      time_point_t start_point_;

      /**
       * 1�P�[�X�̎��Ԃ�ێ�����\����
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
