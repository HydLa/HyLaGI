#ifndef _INCLUDED_HYDLA_NON_PREV_SEARCHER_H_
#define _INCLUDED_HYDLA_NON_PREV_SEARCHER_H_

#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "DefaultTreeVisitor.h"

#include "./Types.h"

namespace hydla {
namespace simulator {

/**
 * ����𒲂ׁCprev�ϐ��ȊO�̕ϐ����܂܂�Ă��邩��Ԃ��N���X�D
 */
class NonPrevSearcher : public parse_tree::DefaultTreeVisitor {
public:


  virtual void accept(const boost::shared_ptr<hydla::parse_tree::Node>& n);


  NonPrevSearcher();

  virtual ~NonPrevSearcher();
  
  /**
   * prev�ȊO�̕ϐ����܂ނ����肷��
   * @return: true�Ȃ�prev�ϐ��ȊO�̕ϐ����܂�ł���
   */
  bool judge_non_prev(boost::shared_ptr<parse_tree::Node> node);

  // ���Ɍ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);
  
  // �ϐ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);
private:

  bool non_prev_;
  bool in_prev_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_NON_PREV_SEARCHER_H_
