#include "Automaton.h"
#include "../symbolic_expression/Node.h"
#include <iostream>
#include <vector>
#include <string>
using namespace std;
using namespace hydla;
using namespace simulator;
using namespace symbolic_expression;

Automaton::Automaton(){
  this->id = 0;
  this->name = "no_name";
  this->parent_node = NULL;
  this->color =  "#000000";
  this->write = 0;
  this->trace_path.push_back(this);
}

Automaton::Automaton(std::string name){
  this->id = 0;
  this->name = name;
  this->parent_node = NULL;
  this->color =  "#000000";
  this->write = 0;
  this->trace_path.push_back(this);
}

Automaton::Automaton(std::string name,int id){
  this->id = id;
  this->name = name;
  this->parent_node = NULL;
  this->color =  "#000000";
  this->write = 0;
  this->trace_path.push_back(this);
}

void Automaton::add_next_edge(Automaton * child){
  node_sptr true_node = node_sptr(new hydla::symbolic_expression::True());
  add_next_edge(child,true_node);
}

void Automaton::add_next_edge(Automaton * child,node_sptr guard){
  this->next_edge.push_back(std::pair<Automaton*,node_sptr>(child,guard));
  if(child->parent_node == NULL){
    std::vector<Automaton *> child_trace_path = this->trace_path;
    child_trace_path.push_back(child);
    child->parent_node = this;
    child->trace_path = child_trace_path;
  }
}

void Automaton::set_id(int id){
  this->id = id;
}

void Automaton::set_name(std::string name){
  this->name = name;
}

void Automaton::set_color(std::string color){
  this->color = color;
}

void Automaton::set_color_to_trace_path(std::string color){
  for(typename std::vector<Automaton *>::iterator it = trace_path.begin();it != trace_path.end();it++){
    (*it)->set_color(color);
  }
}

void Automaton::trace(){
  std::cout << "// this is trace of " << this->name << "." << std::endl;
  for(typename std::vector<Automaton *>::iterator it = this->trace_path.begin();it + 1 != this->trace_path.end();it++){
    if(it == this->trace_path.begin()){
      std::cout << "\"init\"[shape=\"point\"];" << std::endl;
      std::cout << "\"init\"" << " -> " << "\"" << (*it)->name << "\"" << "[color=\"" << (*it)->color << "\"];" << std::endl;
    }
    if((it + 1) != trace_path.end()){
      std::cout << "\"" << (*it)->name << "\"" << " " << "[color=\"" << (*it)->color << "\"];" << std::endl;
      if((*it)->color == (*(it + 1))->color){
        std::cout << "\"" << (*it)->name << "\"" << " -> " << "\"" << (*(it + 1))->name << "\"" << "[color=\"" << (*it)->color << "\"];" << std::endl;
      }else{
        std::cout << "\"" << (*it)->name << "\"" << " -> " << "\"" << (*(it + 1))->name << "\"" << ";" << std::endl;
      }
    }
  }
  std::cout << "// " << this->name << " is traced." << std::endl;
}

void Automaton::dump(){
  std::cout << "~~~~~~~~ dump : " << this->name << " start. ~~~~~~~~" << std::endl;
  std::cout << "digraph g{" << std::endl;
  this->trace();
  this->write_reset();
  std::cout << "// nodes and links after " << this->name << "." << std::endl;
  this->dump_node_and_edge();
  this->write_reset();
  std::cout << "}" << std::endl;
  std::cout << "~~~~~~~~ dump : " << this->name << " end.  ~~~~~~~~" << std::endl;
}

void Automaton::output_dot(){
  std::cout << "======== Out put dot file ========" << std::endl;
  this->write_reset();
  std::cout << "digraph g{" << std::endl;
  std::cout << "\"init\"[shape=\"point\"];" << std::endl;
  std::cout << "\"init\"" << " -> " << "\"" << this->name << "\"" << "[color=\"" << this->color << "\"];" << std::endl;
  this->dump_node_and_edge();
  std::cout << "}" << std::endl;
  this->write_reset();
  std::cout << "==================================" << std::endl;
}

void Automaton::dump_node_and_edge(){
  if(write == 0){
    write++;
    std::cout << "\"" << this->name << "\"" << " " << "[color=\"" << this->color << "\"];" << std::endl;
    for(typename std::vector<std::pair<Automaton *,node_sptr>>::iterator it = next_edge.begin();it != next_edge.end();it++){
      if(this->name == (*it).first->name){
        std::cout << "\"" << this->name << "\"" << " -> " << "\"" << this->name << "\"" << " " <<  "[color=\"" << this->color << "\"];" << std::endl;
      }else{
        if(this->color == (*it).first->color){
          std::cout << "\"" << this->name << "\"" << " -> " << "\"" << (*it).first->name << "\"" << "[color=\"" << this->color << "\"];" << std::endl;
        }else{
          std::cout << "\"" << this->name << "\"" << " -> " << "\"" << (*it).first->name << "\"" << ";" << std::endl;
        }
        (*it).first->dump_node_and_edge();
      }
    }
  }
}

void Automaton::write_reset(){
  if(write > 0){
    write = 0;
    for(typename std::vector<std::pair<Automaton *,node_sptr>>::iterator it = next_edge.begin();it != next_edge.end();it++){
      (*it).first->write_reset();
    }
  }
}
