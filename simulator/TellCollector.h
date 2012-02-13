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
  }

    std::vector<std::string> get_print(){
    return v_print;
  }
  std::vector<std::string> get_print_pp(){
    return v_print_pp;
  }
  std::vector<std::string> get_print_ip(){
    return v_print_ip;
  }
  std::vector<std::string> get_scan(){
    return v_scan;
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
  

  // Print
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Print> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::PrintPP> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::PrintIP> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Scan> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Exit> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Abort> node);


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

  /// 探索したalwaysノードのリスト
  visited_always_t   visited_always_;
  //print
  std::vector<std::string> v_print; 
  std::vector<std::string> v_print_pp; 
  std::vector<std::string> v_print_ip; 
  std::vector<std::string> v_scan;
 
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_TELL_COLLECTOR_H_
