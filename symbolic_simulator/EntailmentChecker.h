#ifndef _INCLUDED_HYDLA_ENTAILMENT_CHECKER_H_
#define _INCLUDED_HYDLA_ENTAILMENT_CHECKER_H_

#include "Node.h"
#include "PacketSender.h"
#include "mathlink_helper.h"
#include "ParseTree.h"
#include <map>
#include "Types.h"
#include "TellCollector.h"
#include <boost/shared_ptr.hpp>


namespace hydla {
namespace symbolic_simulator {

class EntailmentChecker
{
public:
  EntailmentChecker(MathLink& ml);

  virtual ~EntailmentChecker();

  bool check_entailment(
    boost::shared_ptr<hydla::parse_tree::Ask> negative_ask,
    hydla::simulator::TellCollector::tells_t& collected_tells);

private:
  MathLink& ml_;

};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_ENTAILMENT_CHECKER_H__

