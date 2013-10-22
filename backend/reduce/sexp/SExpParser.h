#ifndef _INCLUDED_HYDLA_PARSER_S_EXP_PARSER_H_
#define _INCLUDED_HYDLA_PARSER_S_EXP_PARSER_H_

#include "SExpGrammar.h"
#include <boost/spirit/include/classic_ast.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>
#include <string>

namespace hydla {
namespace parser {

/**
 * S式のASTを生成するサービス
 */
class SExpParser {
public:
  typedef boost::spirit::classic::position_iterator<char const *>             pos_iter_t;
  typedef boost::spirit::classic::tree_parse_info<pos_iter_t>                 tree_info_t;
  typedef boost::spirit::classic::tree_match<pos_iter_t>::tree_iterator       tree_iter_t;
  typedef boost::spirit::classic::tree_match<pos_iter_t>::const_tree_iterator const_tree_iter_t;

  /**
   * 入力として与えられたS式の文字列を解釈して、木を構築
   */
  static tree_info_t parse(const std::string& input_str);
  static std::string get_string_from_tree(const_tree_iter_t var_iter);
  
private:
   SExpParser();
  ~SExpParser();

};

//std::ostream& operator<<(std::ostream& s, const SExpParser& sp);
std::ostream& operator<<(std::ostream& s, const SExpParser::const_tree_iter_t& iter);

} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_PARSER_S_EXP_PARSER_H_
