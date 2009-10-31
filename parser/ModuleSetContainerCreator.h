#ifndef _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_CREATOR_H_
#define _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_CREATOR_H_

#include <vector>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "TreeVisitor.h"

#include "ModuleSet.h"
#include "ModuleSetList.h"

namespace hydla {
namespace ch {

template <class Container>
class ModuleSetContainerCreator : public hydla::parse_treeTreeVisitor {
public:
  typedef typename boost::shared_ptr<Container> container_sptr;
  typedef std::vector<container_sptr>           container_stack_t;

  ModuleSetContainerCreator(ParseTree& parse_tree) :
    parse_tree_(parse_tree)
  {}

  virtual ~ModuleSetContainerCreator()
  {}

  // åƒÇ—èoÇµ
  virtual void visit(ConstraintCaller* node)
  {
    container_name_ = node->get_name();
  }

  virtual void visit(ProgramCaller* node)
  {
    container_name_ = node->get_name();
  }

  // êßñÒéÆ
  virtual void visit(Constraint* node)
  {
    new ModuleSet(std::string("A"), node_sptr())

  }

  // êßñÒäKëwíËã`ââéZéq
  virtual void visit(Weaker* node);
  virtual void visit(Parallel* node);

private:
  ParseTree&        parse_tree_;
  container_stack_t mod_set_stack_;
  std::string       container_name_;
};

} // namespace ch
} // namespace hydla

#endif // _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_CREATOR_H_