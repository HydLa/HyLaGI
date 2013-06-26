
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
	
	/**
	 * 変換に用いる変数の定義
	 * その構造体
 	*/
	enum ResultCheckOccurrence 
	{
		Reachable,
		Unreachable,
		Unknown
	};
	
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
	phases_t passed_edge;
	phases_t passed_node;


	typedef Phase																edge_t;
  typedef std::vector<edge_t>									edges_t;

	typedef Phase																node_t;
  typedef std::vector<node_t>              		nodes_t;
	
	void viewEdge(phase_t edge);
	void viewNode(phase_result_sptr_t node);
	typedef std::multimap<phase_result_sptr_t, phase_t>					 unknown_map_t;
	
	// エッジ判定ステップの結果
	struct ResultCheckReachable
	{
		// あるnodeから派生するエッジ  ノードとの組で表現
		unknown_map_t reachable;
		unknown_map_t unknown;
		ResultCheckReachable(unknown_map_t reachable, unknown_map_t unknown): reachable(reachable), unknown(unknown){}
		ResultCheckReachable(){}
	};
	typedef ResultCheckReachable																result_check_reachable_t;

	
	struct CurrentCondition
	{
		phase_result_sptrs_t phase_results;
		std::vector<phase_result_sptrs_t> ls;
		phase_result_sptrs_t loop;
		bool is_loop_step;
		unknown_map_t unknown_map;
		int loop_count;
		int loop_start_id;
		// なぜか警告が出るのでコメントアウト．必要もないので
		/*
		CurrentCondition(
			phase_result_sptrs_t phase_results, 
			std::vector<phase_result_sptrs_t> ls, 
			phase_result_sptrs_t loop,
			bool is_loop_step,
			int loop_count,
			int loop_start_id,
			unknown_map_t unknown_map
		): phase_results(phase_results), ls(ls), loop(loop), is_loop_step(is_loop_step), loop_count(loop_count), loop_start_id(loop_start_id), unknown_map(unknown_map){}
		*/
		CurrentCondition(){}
	};

	typedef CurrentCondition															 current_condition_t;
	typedef std::deque<current_condition_t> 					 		 current_conditions_t;
	
	typedef std::map<phase_result_sptrs_t, unknown_map_t> 	ha_result_t;

	// エッジ通過可能性判定の結果を保持
	result_check_reachable_t result_check_reachable_;
	
	// 状態キュー
	current_conditions_t cc_vec_;
	
	// 変換結果を保持
	ha_result_t ha_results_;
	
	// ループ判定ステップを続けるか（lsの要素にloopを部分集合とするものがあるか）
	bool check_continue(current_condition_t *cc);
	
	// lsにループ候補をset
	void set_possible_loops(phase_result_sptr_t result, current_condition_t *cc);
	
	// loopがlsの最大要素と同じかどうか
	bool loop_eq_max_ls(current_condition_t cc);
	
	// エッジ判定ステップ ノードを回り、それぞれcheckEdgeを行う
	void checkNode(current_condition_t cc);
	// ノードごとのエッジ判定
	void checkEdge(phase_result_sptr_t node, current_condition_t cc);
	// フェーズがすでに通過したものかどうか判定 通過してたらtrue
	bool check_passed_phase(phases_t passed_phase, phase_t phase);
	
	void create_asks_vec(boost::shared_ptr<parse_tree::Ask> ask);	
	std::vector<negative_asks_t> vec_negative_asks;
	std::vector<positive_asks_t> vec_positive_asks;


	
	// フェーズ同士が同じものかどうか判定
	bool compare_phase(phase_t p1, phase_t p2);
	// nodeからedgeに遷移する可能性はあるか判定 通過可能性の判定
	ResultCheckOccurrence check_occurrence(phase_result_sptr_t node, phase_t edge, current_condition_t cc);

	// 現phase_resultがphase_resultsに含まれるか
	bool check_contain(phase_result_sptr_t result, current_condition_t cc);

	// ２つのphase_resultのphase、モジュール集合、positive_askが等しいかどうか判定
	bool compare_phase_result(phase_result_sptr_t r1, phase_result_sptr_t r2);
 
	// lsの中身表示
	void viewLs(current_condition_t cc);
	// phase_result_sptrs_tの中身表示
	void viewPrs(phase_result_sptrs_t results);
	// phase_result_sptr_tの中身表示
	void viewPr(phase_result_sptr_t result);
	// asksの中身表示
	void viewAsks(ask_set_t asks);
	
	// 生成された全てのHAを出力（dot言語）
	void output_ha();
	
	// phase_resultsをhaのdot言語に変換する
	void convert_phase_results_to_ha(phase_result_sptrs_t result, unknown_map_t unknown_map);
	
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


}//namespace hydla
}//namespace simulator 

#endif // _INCLUDED_HYDLA_HAConverter_SIMULATOR_H_

