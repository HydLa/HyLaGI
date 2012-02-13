#ifndef _INCLUDED_HYDLA_CONTINUITY_MAP_MAKER_H_
#define _INCLUDED_HYDLA_CONTINUITY_MAP_MAKER_H_

#include <vector>
#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "TreeVisitor.h"

#include "./Types.h"

namespace hydla {
namespace simulator {

/**
 * Tellノードを調べ，連続性の根拠となる変数（とその微分値）の出現を数えるクラス
 */
class ContinuityMapMaker : public parse_tree::TreeVisitor {
public:
  
  ContinuityMapMaker();

  virtual ~ContinuityMapMaker();
  
  /** 
   * 
   */
  void visit_node(boost::shared_ptr<parse_tree::Node> node, const bool& in_IP, const bool& negative)
  {
    in_interval_ = in_IP;
    negative_ = negative;
    differential_count_ = 0;
    accept(node);
  }

  /**
   * 初期状態に戻す
   */
  void reset()
  {
    variables_.clear();
  }
  
  void set_continuity_map(const continuity_map_t& map){
    variables_ = map;
  }
  
  continuity_map_t get_continuity_map(){
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

virtual void visit(boost::shared_ptr<hydla::parse_tree::Print> node);
virtual void visit(boost::shared_ptr<hydla::parse_tree::PrintPP> node);
virtual void visit(boost::shared_ptr<hydla::parse_tree::PrintIP> node);
virtual void visit(boost::shared_ptr<hydla::parse_tree::Scan> node);
virtual void visit(boost::shared_ptr<hydla::parse_tree::Exit> node);
virtual void visit(boost::shared_ptr<hydla::parse_tree::Abort> node);

private:

  
  // 集めた制約中に出現する変数とその微分回数のマップ
  continuity_map_t  variables_;
  int differential_count_;
  bool in_interval_;
  // 負数を追加するかどうか
  bool negative_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_CONTINUITY_MAP_MAKER_H_
