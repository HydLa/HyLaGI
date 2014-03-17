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
 * ParseTreeをdot言語形式でダンプする
 */
class ParseTreeGraphvizDumper : 
  public hydla::parse_tree::TreeVisitor
{
public:
  typedef hydla::parse_tree::node_sptr node_sptr;

  ParseTreeGraphvizDumper();

  virtual ~ParseTreeGraphvizDumper();

  /**
   * dot言語形式での出力をおこなう
   */
  std::ostream& dump(std::ostream& s, const node_sptr& node);
  
  // 定義
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintDefinition> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ProgramDefinition> node);

  // 呼び出し
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node);

  // 制約式
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Constraint> node);

  // Ask制約
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // Tell制約
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);

  // 比較演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Equal> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Less> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Greater> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node);

  // 論理演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalOr> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Not> node);
  
  // 算術二項演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Times> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Divide> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Power> node);
  
  // 算術単項演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);
  
  // 制約階層定義演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Weaker> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parallel> node);
  
  // 時相演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Always> node);
  
  // 微分
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // 左極限
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);
 
  //Print 
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Print> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::PrintPP> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::PrintIP> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Scan> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Exit> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Abort> node);

  //SystemVariable
  virtual void visit(boost::shared_ptr<hydla::parse_tree::SVtimer> node);

  virtual void visit(boost::shared_ptr<hydla::parse_tree::True> node);
  
  virtual void visit(boost::shared_ptr<hydla::parse_tree::False> node);
  //関数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Function> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnsupportedFunction> node);
  
  // 円周率
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Pi> node);
  // 自然対数の底
  virtual void visit(boost::shared_ptr<hydla::parse_tree::E> node);
  // 変数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // 数字
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Float> node);

  // Parameter
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parameter> node);
  // SymbolicT
  virtual void visit(boost::shared_ptr<hydla::parse_tree::SymbolicT> node);
  // Infinity
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Infinity> node);

private:
  void dump_node(boost::shared_ptr<hydla::parse_tree::FactorNode> node);
  void dump_node(boost::shared_ptr<hydla::parse_tree::UnaryNode> node);
  void dump_node(boost::shared_ptr<hydla::parse_tree::BinaryNode> node);
  void dump_node(boost::shared_ptr<hydla::parse_tree::ArbitraryNode> node);

  typedef int         graph_node_id_t;
  typedef std::string graph_node_info_t;

  graph_node_id_t node_id_;
  std::map<graph_node_id_t, graph_node_info_t>    nodes_;
  std::multimap<graph_node_id_t, graph_node_id_t> edges_;
};

} //namespace parser
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_GRAPHVIZ_DUMPER_H_
