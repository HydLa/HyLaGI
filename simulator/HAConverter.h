	
#ifndef _INCLUDED_HYDLA_HAConverter_SIMULATOR_H_
#define _INCLUDED_HYDLA_HAConverter_SIMULATOR_H_


#include "BatchSimulator.h"
#include "Node.h"
#include "DefaultTreeVisitor.h"
#include "PhaseResult.h"

#include <map>
#include <string>


namespace hydla {
namespace simulator {

class HAConverter: public BatchSimulator{
public:
	
	HAConverter(Opts &opts);

  virtual ~HAConverter();

  virtual phase_result_const_sptr_t simulate();

  virtual void process_one_todo(simulation_todo_sptr_t& todo);

protected:

	typedef hydla::ch::module_set_sptr 												module_set_sptr_t;
	typedef std::vector<module_set_sptr_t>					 					module_set_sptrs_t;
		
	// エッジ構造体 成り立つガード条件＋採用したモジュール集合
	struct Phase
	{
		module_set_sptr_t module_set;
		negative_asks_t negative_asks;
		positive_asks_t positive_asks;
		
		Phase(module_set_sptr_t module_set, negative_asks_t negative_asks, positive_asks_t positive_asks): 
			module_set(module_set), negative_asks(negative_asks), positive_asks(positive_asks){}
		
		Phase(){}
	};
	
	typedef Phase																phase_t;
  typedef std::vector<phase_t>								phases_t;

	typedef phase_result_sptrs_t													 current_condition_t;
	typedef std::deque<current_condition_t> 					 		 current_conditions_t;
	
	typedef std::deque<current_condition_t> 	ha_results_t;

	// 状態キュー
	current_conditions_t cc_vec_;
	
	// 変換結果を保持
	ha_results_t ha_results_;
	
	// 実行済みかどうかのチェック
	bool check_already_exec(phase_result_sptr_t phase, current_condition_t cc);

	// 各パラメータが部分集合となっているかのチェック
	bool check_subset(phase_result_sptr_t phase, phase_result_sptr_t past_phase);

	// AがBの範囲に含まれる場合trueを返す
	bool compare_parameter_range(range_t A, range_t B);	
	
	// 変数がガード条件に出現するか
	bool check_guard_variable(phase_result_sptr_t phase, std::string name, int derivative_count);
	
	// ２つのphase_resultのphase、モジュール集合、positive_askが等しいかどうか判定
	bool compare_phase_result(phase_result_sptr_t r1, phase_result_sptr_t r2);

	// フェーズ同士が同じものかどうか判定
	bool compare_phase(phase_t p1, phase_t p2);

	// 現phase_resultがphase_resultsに含まれるか
	bool check_contain(phase_result_sptr_t result, current_condition_t cc);
 
	// phase_result_sptrs_tの中身表示
	void viewPrs(phase_result_sptrs_t results);
	// phase_result_sptr_tの中身表示
	void viewPr(phase_result_sptr_t result);
	// asksの中身表示
	void viewAsks(ask_set_t asks);
	
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

class GuardGetter : public parse_tree::DefaultTreeVisitor {
public:
	virtual void accept(const boost::shared_ptr<parse_tree::Node>& n);
	
	GuardGetter();
	virtual ~GuardGetter();
	
	// Ask
	virtual void visit(boost::shared_ptr<parse_tree::Ask> node);
	
	ask_set_t asks;
};//GuardGetter

class VaribleGetter : public parse_tree::DefaultTreeVisitor {
public:
	virtual void accept(const boost::shared_ptr<parse_tree::Node>& n);

	typedef struct GuardVariable
	{
		int diff_cnt;
		std::string name;
	}guard_variable_t;
	
	typedef std::vector<guard_variable_t>  guard_variable_vec_t;

	VaribleGetter();
	virtual ~VaribleGetter();
	
	// 微分
	virtual void visit(boost::shared_ptr<parse_tree::Differential> node);
	// 変数
	virtual void visit(boost::shared_ptr<parse_tree::Variable> node);
	
	int tmp_diff_cnt;
	
	guard_variable_vec_t  vec_variable;
	guard_variable_vec_t::iterator it;
	
	guard_variable_vec_t::iterator get_iterator()
	{
		return it;
	}

	void guard_variable_vec_t::iterator it_begin()
	{
		it = vec_variable.begin();
	}
	
	void inc_it()
	{
		it++;
	}
	
};//VaribleGetter


}//namespace hydla
}//namespace simulator 

#endif // _INCLUDED_HYDLA_HAConverter_SIMULATOR_H_

