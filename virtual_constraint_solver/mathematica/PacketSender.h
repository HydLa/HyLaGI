#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_PACKET_SENDER_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_PACKET_SENDER_H_

#include <map>

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/tuple/tuple_io.hpp>

#include "Node.h"
#include "TreeVisitor.h"
#include "mathlink_helper.h"
#include "ParseTree.h"

namespace hydla {
namespace vcs {
namespace mathematica {

class PacketSender : 
  public hydla::parse_tree::TreeVisitor
{
public:
  enum now_phase_t 
  { 
    NP_POINT_PHASE, 
    NP_INTERVAL_PHASE,
  };

  /**
   * 変数データ
   * (変数名， 微分回数,  prev変数かどうか)
   */
  typedef boost::tuple<std::string, int, bool> var_info_t;

  typedef std::set<var_info_t>                 var_info_list_t;
  typedef var_info_list_t::const_iterator      vars_const_iterator;
  typedef hydla::parse_tree::node_sptr         node_sptr;

  // Mathematicaに送る際に変数名につける接頭語 "usrVar"
  static const std::string var_prefix;

  PacketSender(MathLink& ml, now_phase_t phase);

  virtual ~PacketSender();

  vars_const_iterator vars_begin() const { return vars_.begin(); }

  vars_const_iterator vars_end() const { return vars_.end(); }

  /**
   * 与えられたノードの送信をおこなう
   *
   * ノードの送信をおこなう際は直接visit関数を呼ばずに，
   * 必ずこの関数を経由すること
   */
  void put_node(const node_sptr& node);

  /**
   * put_nodeの際に送信された変数群の送信をおこなう
   */
  void put_vars();

  /**
   * put_nodeの際に送信された変数群のデータを消去し，初期化する
   */
  void clear();

  /**
   * 変数の送信
   */
  void put_var(const var_info_t var);


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

  /// 送信された変数の一覧
  var_info_list_t vars_;

  // Differentialノードを何回通ったか
  int differential_count_;

  /// Prevノードの下にいるかどうか
  bool in_prev_;

  now_phase_t phase_; // 現在のフェーズ
};

} // namespace mathematica
} // namespace simulator
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

#endif //_INCLUDED_HYDLA_VCS_MATHEMATICA_PACKET_SENDER_H_

