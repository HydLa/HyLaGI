#ifndef _INCLUDED_HYDLA_ASK_COLLECTOR_H_
#define _INCLUDED_HYDLA_ASK_COLLECTOR_H_

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
 * ask�m�[�h���W�߂�r�W�^�[�N���X
 */
class AskCollector : public parse_tree::TreeVisitor {
public:
  AskCollector(const module_set_sptr& module_set);
  virtual ~AskCollector();

  /** 
   * ask�m�[�h���W�߂�
   *
   * @param expanded_always  �W�J�ς�always�m�[�h�̏W��
   *                           �iask�̒��ɂ�����always���W�J���ꂽ���́j
   * @param negative_asks    �K�[�h�������G���e�[���s�\��ask�m�[�h�̏W��
   * @param positive_asks    �K�[�h�������G���e�[���\��ask�m�[�h�̏W��
   */
  void collect_ask(const  expanded_always_t* expanded_always,                   
                   positive_asks_t*          positive_asks,
                   negative_asks_t*          negative_asks);


  // ����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Constraint> node);

  // Ask����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // Tell����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);

  // �_����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);
  
  // �������Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Always> node);

private:
  module_set_sptr          module_set_; 
  const expanded_always_t* expanded_always_;                   
  negative_asks_t*         negative_asks_;
  positive_asks_t*         positive_asks_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_ASK_COLLECTOR_H_
