#ifndef _INCLUDED_HYDLA_SIMULATOR_TYPES_H_
#define _INCLUDED_HYDLA_SIMULATOR_TYPES_H_

#include <vector>
#include <set>
#include <list>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "ModuleSet.h"
#include "ModuleSetContainer.h"
#include "ParseTree.h"

namespace hydla {
namespace simulator {

/**
 * 処理のフェーズ
 */
typedef enum Phase_ {
  PointPhase,
  IntervalPhase,
} Phase;

/**
 * 変化したask制約の状態
 */
typedef enum AskState_ {
  Positive2Negative,
  Negative2Positive,
} AskState;


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


typedef enum OutputFormat_ {
  fmtTFunction,
  fmtNumeric,
  fmtMathematica,
  fmtNInterval,
} OutputFormat;
  

typedef struct Opts_ {
  std::string mathlink;
  bool debug_mode;
  std::string max_time;
  int max_step;
  bool nd_mode;
  bool interactive_mode;
  bool time_measurement;
  bool profile_mode;
  bool parallel_mode;
  OutputFormat output_format;
  bool dump_in_progress;
  bool exclude_error;
  std::string output_interval;
  int output_precision;
  int approx_precision;
  std::string solver;
  hydla::parse_tree::node_sptr assertion;
  DefaultContinuity default_continuity;
  std::set<std::string> output_variables;
} Opts;

/**
 * 
 */
typedef enum{
  TIME_LIMIT,
  SOME_ERROR,
  INCONSISTENCY,
  ASSERTION,
  NOT_UNIQUE_IN_INTERVAL,
  NONE
}CauseOfTermination;

typedef hydla::parse_tree::node_id_t                      node_id_t;
typedef boost::shared_ptr<hydla::ch::ModuleSet>           module_set_sptr;
typedef hydla::ch::ModuleSetContainer                     module_set_container_t;
typedef boost::shared_ptr<module_set_container_t>  module_set_container_sptr;
typedef hydla::ch::ModuleSetContainer::module_set_list_t  module_set_list_t;

typedef std::vector<boost::shared_ptr<hydla::parse_tree::Node> > constraints_t;
typedef std::vector<boost::shared_ptr<hydla::parse_tree::Tell> > tells_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Tell> >    collected_tells_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Always> >  expanded_always_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Ask> >     ask_set_t;
typedef ask_set_t                                                positive_asks_t;
typedef ask_set_t                                                negative_asks_t;
typedef std::vector<std::pair<AskState, node_id_t> >             changed_asks_t;
typedef std::vector<tells_t>                                     not_adopted_tells_list_t;
typedef std::map<std::string, int>                               continuity_map_t;


typedef boost::shared_ptr<hydla::parse_tree::ParseTree>  parse_tree_sptr;

typedef boost::shared_ptr<const hydla::ch::ModuleSet>    module_set_const_sptr;
typedef boost::shared_ptr<hydla::ch::ModuleSetContainer> module_set_container_sptr;  


std::ostream& operator<<(std::ostream& s, const ask_set_t& a);
std::ostream& operator<<(std::ostream& s, const tells_t& a);
std::ostream& operator<<(std::ostream& s, const collected_tells_t& a);
std::ostream& operator<<(std::ostream& s, const expanded_always_t& a);
std::ostream& operator<<(std::ostream& s, const continuity_map_t& continuity_map);

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_TYPES_H_

