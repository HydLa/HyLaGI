#ifndef _INCLUDED_HYDLA_TELL_COLLECTOR_H_
#define _INCLUDED_HYDLA_TELL_COLLECTOR_H_

#include <vector>
#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "ModuleSet.h"
#include "TreeVisitor.h"

#include "./Types.h"

namespace hydla {
namespace simulator {

/**
 * tell�m�[�h���W�߂�r�W�^�[�N���X
 * �m�[�h�̒��ɏo������ϐ��i�Ƃ��̔����񐔁j�������ɒ��ׂ�
 */
class TellCollector : public parse_tree::TreeVisitor {
public:
  
  TellCollector(const module_set_sptr& module_set, bool in_IP = false);

  virtual ~TellCollector();

  /** 
   * ���ׂĂ�tell�m�[�h���W�߂�
   *
   * @param expanded_always  �W�J�ς�always�m�[�h�̏W��
   *                           �iask�̒��ɂ�����always���W�J���ꂽ���́j
   * @param all_tells        �W�߂�ꂽtell�m�[�h�̏W��
   * @param positive_asks    �K�[�h�������G���e�[���\��ask�m�[�h�̏W��
   */
  void collect_all_tells(tells_t*                 all_tells,
                         const expanded_always_t* expanded_always,
                         const positive_asks_t*   positive_asks)
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
  void collect_new_tells(tells_t*                 new_tells,
                         const expanded_always_t* expanded_always,                   
                         const positive_asks_t*   positive_asks)
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
    variables_.clear();
  }
  
  continuity_map_t get_variables(){
    return variables_;
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
  



    // ��r���Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Equal> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Less> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Greater> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node);

  // �_�����Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalOr> node);

  // �Z�p�񍀉��Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Times> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Divide> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Power> node);

  // �Z�p�P�����Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);

  // ����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // ���Ɍ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);

  // �ے�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Not> node);
  
  // �O�p�֐�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Sin> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Cos> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tan> node);
  // �t�O�p�֐�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Asin> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Acos> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Atan> node);
  // �~����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Pi> node);
  // �ΐ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Log> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ln> node);
  // ���R�ΐ��̒�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::E> node);
  // �C�ӂ̕�����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ArbitraryFactor> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ArbitraryUnary> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ArbitraryBinary> node);
  // �ϐ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);
  // ����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);
  // �L���萔
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parameter> node);
  // t
  virtual void visit(boost::shared_ptr<hydla::parse_tree::SymbolicT> node);



private:
  typedef std::set<boost::shared_ptr<hydla::parse_tree::Always> >   visited_always_t;

  void collect(tells_t*                 tells,
               const expanded_always_t* expanded_always,                   
               const positive_asks_t*   positive_asks);

  /// ���W�������Ȃ��Ώۂ̐��񃂃W���[���W��
  module_set_sptr    module_set_; 

  /// �L���ƂȂ��Ă���ask�̃��X�g
  const positive_asks_t*   positive_asks_;

  /// ���W����tell�m�[�h�̃��X�g
  tells_t*           tells_;

  /// ���W�ς݂�tell�m�[�h�̃��X�g
  collected_tells_t  collected_tells_;

  /// ���ׂĂ�tell�m�[�h�����W���邩�ǂ���
  bool               collect_all_tells_;

  /// �L���ƂȂ��Ă���ask�m�[�h�̎q�m�[�h���ǂ���
  bool               in_positive_ask_;

  /// �����ƂȂ��Ă���ask�m�[�h�̎q�m�[�h���ǂ���
  bool               in_negative_ask_;

  /// �W�J�ς�always�m�[�h�̃��X�g����̒T�����ǂ���
  bool               in_expanded_always_;
  
  // �W�߂����񒆂ɏo������ϐ��Ƃ��̔����񐔂̃}�b�v
  continuity_map_t  variables_;
  int differential_count_;
  bool in_interval_;

  /// �T������always�m�[�h�̃��X�g
  visited_always_t   visited_always_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_TELL_COLLECTOR_H_
