#include "Automaton.h"
#include "PhaseResult.h"
#include "PhaseSimulator.h"
#include "../symbolic_expression/Node.h"
#include "ConsistencyChecker.h"
#include "ValueModifier.h"
#include <iostream>
#include <vector>
#include <string>
using namespace std;
using namespace hydla;
using namespace simulator;
using namespace symbolic_expression;

Automaton::Automaton(string name){
  this->name = name;
  this->parent_node = NULL;
  this->color = "#000000";
  this->write = 0;
  automaton_node_list_t self;
  self.push_back(this);
  this->trace_path = self;
}

void Automaton::add_next_link(Automaton* child){
  this->next_link.push_back(child);
  if(child->parent_node == NULL){
    automaton_node_list_t child_trace_path = this->trace_path;
    child_trace_path.push_back(child);
    child->parent_node = this;
    child->trace_path = child_trace_path;
  }
}

void Automaton::set_color(string color){
  this->color = color;
}

void Automaton::set_color_to_trace_path(string color){
  for(automaton_node_list_t::iterator it = trace_path.begin();it != trace_path.end();it++){
    (*it)->set_color(color);
  }
}

void Automaton::trace(){
  cout << "// this is trace of " << this->name << "." << endl;
  for(automaton_node_list_t::iterator it = this->trace_path.begin();it + 1 != this->trace_path.end();it++){
    if((it + 1) != trace_path.end()){
      cout << "\"" << (*it)->name << "\"" << " " << "[color=" << (*it)->color << "];" << endl;
      if((*it)->color == (*(it + 1))->color){
        cout << "\"" << (*it)->name << "\"" << " -> " << "\"" << (*(it + 1))->name << "\"" << "[color=" << (*it)->color << "];" << endl;
      }else{
        cout << "\"" << (*it)->name << "\"" << " -> " << "\"" << (*(it + 1))->name << "\"" << ";" << endl;
      }
    }
  }
  cout << "// " << this->name << " is traced." << endl;
}

void Automaton::dump(){
  cout << "~~~~~~~~~~~ dump : " << this->name << " ~~~~~~~~~~~" << endl;
  cout << "digraph g{" << endl;
  this->trace();
  this->write_reset();
  cout << "// nodes and links after " << this->name << "." << endl;
  this->dump_node_and_link();
  this->write_reset();
  cout << "}" << endl;
  cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

void Automaton::output_dot(){
  this->write_reset();
  cout << "digraph g{" << endl;
  cout << "\"init\"[shape=\"point\"];" << endl;
  cout << "\"init\"" << " -> " << "\"" << this->name << "\"" << "[color=" << this->color << "];" << endl;
  this->dump_node_and_link();
  cout << "}" << endl;
  this->write_reset();
}

void Automaton::dump_node_and_link(){
  if(write == 0){
    write++;
    cout << "\"" << this->name << "\"" << " " << "[color=" << this->color << "];" << endl;
    for(automaton_node_list_t::iterator it = next_link.begin();it != next_link.end();it++){
      if(this->name == (*it)->name){
        cout << "\"" << this->name << "\"" << " -> " << "\"" << this->name << "\"" << "[color=" << this->color << "];" << endl;
      }else{
        if(this->color == (*it)->color){
          cout << "\"" << this->name << "\"" << " -> " << "\"" << (*it)->name << "\"" << "[color=" << this->color << "];" << endl;
        }else{
          cout << "\"" << this->name << "\"" << " -> " << "\"" << (*it)->name << "\"" << ";" << endl;
        }
        (*it)->dump_node_and_link();
      }
    }
  }
}

void Automaton::write_reset(){
  if(write > 0){
    write = 0;
    for(automaton_node_list_t::iterator it = next_link.begin();it != next_link.end();it++){
      (*it)->write_reset();
    }
  }
}
