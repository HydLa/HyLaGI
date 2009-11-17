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
 * tell�m�[�h���W�߂�r�W�^�[�N���X
 */
class TellCollector : public parse_tree::TreeVisitor {
public:
  TellCollector();
  virtual ~TellCollector();

  /** 
   * tell�m�[�h���W�߂�
   *
   * @param expanded_always  �W�J�ς�always�m�[�h�̏W��
   *                           �iask�̒��ɂ�����always���W�J���ꂽ���́j
   * @param collected_tells  �W�߂�ꂽtell�m�[�h�̏W��
   * @param positive_asks    �K�[�h�������G���e�[���\��ask�m�[�h�̏W��
   */
  void collect_tell(module_set_t*      ms,
                    expanded_always_t* expanded_always,                   
                    collected_tells_t* collected_tells,
                    positive_asks_t*   positive_asks);

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
  expanded_always_t* expanded_always_;                 
  collected_tells_t* collected_tells_;
  positive_asks_t*   positive_asks_;

  /// ask�m�[�h�̎q�m�[�h���ǂ���
  bool               in_ask_;

  /// �W�J�ς�always�m�[�h�̃��X�g����̒T�����ǂ���
  bool               in_expanded_always_;

  /// �T������always�m�[�h�̃��X�g
  visited_always_t   visited_always_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_TELL_COLLECTOR_H_
