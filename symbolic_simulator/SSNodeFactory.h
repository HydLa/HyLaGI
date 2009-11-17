#ifndef _INCLUDED_HYDLA_SS_NODE_FACTORY_H_
#define _INCLUDED_HYDLA_SS_NODE_FACTORY_H_

#include "SSParseTree.h"
#include "NodeFactory.h"

namespace hydla { 
namespace parse_tree {

class SSNodeFactory : public NodeFactory {
public:
  SSNodeFactory(){}
  virtual ~SSNodeFactory(){}

  virtual boost::shared_ptr<ConstraintDefinition> createConstraintDefinition() const {
    boost::shared_ptr<ConstraintDefinition> p(new SSConstraintDefinition());
    return p;
  }

};


} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_SS_NODE_FACTORY_H_
