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

/* 真・偽・不明 */
typedef enum Trivalent_ {FALSE, UNKNOWN, TRUE} Trivalent;


class EntailmentChecker : protected GuardConstraintBuilder {
public:
  EntailmentChecker(bool debug_mode = false);

  virtual ~EntailmentChecker();

  Trivalent check_entailment(
    const boost::shared_ptr<Ask>& negative_ask,
    ConstraintStore& constraint_store);

  // 変数
  virtual void visit(boost::shared_ptr<Variable> node);

private:
  std::set<rp_constraint> constraints_;
  std::set<std::string> prevs_in_guard_;
  // protected boost::bimaps::bimap<std::string, int> vars_;
  bool debug_mode_;

  void finalize();
  void create_initial_box(rp_box *b);
  bool is_guard_about_undefined_prev();
  bool solve_hull(std::set<rp_constraint> c, rp_box b);
  //std::set<rp_constraint> copy_constraints();

};

} //namespace bp_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_BP_SIMULATOR_ENTAILMENT_CHECKER_H__
