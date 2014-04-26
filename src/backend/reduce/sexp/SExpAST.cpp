#include "SExpAST.h"

#include "../../../common/Logger.h"
#include "SExpGrammar.h"
#include <iostream>
#include <sstream>

using namespace hydla::symbolic_expression; // NegativeとNumber用

namespace hydla {
namespace parser {

const std::string SExpAST::empty_list_s_exp("list");

SExpAST::SExpAST(const std::string& input_str)
  : identifier_(input_str), ast_tree_(SExpParser::parse(input_str.c_str())) {}

SExpAST::~SExpAST(){}

SExpAST::SExpAST(const SExpAST& sp)
  : identifier_(sp.identifier_), ast_tree_(sp.ast_tree_) {}

bool SExpAST::operator==(const SExpAST& rhs){
  return identifier_ == rhs.get_id();
}

bool SExpAST::operator!=(const SExpAST& rhs){
  return !(identifier_ == rhs.get_id());
}

std::string SExpAST::get_id() const {
  return identifier_;
}

SExpAST::const_tree_iter_t SExpAST::get_tree_iterator() const {
  return ast_tree_.trees.begin();
}

SExpAST::const_tree_iter_t SExpAST::root() const {
  return ast_tree_.trees.begin();
}

std::ostream& SExpAST::dump_tree(std::ostream& outstream) const {
  return dump_tree(outstream, root(), 0);
}

std::ostream& SExpAST::dump_tree(std::ostream& outstream, const_tree_iter_t iter, int nest) const {
  for(int i = 0; i < nest; ++i)
    outstream << "  ";

  if (iter->value.id() == SExpGrammar::RI_Identifier)
    outstream << "identifier " << "\"" << std::string(iter->value.begin(), iter->value.end()) << "\"";
  else if (iter->value.id() == SExpGrammar::RI_Number)
    outstream << "integer " << "\"" << std::string(iter->value.begin(), iter->value.end()) << "\"";
  else if (iter->value.id() == SExpGrammar::RI_String)
    outstream << "string "<< "\"" << std::string(iter->value.begin(), iter->value.end()) << "\"";
  else if (iter->value.id() == SExpGrammar::RI_List)
    outstream << "list ";
  else if (iter->value.id() == SExpGrammar::RI_Data)
    outstream << "data ";
  else
    outstream << "Node '" << std::string(iter->value.begin(), iter->value.end()) << "'";

  outstream << std::endl;

  for(size_t j = 0; j < iter->children.size(); ++j)
    dump_tree(outstream, iter->children.begin()+j, nest+1);

  return outstream;
}

SExpAST::SExpAST(){}

std::ostream& operator<<(std::ostream& s, const SExpAST& ast)
{
  return ast.dump_tree(s);
}



} // namespace vcs
} // namespace hydla

