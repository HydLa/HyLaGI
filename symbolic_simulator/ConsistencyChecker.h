#ifndef _INCLUDED_HYDLA_CONSISTENCY_CHECKER_H_
#define _INCLUDED_HYDLA_CONSISTENCY_CHECKER_H_

#include "Node.h"
#include "PacketSender.h"
#include "mathlink_helper.h"
#include "ParseTree.h"
#include <map>
#include "TellCollector.h"
#include "ConstraintStoreBuilderPoint.h"

namespace hydla {
namespace symbolic_simulator {

class ConsistencyChecker
{
public:
  ConsistencyChecker(MathLink& ml, bool debug_mode = true);

  virtual ~ConsistencyChecker();

  bool is_consistent(hydla::simulator::tells_t& collected_tells,
                     hydla::symbolic_simulator::ConstraintStore& constraint_store);

private:
  MathLink& ml_;
  /// �f�o�b�O�o�͂����邩�ǂ���
  bool               debug_mode_;

};

} // namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_CONSISTENCY_CHECKER_H_

