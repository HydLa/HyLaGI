#ifndef _INCLUDED_HYDLA_VCS_REDUCE_REDUCE_STRING_SENDER_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_REDUCE_STRING_SENDER_H_

#include <boost/shared_ptr.hpp>
#include <string>
#include <ostream>

#include "Node.h"
#include "TreeVisitor.h"
#include "ParseTree.h"
#include "REDUCELink.h"

// TODO:なんとかする
#include "../mathematica/PacketSender.h"
using namespace hydla::vcs::mathematica;


namespace hydla {
namespace vcs {
namespace reduce {

/**
 * ParseTreeのノード集合に対するVisitorクラス
 */
class REDUCEStringSender :
  public hydla::parse_tree::TreeVisitor
{
public:

  // TODO:何とかする
  /**
   * 変数データ
   * (変数名， 微分回数,  prev変数かどうか)
   */
  typedef PacketSender::var_info_t          var_info_t;
  typedef PacketSender::var_info_list_t     var_info_list_t;
  typedef PacketSender::vars_const_iterator vars_const_iterator;
  typedef PacketSender::node_sptr           node_sptr;


  REDUCEStringSender(REDUCELink& cl);

  virtual ~REDUCEStringSender();

  vars_const_iterator vars_begin() const { return vars_.begin(); }

  vars_const_iterator vars_end() const { return vars_.end(); }

  /**
   * 与えられたノードの送信をおこなう
   *
   * ノードの送信をおこなう際は直接visit関数を呼ばずに，
   * 必ずこの関数を経由すること
   */
  void put_node(const node_sptr& node, bool ignore_prev = false, bool entailed = true);

  /**
   * 変数の送信
   */
  void put_var(const var_info_t var, bool init_var = false);

  /**
   * put_nodeの際に送信された変数群の送信をおこなう
   */
  void put_vars(bool ignore_prev = false);

  /**
   * put_nodeの際に送信された変数群のデータを消去し，初期化する
   */
  void clear();


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
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Power> node);

  // 算術単項演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);

  // 微分
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // 左極限
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);

  // 否定
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Not> node);

  // 変数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // 数字
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);

  // 記号定数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parameter> node);

  // t
  virtual void visit(boost::shared_ptr<hydla::parse_tree::SymbolicT> node);


private:
  REDUCELink& cl_;

  /// 送信された変数の一覧
  var_info_list_t vars_;

  // Differentialノードを何回通ったか
  int differential_count_;

  /// Prevノードの下にいるかどうか
  bool in_prev_;

  /// PreviousPointノードの下にいるかどうか
  bool in_prev_point_;

  // prev制約を無視するかどうか
  bool ignore_prev_;
  

};

} //namespace reduce
} //namespace vcs
} //namespace hydla

#endif //_INCLUDED_HYDLA_VCS_REDUCE_REDUCE_STRING_SENDER_H_
