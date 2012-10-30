#ifndef _INCLUDED_HYDLA_TIMER_H_
#define _INCLUDED_HYDLA_TIMER_H_

#include <string>

#include <iomanip>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) 

#include < time.h >
#include <windows.h>
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
  
#else

#include <sys/time.h> /// Linux�p�H

#endif

#define TIMER_PLACES 6

namespace hydla{
  namespace timer{
    
    typedef double elapsed_time_t;

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) 
    struct timeval {
      long tv_sec;
      long tv_usec;
    };
#endif

    class Timer
    {
    public:
      Timer();
      ~Timer();
      /**
       * �v���J�n���������ݎ����ɐݒ肷��
       */
      void restart();

      /**
       * �Ă΂ꂽ���_�ł̌o�ߎ����𓾂�
       */
      elapsed_time_t get_time();

      /**
       * elapsed_time_�̒l�Ɍo�ߎ��Ԃ����Z����
       */
      void count_time();

      /**
       * �����o�ϐ���S�ď���������
       */
      void reset();

      /**
       * �v���������Ԃ𕶎���ŕԂ�
       */
      std::string get_time_string();

      /**
       * �Ă΂ꂽ���_�ł̃^�C�}�[����������̌o�ߎ�����
       * �\������
       */
      void elapsed();
      void elapsed(std::string str);

      /**
       * �v�����Ԃ̒l��0�Ȃ�true��Ԃ�
       */
      bool is_zero();
      
    private:
      /**
       * ����J�n���̒l
       */
      struct timeval start_point_;
      elapsed_time_t elapsed_time_;

    };
  }  //  namespace timer
}  //  namespace hydla

#endif  // _INCLUDED_HYDLA_TIMER_H_
