#ifndef _INCLUDED_HYDLA_ENTAILMENT_CHECKER_H_
#define _INCLUDED_HYDLA_ENTAILMENT_CHECKER_H_

#include "Node.h"
#include "PacketSender.h"
#include "mathlink_helper.h"
#include "ParseTree.h"
#include "Types.h"
#include <boost/shared_ptr.hpp>
#include "ConstraintStoreBuilderPoint.h"


namespace hydla {
namespace symbolic_simulator {

class EntailmentChecker
{
public:
  EntailmentChecker(MathLink& ml, bool debug_mode = true);

  virtual ~EntailmentChecker();

  bool check_entailment(
    const boost::shared_ptr<hydla::parse_tree::Ask>& negative_ask,
    hydla::symbolic_simulator::ConstraintStore& constraint_store);
private:
  MathLink& ml_;
  /// デバッグ出力をするかどうか
  bool               debug_mode_;

};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_ENTAILMENT_CHECKER_H__

