#ifndef _INCLUDED_HYDLA_SIMULATOR_H_
#define _INCLUDED_HYDLA_SIMULATOR_H_

#include <iostream>
#include <string>
#include <stack>
#include <cassert>


#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
//#include <boost/function.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/xpressive/xpressive.hpp>

#include "Logger.h"

#include "Node.h"
#include "ParseTree.h"
#include "ModuleSetContainer.h"
#include "ModuleSetContainerCreator.h"
#include "ModuleSetGraph.h"
#include "ModuleSetList.h"


#include "AskDisjunctionSplitter.h"
#include "AskDisjunctionFormatter.h"

#include "PhaseResult.h"
#include "InitNodeRemover.h"
#include "TreeInfixPrinter.h"
#include "DefaultParameter.h"

namespace {
  using namespace hydla::ch;
  using namespace hydla::simulator;
  using namespace hydla::parse_tree;
  struct ModuleSetContainerInitializer {
    typedef boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree_sptr;
    typedef hydla::simulator::module_set_container_sptr         module_set_container_sptr;
    template<typename MSCC>
      static void init(
          const parse_tree_sptr& parse_tree,
          module_set_container_sptr& msc_original, 
          module_set_container_sptr& msc_no_init,
          parse_tree_sptr& member_parse_tree)
      {
        ModuleSetContainerCreator<MSCC> mcc;
        {
          parse_tree_sptr pt_original(boost::make_shared<ParseTree>(*parse_tree));
          AskDisjunctionFormatter().format(pt_original.get());
          AskDisjunctionSplitter().split(pt_original.get());
          msc_original = mcc.create(pt_original);
        }

        {
          parse_tree_sptr pt_no_init(boost::make_shared<ParseTree>(*parse_tree));
          InitNodeRemover().apply(pt_no_init.get());
          AskDisjunctionFormatter().format(pt_no_init.get());
          AskDisjunctionSplitter().split(pt_no_init.get());
          msc_no_init = mcc.create(pt_no_init);

          // 最適化された形のパースツリーを得る
          member_parse_tree = pt_no_init;
        }
      }
  };
}

namespace hydla {
namespace simulator {

class PhaseSimulator;

//TODO:PhaseResultTypeじゃなくて，ValueType入れるテンプレートクラスにできない？というか設計再検討．このクラスは何に対応している？
//実行アルゴリズムだろうか？
/**
 * シミュレーション全体の進行を担当するクラス
 * ・制約モジュール集合の半順序集合を入力とし，解軌道群を出力する
 * ・各解軌道は，時刻0からPPとIPを時刻が進む方向にのみ繰り返して進めることで得られるものとする
 */
class Simulator
{
public:  
  typedef PhaseResult                                    phase_result_t;
  typedef boost::shared_ptr<const phase_result_t>        phase_result_const_sptr_t;
  typedef phase_result_t::phase_result_sptr_t            phase_result_sptr_t;
  typedef std::vector<phase_result_sptr_t >              phase_result_sptrs_t;
  typedef std::vector<phase_result_const_sptr_t >        phase_result_const_sptrs_t;
  typedef PhaseSimulator                                 phase_simulator_t;
  
  typedef SimulationTodo                                simulation_phase_t;
  typedef boost::shared_ptr<SimulationTodo>             simulation_phase_sptr_t;

  typedef phase_result_t::variable_map_t      variable_map_t;
  typedef phase_result_t::variable_t          variable_t;
  typedef phase_result_t::parameter_t         parameter_t;
  typedef phase_result_t::value_t             value_t;
  typedef phase_result_t::parameter_map_t     parameter_map_t;
  
  typedef std::list<variable_t>                            variable_set_t;
  typedef std::list<parameter_t>                           parameter_set_t;
  typedef value_t                                          time_value_t;

  Simulator(Opts& opts);
  
  virtual ~Simulator(){}

  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  virtual phase_result_const_sptr_t simulate() = 0;
  
  /**
   * 使用するPhaseSimulatorを設定する．
   * この関数に渡すPhaseSimulatorのインスタンスはnewで作成し，呼び出し側でdeleteしないようにする
   */
  void set_phase_simulator(phase_simulator_t *ps);
  
  virtual void initialize(const parse_tree_sptr& parse_tree);
  
  void init_module_set_container(const parse_tree_sptr& parse_tree);
  
  /**
   * シミュレーション時に使用される変数表のオリジナルの作成
   */
  virtual void init_variable_map(const parse_tree_sptr& parse_tree);
  
  /**
   * プロファイリングの結果を取得
   */
  entire_profile_t get_profile(){return profile_vector_;}

protected:

  /**
   * 状態キューに新たな状態を追加する
   */
  virtual void push_simulation_phase(const simulation_phase_sptr_t& state)
  {
    state->phase_result->id = phase_id_++;
    state_stack_.push_front(state);
  }

  /**
   * 状態キューから状態をひとつ取り出す
   */
  simulation_phase_sptr_t pop_simulation_phase()
  {
    simulation_phase_sptr_t state;
    if(opts_->search_method == simulator::DFS){
      state = state_stack_.front();
      state_stack_.pop_front();
    }else{
      state = state_stack_.back();
      state_stack_.pop_back();
    }
    profile_vector_.push_back(state);
    // とりあえず，この関数で取りだしたものは必ずシミュレーションを行うことを前提にする．
    return state;
  }
  
  /// 実際にシミュレータが閉包計算などを行ったフェーズの集合．主にプロファイリングのために取っておく．
  entire_profile_t profile_vector_;

  /**
   * シミュレーション対象となるパースツリー
   */
  parse_tree_sptr parse_tree_;
  
  
  /**
   * シミュレーション中で使用される変数表の原型
   */
  variable_map_t variable_map_;

  
  /*
   * シミュレーション中に使用される変数と記号定数の集合
   */
  variable_set_t variable_set_;
  parameter_set_t parameter_set_;
  int state_id_;

  /**
   * 使用するPhaseSimulator
   */ 
  boost::shared_ptr<phase_simulator_t > phase_simulator_;


  module_set_container_sptr msc_original_;
  module_set_container_sptr msc_no_init_;

  /**
   * 各状態を保存しておくためのスタック
   */
  std::deque<simulation_phase_sptr_t> state_stack_;
  
  parse_tree_sptr parse_tree;
  
  /// 解軌道木の根．初期状態なので，子供以外の情報は入れない
  phase_result_sptr_t result_root_;
  
  Opts*     opts_;
  /**
   * 各PhaseResultに振っていくID
   */
  int phase_id_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_H_
