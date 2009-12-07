#include "ParseTree.h"

#include <algorithm>
#include <boost/bind.hpp>

using namespace std;
using namespace boost;
using namespace hydla::parse_error;

namespace hydla { 
namespace parse_tree {

ParseTree::ParseTree() :
  max_node_id_(0)
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

struct DefinitionDumper {
  DefinitionDumper(std::ostream& str) : 
    s(str),
    first(true)
  {}


  template <typename T>
  void operator()(const T& c) {
    if(first) {
      s << *c.second;
      first = false;
    } else {
      s << ",\n" << *c.second;
    }
  }

  std::ostream& s;
  bool first;
};
}

std::ostream& ParseTree::dump(std::ostream& s) const 
{
  s << "parse_tree[\n"
    << "constraint_definition[\n";
  for_each(cons_def_map_.begin(), 
           cons_def_map_.end(), 
           DefinitionDumper(s));
  s << "],\n";

  s << "program_definition[\n";
  for_each(prog_def_map_.begin(), 
           prog_def_map_.end(), 
           DefinitionDumper(s));
  s << "],\n";

  s << "node_tree[\n";
  if(node_tree_) s << *node_tree_;
  s << "]]\n";

  return s;
}

std::ostream& operator<<(std::ostream& s, const ParseTree& pt)
{
  return pt.dump(s);
}

} //namespace parse_tree
} //namespace hydla
