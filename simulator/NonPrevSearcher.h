#ifndef _INCLUDED_HYDLA_NON_PREV_SEARCHER_H_
#define _INCLUDED_HYDLA_NON_PREV_SEARCHER_H_

#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "DefaultTreeVisitor.h"

namespace hydla {
namespace simulator {

/**
 * 制約を調べ，prev変数以外の変数が含まれているかを返すクラス．
 */
class NonPrevSearcher : public parse_tree::DefaultTreeVisitor {
public:


  virtual void accept(const boost::shared_ptr<hydla::parse_tree::Node>& n);


  NonPrevSearcher();

  virtual ~NonPrevSearcher();
  
  /**
   * prev以外の変数を含むか判定する
   * @return: trueならprev変数以外の変数を含んでいる
   */
  bool judge_non_prev(boost::shared_ptr<parse_tree::Node> node);

  // 左極限
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);
  
  // 変数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);
private:

  bool non_prev_;
  bool in_prev_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_NON_PREV_SEARCHER_H_
