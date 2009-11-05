#ifndef _INCLUDED_HYDLA_COLLECT_TELL_VISITOR_H_
#define _INCLUDED_HYDLA_COLLECT_TELL_VISITOR_H_

#include "Node.h"
#include "TreeVisitor.h"
#include "mathlink_helper.h"

namespace hydla {
namespace symbolic_simulator {

class CollectTellVisitor : public parse_tree::TreeVisitor {
public:
  bool is_consinstent(MathLink& ml);
};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_COLLECT_TELL_VISITOR_H__

