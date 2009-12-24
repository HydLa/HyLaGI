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


using namespace hydla::parse_tree;

namespace hydla {
namespace bp_simulator {

/* ^E‹UE•s–¾ */
typedef enum Trivalent_ {FALSE, UNKNOWN, TRUE} Trivalent;


class EntailmentChecker : public parse_tree::TreeVisitor {
public:
  EntailmentChecker();

  virtual ~EntailmentChecker();

  Trivalent check_entailment(
    const boost::shared_ptr<hydla::parse_tree::Ask>& negative_ask,
    hydla::simulator::tells_t& collected_tells);

  // Tell§–ñ
  virtual void visit(boost::shared_ptr<Tell> node);

  // ”äŠr‰‰Zq
  virtual void visit(boost::shared_ptr<Equal> node);
  virtual void visit(boost::shared_ptr<UnEqual> node);
  virtual void visit(boost::shared_ptr<Less> node);
  virtual void visit(boost::shared_ptr<LessEqual> node);
  virtual void visit(boost::shared_ptr<Greater> node);
  virtual void visit(boost::shared_ptr<GreaterEqual> node);

  // ˜_—‰‰Zq
  virtual void visit(boost::shared_ptr<LogicalAnd> node);
  virtual void visit(boost::shared_ptr<LogicalOr> node);

  // Zp“ñ€‰‰Zq
  virtual void visit(boost::shared_ptr<Plus> node);
  virtual void visit(boost::shared_ptr<Subtract> node);
  virtual void visit(boost::shared_ptr<Times> node);
  virtual void visit(boost::shared_ptr<Divide> node);

  // Zp’P€‰‰Zq
  virtual void visit(boost::shared_ptr<Negative> node);
  virtual void visit(boost::shared_ptr<Positive> node);

  // ”÷•ª
  virtual void visit(boost::shared_ptr<Differential> node);

  // ¶‹ÉŒÀ
  virtual void visit(boost::shared_ptr<Previous> node);
  
  // •Ï”
  virtual void visit(boost::shared_ptr<Variable> node);

  // ”š
  virtual void visit(boost::shared_ptr<Number> node);


private:
  std::map<std::string, int> vars_;
  int in_differential_equality_;
  int in_differential_;

};

} //namespace bp_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_BP_SIMULATOR_ENTAILMENT_CHECKER_H__

