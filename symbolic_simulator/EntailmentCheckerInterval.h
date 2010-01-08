#ifndef _INCLUDED_HYDLA_ENTAILMENT_CHECKER_INTERVAL_H_
#define _INCLUDED_HYDLA_ENTAILMENT_CHECKER_INTERVAL_H_

#include "Node.h"
#include "PacketSenderInterval.h"
#include "mathlink_helper.h"
#include "ParseTree.h"
#include <map>
#include "Types.h"
#include "TellCollector.h"
#include <boost/shared_ptr.hpp>
#include "ConstraintStoreBuilderInterval.h"


namespace hydla {
namespace symbolic_simulator {

class EntailmentCheckerInterval
{
public:
  EntailmentCheckerInterval(MathLink& ml, bool debug_mode);

  virtual ~EntailmentCheckerInterval();

  bool check_entailment(
    const boost::shared_ptr<hydla::parse_tree::Ask>& negative_ask,
    hydla::simulator::tells_t& collected_tells,
    hydla::symbolic_simulator::ConstraintStore& constraint_store);
private:
  MathLink& ml_;
  /// デバッグ出力をするかどうか
  bool               debug_mode_;

};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_ENTAILMENT_CHECKER_INTERVAL_H_

