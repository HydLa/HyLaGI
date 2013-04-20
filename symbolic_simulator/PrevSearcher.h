#ifndef _INCLUDED_HYDLA_PREV_SERACHER_H_
#define _INCLUDED_HYDLA_PREV_SERACHER_H_

#include <vector>
#include <set>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "DefaultTreeVisitor.h"

namespace hydla {
namespace simulator {
namespace symbolic {

/**
 * prevを含むかを調べるビジタークラス．
 */
class PrevSearcher : public parse_tree::DefaultTreeVisitor {
public:
  typedef hydla::parse_tree::node_sptr node_sptr;

  PrevSearcher(){}

  virtual ~PrevSearcher(){}

  /*
   prevを含んでいたらtrueを返す
  */
  bool search_prev(const node_sptr &node){
    include_prev_ = false;
    accept(node);
    return include_prev_;
  }

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node){
    include_prev_ = true;
  }
  
private:
  bool include_prev_;
};

} //namespace symbolic
} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_PREV_SERACHER_H_
