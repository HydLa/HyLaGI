#ifndef _INCLUDED_HYDLA_CONTINUITY_MAP_MAKER_H_
#define _INCLUDED_HYDLA_CONTINUITY_MAP_MAKER_H_

#include <vector>
#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "TreeVisitor.h"

#include "./Types.h"

namespace hydla {
namespace simulator {

/**
 * Tell�m�[�h�𒲂ׁC�A�����̍����ƂȂ�ϐ��i�Ƃ��̔����l�j�̏o���𐔂���N���X
 */
class ContinuityMapMaker : public parse_tree::TreeVisitor {
public:
  
  ContinuityMapMaker();

  virtual ~ContinuityMapMaker();
  
  /** 
   * 
   */
  void visit_node(boost::shared_ptr<parse_tree::Node> node, const bool& in_IP, const bool& negative)
  {
    in_interval_ = in_IP;
    negative_ = negative;
    differential_count_ = 0;
    accept(node);
  }

  /**
   * ������Ԃɖ߂�
   */
  void reset()
  {
    variables_.clear();
  }
  
  void set_continuity_map(const continuity_map_t& map){
    variables_ = map;
  }
  
  continuity_map_t get_continuity_map(){
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

virtual void visit(boost::shared_ptr<hydla::parse_tree::Print> node);
virtual void visit(boost::shared_ptr<hydla::parse_tree::PrintPP> node);
virtual void visit(boost::shared_ptr<hydla::parse_tree::PrintIP> node);
virtual void visit(boost::shared_ptr<hydla::parse_tree::Scan> node);
virtual void visit(boost::shared_ptr<hydla::parse_tree::Exit> node);
virtual void visit(boost::shared_ptr<hydla::parse_tree::Abort> node);

private:

  
  // �W�߂����񒆂ɏo������ϐ��Ƃ��̔����񐔂̃}�b�v
  continuity_map_t  variables_;
  int differential_count_;
  bool in_interval_;
  // ������ǉ����邩�ǂ���
  bool negative_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_CONTINUITY_MAP_MAKER_H_
