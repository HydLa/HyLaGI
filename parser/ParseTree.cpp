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

  string& s;
  bool first;
};
}

std::string ParseTree::to_string() const
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

std::ostream& operator<<(std::ostream& s, const ParseTree& pt)
{
  return s << pt.to_string();
}

} //namespace parse_tree
} //namespace hydla
