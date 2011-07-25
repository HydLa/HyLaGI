#ifndef _INCLUDED_HYDLA_PARSER_S_EXP_PARSER_H_
#define _INCLUDED_HYDLA_PARSER_S_EXP_PARSER_H_

#include <boost/spirit/include/classic_ast.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>
#include <string>

#include "SExpGrammar.h"

namespace hydla {
namespace parser {

class SExpParser {
public:
  typedef boost::spirit::classic::position_iterator<char const *>             pos_iter_t;
  typedef boost::spirit::classic::tree_parse_info<pos_iter_t>                 tree_info_t;
  typedef boost::spirit::classic::tree_match<pos_iter_t>::tree_iterator       tree_iter_t;
  typedef boost::spirit::classic::tree_match<pos_iter_t>::const_tree_iterator const_tree_iter_t;

  // 空集合を表すS式 "list"
  static const std::string empty_list_s_exp;
  
  SExpParser();
  ~SExpParser();


  std::string get_string_from_tree(const_tree_iter_t iter) const;

  int get_derivative_count(const_tree_iter_t iter) const;

  void dump_tree(const_tree_iter_t iter, int nest);

  /**
   * 入力として与えられたS式の文字列を解釈して、木を構築
   */
  int parse_main(const char* input_str);

  /**
   * ASTのイテレータを返す
   */
  const_tree_iter_t get_tree_iterator() const
  {
    return ast_tree_.trees.begin();
  }

private:
  tree_info_t ast_tree_;

};

//std::ostream& operator<<(std::ostream& s, const SExpParser& sp);
std::ostream& operator<<(std::ostream& s, const SExpParser::const_tree_iter_t& iter);

} // namespace parser
} // namespace hydla

#endif // _INCLUDED_HYDLA_PARSER_S_EXP_PARSER_H_
