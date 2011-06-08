#ifndef _INCLUDED_HYDLA_PARSER_S_EXP_PARSER_H_
#define _INCLUDED_HYDLA_PARSER_S_EXP_PARSER_H_

#include <boost/spirit/include/classic_ast.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>

namespace hydla {
namespace parser {

class SExpParser {
public:
  typedef boost::spirit::classic::position_iterator<char const *>       pos_iter_t;
  typedef boost::spirit::classic::tree_match<pos_iter_t>::tree_iterator tree_iter_t;
  
  SExpParser();
  ~SExpParser();


  void dump_tree(boost::spirit::classic::tree_match<char const *>::tree_iterator const &iter, int nest);

  /**
   * “ü—Í‚Æ‚µ‚Ä—^‚¦‚ç‚ê‚½S®‚Ì•¶š—ñ‚ğ‰ğß‚µ‚ÄA–Ø‚ğ\’z
   */
  int parse_main(const char* input_str);

};


} // namespace parser
} // namespace hydla

#endif // _INCLUDED_HYDLA_PARSER_S_EXP_PARSER_H_
