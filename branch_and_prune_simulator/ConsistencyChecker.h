#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_CONSISTENCY_CHECKER_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_CONSISTENCY_CHECKER_H_

#include <stack>
#include <boost/bimap/bimap.hpp>

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
#define RP_RELATION_UNEQUAL 4
#define RP_RELATION_SUP 5
#define RP_RELATION_INF 6

namespace hydla {
namespace bp_simulator {

class ConsistencyChecker : public parse_tree::TreeVisitor {
public:
  ConsistencyChecker();
  ConsistencyChecker(bool debug_mode);

  virtual ~ConsistencyChecker();

  bool is_consistent(hydla::simulator::TellCollector::tells_t& collected_tells);

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
  void create_unary_erep(boost::shared_ptr<hydla::parse_tree::UnaryNode> node, int op);
  void create_binary_erep(boost::shared_ptr<hydla::parse_tree::BinaryNode> node, int op);
  void create_ctr_num(boost::shared_ptr<hydla::parse_tree::BinaryNode> node, int rel);

  rp_vector_variable to_rp_vector();

  std::set<rp_constraint> constraints_;
  boost::bimaps::bimap<std::string, int> vars_;
  //std::map<std::string, int> vars_;

  std::stack<rp_erep> rep_stack_;
  rp_ctr_num ctr_;
  bool in_differential_;
  bool in_prev_;

  bool debug_mode_;

};

} //namespace bp_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_BP_SIMULATOR_CONSISTENCY_CHECKER_H__

