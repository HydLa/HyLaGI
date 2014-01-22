#ifndef _INCLUDED_HYDLA_VARIABLE_SEARCHER_H_
#define _INCLUDED_HYDLA_VARIABLE_SEARCHER_H_

#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "DefaultTreeVisitor.h"

namespace hydla {
namespace simulator {

/**
 * ����𒲂ׁC�ϐ��̏o�����擾����N���X�D
 */
class VariableSearcher : public parse_tree::DefaultTreeVisitor {
public:

  typedef std::set<std::string> variable_set_t;

  VariableSearcher();

  virtual ~VariableSearcher();
  
  /** 
   * �ϐ��̏W���̗v�f������Ɋ܂܂�邩���ׂ�B
   * Ask����̓K�[�h�����𒲂ׂ�B
   * include_prev:prev�ϐ����Ώۂɂ��邩
   */
  bool visit_node(std::set<std::string> variables, boost::shared_ptr<parse_tree::Node> node, const bool& include_prev);
  
  // Ask����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

    // �ϐ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // ���Ɍ�
  void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);

private:

  variable_set_t variables_;
  
  bool in_prev_;
  bool include_prev_;
  bool has_variables_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_VARIABLE_SEARCHER_H_
