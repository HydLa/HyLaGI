#ifndef _INCLUDED_HYDLA_NODE_FACTORY_H_
#define _INCLUDED_HYDLA_NODE_FACTORY_H_

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/utility/enable_if.hpp> 
#include <boost/type_traits/is_same.hpp> 

#include "ParseTree.h"

namespace hydla { 
namespace parse_tree {

#define CREATE_NEW_PT_NODE(NAME)                                        \
  template<typename T>                                                  \
  struct NodeCreator<T,                                                 \
                     typename boost::enable_if<boost::is_same<T, NAME> >::type> \
  {                                                                     \
  boost::shared_ptr<T> operator()(const NodeFactory* nf) const {        \
    return nf->create##NAME();                                          \
  }                                                                     \
  };                                                                    \
                                                                        \
  virtual boost::shared_ptr<NAME> create##NAME() const {                \
    boost::shared_ptr<NAME> p(new NAME());                              \
    return p;                                                           \
  }

class NodeFactory {
public:
  NodeFactory()
  {}
 
  virtual ~NodeFactory()
  {}

  // ev[gฬชม๊ปp
  template<typename T, typename N=void>
  struct NodeCreator;

  template<typename T>
  boost::shared_ptr<T> create() const {
    return NodeCreator<T>()(this);
  }

  //่`
  CREATE_NEW_PT_NODE(ProgramDefinition)
  CREATE_NEW_PT_NODE(ConstraintDefinition)

  //ฤัoต
  CREATE_NEW_PT_NODE(ProgramCaller)
  CREATE_NEW_PT_NODE(ConstraintCaller)
  
  //ง๑ฎ
  CREATE_NEW_PT_NODE(Constraint);

  //Tellง๑
  CREATE_NEW_PT_NODE(Tell)

  //Askง๑
  CREATE_NEW_PT_NODE(Ask)

  //ไrZq
  CREATE_NEW_PT_NODE(Equal)
  CREATE_NEW_PT_NODE(UnEqual)
  CREATE_NEW_PT_NODE(Less)
  CREATE_NEW_PT_NODE(LessEqual)
  CREATE_NEW_PT_NODE(Greater)
  CREATE_NEW_PT_NODE(GreaterEqual)

  //_Zq
  CREATE_NEW_PT_NODE(LogicalAnd)
  CREATE_NEW_PT_NODE(LogicalOr)

  //Zp๑Zq
  CREATE_NEW_PT_NODE(Plus)
  CREATE_NEW_PT_NODE(Subtract)
  CREATE_NEW_PT_NODE(Times)
  CREATE_NEW_PT_NODE(Divide)
  
  //ZpPZq
  CREATE_NEW_PT_NODE(Negative)
  CREATE_NEW_PT_NODE(Positive)

  //ง๑Kw่`Zq
  CREATE_NEW_PT_NODE(Weaker)
  CREATE_NEW_PT_NODE(Parallel)

  // Zq
  CREATE_NEW_PT_NODE(Always)

  //๗ช
  CREATE_NEW_PT_NODE(Differential)
  
  //ถษภ
  CREATE_NEW_PT_NODE(Previous)

  //ฯEฉฯ
  CREATE_NEW_PT_NODE(Variable)

  //
  CREATE_NEW_PT_NODE(Number)
};

} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_NODE_FACTORY_H_
