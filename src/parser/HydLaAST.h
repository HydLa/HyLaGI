#ifndef _INCLUDED_HYDLA_PARSER_HYDLA_AST_H_
#define _INCLUDED_HYDLA_PARSER_HYDLA_AST_H_

#include <ostream>

#include <boost/spirit/include/classic_multi_pass.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>
#include <boost/spirit/include/classic_ast.hpp>

namespace hydla {
namespace parser {

class HydLaAST {
public:
  typedef boost::spirit::classic::multi_pass<std::istreambuf_iterator<char> > multipass_iter_t;
  typedef boost::spirit::classic::position_iterator<multipass_iter_t> pos_iter_t;
  typedef boost::spirit::classic::node_val_data_factory<> node_val_data_factory_t;
  typedef boost::spirit::classic::tree_parse_info<pos_iter_t, node_val_data_factory_t> tree_info_t;
  typedef boost::spirit::classic::tree_match<pos_iter_t, node_val_data_factory_t>::const_tree_iterator const_tree_iter_t;
  typedef boost::spirit::classic::tree_match<pos_iter_t, node_val_data_factory_t>::tree_iterator tree_iter_t;
  typedef boost::spirit::classic::tree_node<boost::spirit::classic::node_val_data<pos_iter_t> >  tree_node_t;

  HydLaAST();
  ~HydLaAST();

  /**
   * 入力を解析し、ASTを構築する
   */
  void parse(std::istream& s);
  void parse_constraint(std::istream& s);
  void parse_flie(const std::string& filename);
  void parse_string(const std::string& str);

  /**
   * ASTの構造を出力する
   */
  std::ostream& dump(std::ostream& stream) const
  {
    return dump(stream, ast_tree_.trees.begin(), 0);
  }

  /**
   * ASTのイテレータを返す
   */
   const_tree_iter_t get_tree_iterator() const
   {
     return ast_tree_.trees.begin();
   }

private:
  std::ostream& dump(std::ostream& outstream, const_tree_iter_t iter, int nest) const;
  
  tree_info_t ast_tree_;
};

std::ostream& operator<<(std::ostream& s, const HydLaAST& ast);


} // namespace parser
} // namespace hydla

#endif //_INCLUDED_HYDLA_PARSER_HYDLA_AST_H_
