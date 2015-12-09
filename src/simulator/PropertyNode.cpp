#include "PropertyNode.h"
#include "Automaton.h"
#include "Utility.h"
#include "../symbolic_expression/Node.h"
#include <string>
#include <vector>

using namespace std;
using namespace hydla;
using namespace symbolic_expression;

PropertyNode::PropertyNode(int id, PropertyNodeType type):Automaton(to_string(id),id){
  this->type = type;
}
