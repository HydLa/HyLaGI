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
  typedef unsigned int collect_flag_t;
  static const collect_flag_t ENABLE_COLLECT_NON_TYPED_ASK  = 0x01;
  static const collect_flag_t ENABLE_COLLECT_DISCRETE_ASK   = 0x02;
  static const collect_flag_t ENABLE_COLLECT_CONTINUOUS_ASK = 0x04;

  AskCollector(const module_set_sptr& module_set, 
    collect_flag_t collect_type = 
      ENABLE_COLLECT_NON_TYPED_ASK | 
      ENABLE_COLLECT_DISCRETE_ASK |
      ENABLE_COLLECT_CONTINUOUS_ASK);

  virtual ~AskCollector();

  /** 
   * ask�m�[�h���W�߂�
   *
   * @param expanded_always  �W�J�ς�always�m�[�h�̏W��
   *                           �iask�̒��ɂ�����always���W�J���ꂽ���́j
   * @param negative_asks    �K�[�h�������G���e�[���s�\��ask�m�[�h�̏W��
   * @param positive_asks    �K�[�h�������G���e�[���\��ask�m�[�h�̏W��
   */
  void collect_ask(expanded_always_t* expanded_always,                   
                   positive_asks_t*   positive_asks,
                   negative_asks_t*   negative_asks);


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
  
  // ���W���[���̎㍇��
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Weaker> node);

  // ���W���[���̕��񍇐�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parallel> node);
   
  // ����Ăяo��
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node);
  
  // �v���O�����Ăяo��
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node);

private:
  typedef std::set<boost::shared_ptr<hydla::parse_tree::Always> >   visited_always_t;

  /// ���W�������Ȃ��Ώۂ̐��񃂃W���[���W��
  module_set_sptr          module_set_; 

  expanded_always_t*       expanded_always_;                   
  
  /// �����ƂȂ��Ă���ask�̃��X�g
  negative_asks_t*         negative_asks_;

  /// �L���ƂȂ��Ă���ask�̃��X�g
  positive_asks_t*         positive_asks_;

  /// �W�J�ς�always�m�[�h�̃��X�g����̒T�����ǂ���
  bool               in_expanded_always_;

  /// �L���ƂȂ��Ă���ask�̒����ǂ���
  bool                in_positive_ask_;

  /// �T������always�m�[�h�̃��X�g
  visited_always_t   visited_always_;

  expanded_always_t  new_expanded_always_;

  collect_flag_t collect_type_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_ASK_COLLECTOR_H_
