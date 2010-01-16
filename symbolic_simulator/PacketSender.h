#ifndef _INCLUDED_HYDLA_PACKET_SENDER_H_
#define _INCLUDED_HYDLA_PACKET_SENDER_H_

#include <map>

#include "Node.h"
#include "TreeVisitor.h"
#include "mathlink_helper.h"
#include "ParseTree.h"


namespace hydla {
namespace symbolic_simulator {

enum now_phase_t { NP_POINT_PHASE, NP_INTERVAL_PHASE };

class PacketSender : public hydla::parse_tree::TreeVisitor
{
public:
  typedef std::pair<std::string, int> var_info_t;
  typedef std::set<var_info_t>::const_iterator const_iterator;

  typedef hydla::parse_tree::node_sptr node_sptr;

  // Mathematicaに送る際に変数名につける接頭語 "usrVar"
  static const std::string var_prefix;

  // var_info_tに対する便利関数
  static const std::string get_var_name(const var_info_t vi)
  {
    return vi.first;
  }

  // var_info_tに対する便利関数
  static const int get_var_differential_count(const var_info_t vi)
  {
    return std::abs(vi.second)-1;
  }

  // var_info_tに対する便利関数
  static const bool is_var_prev(const var_info_t vi)
  {
    return (vi.second < 0);
  }

  PacketSender(MathLink& ml, now_phase_t phase=NP_POINT_PHASE);

  virtual ~PacketSender();

  const_iterator begin() const { return vars_.begin(); }

  const_iterator end() const { return vars_.end(); }

  void put_node(const node_sptr& node);

  void put_vars();

  //void put_cs(ConstraintStoreInterval constraint_store);

  //void put_cs_vars(ConstraintStoreInterval constraint_store);

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

  // 算術二項演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Times> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Divide> node);

  // 算術単項演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);

  // 微分
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // 左極限
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);
  
  // 変数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // 数字
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);

private:
  MathLink& ml_;

  // pair<変数名, (prev変数なら-1*)微分回数+1>
  std::set<std::pair<std::string, int> > vars_;

  std::string vars_str_;

  // Differentialノードを何回通ったか
  int differential_count_;

  /// Prevノードの下にいるかどうか（通常変数なら1、prev変数だと-1などにするか？）
  bool in_prev_;

  now_phase_t phase_; // 現在のフェーズ

  std::string debug_string_; // デバッグ出力用一時変数

};

} // namespace symbolic_simulator
} // namespace hydla



//#include "Node.h"
//#include "TreeVisitor.h"
//#include "mathlink_helper.h"
//#include "ParseTree.h"
//#include <map>
//#include "ConstraintStoreBuilderPoint.h"
//
//
//namespace hydla {
//namespace symbolic_simulator {
//
//class PacketSender : public parse_tree::TreeVisitor
//{
//public:
//
//  PacketSender(MathLink& ml, bool debug_mode);
//
//  virtual ~PacketSender();
//
//  void put_vars();
//
//  void put_cs(ConstraintStore constraint_store);
//
//  void put_cs_vars(ConstraintStore constraint_store);
//
//  // Ask制約
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);
//
//  // Tell制約
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);
//
//  // 比較演算子
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Equal> node);
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node);
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Less> node);
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node);
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Greater> node);
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node);
//
//  // 論理演算子
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalOr> node);
//
//  // 算術二項演算子
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Plus> node);
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Subtract> node);
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Times> node);
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Divide> node);
//
//  // 算術単項演算子
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);
//
//  // 微分
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);
//
//  // 左極限
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);
//  
//  // 変数
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);
//
//  // 数字
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);
//
//
//private:
//  MathLink& ml_;
//  std::set<std::pair<std::string, int> > vars_;
//  std::string vars_str_;
//  /// Differentialノードを何回通ったか
//  int differential_count_;
//  /// Prevノードの下にいるかどうか
//  // （通常変数なら1、prev変数だと-1などにするか？）
//  bool in_prev_;
//  /// デバッグ出力をするかどうか
//  bool debug_mode_;
//
//};
//
//} //namespace symbolic_simulator
//} // namespace hydla

#endif //_INCLUDED_HYDLA_PACKET_SENDER_H_

