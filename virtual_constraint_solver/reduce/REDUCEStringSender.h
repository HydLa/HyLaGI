#ifndef _INCLUDED_HYDLA_VCS_REDUCE_REDUCE_STRING_SENDER_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_REDUCE_STRING_SENDER_H_

#include <boost/shared_ptr.hpp>
#include <string>
#include <ostream>

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

  typedef std::map<std::string, int> max_diff_map_t;

  // TODO:何とかする
  /**
   * 変数データ
   * (変数名， 微分回数,  prev変数かどうか)
   */
  typedef PacketSender::var_info_t          var_info_t;
  typedef PacketSender::var_info_list_t     var_info_list_t;
  typedef PacketSender::vars_const_iterator vars_const_iterator;
  typedef PacketSender::node_sptr           node_sptr;

  // REDUCEに送る際に変数名につける接頭語 "usrvar"
  static const std::string var_prefix;
  // REDUCEに送る際に定数名につける接頭語
  static const std::string par_prefix;
  // 空集合を表すREDUCE入力用文字列 "{}"
  static const std::string empty_list_string;

  REDUCEStringSender();
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

  /**
   * 変数の最大微分回数をもとめる
   */
  void create_max_diff_map(max_diff_map_t& max_diff_map);


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

  // 三角関数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Sin> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Cos> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tan> node);
  // 逆三角関数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Asin> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Acos> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Atan> node);
  // 円周率
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Pi> node);
  // 対数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Log> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ln> node);
  // 自然対数の底
  virtual void visit(boost::shared_ptr<hydla::parse_tree::E> node);
  // 任意の文字列
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ArbitraryFactor> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ArbitraryUnary> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ArbitraryBinary> node);

  // 変数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // 数字
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);

  // 記号定数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parameter> node);

  // t
  virtual void visit(boost::shared_ptr<hydla::parse_tree::SymbolicT> node);


private:
  REDUCELink* cl_;

protected:
  /// 送信された変数の一覧
  var_info_list_t vars_;

  // Differentialノードを何回通ったか
  int differential_count_;

  /// Prevノードの下にいるかどうか
  int in_prev_;

  // prev制約を無視するかどうか
  bool ignore_prev_;

  // notを適用するかどうか
  bool apply_not_;

};

} //namespace reduce
} //namespace vcs
} //namespace hydla

#endif //_INCLUDED_HYDLA_VCS_REDUCE_REDUCE_STRING_SENDER_H_
