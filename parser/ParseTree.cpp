#include "ParseTree.h"

#include <iostream>
#include <iterator>
#include <algorithm>
#include <boost/bind.hpp>

#include "ParseTreeSemanticAnalyzer.h"

using namespace std;
using namespace boost;
using namespace hydla::parser;
using namespace hydla::parse_error;

namespace hydla { 
namespace parse_tree {

ParseTree::ParseTree() :
  max_node_id_(0)
{}

    
ParseTree::ParseTree(const ParseTree& pt) :
  node_tree_(pt.node_tree_->clone()),
  cons_def_map_(pt.cons_def_map_),
  prog_def_map_(pt.prog_def_map_),
  node_map_(pt.node_map_),
  max_node_id_(pt.max_node_id_)
{
  std::cout << "#AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAa\n"
    <<*node_tree_;

  analyze_tree();
}

ParseTree::~ParseTree()
{}

void ParseTree::analyze_tree()
{
  variable_map_.clear();

  ParseTreeSemanticAnalyzer analyer;  
  analyer.analyze(this);
}

ParseTree::difinition_type_t 
ParseTree::create_definition_key(boost::shared_ptr<Definition> d)
{
  std::string name = d->get_name();
  int bound_variable_count = d->bound_variable_size();

  // 既に制約/プログラム定義として定義されていないかどうか
  if(cons_def_map_.find(make_pair(name, bound_variable_count)) != cons_def_map_.end() ||
     prog_def_map_.find(make_pair(name, bound_variable_count)) != prog_def_map_.end()) 
  {
    throw MultipleDefinition(name);
  }

  return make_pair(name, bound_variable_count);
}

namespace {
  template<typename T>
  void definition_dumper(std::ostream& s, const T& m)
  {
    typedef typename T::const_iterator citer_t;
    citer_t it  = m.begin();
    citer_t end = m.end();

    if(it!=end) s << "  " << *(it++)->second;
    for(; it!=end; ++it) {
      s << ",\n  " << *it->second;
    }
  }

  template<typename T>
  void variable_dumper(std::ostream& s, const T& m)
  {
    typedef typename T::const_iterator citer_t;
    citer_t it  = m.begin();
    citer_t end = m.end();

    if(it!=end) {
      s << "  " << it->first << ":" << it->second;
      ++it;
    }
    for(; it!=end; ++it) {
      s << ",\n  " << it->first << ":" << it->second;
    }
  }
}

std::ostream& ParseTree::dump(std::ostream& s) const 
{
  s << "parse_tree[\n"
    << "constraint_definition[\n";  
  definition_dumper(s, cons_def_map_);
  s << "],\n";

  s << "program_definition[\n";
  definition_dumper(s, prog_def_map_);
  s << "],\n";

  s << "node_tree[\n";
  if(node_tree_) s << "  " << *node_tree_;
  s << "],\n";

  s << "variables[\n";
  variable_dumper(s, variable_map_);
  s << "]]";

  return s;
}

std::ostream& operator<<(std::ostream& s, const ParseTree& pt)
{
  return pt.dump(s);
}

} //namespace parse_tree
} //namespace hydla
