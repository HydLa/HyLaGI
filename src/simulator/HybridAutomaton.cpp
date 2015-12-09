#include "PhaseResult.h"
#include "PhaseSimulator.h"
#include "PropertyNode.h"
#include "../symbolic_expression/Node.h"
#include "HybridAutomaton.h"
#include "ConsistencyChecker.h"
#include "ValueModifier.h"
#include <iostream>
#include <vector>
#include <string>
using namespace std;
using namespace hydla;
using namespace simulator;
using namespace symbolic_expression;

HybridAutomaton::HybridAutomaton(phase_result_sptr_t set_phase):Automaton("Phase" + to_string(set_phase->id)){
  this->phase = set_phase;
}
HybridAutomaton::HybridAutomaton(string name,phase_result_sptr_t set_phase):Automaton(name){
  this->phase = set_phase;
}
