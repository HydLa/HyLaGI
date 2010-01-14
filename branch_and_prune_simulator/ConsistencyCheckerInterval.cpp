#include "ConsistencyCheckerInterval.h"

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace bp_simulator {

  ConsistencyCheckerInterval::ConsistencyCheckerInterval()
  {
  }

  ConsistencyCheckerInterval::~ConsistencyCheckerInterval()
  {
  }

  bool ConsistencyCheckerInterval::is_consistent(tells_t& collected_tells,
                                                 ConstraintStoreInterval& constraint_store)
  {
    return true;
  }

  // TellêßñÒ
  void ConsistencyCheckerInterval::visit(boost::shared_ptr<Tell> node)
  {
  }


}
}
