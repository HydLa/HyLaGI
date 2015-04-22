#include "PropertyNode.h"
#include "../symbolic_expression/Node.h"
#include <string>
#include <vector>

using namespace std;
using namespace hydla;
using namespace symbolic_expression;

PropertyNode::PropertyNode(int set_id, PropertyNodeType set_type){
  id = set_id;
  type = set_type;
}

PropertyNode::~PropertyNode(){
}

void PropertyNode::addLink(node_sptr guard,PropertyNode* child){
  link.push_back(pair<node_sptr,PropertyNode*>(guard,child));
}

void PropertyNode::dump(){
  if(write == 0){
    write++;
    string name = "\"Property" + to_string(id) + "\"";
    cout << name << " ";
    if(type != NOMAL){
      cout << "[peripheries=2];" << endl;
    }else{
      cout << ";" << endl;
    }
    for(Property_link_t::iterator it = link.begin();it != link.end();it++){
      if(id != it->second->id){
        cout << name << " -> " << "\"Property" << it->second->id << "\" ";
        cout << "[label=\"" << it->first->get_string() << "\"];" << endl;
        it->second->dump();
      }else{
        cout << name << " -> " << name << " ";
        cout << "[label=\"" << it->first->get_string() << "\"];" << endl;
      }
    }
  }
}

void PropertyNode::dot(){
  write_reset();
  cout << "~~~~~~~~~~ property automaton ~~~~~~~~~" << endl;
  cout << "digraph g{" << endl;
  cout << "\"init\"[shape=\"point\"];" << endl;
  cout << "\"init\"" << " -> " << "\"Property" << to_string(id) << "\"" << ";" << endl;
  dump();
  cout << "}" << endl;
  cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

void PropertyNode::write_reset(){
  if(write > 0){
    write = 0;
    for(Property_link_t::iterator it = link.begin();it != link.end();it++){
      it->second->write_reset();
    }
  }
}
