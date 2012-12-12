#ifndef _INCLUDED_HYDLA_VARIABLE_FINDER_H_
#define _INCLUDED_HYDLA_VARIABLE_FINDER_H_

#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "DefaultTreeVisitor.h"

#include "./Types.h"

namespace hydla {
namespace simulator {

/**
 * ����𒲂ׁC�ϐ��̏o�����擾����N���X�D
 */
class VariableFinder : public parse_tree::DefaultTreeVisitor {
public:

  typedef std::set< std::pair<std::string, int> > variable_set_t;

  VariableFinder();

  virtual ~VariableFinder();
  
  /** 
   * ����𒲂ׁC�ϐ��̏o�����擾����
   */
  void visit_node(boost::shared_ptr<parse_tree::Node> node, const bool& in_IP);
  
  void clear();
  
  variable_set_t get_variable_set() const;
  
  variable_set_t get_prev_variable_set() const;
  
  // Ask����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // ����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // ���Ɍ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);

  // �ϐ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

private:

  variable_set_t variables_, prev_variables_;
  
  int differential_count_;
  bool in_interval_;
  bool in_prev_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_VARIABLE_FINDER_H_
