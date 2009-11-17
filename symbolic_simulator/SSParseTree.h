#ifndef _INCLUDED_HYDLA_SS_PARSE_TREE_H_
#define _INCLUDED_HYDLA_SS_PARSE_TREE_H_

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "ParseTree.h"

namespace hydla { 
namespace parse_tree {

class SSConstraintDefinition : public ConstraintDefinition {
public:
  SSConstraintDefinition(){}
  virtual ~SSConstraintDefinition(){}

  virtual void exec() {};
};

} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_SS_PARSE_TREE_H_
