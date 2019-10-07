#include "ParseTree.h"

#include <boost/foreach.hpp>
#include <iterator>

#include "DefinitionContainer.h"
#include "Logger.h"
#include "ParseTreeGraphvizDumper.h"
#include "ParseTreeJsonDumper.h"

#include "ParseTreeSemanticAnalyzer.h"
#include "Parser.h"

using namespace std;
using namespace boost;
using namespace hydla::parser;
using namespace hydla::parser::error;
using namespace hydla::logger;

namespace hydla {
namespace parse_tree {

ParseTree::ParseTree() {}

ParseTree::ParseTree(const ParseTree &pt)
    : node_tree_(pt.node_tree_ ? pt.node_tree_->clone()
                               : symbolic_expression::node_sptr()),
      variable_map_(pt.variable_map_) {}

ParseTree::~ParseTree() {}

void ParseTree::parse(std::istream &stream) {
  auto detail = logger::Detail(__FUNCTION__);

  // ParseTreeの構築
  DefinitionContainer<hydla::symbolic_expression::ConstraintDefinition>
      constraint_definition;
  DefinitionContainer<hydla::symbolic_expression::ProgramDefinition>
      program_definition;
  DefinitionContainer<hydla::symbolic_expression::ExpressionListDefinition>
      expression_list_definition;
  DefinitionContainer<hydla::symbolic_expression::ProgramListDefinition>
      program_list_definition;

  Parser parser(stream);
  node_tree_ = parser.parse(assertion_node_tree_, constraint_definition,
                            program_definition, expression_list_definition,
                            program_list_definition);
  HYDLA_LOGGER_DEBUG("\n--- Parse Tree ---\n", *this);
  HYDLA_LOGGER_DEBUG("\n--- Constraint Definition ---\n",
                     constraint_definition);
  HYDLA_LOGGER_DEBUG("\n--- Program Definition ---\n", program_definition);
  HYDLA_LOGGER_DEBUG("\n--- Expression List Definition ---\n",
                     expression_list_definition);
  HYDLA_LOGGER_DEBUG("\n--- Program List Definition ---\n",
                     program_list_definition);
  if (assertion_node_tree_)
    HYDLA_LOGGER_DEBUG("\n--- Assertion Tree ---\n", *assertion_node_tree_);
  // 意味解析
  ParseTreeSemanticAnalyzer analyzer(constraint_definition, program_definition,
                                     expression_list_definition,
                                     program_list_definition, this);
  /*analyer_ = new ParseTreeSemanticAnalyzer(
      constraint_definition, program_definition, expression_list_definition,
      program_list_definition, this);*/
  analyzer.analyze(node_tree_);
  HYDLA_LOGGER_DEBUG("\n--- Analyzed Parse Tree ---\n", *this);
}

symbolic_expression::node_sptr
ParseTree::swap_tree(const symbolic_expression::node_sptr &tree) {
  symbolic_expression::node_sptr ret(node_tree_);
  node_tree_ = tree;
  return ret;
}

std::ostream &ParseTree::to_graphviz(std::ostream &s) const {
  ParseTreeGraphvizDumper dumper;
  if (node_tree_)
    dumper.dump(s, node_tree_);
  else
    s << "ParseTree is empty." << std::endl;
  return s;
}

std::ostream &ParseTree::dump_in_json(std::ostream &s) const {
  ParseTreeJsonDumper dumper;
  if (node_tree_)
    dumper.dump(s, node_tree_);
  else
    s << "ParseTree is empty." << std::endl;
  return s;
}

bool ParseTree::register_variable(const std::string &name,
                                  int differential_count) {
  variable_map_t::iterator it = variable_map_.find(name);
  if (it == variable_map_.end() || it->second < differential_count) {
    variable_map_[name] = differential_count;
    return true;
  }
  return false;
}

int ParseTree::get_differential_count(const std::string &name) const {
  int ret = -1;

  variable_map_t::const_iterator it = variable_map_.find(name);
  if (it != variable_map_.end()) {
    ret = it->second;
  }

  return ret;
}

void ParseTree::clear() {
  node_tree_.reset();
  variable_map_.clear();
}

namespace {
template <typename T> void definition_dumper(std::ostream &s, const T &m) {
  typedef typename T::const_iterator citer_t;
  citer_t it = m.begin();
  citer_t end = m.end();

  if (it != end)
    s << "  " << *(it++)->second;
  for (; it != end; ++it) {
    s << ",\n  " << *it->second;
  }
}

template <typename T> void variable_dumper(std::ostream &s, const T &m) {
  typedef typename T::const_iterator citer_t;
  citer_t it = m.begin();
  citer_t end = m.end();

  if (it != end) {
    s << "  " << it->first << ":" << it->second;
    ++it;
  }
  for (; it != end; ++it) {
    s << ",\n  " << it->first << ":" << it->second;
  }
}
} // namespace

std::ostream &ParseTree::dump(std::ostream &s) const {
  /*
<< "constraint_definition[\n";
definition_dumper(s, cons_def_map_);
s << "],\n";

s << "program_definition[\n";
definition_dumper(s, prog_def_map_);
s << "],\n";
*/
  s << "--- node_tree ---\n";
  if (node_tree_) {
    s << *node_tree_;
  }
  s << "\n";

  s << "--- variables ---\n";
  BOOST_FOREACH (const variable_map_t::value_type &i, variable_map_) {
    s << i.first << " : " << i.second << "\n";
  }

  return s;
}

std::ostream &operator<<(std::ostream &s, const ParseTree &pt) {
  return pt.dump(s);
}

} // namespace parse_tree
} // namespace hydla
