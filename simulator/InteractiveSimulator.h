#ifndef _INCLUDED_HYDLA_INTERACTIVE_SIMULATOR_H_
#define _INCLUDED_HYDLA_INTERACTIVE_SIMULATOR_H_

#include "Simulator.h"
#include <fstream>
#include "SymbolicPhaseSimulator.h"
#include "Logger.h"
#include <sstream>
#include "SymbolicTrajPrinter.h"

using namespace std;
using namespace hydla::logger;

namespace hydla {
namespace simulator {


/*
 * template method for input 
 */
template<typename T> T excin(string message="")
{
  T ret;
  while(1)
  {
    if(!message.empty()) cout << message << endl;
    cout << '>' ;
    cin >> ret;
    if(!cin.fail()) break;
    cin.clear();
    cin.ignore( 1024, '\n' );
  }
  cin.clear();
  cin.ignore( 1024, '\n' );
  return ret;
}

typedef hydla::vcs::SymbolicVirtualConstraintSolver     solver_t;

class InteractiveSimulator:public Simulator{
public:

  InteractiveSimulator(Opts &opts):Simulator(opts){}

  virtual ~InteractiveSimulator(){}

  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */

  virtual phase_result_const_sptr_t simulate();
  
protected:
  
  static int select_phase(PhaseSimulator::result_list_t& results);
  
  /**
   * 入力を受付けつつ，コマンドを処理する
   * @return 0:終了 1~: シミュレーションするステップの数
   */
  int input_and_process_command(simulation_todo_sptr_t& phase);

  /**
   * hyroseのオプションを自由に変更する
   * 現在debugオプションのみ
   * TODO:他のオプションも指定できるようにする
   */
  int select_options(){assert(0); return 0;}

  /**
   * interactiveモードにおけるヘルプを表示する
   */
  int show_help();
  
  /**
   * 変数値の変更
   */
  int change_variable(simulation_todo_sptr_t& todo);

  /**
   * 変数値の近似
   */
  int approx_variable(simulation_todo_sptr_t& todo);
  
  /**
   * 現在のフェーズを出力する
   */
  void print(phase_result_sptr_t& phase);
  
  /**
   * x = 0 が満たされたらシミュレーションを一時停止，のように停止条件を設定する
   * TODO: 実装する
   */
  int set_breakpoint(){assert(0); return 0;}
  
  /**
   * save state
   * TODO: 実装する
   */
  int save_state(simulation_todo_sptr_t& simulation_phase){assert(0); return 0;}

  /**
   * load state
   * TODO: 実装する
   */
  int load_state(simulation_todo_sptr_t& simulation_phase){assert(0); return 0;}
  
  /**
   * 1フェーズだけ出力する．Todoを渡されてもフェーズの情報だけ出力する
   */
  static void print_phase(phase_result_sptr_t& p){printer_.output_one_phase(p);}  
  static void print_phase(simulation_todo_sptr_t& t){printer_.output_one_phase(t->parent);}
  
  /**
   * シミュレーションが何らかの条件で終了した場合にその旨を出力する
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

  //Print unsat cores in a phase
  int find_unsat_core(simulation_todo_sptr_t&);

  static hydla::output::SymbolicTrajPrinter printer_;
  std::vector<simulation_todo_sptr_t> all_todo_;
};

} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_INTERACTIVE_SIMULATOR_H_

