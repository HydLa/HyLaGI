#ifndef _INCLUDED_HYDLA_CONTINUITY_MAP_MAKER_H_
#define _INCLUDED_HYDLA_CONTINUITY_MAP_MAKER_H_

#include <vector>
#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "DefaultTreeVisitor.h"

namespace hydla {
namespace simulator {

typedef std::map<std::string, int>                               continuity_map_t;

/**
 * Tellノードを調べ，連続性の根拠となる変数（とその微分値）の出現を数えるクラス
 */
class ContinuityMapMaker : public parse_tree::DefaultTreeVisitor {
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

  // Ask制約
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // 微分
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // 左極限
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);


  // 変数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

private:

  
  // 集めた制約中に出現する変数とその微分回数のマップ
  continuity_map_t  variables_;
  int differential_count_;
  bool in_interval_;
  // 負数を追加するかどうか
  bool negative_;
};


std::ostream& operator<<(std::ostream& s, const continuity_map_t& continuity_map);

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_CONTINUITY_MAP_MAKER_H_
