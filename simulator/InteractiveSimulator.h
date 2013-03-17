#ifndef _INCLUDED_HYDLA_INTERACTIVE_SIMULATOR_H_
#define _INCLUDED_HYDLA_INTERACTIVE_SIMULATOR_H_

#include "Simulator.h"
#include <fstream>
#include "SymbolicSimulator.h"
#include "Logger.h"
#include <sstream>
#include "SymbolicTrajPrinter.h"

using namespace std;
using namespace hydla::logger;

namespace hydla {
namespace simulator {

typedef hydla::vcs::SymbolicVirtualConstraintSolver     solver_t;

class InteractiveSimulator:public Simulator{
public:

  InteractiveSimulator(Opts &opts):Simulator(opts){}

  virtual ~InteractiveSimulator(){}

  /**
   * �^����ꂽ����⃂�W���[���W�������ɃV�~�����[�V�������s�������Ȃ�
   */

  virtual phase_result_const_sptr_t simulate();
  
protected:
  
  static int select_phase(PhaseSimulator::result_list_t& results);
  
  /**
   * ���͂���t���C�R�}���h����������
   * @return 0:�I�� 1~: �V�~�����[�V��������X�e�b�v�̐�
   */
  int input_and_process_command(simulation_todo_sptr_t& phase);

  /**
   * hyrose�̃I�v�V���������R�ɕύX����
   * ����debug�I�v�V�����̂�
   * TODO:���̃I�v�V�������w��ł���悤�ɂ���
   */
  int select_options(){assert(0); return 0;}

  /**
   * interactive���[�h�ɂ�����w���v��\������
   */
  int show_help();
  
  /**
   * �ϐ��l�̕ύX
   */
  int change_variable(simulation_todo_sptr_t& phase);
  
  /**
   * ���݂̃t�F�[�Y���o�͂���
   */
  void print(phase_result_sptr_t& phase);
  
  /**
   * x = 0 ���������ꂽ��V�~�����[�V�������ꎞ��~�C�̂悤�ɒ�~������ݒ肷��
   * TODO: ��������
   */
  int set_breakpoint(){assert(0); return 0;}
  
  /**
   * save state
   * TODO: ��������
   */
  int save_state(simulation_todo_sptr_t& simulation_phase){assert(0); return 0;}

  /**
   * load state
   * TODO: ��������
   */
  int load_state(simulation_todo_sptr_t& simulation_phase){assert(0); return 0;}
  
  /**
   * 1�t�F�[�Y�����o�͂���DTodo��n����Ă��t�F�[�Y�̏�񂾂��o�͂���
   */
  static void print_phase(phase_result_sptr_t& p){printer_.output_one_phase(p);}  
  static void print_phase(simulation_todo_sptr_t& t){printer_.output_one_phase(t->parent);}
  
  /**
   * �V�~�����[�V���������炩�̏����ŏI�������ꍇ�ɂ��̎|���o�͂���
   */
  void print_end(phase_result_sptr_t& p);
  

  template <typename ElementT> static unsigned int select_case(std::vector<ElementT>& vec)
  {
    if(vec.size()>1)
    {
      std::cout << "simulation branched off into "<< vec.size() << "cases" << std::endl;
      std::cout << "-------------------------------------" << std::endl;
      for(unsigned int i=0;i<vec.size();i++)
      {
        std::cout << "-----Case " << i << "--------------------------" << std::endl;
        print_phase(vec[i]);
      }
      std::cout << "-------------------------------------" << std::endl;
      std::cout << "-------------------------------------" << std::endl;
      
      unsigned int select_num;
      do
      {
        select_num = excin<unsigned int>("input case number you want to select");
      }while(select_num >= vec.size());
      return select_num;
    }
    else
    {
      return 0;
    }
  }

  static hydla::output::SymbolicTrajPrinter printer_;
  std::vector<simulation_todo_sptr_t> all_todo_;
  boost::shared_ptr<solver_t> solver_;
};

} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_INTERACTIVE_SIMULATOR_H_

