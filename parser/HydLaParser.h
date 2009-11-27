#ifndef _INCLUDED_HYDLA_PARSER_H_
#define _INCLUDED_HYDLA_PARSER_H_

#include <map>
#include <istream>
#include <iterator>

#include <boost/spirit/include/classic_multi_pass.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>
#include <boost/spirit/include/classic_ast.hpp>

#include "ParseTree.h"
#include "NodeFactory.h"
#include "TreeVisitor.h"

namespace hydla {
namespace parser {

class HydLaParser {
public:
  typedef boost::spirit::classic::multi_pass<std::istreambuf_iterator<char> > multipass_iter_t;
  typedef boost::spirit::classic::position_iterator<multipass_iter_t> pos_iter_t;
  typedef boost::spirit::classic::node_val_data_factory<> node_val_data_factory_t;
  typedef boost::spirit::classic::tree_parse_info<pos_iter_t, node_val_data_factory_t> tree_info_t;
  typedef boost::spirit::classic::tree_match<pos_iter_t, node_val_data_factory_t>::const_tree_iterator const_tree_iter_t;
  typedef boost::spirit::classic::tree_match<pos_iter_t, node_val_data_factory_t>::tree_iterator       tree_iter_t;
  typedef boost::spirit::classic::tree_node<boost::spirit::classic::node_val_data<pos_iter_t> > tree_node_t;

  typedef std::map<std::string, std::string> module_map_t;
  typedef std::map<std::string, int>         variable_map_t;

  typedef boost::shared_ptr<parse_tree::NodeFactory> node_factory_sptr;

  HydLaParser(node_factory_sptr node_factory);
  HydLaParser(node_factory_sptr node_factory, bool debug_dump);
  ~HydLaParser();

  /**
   * 入力を解析し、パースツリーを構築する
   */
  void parse(std::istream& s);
  void parse_flie(const std::string& filename);
  void parse_string(const std::string& str);

  /**
   * ASTの構造を出力する
   */
  std::ostream& dump_ast(std::ostream& outstream)
  {
    return dump_ast(outstream, ast_tree_.trees.begin(), 0);
  }

  /**
   * ParseTreeの構造を出力する
   */ 
  std::ostream& dump_parse_tree(std::ostream& outstream)
  {
    outstream << parse_tree_.to_string();
    return outstream;
  }

  std::string create_interlanguage(std::string max_time);

  /**
   * ParseTreeに対して任意のVisitorを適用する
   */
  void dispatch(parse_tree::TreeVisitor* visitor)
  {
    parse_tree_.dispatch(visitor);
  }

  parse_tree::ParseTree& parse_tree() 
  {
    return parse_tree_;
  }

  /**
   * すべてのツリーデータを破棄し、初期状態に戻す
   */
  void clear_tree()
  {
    ast_tree_ = tree_info_t();
    parse_tree_.clear();
  }

private:
  /**
   * 入力を元にASTを構築する
   */
  void create_ast(std::istream& inter);

  /*
   * ASTからParseTreeを作成
   */
  void create_parse_tree() 
  {
      create_parse_tree(*ast_tree_.trees.begin());
  }

  /**
   * ParseTreeに対して事前処理（制約呼び出しの参照解決等）をかける
   */
  void execute_preprocess() 
  {
      parse_tree_.preprocess();
  }

  std::ostream& dump_ast(std::ostream& outstream, tree_iter_t iter, int nest);

  boost::shared_ptr<parse_tree::Node> 
    create_parse_tree(const tree_node_t &tree_node);

  void parse_common();

  bool                  debug_dump_;
  tree_info_t           ast_tree_;
  node_factory_sptr     node_factory_;
  parse_tree::ParseTree parse_tree_;
};

} //namespace parser
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSER_H_
