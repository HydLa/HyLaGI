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

#include <sys/time.h> /// Linux�p�H

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
       * �v���J�n���������ݎ����ɐݒ肷��
       */
      void restart();

      /**
       * �^�C�}�[������������
       */      
      static void init_timer();

      /**
       * �Ă΂ꂽ���_�ł̌o�ߎ����𓾂�
       */
      elapsed_time_t get_time();

      /**
       * �Ă΂ꂽ���_�ł̌o�ߎ�����
       * p�ɑΉ�����ϐ��ɉ��Z����
       */
      void count_time(std::string p);


      /**
       * �Ă΂ꂽ���_�ł̃^�C�}�[����������̌o�ߎ�����
       * �\������
       */
      void elapsed();
      void elapsed(std::string str);

      /**
       * �etime�̒l���o�͂���
       */
      static void output_time(bool nd_mode);

      /**
       * �V����phase_time_sptr_t��stack��push����
       */
      static void push_new_phase_time();

      /**
       * current_phase_time_��V����phase�ɍX�V����
       */
      static void update_phase_time();
    
      /**
       * �V�~�����[�V�������Ă���phase��point_phase�ł��邱�Ƃ�ݒ肷��
       */  
      static void is_point_phase();

      /**
       * output_time����Ăяo�����֐�
       */
      static void output_phase_time_node(phase_time_sptr_t &node, std::vector<std::string> &result, int &case_num, int &phase_num, bool nd_mode);

      /**
       * �V����phase_time_sptr_t�����Ԃ�
       */
      static phase_time_sptr_t make_new_phase_time();

      /**
       * phase_time�ɕۑ�����Ă�����e�𓾂�
       */
      static std::string get_phase_time_output(phase_time_sptr_t &node);
      
    private:
      /**
       * ����J�n���̒l
       */
      time_point_t start_point_;

      /**
       * phase_time��root
       */
      static phase_time_sptr_t phase_time_root_;

      /**
       * ���ɃV�~�����[�V��������phase_time_sptr_t�����Ă����X�^�b�N
       */
      static std::stack< phase_time_sptr_t > phase_time_stack_;

      /**
       * �V�~�����[�V��������Phase�ɑΉ�����phase_time_sptr_t
       */
      static phase_time_sptr_t current_phase_time_;

    };
  }  //  namespace timer
}  //  namespace hydla

#endif  // _INCLUDED_HYDLA_TIMER_H_
