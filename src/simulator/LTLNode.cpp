#include "PhaseResult.h"
#include "PhaseSimulator.h"
#include "PropertyNode.h"
#include "../symbolic_expression/Node.h"
#include "LTLNode.h"
#include "ConsistencyChecker.h"
#include "ValueModifier.h"
#include <iostream>
#include <vector>
#include <string>
using namespace std;
using namespace hydla;
using namespace simulator;
using namespace symbolic_expression;

LTLNode::LTLNode(phase_result_sptr_t set_phase,PropertyNode* set_property):Automaton("Property" + (set_property->name) + " Phase" + to_string(set_phase->id)){
  this->phase = set_phase;
  this->property = set_property;
  this->parent_node = NULL;
  this->checked_next_link = false;
}

LTLNode::LTLNode(string name,phase_result_sptr_t set_phase,PropertyNode* set_property):Automaton(name){
  this->phase = set_phase;
  this->property = set_property;
  this->parent_node = NULL;
  this->checked_next_link = false;
}


bool LTLNode::acceptanceState(){
  return (property->type == ACCEPTANCE_STATE);
}

bool LTLNode::acceptanceCycle(){
  return (property->type == ACCEPTANCE_CYCLE);
}
