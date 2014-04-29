#ifndef _INCLUDED_HYDLA_PREV_SERACHER_H_
#define _INCLUDED_HYDLA_PREV_SERACHER_H_

#include <vector>
#include <set>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "DefaultTreeVisitor.h"

namespace hydla {
namespace simulator {

/**
 * prevを含むかを調べるビジタークラス．
 */
class PrevSearcher : public symbolic_expression::DefaultTreeVisitor {
public:
  typedef hydla::symbolic_expression::node_sptr node_sptr;

  PrevSearcher(){}

  virtual ~PrevSearcher(){}

  /*
   prevを含んでいたらtrueを返す
  */
  bool search_prev(const symbolic_expression::node_sptr &node){
    include_prev_ = false;
    accept(node);
    return include_prev_;
  }

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Previous> node)  {
    include_prev_ = true;
  }
  
private:
  bool include_prev_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_PREV_SERACHER_H_
