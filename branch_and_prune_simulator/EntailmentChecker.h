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

  // ïœêî
  virtual void visit(boost::shared_ptr<Variable> node);

private:
  std::set<rp_constraint> guards_, not_guards_;
  std::set<rp_constraint> constraints_;
  std::set<std::string> prevs_in_guard_;
  // protected boost::bimaps::bimap<std::string, int> vars_;
  bool is_tell_ctr_;
  bool debug_mode_;

  void create_initial_box(rp_box *b);
  bool is_guard_about_undefined_prev();
  bool solve_hull(std::set<rp_constraint> c, rp_box b);
  std::set<rp_constraint> copy_constraints();
  rp_vector_variable to_rp_vector();

};

} //namespace bp_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_BP_SIMULATOR_ENTAILMENT_CHECKER_H__

