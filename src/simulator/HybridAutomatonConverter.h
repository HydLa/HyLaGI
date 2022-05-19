#pragma once

#include "Automaton.h"
#include "ConsistencyChecker.h"
#include "Simulator.h"

#include <random>

namespace hydla {
namespace simulator {

typedef std::list<AutomatonNode *> HA_node_list_t;

class HybridAutomatonConverter : public Simulator {
public:
  HybridAutomatonConverter(Opts &opts);
  virtual ~HybridAutomatonConverter();
  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  virtual phase_result_sptr_t simulate();

private:
  bool check_including(AutomatonNode *larger, AutomatonNode *smaller);
  void HA_translate(phase_result_sptr_t current,
                    AutomatonNode *current_automaton_node,
                    HA_node_list_t created_nodes);
  AutomatonNode *create_phase_node(phase_result_sptr_t phase);
  AutomatonNode *transition(AutomatonNode *current, phase_result_sptr_t phase,
                            HA_node_list_t &trace_path);
  AutomatonNode *detect_loop(AutomatonNode *new_node,
                             HA_node_list_t trace_path);


  /**
   * @fn
   * @brief HA モードにおいてフェーズ間の初期値抽象化を利用した包含判定を行う
   * @param current: 今計算しているフェーズに対応する HA のノード
   * @param past: 過去に計算したフェーズに対応する HA のノード
   * @return 包含判定結果の true/false が返る
   */
  bool check_including_abstraction(AutomatonNode *current,
                                   AutomatonNode *past);

  /**
   * @fn
   * @brief HA モードにおいて初期値の抽象化を行い, フェーズ間の包含スコアを最大化する
   * @param current: 今計算しているフェーズに対応する HA のノード
   * @param past: 過去に計算したフェーズに対応する HA のノード
   * @param abstractTL: 自動抽象化に設ける時間制限
   * @param T0: 焼きなましにおける温度の上限（T1 は下限）
   * @return フェーズ間の包含を時間内に可能な限り最大化した際の包含スコア
   */
  double maximize_inclusion(AutomatonNode *current, 
                            AutomatonNode *past, 
                            double abstractTL, 
                            double T0, double T1
                           );

  /**
   * @fn
   * @brief どの程度大きいノードが小さいノードを含むかを示す包含スコアを計算する
   * @param current: 今計算したノード
   * @param past: 過去に計算済みのノード
   * @param new_parm_cons: 抽象化した上でパラメタに課す制約
   * @return 包含の度合いを0以上1以下の浮動小数点で表したものが返る
   */
  double calculate_inclusion_score(AutomatonNode *current, ConstraintStore current_param_cons,
                                   AutomatonNode *past, ConstraintStore past_param_cons
                                  );

  /**
   * @fn
   * パラメタの抽象化を行う
   * @brief パラメタの抽象化を行う
   * @todo フェーズ2以降で生成されたパラメタに対応できるようにする（現在のフェーズで抽象化されたパラメタは過去に生成されたパラメタを含むはずなので問題ないかも？）
   * @param state: 抽象化対象のノード
   * @param param_cons: 現在 state のパラメタに課している制約
   * @return 新しくパラメタに課す制約
   */
  ConstraintStore abstract_cp(AutomatonNode *state, ConstraintStore param_cons);

  Automaton current_automaton;
  std::list<Automaton> result_automata;
  double abstractionTL;
  std::random_device seed_gen;
  std::mt19937 engine;
};

} // namespace simulator
} // namespace hydla
