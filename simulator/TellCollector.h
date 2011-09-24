#ifndef _INCLUDED_HYDLA_TELL_COLLECTOR_H_
#define _INCLUDED_HYDLA_TELL_COLLECTOR_H_

#include <vector>
#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "ModuleSet.h"
#include "TreeVisitor.h"

#include "./Types.h"

namespace hydla {
namespace simulator {

/**
 * tellノードを集めるビジタークラス
 * ノードの中に出現する変数（とその微分回数）も同時に調べる
 */
class TellCollector : public parse_tree::TreeVisitor {
public:
  
  TellCollector(const module_set_sptr& module_set, bool in_IP = false);

  virtual ~TellCollector();

  /** 
   * すべてのtellノードを集める
   *
   * @param expanded_always  展開済みalwaysノードの集合
   *                           （askの中にあったalwaysが展開されたもの）
   * @param all_tells        集められたtellノードの集合
   * @param positive_asks    ガード条件がエンテール可能なaskノードの集合
   */
  void collect_all_tells(tells_t*                 all_tells,
                         const expanded_always_t* expanded_always,
                         const positive_asks_t*   positive_asks)
  {
    collect_all_tells_ = true;
    collect(all_tells, expanded_always, positive_asks);
  }

  /** 
   * まだ集められていないtellノードを集める
   *
   * @param expanded_always  展開済みalwaysノードの集合
   *                           （askの中にあったalwaysが展開されたもの）
   * @param all_tells        集められたtellノードの集合
   * @param positive_asks    ガード条件がエンテール可能なaskノードの集合
   */
  void collect_new_tells(tells_t*                 new_tells,
                         const expanded_always_t* expanded_always,                   
                         const positive_asks_t*   positive_asks)
  {
    collect_all_tells_ = false;
    collect(new_tells, expanded_always, positive_asks);
  }

  /**
   * 収集済みのtellノードの集合を得る
   *
   * @param collected_tells 集められたtellノードの集合
   */
  void collected_tells(tells_t* collected_tells);

  /**
   * 収集済みのtellノードの記録を消去し，初期状態に戻す
   */
  void reset()
  {
    collected_tells_.clear();
    variables_.clear();
  }
  
  continuity_map_t get_variables(){
    return variables_;
  }

  // 制約式
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Constraint> node);

  // Ask制約
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // Tell制約
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);

  // 論理積
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);
  
  // 時相演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Always> node);

  // モジュールの弱合成
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Weaker> node);

  // モジュールの並列合成
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parallel> node);
   
  // 制約呼び出し
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node);
  
  // プログラム呼び出し
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node);
  



    // 比較演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Equal> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Less> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Greater> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node);

  // 論理演算子
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
  typedef std::set<boost::shared_ptr<hydla::parse_tree::Always> >   visited_always_t;

  void collect(tells_t*                 tells,
               const expanded_always_t* expanded_always,                   
               const positive_asks_t*   positive_asks);

  /// 収集をおこなう対象の制約モジュール集合
  module_set_sptr    module_set_; 

  /// 有効となっているaskのリスト
  const positive_asks_t*   positive_asks_;

  /// 収集したtellノードのリスト
  tells_t*           tells_;

  /// 収集済みのtellノードのリスト
  collected_tells_t  collected_tells_;

  /// すべてのtellノードを収集するかどうか
  bool               collect_all_tells_;

  /// 有効となっているaskノードの子ノードかどうか
  bool               in_positive_ask_;

  /// 無効となっているaskノードの子ノードかどうか
  bool               in_negative_ask_;

  /// 展開済みalwaysノードのリストからの探索かどうか
  bool               in_expanded_always_;
  
  // 集めた制約中に出現する変数とその微分回数のマップ
  continuity_map_t  variables_;
  int differential_count_;
  bool in_interval_;

  /// 探索したalwaysノードのリスト
  visited_always_t   visited_always_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_TELL_COLLECTOR_H_
