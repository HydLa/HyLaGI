#include "ParseTree.h"

#include <iostream>
#include <iterator>
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include "Logger.h"
#include "HydLaAST.h"
#include "NodeIDUpdater.h"
#include "NodeIDRebuilder.h"
#include "NodeTreeGenerator.h"
#include "ParseTreeSemanticAnalyzer.h"
#include "ParseTreeGraphvizDumper.h"
#include "DefinitionContainer.h"


using namespace std;
using namespace boost;
using namespace hydla::parser;
using namespace hydla::parser::error;
using namespace hydla::logger;

namespace hydla { 
namespace parse_tree {

ParseTree::ParseTree() :
  max_node_id_(INITIAL_MAX_NODE_ID)
{}

    
ParseTree::ParseTree(const ParseTree& pt) :
  node_tree_(pt.node_tree_ ? pt.node_tree_->clone() : symbolic_expression::node_sptr()),
  variable_map_(pt.variable_map_),
  node_map_(pt.node_map_),
  max_node_id_(pt.max_node_id_)
{
  update_node_id_list();
}

ParseTree::~ParseTree()
{}

void ParseTree::parse(std::istream& stream) 
{
  HYDLA_LOGGER_DEBUG("");

  // ASTの構築
  HydLaAST ast;
  ast.parse(stream);
  HYDLA_LOGGER_DEBUG("--- AST Tree ---\n", ast);
  

  // ParseTreeの構築
  DefinitionContainer<hydla::symbolic_expression::ConstraintDefinition> constraint_definition;
  DefinitionContainer<hydla::symbolic_expression::ProgramDefinition>    program_definition;
  NodeTreeGenerator genarator(assertion_node_tree_, constraint_definition, program_definition);
  node_tree_ = genarator.generate(ast.get_tree_iterator());
  HYDLA_LOGGER_DEBUG("--- Parse Tree ---\n", *this);
  HYDLA_LOGGER_DEBUG("--- Constraint Definition ---\n", constraint_definition);
  HYDLA_LOGGER_DEBUG("--- Program Definition ---\n",    program_definition);
  if(assertion_node_tree_)
    HYDLA_LOGGER_DEBUG("--- Assertion Tree ---\n", *assertion_node_tree_);
  // 意味解析
  ParseTreeSemanticAnalyzer analyer(constraint_definition, program_definition, this);  
  analyer.analyze(node_tree_);
  update_node_id_list();
  HYDLA_LOGGER_DEBUG("--- Analyzed Parse Tree ---\n", *this);
  HYDLA_LOGGER_DEBUG("");
}

void ParseTree::rebuild_node_id_list()
{
  node_map_.clear();
  max_node_id_ = INITIAL_MAX_NODE_ID;

  NodeIDRebuilder rebuilder;
  rebuilder.rebuild(this);  
}

void ParseTree::update_node_id_list()
{
  NodeIDUpdater updater;
  updater.update(this);
}

symbolic_expression::node_sptr ParseTree::swap_tree(const symbolic_expression::node_sptr& tree) 
{
  symbolic_expression::node_sptr ret(node_tree_);
  node_tree_ = tree;
  update_node_id_list();
  return ret;
}

std::ostream& ParseTree::to_graphviz(std::ostream& s) const
{
  ParseTreeGraphvizDumper dumper;
  if(node_tree_)
    dumper.dump(s, node_tree_);
  else
    s << "ParseTree is empty." << std::endl;
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

symbolic_expression::node_id_t ParseTree::register_node(const symbolic_expression::node_sptr& n)
{
  assert(n->get_id() == 0);
  assert(node_map_.find(n->get_id()) == node_map_.end());
    
  ++max_node_id_;
  n->set_id(max_node_id_);
  node_map_.insert(std::make_pair(max_node_id_, n));
  return max_node_id_;
}

void ParseTree::update_node(symbolic_expression::node_id_t id, const symbolic_expression::node_sptr& n)
{
  node_map_t::iterator it = node_map_.find(id);
  assert(it != node_map_.end());

  it->second = n;
}

void ParseTree::remove_node(symbolic_expression::node_id_t id)
{
  node_map_t::iterator it = node_map_.find(id);
  if(it!=node_map_.end()) {
    node_map_.erase(it);
  }
}


/*
symbolic_expression::node_id_t ParseTree::register_node(const symbolic_expression::node_sptr& n)
{
  assert(n->get_id() == 0);
  assert(node_map_.by<tag_symbolic_expression::node_sptr>().find(n) == 
            node_map_.by<tag_symbolic_expression::node_sptr>().end());
    
  ++max_node_id_;
  n->set_id(max_node_id_);
  node_map_.insert(node_map_value_t(max_node_id_, n));
  return max_node_id_;
}

void ParseTree::update_node(symbolic_expression::node_id_t id, const symbolic_expression::node_sptr& n)
{
  node_map_t::map_by<tag_node_id>::iterator it = 
    node_map_.by<tag_node_id>().find(id);
  assert(it != node_map_.by<tag_node_id>().end());

  node_map_.by<tag_node_id>().
    modify_data(it, boost::bimaps::_data = n);
}

void ParseTree::update_node_id(symbolic_expression::node_id_t id, const symbolic_expression::node_sptr& n)
{
  node_map_t::map_by<tag_symbolic_expression::node_sptr>::iterator it = 
    node_map_.by<tag_symbolic_expression::node_sptr>().find(n);
  assert(it != node_map_.by<tag_symbolic_expression::node_sptr>().end());
      
  node_map_.by<tag_symbolic_expression::node_sptr>().
    modify_data(it, boost::bimaps::_data = id);
}
*/

void ParseTree::make_node_id_list()
{

  node_id_list_.clear();

  node_map_const_iterator it = node_map_begin();
  node_map_const_iterator end = node_map_end();
  for(; it!=end; ++it)
  {
    node_id_list_.insert((*it).first);
  }
}

void ParseTree::clear()
{
  max_node_id_ = INITIAL_MAX_NODE_ID;
  node_map_.clear();
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
      /*
    << "constraint_definition[\n";  
  definition_dumper(s, cons_def_map_);
  s << "],\n";

  s << "program_definition[\n";
  definition_dumper(s, prog_def_map_);
  s << "],\n";
  */
  s << "--- node_tree ---\n";
  if(node_tree_){
    s << *node_tree_;
  }
  s << "\n";
  

  s << "--- variables ---\n";
  BOOST_FOREACH(const variable_map_t::value_type& i, 
                variable_map_) 
  {
    s << i.first << " : " << i.second << "\n";
  }

  s << "--- node id table (id -> sptr) ---\n";
  BOOST_FOREACH(const node_map_value_t& i, node_map_) 
  {
    s << i.first << " => " 
      << i.second << "\n";
  }
/*
  s << "--- node id table (sptr -> id) ---\n";
  BOOST_FOREACH(const node_map_t::map_by<tag_symbolic_expression::node_sptr>::value_type& i, 
                node_map_.by<tag_symbolic_expression::node_sptr>()) 
  {
    s << i.get<tag_symbolic_expression::node_sptr>() << " => " 
      << i.get<tag_node_id>() << "\n";
  }
*/
  return s;
}

std::ostream& operator<<(std::ostream& s, const ParseTree& pt)
{
  return pt.dump(s);
}

} //namespace parse_tree
} //namespace hydla
