#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_CONSISTENCY_CHECKER_INTERVAL_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_CONSISTENCY_CHECKER_INTERVAL_H_

#include "BPTypes.h"
#include "ConstraintStoreInterval.h"

// parser
#include "Node.h"
#include "TreeVisitor.h"
#include "ParseTree.h"

// simulator
#include "TellCollector.h"

// librealpaverbasic
#include "realpaverbasic.h"

// symbolic_simulator
#include "../symbolic_simulator/mathlink_helper.h"

namespace hydla {
namespace bp_simulator {

class ConsistencyCheckerInterval : public parse_tree::TreeVisitor {
public:
  ConsistencyCheckerInterval();

  virtual ~ConsistencyCheckerInterval();

  bool is_consistent(simulator::tells_t& collected_tells,
    ConstraintStoreInterval& constraint_store);

  // TellêßñÒ
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);

  // î‰ärââéZéq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Equal> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Less> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Greater> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node);

  // éZèpìÒçÄââéZéq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Times> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Divide> node);

  // éZèpíPçÄââéZéq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);

  // î˜ï™
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // ç∂ã…å¿
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);

  // ïœêî
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // êîéö
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);

private:
  var_name_map_t vars_;
  rp_ctr_num ctr_;
  bool in_prev_;
  unsigned int derivative_count_;
  std::set<rp_constraint> constraints_;
  MathLink ml_;
  std::string send_expression_str;

};

}
}

#endif //_INCLUDED_HYDLA_BP_SIMULATOR_CONSISTENCY_CHECKER_INTERVAL_H_