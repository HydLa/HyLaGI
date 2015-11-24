#include "PropertyNode.h"
#include "Automaton.h"
#include "Utility.h"
#include "../symbolic_expression/Node.h"
#include <string>
#include <vector>

using namespace std;
using namespace hydla;
using namespace symbolic_expression;

PropertyNode::PropertyNode(int set_name, PropertyNodeType set_type):Automaton(to_string(set_name)){
  type = set_type;
}

PropertyNode::~PropertyNode(){
}

void PropertyNode::add_next_link(node_sptr guard,PropertyNode* child){
  link.push_back(pair<node_sptr,PropertyNode*>(guard,child));
}

// void PropertyNode::dump(){
//   if(write == 0){
//     write++;
//     string name = "\"Property" + to_string(name) + "\"";
//     cout << name << " ";
//     if(type != NOMAL){
//       cout << "[peripheries=2];" << endl;
//     }else{
//       cout << ";" << endl;
//     }
//     for(Property_link_t::iterator it = link.begin();it != link.end();it++){
//       if(name != it->second->name){
//         cout << name << " -> " << "\"Property" << it->second->name << "\" ";
//         cout << "[label=\"" << it->first->get_string() << "\"];" << endl;
//         it->second->dump();
//       }else{
//         cout << name << " -> " << name << " ";
//         cout << "[label=\"" << it->first->get_string() << "\"];" << endl;
//       }
//     }
//   }
// }

// void PropertyNode::dot(){
//   write_reset();
//   cout << "~~~~~~~~~~ property automaton ~~~~~~~~~" << endl;
//   cout << "digraph g{" << endl;
//   cout << "\"init\"[shape=\"point\"];" << endl;
//   cout << "\"init\"" << " -> " << "\"Property" << to_string(name) << "\"" << ";" << endl;
//   dump();
//   cout << "}" << endl;
//   cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
// }

// void PropertyNode::write_reset(){
//   if(write > 0){
//     write = 0;
//     for(Property_link_t::iterator it = link.begin();it != link.end();it++){
//       it->second->write_reset();
//     }
//   }
// }
