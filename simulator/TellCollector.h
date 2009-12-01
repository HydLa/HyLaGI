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
  typedef hydla::ch::module_set_sptr module_set_sptr;
  typedef std::set<boost::shared_ptr<hydla::parse_tree::Always> >   visited_always_t;
  typedef std::vector<boost::shared_ptr<hydla::parse_tree::Tell> >  tells_t;
  typedef std::set<boost::shared_ptr<hydla::parse_tree::Tell> >     collected_tells_t;

  TellCollector(const module_set_sptr& module_set);
  virtual ~TellCollector();

  /** 
   * ���ׂĂ�tell�m�[�h���W�߂�
   *
   * @param expanded_always  �W�J�ς�always�m�[�h�̏W��
   *                           �iask�̒��ɂ�����always���W�J���ꂽ���́j
   * @param all_tells        �W�߂�ꂽtell�m�[�h�̏W��
   * @param positive_asks    �K�[�h�������G���e�[���\��ask�m�[�h�̏W��
   */
  void collect_all_tells(tells_t*           all_tells,
                         expanded_always_t* expanded_always,
                         positive_asks_t*   positive_asks)
  {
    collect_all_tells_ = true;
    collect(all_tells, expanded_always, positive_asks);
  }

  /** 
   * �܂��W�߂��Ă��Ȃ�tell�m�[�h���W�߂�
   *
   * @param expanded_always  �W�J�ς�always�m�[�h�̏W��
   *                           �iask�̒��ɂ�����always���W�J���ꂽ���́j
   * @param all_tells        �W�߂�ꂽtell�m�[�h�̏W��
   * @param positive_asks    �K�[�h�������G���e�[���\��ask�m�[�h�̏W��
   */
  void collect_new_tells(tells_t*           new_tells,
                         expanded_always_t* expanded_always,                   
                         positive_asks_t*   positive_asks)
  {
    collect_all_tells_ = false;
    collect(new_tells, expanded_always, positive_asks);
  }

  /**
   * ���W�ς݂�tell�m�[�h�̏W���𓾂�
   *
   * @param collected_tells �W�߂�ꂽtell�m�[�h�̏W��
   */
  void collected_tells(tells_t* collected_tells);

  /**
   * ���W�ς݂�tell�m�[�h�̋L�^���������C������Ԃɖ߂�
   */
  void reset()
  {
    collected_tells_.clear();
  }

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
  void collect(tells_t*           tells,
               expanded_always_t* expanded_always,                   
               positive_asks_t*   positive_asks);

  module_set_sptr    module_set_; 

  positive_asks_t*   positive_asks_;

  /// ���W����tell�m�[�h�̃��X�g
  tells_t*           tells_;

  /// ���W�ς݂�tell�m�[�h�̃��X�g
  collected_tells_t  collected_tells_;

  /// ���ׂĂ�tell�m�[�h�����W���邩�ǂ���
  bool               collect_all_tells_;

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
