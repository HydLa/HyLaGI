#ifndef _INCLUDED_HYDLA_ASK_COLLECTOR_H_
#define _INCLUDED_HYDLA_ASK_COLLECTOR_H_

#include <vector>
#include <set>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "ModuleSet.h"
#include "TreeVisitor.h"

namespace hydla {
namespace simulator {

class AskCollector : public parse_tree::TreeVisitor {
public:
  AskCollector();
  virtual ~AskCollector();
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_ASK_COLLECTOR_H_
