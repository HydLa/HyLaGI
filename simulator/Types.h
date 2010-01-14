#ifndef _INCLUDED_HYDLA_SIMULATOR_TYPES_H_
#define _INCLUDED_HYDLA_SIMULATOR_TYPES_H_

#include <vector>
#include <set>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "ModuleSet.h"
#include "ModuleSetContainer.h"

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

typedef hydla::parse_tree::node_id_t                      node_id_t;
typedef boost::shared_ptr<hydla::ch::ModuleSet>           module_set_sptr;
typedef boost::shared_ptr<hydla::ch::ModuleSetContainer>  module_set_container_sptr;

typedef std::vector<boost::shared_ptr<hydla::parse_tree::Tell> > tells_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Tell> >    collected_tells_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Always> >  expanded_always_t;
typedef std::vector<node_id_t>                                   expanded_always_id_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Ask> >     positive_asks_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Ask> >     negative_asks_t;
typedef std::vector<std::pair<AskState, node_id_t> >             changed_asks_t;

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_TYPES_H_

