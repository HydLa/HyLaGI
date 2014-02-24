#ifndef _INCLUDED_HYDLA_NodeAccessor_H_
#define _INCLUDED_HYDLA_NodeAccessor_H_

#include "BatchSimulator.h"
#include "Node.h"
#include "DefaultTreeVisitor.h"
#include "PhaseResult.h"

namespace hydla {
  namespace simulator {

class GuardGetter : public parse_tree::DefaultTreeVisitor {
 public:
  virtual void accept(const boost::shared_ptr<parse_tree::Node>& n);
  
  GuardGetter();
  virtual ~GuardGetter();
  
  // Ask
  virtual void visit(boost::shared_ptr<parse_tree::Ask> node);
  
  ask_set_t asks;
};//GuardGetter

class VaribleGetter : public parse_tree::DefaultTreeVisitor {
 public:
  virtual void accept(const boost::shared_ptr<parse_tree::Node>& n);

  typedef struct GuardVariable
  {
    int diff_cnt;
    std::string name;
  }guard_variable_t;
  
  typedef std::vector<guard_variable_t>  guard_variable_vec_t;

  VaribleGetter();
  virtual ~VaribleGetter();
  
  // 微分
  virtual void visit(boost::shared_ptr<parse_tree::Differential> node);
  // 変数
  virtual void visit(boost::shared_ptr<parse_tree::Variable> node);
  
  int tmp_diff_cnt;
  
  guard_variable_vec_t  vec_variable;
  guard_variable_vec_t::iterator it;
  
  guard_variable_vec_t::iterator get_iterator()
    {
      return it;
    }

  void it_begin()
  {
    it = vec_variable.begin();
  }
  
  void inc_it()
  {
    it++;
  }
  
};//VaribleGetter

}//namespace hydla
}//namespace simulator 

#endif // _INCLUDED_HYDLA_NodeAccessor_H_

