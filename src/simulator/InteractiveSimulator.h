#pragma once

#include "Simulator.h"
#include <fstream>
#include "PhaseSimulator.h"
#include "Logger.h"
#include <sstream>
#include "SymbolicTrajPrinter.h"
#include "Node.h"
#include "ParseTree.h"
#include "JsonWriter.h"


namespace hydla {
namespace simulator {
/*
 * template method for input 
 */
template<typename T> T excin(std::string message="")
{
  T ret;
  while(1)
  {
    if(!message.empty()) std::cout << message << std::endl;
    std::cout << '>' ;
    std::cin >> ret;
    if(!std::cin.fail()) break;
    std::cin.clear();
    std::cin.ignore( 1024, '\n' );
  }
  std::cin.clear();
  std::cin.ignore( 1024, '\n' );
  return ret;
}

class InteractiveSimulator:public Simulator{
public:
/*
  typedef boost::spirit::classic::multi_pass<std::istreambuf_iterator<char> > multipass_iter_t;
  typedef boost::spirit::classic::position_iterator<multipass_iter_t> pos_iter_t;
  typedef boost::spirit::classic::node_val_data_factory<> node_val_data_factory_t;
  typedef boost::spirit::classic::tree_parse_info<pos_iter_t, node_val_data_factory_t> tree_info_t;
*/
  InteractiveSimulator(Opts &opts):Simulator(opts), printer_(backend){}

  virtual ~InteractiveSimulator(){}

  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */

  virtual phase_result_sptr_t simulate();
  
protected:
  
  /**
   * 入力を受付けつつ，コマンドを処理する
   * @return 0:終了 1~: シミュレーションするステップの数
   */
  int input_and_process_command(phase_result_sptr_t& phase);

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
  int change_variable(phase_result_sptr_t& todo);

  /*
   * change current time
   */
  int change_time(phase_result_sptr_t& todo);

  /**
   * 変数値の近似
   */
  int approx_variable(phase_result_sptr_t& todo);
  
  /**
   * 現在のフェーズを出力する
   */
  void print(phase_result_sptr_t& phase);
  
  /**
   * x = 0 が満たされたらシミュレーションを一時停止，のように停止条件を設定する
   * TODO: 実装する
   */
  int set_breakpoint(phase_result_sptr_t& simulation_phase);
  
  /**
   * save state
   */
  int save_state(phase_result_sptr_t& simulation_phase);

  /**
   * load state
   */
  int load_state(phase_result_sptr_t& simulation_phase);
  
  /**
   * 1フェーズだけ出力する．Todoを渡されてもフェーズの情報だけ出力する
   */
  void print_phase(phase_result_sptr_t& p){printer_.output_one_phase(p);}  
  
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
// TODO: implement this by nonstatic way
//        print_phase(vec[i]);
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
  int find_unsat_core(phase_result_sptr_t&);

//  tree_info_t parse(std::stringstream& stream);

  hydla::io::SymbolicTrajPrinter printer_;
  std::vector<phase_result_sptr_t> all_todo_;
};

} // simulator
} // hydla

