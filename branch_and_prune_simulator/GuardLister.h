#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_GUARD_LISTER_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_GUARD_LISTER_H_

#include <set>

#include "Node.h"
#include "TreeVisitor.h"
#include "ParseTree.h"

namespace hydla {
namespace bp_simulator {

class GuardLister : public parse_tree::TreeVisitor {
public:
  GuardLister();
  virtual ~GuardLister();

  std::set<parse_tree::node_sptr> get_guard_list(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // AskêßñÒ
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // î‰ärââéZéq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Equal> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Less> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Greater> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node);

  // ò_óùââéZéq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);

private:
  std::set<parse_tree::node_sptr> nodes_;
};

} // namespace bp_simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_BP_SIMULATOR_GUARD_LISTER_H_