	
#ifndef _INCLUDED_HYDLA_HAConverter_SIMULATOR_H_
#define _INCLUDED_HYDLA_HAConverter_SIMULATOR_H_


#include "HybridAutomata.h"

#include <map>
#include <string>


namespace hydla {
namespace simulator {

	
class HAConverter: public HybridAutomata{
public:
	
	HAConverter(Opts &opts);

  virtual ~HAConverter();

  virtual phase_result_const_sptr_t simulate();

  virtual void process_one_todo(simulation_todo_sptr_t& todo);
	
	typedef phase_result_sptrs_t													 current_condition_t;
	typedef std::deque<current_condition_t> 					 		 current_conditions_t;

	typedef std::deque<current_condition_t> 	ha_results_t;

	// 状態キュー
	current_conditions_t cc_vec_;
	
	// 変換結果を保持
	ha_results_t ha_results_;
	
	// 変換結果を返す
	ha_results_t get_results()
	{
		return ha_results_;
	}
	
protected:

	// check_subsetがtrueとなったときのphase_idを保持  状態遷移列の出力時に必要
	int subset_id;

	// 実行済みかどうかのチェック
	bool check_already_exec(phase_result_sptr_t phase, current_condition_t cc);

	// 各パラメータが部分集合となっているかのチェック
	bool check_subset(phase_result_sptr_t phase, phase_result_sptr_t past_phase);

	// 変数パラメータを探索して表示
	void search_variable_parameter(parameter_map_t map, std::string name, int diff_cnt);
	
	// ２つのphase_resultのphase、モジュール集合、positive_askが等しいかどうか判定
	bool compare_phase_result(phase_result_sptr_t r1, phase_result_sptr_t r2);
	
	// 生成された全てのHAを出力（dot言語）
	void output_ha();
	
	// phase_resultsをhaのdot言語に変換する
	void convert_phase_results_to_ha(phase_result_sptrs_t result);
	
	// ha_resultにHA変換に必要な情報をpushする
	void push_result(current_condition_t cc);
	
	// asksを連ねた文字列を取得
	std::string get_asks_str(ask_set_t asks);
	
	// 状態キューに新たな状態を追加する
	// push_simulation_phaseと会わせる必要あり
	void push_current_condition(const current_condition_t cc)
	{
	  //HYDLA_LOGGER_HA("push cc");
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
	



};//HAConverter


}//namespace hydla
}//namespace simulator 

#endif // _INCLUDED_HYDLA_HAConverter_SIMULATOR_H_

