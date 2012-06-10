#ifndef _INCLUDED_SYMBOLIC_TYPES_H_
#define _INCLUDED_SYMBOLIC_TYPES_H_

#include "ParseTree.h"

#include "Simulator.h"
#include "DefaultVariable.h"

#include "SymbolicValue.h"
#include "ValueRange.h"
#include "DefaultParameter.h"

#include "Types.h"

namespace hydla {
namespace symbolic_simulator {

  
  typedef simulator::node_id_t                   node_id_t;
  typedef simulator::module_set_sptr             module_set_sptr;
  typedef simulator::module_set_container_sptr   module_set_container_sptr;
  typedef simulator::module_set_list_t           module_set_list_t;

  typedef SymbolicValue                          value_t;
  typedef SymbolicValue                          time_t;
  typedef simulator::PhaseState<value_t>         phase_state_t;
  typedef phase_state_t::phase_state_sptr_t      phase_state_sptr_t;
  typedef std::vector<phase_state_sptr_t>        phase_state_sptrs_t;
  typedef simulator::DefaultVariable             variable_t;
  typedef simulator::DefaultParameter<value_t>   parameter_t;
  typedef simulator::ValueRange<value_t>         value_range_t;
  typedef simulator::VariableMap<variable_t*, 
                                        value_t> variable_map_t;
  typedef simulator::VariableMap<parameter_t*, 
                                        value_range_t> parameter_map_t;
  typedef simulator::continuity_map_t            continuity_map_t;
  typedef simulator::constraints_t               constraints_t;
  typedef simulator::tells_t                     tells_t;
  typedef simulator::collected_tells_t           collected_tells_t;
  typedef simulator::expanded_always_t           expanded_always_t;
  typedef simulator::expanded_always_id_t        expanded_always_id_t;
  typedef simulator::ask_set_t                   ask_set_t;
  typedef simulator::positive_asks_t             positive_asks_t;
  typedef simulator::negative_asks_t             negative_asks_t;
  typedef simulator::not_adopted_tells_list_t    not_adopted_tells_list_t;
  typedef simulator::continuity_map_t            continuity_map_t;
  typedef simulator::Simulator<phase_state_t>::variable_set_t  variable_set_t;
  typedef simulator::Simulator<phase_state_t>::parameter_set_t  parameter_set_t;

                                        

  typedef boost::shared_ptr<phase_state_t> phase_state_sptr;
  
  typedef simulator::Simulator<phase_state_t> simulator_t;
  
  
  typedef enum CalculateClosureResult_ {
    CC_TRUE,
    CC_FALSE,
    CC_BRANCH
  } CalculateClosureResult;

  typedef enum OutputFormat_ {
    fmtTFunction,
    fmtNumeric,
    fmtMathematica,
    fmtNInterval,
  } OutputFormat;
  
  typedef enum OutputStyle_ {
    styleTree,
    styleList,
  } OutputStyle;
  
  typedef enum Mode_{
    ContinuousMode,
    DiscreteMode,
  } Mode;
  
  /*
   * デフォルト連続性
   */
  typedef enum DefaultContinuity_{
    CONT_NONE = 0,  // 連続性を仮定しない（現状ではIPの連続性を明示する方法が無いため，ほぼ使用不能）
    CONT_WEAK,      // 制約ストアに微分値に関する制約が入っていたら，その微分回数未満についてはすべて連続
    CONT_GUARD,       // WEAKに加え，ガード条件の後件までを見て連続性制約を追加する
    CONT_STRONG_IP, // WEAKに加え，「何も言及されていなければ全部そのまま(最大微分回数＋１ = ０）」をIP限定で
    CONT_STRONG,    // 上記をIPでもPPでも有効にする
    CONT_NUM        // デフォルト連続性の種類の総数
  } DefaultContinuity;

  typedef struct Opts_ {
    std::string mathlink;
    bool debug_mode;
    std::string max_time;
    int max_step;
    bool nd_mode;
    OutputStyle output_style;
    bool interactive_mode;
    bool profile_mode;
    bool parallel_mode;
    OutputFormat output_format;
    bool dump_in_progress;
    bool exclude_error;
    time_t output_interval;
    int output_precision;
    int approx_precision;
    std::string solver;
    hydla::parse_tree::node_sptr assertion;
    DefaultContinuity default_continuity;
    std::set<std::string> output_variables;
  } Opts;

} // namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_SYMBOLIC_TYPES_H_
