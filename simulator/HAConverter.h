#ifndef _INCLUDED_HYDLA_HAConverter_SIMULATOR_H_
#define _INCLUDED_HYDLA_HAConverter_SIMULATOR_H_

#include "Simulator.h"


namespace hydla {
namespace simulator {

class HAConverter:public Simulator{
public:
  typedef PhaseResult                                       phase_result_t;
  typedef boost::shared_ptr<const phase_result_t>           phase_result_const_sptr_t;
  typedef PhaseSimulator                                    phase_simulator_t;
  typedef phase_result_t::phase_result_sptr_t               phase_result_sptr_t;
  typedef std::vector<phase_result_sptr_t >                 phase_result_sptrs_t;
  
  typedef SimulationTodo                                   simulation_phase_t;
  typedef boost::shared_ptr<SimulationTodo>                simulation_phase_sptr_t;
  typedef std::vector<simulation_phase_sptr_t>              simulation_phases_t;

  typedef phase_result_t::variable_map_t variable_map_t;
  typedef phase_result_t::variable_t     variable_t;
  typedef phase_result_t::parameter_t    parameter_t;
  typedef phase_result_t::value_t        value_t;
  typedef phase_result_t::parameter_map_t     parameter_map_t;
  
  typedef std::list<variable_t>                            variable_set_t;
  typedef std::list<parameter_t>                           parameter_set_t;
  typedef value_t                                          time_value_t;
	
  HAConverter(Opts &opts);

  virtual ~HAConverter();
  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  virtual phase_result_const_sptr_t simulate();
  /**
   * シミュレータの初期化を行う
   */
  virtual void initialize(const parse_tree_sptr& parse_tree);
	
private: 
   //とりあえずSequentialSimulator.hと同じ  あとあと必要に応じて追加
   /**
   * シミュレーション中で使用される変数表の原型
   */
  variable_map_t variable_map_;

  
  /*
   * シミュレーション中に使用される変数と記号定数の集合
   */
  variable_set_t variable_set_;
  parameter_set_t parameter_set_;
  
  
  /**
   * シミュレーション対象となるパースツリー
   */
  parse_tree_sptr parse_tree_;
	
	
	/**
	 * 変換に用いる変数の定義
	 * その構造体
 	*/
	struct CurrentCondition
	{
		phase_result_sptrs_t phase_results;
		std::vector<phase_result_sptrs_t> ls;
		phase_result_sptrs_t loop;
		bool is_loop_step;
		int loop_start_id;	
		CurrentCondition(
			phase_result_sptrs_t phase_results, 
			std::vector<phase_result_sptrs_t> ls, 
			phase_result_sptrs_t loop,
			bool is_loop_step,
			int loop_start_id
		): phase_results(phase_results), ls(ls), loop(loop), is_loop_step(is_loop_step), loop_start_id(loop_start_id){}
		CurrentCondition(){}
	};

	typedef CurrentCondition															 current_condition_t;
	typedef std::deque<current_condition_t> 					 		 current_conditions_t;
	
	//current_condition_sptr_t cc_;
	
	current_conditions_t cc_vec_;
		
	std::vector<phase_result_sptrs_t> ha_results_;
	
	// ループ判定ステップを続けるか（lsの要素にloopを部分集合とするものがあるか）
	bool check_continue(current_condition_t cc);
	
	// lsにループ候補をset
	void set_possible_loops(phase_result_sptr_t result, current_condition_t *cc);
	
	// loopがlsの最大要素と同じかどうか
	bool loop_eq_max_ls(current_condition_t cc);
	
	// エッジ判定ステップ
	void check_edge_step();
	
	// 現phase_resultがphase_resultsに含まれるか
	bool check_contain(phase_result_sptr_t result, current_condition_t cc);

	// ２つのphase_resultのphase、モジュール集合、positive_askが等しいかどうか判定
	bool compare_phase_result(phase_result_sptr_t r1, phase_result_sptr_t r2);
 
	// lsの中身表示
	void viewLs(current_condition_t cc);
	// phase_result_sptrs_tの中身表示
	void viewPrs(phase_result_sptrs_t results);
	
	// 生成された全てのHAを出力（dot言語）
	void output_ha();
	
	// phase_resultsをhaのdot言語に変換する
	void convert_phase_results_to_ha(phase_result_sptrs_t result);
	
	// ha_resultにHA変換に必要な情報をpushする
	void push_result(current_condition_t cc);
	
	// 状態キューに新たな状態を追加する
	// push_simulation_phaseと会わせる必要あり
	void push_current_condition(const current_condition_t cc)
	{
		HYDLA_LOGGER_HA("push cc");
		cc_vec_.push_front(cc);
	}

	// 状態キューから状態をひとつ取り出す
	// pop_simulation_phase()と合わせる必要あり
	current_condition_t pop_current_condition()
	{
    current_condition_t cc;
    if(opts_->search_method == simulator::DFS){
      cc = cc_vec_.front();
      cc_vec_.pop_front();
    }else{
      cc = cc_vec_.back();
      cc_vec_.pop_back();
    }
    return cc;

	}
	
};
	
} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_HAConverter_SIMULATOR_H_

