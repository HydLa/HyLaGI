#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_CONSTRAINT_BUILDER_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_CONSTRAINT_BUILDER_H_

#include <stack>
#include <boost/bimap/bimap.hpp>

#include "BPTypes.h"

// parser
#include "Node.h"
#include "TreeVisitor.h"
#include "ParseTree.h"

// simulator
#include "TellCollector.h"

// librealpaverbasic
#include "realpaverbasic.h"

//#define RP_RELATION_EQUAL     1
//#define RP_RELATION_SUPEQUAL  2
//#define RP_RELATION_INFEQUAL  3
//#define RP_RELATION_UNEQUAL 4
//#define RP_RELATION_SUP 5
//#define RP_RELATION_INF 6

namespace hydla {
namespace bp_simulator {

class ConstraintBuilder : public parse_tree::TreeVisitor {
public:
  ConstraintBuilder();

  virtual ~ConstraintBuilder();

  // ärZq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Equal> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Less> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Greater> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node);

  // ZpñZq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Times> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Divide> node);

  // ZpPZq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);

  // ÷ª
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // ¶ÉÀ
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);

  // Ï
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // 
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);

protected:
  var_name_map_t vars_;
  rp_ctr_num ctr_;
  bool in_prev_;
  unsigned int derivative_count_;

  void create_ctr_num(boost::shared_ptr<hydla::parse_tree::BinaryNode> node, int rel);
  rp_vector_variable to_rp_vector() const;

private:
  void create_unary_erep(boost::shared_ptr<hydla::parse_tree::UnaryNode> node, int op);
  void create_binary_erep(boost::shared_ptr<hydla::parse_tree::BinaryNode> node, int op);

  std::stack<rp_erep> rep_stack_;

};

class GuardConstraintBuilder : public ConstraintBuilder {
public:

  void create_guard_expr(boost::shared_ptr<hydla::parse_tree::Ask> node,
    std::set<rp_constraint>& guards,
    std::set<rp_constraint>& not_guards,
    var_name_map_t& vars);

  // Ask§ñ
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // ärZq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Equal> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Less> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Greater> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node);

  // _Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);

protected:
  std::set<rp_constraint> guards_;
  std::set<rp_constraint> not_guards_;

};

} //namespace bp_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_BP_SIMULATOR_CONSTRAINT_BUILDER_H__
