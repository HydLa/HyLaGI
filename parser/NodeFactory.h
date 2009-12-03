#ifndef _INCLUDED_HYDLA_NODE_FACTORY_H_
#define _INCLUDED_HYDLA_NODE_FACTORY_H_

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/utility/enable_if.hpp> 
#include <boost/type_traits/is_same.hpp> 

#include "ParseTree.h"

namespace hydla { 
namespace parser {

#define DEFAULT_NODE_FACTORY_PT_NODE(NAME)              \
  boost::shared_ptr<hydla::parse_tree::NAME>            \
  operator()(hydla::parse_tree::NAME)                   \
  {                                                     \
    return boost::shared_ptr<hydla::parse_tree::NAME>(  \
      new hydla::parse_tree::NAME());                   \
  }                                                      

struct DefaultNodeFactory
{

  //่`
  DEFAULT_NODE_FACTORY_PT_NODE(ProgramDefinition)
  DEFAULT_NODE_FACTORY_PT_NODE(ConstraintDefinition)

  //ฤัoต
  DEFAULT_NODE_FACTORY_PT_NODE(ProgramCaller)
  DEFAULT_NODE_FACTORY_PT_NODE(ConstraintCaller)
  
  //ง๑ฎ
  DEFAULT_NODE_FACTORY_PT_NODE(Constraint);

  //Tellง๑
  DEFAULT_NODE_FACTORY_PT_NODE(Tell)

  //Askง๑
  DEFAULT_NODE_FACTORY_PT_NODE(Ask)

  //ไrZq
  DEFAULT_NODE_FACTORY_PT_NODE(Equal)
  DEFAULT_NODE_FACTORY_PT_NODE(UnEqual)
  DEFAULT_NODE_FACTORY_PT_NODE(Less)
  DEFAULT_NODE_FACTORY_PT_NODE(LessEqual)
  DEFAULT_NODE_FACTORY_PT_NODE(Greater)
  DEFAULT_NODE_FACTORY_PT_NODE(GreaterEqual)

  //_Zq
  DEFAULT_NODE_FACTORY_PT_NODE(LogicalAnd)
  DEFAULT_NODE_FACTORY_PT_NODE(LogicalOr)

  //Zp๑Zq
  DEFAULT_NODE_FACTORY_PT_NODE(Plus)
  DEFAULT_NODE_FACTORY_PT_NODE(Subtract)
  DEFAULT_NODE_FACTORY_PT_NODE(Times)
  DEFAULT_NODE_FACTORY_PT_NODE(Divide)
  
  //ZpPZq
  DEFAULT_NODE_FACTORY_PT_NODE(Negative)
  DEFAULT_NODE_FACTORY_PT_NODE(Positive)

  //ง๑Kw่`Zq
  DEFAULT_NODE_FACTORY_PT_NODE(Weaker)
  DEFAULT_NODE_FACTORY_PT_NODE(Parallel)

  // Zq
  DEFAULT_NODE_FACTORY_PT_NODE(Always)

  //๗ช
  DEFAULT_NODE_FACTORY_PT_NODE(Differential)
  
  //ถษภ
  DEFAULT_NODE_FACTORY_PT_NODE(Previous)

  //ฯEฉฯ
  DEFAULT_NODE_FACTORY_PT_NODE(Variable)

  //
  DEFAULT_NODE_FACTORY_PT_NODE(Number)

};

} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_NODE_FACTORY_H_
