#ifndef _INCLUDED_HYDLA_ASK_CONSTRAINT_VARIABLE_FINDER_H_
#define _INCLUDED_HYDLA_ASK_CONSTRAINT_VARIABLE_FINDER_H_

namespace hydla {
namespace simulator {
namespace symbolic {

class AskConstraintVariableFinder : public VariableFinder{
  void visit(boost::shared_ptr<hydla::parse_tree::Ask> node){
    accept(node->get_child());
  }
};

} //namespace symbolic
} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_ASK_CONSTRAINT_VARIABLE_FINDER_H_
