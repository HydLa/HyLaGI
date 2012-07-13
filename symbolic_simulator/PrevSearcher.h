#ifndef _INCLUDED_HYDLA_PREV_SERACHER_H_
#define _INCLUDED_HYDLA_PREV_SERACHER_H_

#include <vector>
#include <set>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "DefaultTreeVisitor.h"

namespace hydla {
namespace symbolic_simulator {

/**
 * prev���܂ނ��𒲂ׂ�r�W�^�[�N���X�D
 */
class PrevSearcher : public parse_tree::DefaultTreeVisitor {
public:
  PrevSearcher(){}

  virtual ~PrevSearcher(){}

  /*
   prev���܂�ł�����true��Ԃ�
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

} //namespace symbolic_simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_PREV_SERACHER_H_
