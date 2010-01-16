#ifndef _INCLUDED_HYDLA_CONSISTENCY_CHECKER_INTERVAL_H_
#define _INCLUDED_HYDLA_CONSISTENCY_CHECKER_INTERVAL_H_

#include "Node.h"
#include "PacketSender.h"
#include "mathlink_helper.h"
#include "ParseTree.h"
#include "Types.h"
#include "ConstraintStoreBuilderInterval.h"

namespace hydla {
namespace symbolic_simulator {

class ConsistencyCheckerInterval
{
public:
  ConsistencyCheckerInterval(MathLink& ml, bool debug_mode = true);

  virtual ~ConsistencyCheckerInterval();

  bool is_consistent(hydla::simulator::tells_t& collected_tells,
                     hydla::symbolic_simulator::ConstraintStoreInterval& constraint_store);

private:
  MathLink& ml_;
  /// デバッグ出力をするかどうか
  bool               debug_mode_;

};

} // namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_CONSISTENCY_CHECKER_INTERVAL_H_

