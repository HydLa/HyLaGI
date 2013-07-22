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
   * ����𒲂ׁC�ϐ��̏o�����擾����
   */
  bool visit_node(std::set<std::string>, boost::shared_ptr<parse_tree::Node> node, const bool& in_IP);
  
  void clear();
  
  variable_set_t get_variable_set() const;
  
  variable_set_t get_prev_variable_set() const;
  
  // Ask����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

    // �ϐ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

private:

  variable_set_t variables_, prev_variables_;
  
  bool in_interval_;
  bool in_prev_;
  bool has_variables_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_VARIABLE_SEARCHER_H_
