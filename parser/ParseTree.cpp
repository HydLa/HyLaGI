#include "ParseTree.h"

#include <iostream>
#include <iterator>
#include <algorithm>
#include <boost/bind.hpp>

#include "NodeIDUpdater.h"
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
  variable_map_(pt.variable_map_),
  node_map_(pt.node_map_),
  max_node_id_(pt.max_node_id_)
{
  uptate_node_id();
}

ParseTree::~ParseTree()
{}


void ParseTree::uptate_node_id()
{
  NodeIDUpdater updater;
  updater.update(this);
}

void ParseTree::semantic_analyze()
{
  ParseTreeSemanticAnalyzer analyer;  
  analyer.analyze(this);

  uptate_node_id();
}

void ParseTree::set_tree(const node_sptr& tree) 
{
  node_tree_ = tree;
  semantic_analyze();
}

node_sptr ParseTree::swap_tree(const node_sptr& tree) 
{
  node_sptr ret(node_tree_);
  node_tree_ = tree;
  uptate_node_id();
  return ret;
}

bool ParseTree::register_variable(const std::string& name, 
                                  int differential_count)
{
  variable_map_t::iterator it = variable_map_.find(name);
  if(it == variable_map_.end() ||
     it->second < differential_count) 
  {
    variable_map_[name] = differential_count;
    return true;
  }
  return false;
}

int ParseTree::get_differential_count(const std::string& name) const
{
  int ret = -1;

  variable_map_t::const_iterator it = variable_map_.find(name);
  if(it != variable_map_.end()) {
    ret =  it->second;
  }

  return ret;
}

const boost::shared_ptr<ConstraintDefinition> 
ParseTree::get_constraint_difinition(const difinition_type_t& def) const
{
  constraint_def_map_t::const_iterator it = cons_def_map_.find(def);
  if(it == cons_def_map_.end()) {
    return boost::shared_ptr<ConstraintDefinition>();
  }
  return it->second;
}

const boost::shared_ptr<ProgramDefinition> 
ParseTree::get_program_difinition(const difinition_type_t& def) const
{
  program_def_map_t::const_iterator it = prog_def_map_.find(def);
  if(it == prog_def_map_.end()) {
    return boost::shared_ptr<ProgramDefinition>();
  }
  return it->second;
}

node_id_t ParseTree::register_node(const node_sptr& n)
{
  assert(n->get_id() == 0);
  assert(node_map_.right.find(n) == node_map_.right.end());
    
  ++max_node_id_;
  n->set_id(max_node_id_);
  node_map_.insert(node_map_value_t(max_node_id_, n));
  return max_node_id_;
}

void ParseTree::update_node(node_id_t id, const node_sptr& n)
{
  node_map_t::left_iterator it = node_map_.left.find(id);
  assert(it != node_map_.left.end());

  node_map_.left.modify_data(it, boost::bimaps::_data = n);
}

void ParseTree::update_node_id(node_id_t id, const node_sptr& n)
{
  node_map_t::right_iterator it = node_map_.right.find(n);
  assert(it != node_map_.right.end());
      
  node_map_.right.modify_data(it, boost::bimaps::_data = id);
}

void ParseTree::clear()
{
  max_node_id_ = 0;
  node_tree_.reset();
  cons_def_map_.clear();
  prog_def_map_.clear();
  variable_map_.clear();
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
