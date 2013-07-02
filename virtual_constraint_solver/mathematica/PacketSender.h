#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_PACKET_SENDER_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_PACKET_SENDER_H_

#include <map>

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/tuple/tuple_io.hpp>

#include <boost/bimap/bimap.hpp>

#include "DefaultTreeVisitor.h"
#include "mathlink_helper.h"
#include "ParseTree.h"
#include "MathematicaVCS.h"

namespace hydla {
namespace vcs {
namespace mathematica {


enum now_phase_t 
{ 
  NP_POINT_PHASE, 
  NP_INTERVAL_PHASE,
};

enum VariableArg {
  VA_Prev,
  VA_None,  
  VA_Time,
  VA_Zero,
};

extern const std::string var_prefix;
extern const std::string par_prefix;

class PacketSender : 
  public hydla::parse_tree::DefaultTreeVisitor, hydla::simulator::ValueVisitor
{
public:
  
  typedef std::map<std::string, int> max_diff_map_t;
  /**
   * 変数データ
   * (変数名， 微分回数，送信形式)
   */
  typedef boost::tuple<std::string, int, VariableArg>       var_info_t;
  typedef std::set<var_info_t>                 var_info_list_t;

  /**
   * 記号定数データ
   * (元の変数名， 微分回数，id)
   */
  typedef boost::tuple<std::string, int, int>       par_info_t;
  typedef std::set<par_info_t>                 par_info_list_t;
  typedef par_info_list_t::const_iterator      pars_const_iterator;
  typedef var_info_list_t::const_iterator      vars_const_iterator;
  typedef hydla::parse_tree::node_sptr         node_sptr;

  // Mathematicaに送る際に変数名につける接頭語 "usrVar"
  // Mathematicaに送る際に定数名につける接頭語

  PacketSender(MathLink& ml);

  virtual ~PacketSender();
  
  
  /// 静的メンバの初期化を行う
  static void initialize();

  vars_const_iterator vars_begin() const { return vars_.begin(); }

  vars_const_iterator vars_end() const { return vars_.end(); }


  void put_value(value_t, VariableArg var);

  /**
   * 与えられたノードの送信をおこなう
   *
   * ノードの送信をおこなう際は直接visit関数を呼ばずに，
   * 必ずこの関数を経由すること
   */

  void put_node(const node_sptr& node, VariableArg variable_arg);
  /**
   * リスト形式で送信する
   */
  void put_nodes(const std::vector<node_sptr> &constraints, VariableArg variable_arg);

  /**
   * 変数の送信
   */
  void put_var(const var_info_t var);
  void put_var(const std::string& variable_name, const int& diff_count, VariableArg variable_arg);

  /**
   * put_nodeの際に送信された変数群の送信をおこなう
   * 微分値が送信された場合は，その微分元も送信するものとする．
   */
  void put_vars();
  
  
  /**
   * 上2つの記号定数版
   */
  void put_par(const par_info_t par);
  void put_par(const std::string& name, const int& diff_count, const int& id);
  void put_pars();



  /**
   * put_nodeの際に送信された変数群のデータを消去し，初期化する
   */
  void clear();

  void create_max_diff_map(max_diff_map_t& max_diff_map);

  virtual void visit(hydla::simulator::symbolic::SymbolicValue& value);

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
  
  // コマンド文
  virtual void visit(boost::shared_ptr<hydla::parse_tree::PrintPP> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::PrintIP> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Scan> node);

  // 算術単項演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);

  // 微分
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // 左極限
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);

  // 否定
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Not> node);
  
  // 関数
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

  // 記号定数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parameter> node);

  // t
  virtual void visit(boost::shared_ptr<hydla::parse_tree::SymbolicT> node);
  
  // 無限大
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Infinity> node);
  
  typedef std::pair<std::string, int> function_t;
  typedef boost::bimaps::bimap<function_t, function_t > function_map_t;
  static function_map_t function_map_;
private:
  MathLink* ml_;
  /// 送信された変数の一覧
  var_info_list_t vars_;
  par_info_list_t pars_;

  // Differentialノードを何回通ったか
  int differential_count_;

  /// Prevノードの下にいるかどうか
  int in_prev_;

  /// 変数の引数として送る物
  VariableArg variable_arg_;

  // prev制約を無視するかどうか
  bool ignore_prev_;
};

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_PACKET_SENDER_H_
