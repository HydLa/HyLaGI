#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_ENTAILMENT_CHECKER_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_ENTAILMENT_CHECKER_H_

#include <map>
#include <boost/shared_ptr.hpp>

// parser
#include "Node.h"
#include "TreeVisitor.h"
#include "ParseTree.h"

// simulator
#include "Types.h"
#include "TellCollector.h"

#include "ConstraintBuilder.h"
#include "ConstraintStore.h"

using namespace hydla::parse_tree;

namespace hydla {
namespace bp_simulator {

/* ê^ÅEãUÅEïsñæ */
typedef enum Trivalent_ {FALSE, UNKNOWN, TRUE} Trivalent;


class EntailmentChecker : public ConstraintBuilder {
public:
  EntailmentChecker();
  EntailmentChecker(bool debug_mode);

  virtual ~EntailmentChecker();

  Trivalent check_entailment(
    const boost::shared_ptr<hydla::parse_tree::Ask>& negative_ask,
    hydla::simulator::tells_t& collected_tells,
    ConstraintStore& constraint_store);

  // AskêßñÒ
  virtual void visit(boost::shared_ptr<Ask> node);
  // TellêßñÒ
  virtual void visit(boost::shared_ptr<Tell> node);

  // î‰ärââéZéq
  virtual void visit(boost::shared_ptr<Equal> node);
  virtual void visit(boost::shared_ptr<UnEqual> node);
  virtual void visit(boost::shared_ptr<Less> node);
  virtual void visit(boost::shared_ptr<LessEqual> node);
  virtual void visit(boost::shared_ptr<Greater> node);
  virtual void visit(boost::shared_ptr<GreaterEqual> node);

  // ò_óùââéZéq
  virtual void visit(boost::shared_ptr<LogicalAnd> node);
  //virtual void visit(boost::shared_ptr<LogicalOr> node);

private:
  std::set<rp_constraint> guards_, not_guards;
  bool debug_mode_;

};

} //namespace bp_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_BP_SIMULATOR_ENTAILMENT_CHECKER_H__

