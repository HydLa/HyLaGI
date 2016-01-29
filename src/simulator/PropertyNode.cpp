#include "PropertyNode.h"
#include "Automaton.h"
#include "Utility.h"
#include "../symbolic_expression/Node.h"
#include <string>
#include <vector>

using namespace std;
using namespace hydla;
using namespace symbolic_expression;
using namespace simulator;

PropertyNode::PropertyNode(int id, PropertyNodeType type):AutomatonNode(phase_result_sptr_t(), to_string(id),id){
  this->type = type;
  if(type == ACCEPTANCE_CYCLE | type == ACCEPTANCE_STATE){
    this->set_peripheries(2);
  }
}
