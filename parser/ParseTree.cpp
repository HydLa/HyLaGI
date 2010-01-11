#include "ParseTree.h"

#include <iostream>
#include <iterator>
#include <algorithm>
#include <fstream>
#include <boost/bind.hpp>

#include "Logger.h"
#include "HydLaAST.h"
#include "NodeIDUpdater.h"
#include "NodeTreeGenerator.h"
#include "ParseTreeSemanticAnalyzer.h"
#include "ParseTreeGraphvizDumper.h"
#include "DefinitionContainer.h"

#include "DefaultNodeFactory.h"


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
  variable_map_(pt.variable_map_),
  node_map_(pt.node_map_),
  max_node_id_(pt.max_node_id_)
{
  uptate_node_id();
}

ParseTree::~ParseTree()
{}

void ParseTree::parse(std::istream& stream) 
{
  HydLaAST ast;
  ast.parse(stream);

  HYDLA_LOGGER_DEBUG("#*** AST Tree ***\n", ast);

    
  DefinitionContainer<hydla::parse_tree::ConstraintDefinition> constraint_definition;
  DefinitionContainer<hydla::parse_tree::ProgramDefinition>    program_definition;
  boost::shared_ptr<NodeFactory> node_factory(new DefaultNodeFactory());
  NodeTreeGenerator genarator(constraint_definition, program_definition, node_factory);
  node_tree_ = genarator.generate(ast.get_tree_iterator());

  HYDLA_LOGGER_DEBUG("#*** Parse Tree ***\n", *this);

  HYDLA_LOGGER_DEBUG("#*** Constraint Definition ***\n", constraint_definition);
  HYDLA_LOGGER_DEBUG("#*** Program Definition ***\n",    program_definition);
    
  ParseTreeSemanticAnalyzer analyer(constraint_definition, program_definition, this);  
  analyer.analyze(node_tree_);

  HYDLA_LOGGER_DEBUG("#*** Analyzed Parse Tree ***\n", *this);

  uptate_node_id();
}

void ParseTree::parse_flie(const std::string& filename) 
{
  ifstream in(filename.c_str());
  if (!in) {    
    throw std::runtime_error(string("cannot open \"") + filename + "\"");
  }

  parse(in);
}

void ParseTree::parse_string(const std::string& str)
{    
  stringstream in(str);
  parse(in);
}

void ParseTree::uptate_node_id()
{
  NodeIDUpdater updater;
  updater.update(this);
}

void ParseTree::set_tree(const node_sptr& tree) 
{
  node_tree_ = tree;
//  semantic_analyze();
}

node_sptr ParseTree::swap_tree(const node_sptr& tree) 
{
  node_sptr ret(node_tree_);
  node_tree_ = tree;
  uptate_node_id();
  return ret;
}

std::ostream& ParseTree::to_graphviz(std::ostream& s) const
{
  ParseTreeGraphvizDumper dumper;
  dumper.dump(s, node_tree_);
  return s;
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
  variable_map_.clear();
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
  s << "parse_tree[\n";
    /*
    << "constraint_definition[\n";  
  definition_dumper(s, cons_def_map_);
  s << "],\n";

  s << "program_definition[\n";
  definition_dumper(s, prog_def_map_);
  s << "],\n";
  */

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
