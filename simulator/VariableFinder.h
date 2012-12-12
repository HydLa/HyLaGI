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
 * 制約を調べ，変数の出現を取得するクラス．
 */
class VariableFinder : public parse_tree::DefaultTreeVisitor {
public:

  typedef std::set< std::pair<std::string, int> > variable_set_t;

  VariableFinder();

  virtual ~VariableFinder();
  
  /** 
   * 制約を調べ，変数の出現を取得する
   */
  void visit_node(boost::shared_ptr<parse_tree::Node> node, const bool& in_IP);
  
  void clear();
  
  variable_set_t get_variable_set() const;
  
  variable_set_t get_prev_variable_set() const;
  
  // Ask制約
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // 微分
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // 左極限
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);

  // 変数
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
