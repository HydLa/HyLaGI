#ifndef _INCLUDED_HYDLA_PARSE_TREE_GRAPHVIZ_DUMPER_H_
#define _INCLUDED_HYDLA_PARSE_TREE_GRAPHVIZ_DUMPER_H_

#include <ostream>
#include <map>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "TreeVisitor.h"
#include "ParseTree.h"

namespace hydla { 
namespace parser {
  
/**
 * ParseTree‚ğdotŒ¾ŒêŒ`®‚Åƒ_ƒ“ƒv‚·‚é
 */
class ParseTreeGraphvizDumper : 
  public hydla::parse_tree::TreeVisitor
{
public:
  typedef hydla::parse_tree::node_sptr node_sptr;

  ParseTreeGraphvizDumper();

  virtual ~ParseTreeGraphvizDumper();

  /**
   * dotŒ¾ŒêŒ`®‚Å‚Ìo—Í‚ğ‚¨‚±‚È‚¤
   */
  std::ostream& dump(std::ostream& s, const node_sptr& node);
  
  // ’è‹`
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintDefinition> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ProgramDefinition> node);

  // ŒÄ‚Ño‚µ
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node);

  // §–ñ®
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Constraint> node);

  // Ask§–ñ
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // Tell§–ñ
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);

  // ”äŠr‰‰Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Equal> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Less> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Greater> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node);

  // ˜_—‰‰Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalOr> node);
  
  // Zp“ñ€‰‰Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Times> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Divide> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Power> node);
  
  // Zp’P€‰‰Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);
  
  // §–ñŠK‘w’è‹`‰‰Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Weaker> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parallel> node);
  
  // ‘Š‰‰Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Always> node);
  
  // ”÷•ª
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // ¶‹ÉŒÀ
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);
  
  
  // OŠpŠÖ”
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Sin> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Cos> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tan> node);
  // ‹tOŠpŠÖ”
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Asin> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Acos> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Atan> node);
  // ‰~ü—¦
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Pi> node);
  // ‘Î”
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Log> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ln> node);
  // ©‘R‘Î”‚Ì’ê
  virtual void visit(boost::shared_ptr<hydla::parse_tree::E> node);
  // ”CˆÓ‚Ì•¶š—ñ
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ArbitraryBinary> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ArbitraryUnary> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ArbitraryFactor> node);

  // •Ï”
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // ”š
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);


private:
  void dump_node(boost::shared_ptr<hydla::parse_tree::FactorNode> node);
  void dump_node(boost::shared_ptr<hydla::parse_tree::UnaryNode> node);
  void dump_node(boost::shared_ptr<hydla::parse_tree::BinaryNode> node);

  typedef int         graph_node_id_t;
  typedef std::string graph_node_info_t;

  graph_node_id_t node_id_;
  std::map<graph_node_id_t, graph_node_info_t>    nodes_;
  std::multimap<graph_node_id_t, graph_node_id_t> edges_;
};

} //namespace parser
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_GRAPHVIZ_DUMPER_H_