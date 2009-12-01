#ifndef _INCLUDED_HYDLA_TELL_COLLECTOR_H_
#define _INCLUDED_HYDLA_TELL_COLLECTOR_H_

#include <vector>
#include <set>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "ModuleSet.h"
#include "TreeVisitor.h"

#include "./Types.h"

namespace hydla {
namespace simulator {

/**
 * tellノードを集めるビジタークラス
 */
class TellCollector : public parse_tree::TreeVisitor {
public:
  typedef hydla::ch::module_set_sptr module_set_sptr;
  typedef std::set<boost::shared_ptr<hydla::parse_tree::Always> >   visited_always_t;
  typedef std::vector<boost::shared_ptr<hydla::parse_tree::Tell> >  tells_t;
  typedef std::set<boost::shared_ptr<hydla::parse_tree::Tell> >     collected_tells_t;

  TellCollector(const module_set_sptr& module_set);
  virtual ~TellCollector();

  /** 
   * すべてのtellノードを集める
   *
   * @param expanded_always  展開済みalwaysノードの集合
   *                           （askの中にあったalwaysが展開されたもの）
   * @param all_tells        集められたtellノードの集合
   * @param positive_asks    ガード条件がエンテール可能なaskノードの集合
   */
  void collect_all_tells(tells_t*           all_tells,
                         expanded_always_t* expanded_always,
                         positive_asks_t*   positive_asks)
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
  void collect_new_tells(tells_t*           new_tells,
                         expanded_always_t* expanded_always,                   
                         positive_asks_t*   positive_asks)
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

private:
  void collect(tells_t*           tells,
               expanded_always_t* expanded_always,                   
               positive_asks_t*   positive_asks);

  module_set_sptr    module_set_; 

  positive_asks_t*   positive_asks_;

  /// 収集したtellノードのリスト
  tells_t*           tells_;

  /// 収集済みのtellノードのリスト
  collected_tells_t  collected_tells_;

  /// すべてのtellノードを収集するかどうか
  bool               collect_all_tells_;

  /// askノードの子ノードかどうか
  bool               in_ask_;

  /// 展開済みalwaysノードのリストからの探索かどうか
  bool               in_expanded_always_;

  /// 探索したalwaysノードのリスト
  visited_always_t   visited_always_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_TELL_COLLECTOR_H_
