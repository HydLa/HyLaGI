#include "ParseTree.h"

#include <algorithm>
#include <boost/bind.hpp>

using namespace std;
using namespace boost;
using namespace hydla::parse_error;

namespace hydla { 
namespace parse_tree {

ParseTree::ParseTree()
{}

ParseTree::~ParseTree()
{}

void ParseTree::addConstraintDefinition(boost::shared_ptr<ConstraintDefinition> d) 
{
  cons_def_map_.insert(
    make_pair(create_definition_key(d), d));
}

void ParseTree::addProgramDefinition(boost::shared_ptr<ProgramDefinition> d) 
{
  prog_def_map_.insert(
    make_pair(create_definition_key(d), d));
}

difinition_type_t 
ParseTree::create_definition_key(boost::shared_ptr<Definition> d)
{
  std::string name = d->get_name();
  int bound_variable_count = d->get_bound_variables()->size();

  // 既に制約/プログラム定義として定義されていないかどうか
  if(cons_def_map_.find(make_pair(name, bound_variable_count)) != cons_def_map_.end() ||
     prog_def_map_.find(make_pair(name, bound_variable_count)) != prog_def_map_.end()) 
  {
    throw MultipleDefinition(name);
  }

  return make_pair(name, bound_variable_count);
}

namespace {

template <typename T>
struct AppendDefinitionString {
  AppendDefinitionString(string& str) : 
    s(str),
    first(true)
  {}

  void operator()(T c) {
    if(first) {
      s += c.second->to_string();
      first = false;
    } else {
      s += ",\n" + c.second->to_string();
    }
  }

  bool first;
  string& s;
};
}

std::string ParseTree::to_string()
{
  string str;
  str += "parse_tree[\n";
  str += "constraint_definition[\n";
  for_each(cons_def_map_.begin(), cons_def_map_.end(), 
           AppendDefinitionString<pair<difinition_type_t, 
                                       constraint_def_map_value_t> >(str));
  str += "],\n";

  str += "program_definition[\n";
  for_each(prog_def_map_.begin(), prog_def_map_.end(), 
           AppendDefinitionString<pair<difinition_type_t, 
                                       program_def_map_value_t> >(str));
  str += "],\n";

  str += "node_tree[\n";
  if(node_tree_) str += node_tree_->to_string();
  str += "]]\n";

  return str;
}

void ParseTree::preprocess()
{
  if(node_tree_) {
    formal_arg_map_t fam;
    preprocess_arg_t arg(variable_map_, prog_def_map_, cons_def_map_, fam);
    node_tree_->preprocess(node_tree_, arg);
  }
}

} //namespace parse_tree
} //namespace hydla
