#ifndef _INCLUDED_HYDLA_PARSER_S_EXP_PARSE_TREE_H_
#define _INCLUDED_HYDLA_PARSER_S_EXP_PARSE_TREE_H_

#include "../../../simulator/DefaultParameter.h"
#include "../../SymbolicVirtualConstraintSolver.h"
#include "SExpParser.h"

namespace hydla {
namespace parser {

/**
 * S式の抽象構文木を保持するクラス
 * DDDにおけるエンティティ
 */
class SExpParseTree{
public:

  typedef SExpParser::const_tree_iter_t                         const_tree_iter_t;
  typedef SExpParser::pos_iter_t                                pos_iter_t;
  typedef SExpParser::tree_info_t                               tree_info_t;
  typedef SExpParser::tree_iter_t                               tree_iter_t;
  typedef hydla::parse_tree::node_sptr                          node_sptr;
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::value_t  value_t;

  /**
   * 空集合を表すS式 "list"
   */
  static const std::string empty_list_s_exp;
 
  /**
   * @param input_str パースするS式の文字列
   */
  SExpParseTree(const std::string& input_str);
  ~SExpParseTree();
  SExpParseTree(const SExpParseTree& sp);

  /**
   * string_map_の初期化
   * ノードと文字列の対応関係を作っておく
   */
  static void initialize();

  bool operator==(const SExpParseTree& rhs);
  bool operator!=(const SExpParseTree& rhs);
  std::string get_id() const;


  int get_derivative_count(const_tree_iter_t iter) const;

  void dump_tree(const_tree_iter_t iter, int nest = 0) const;

  /** 
   * S式とってstringに変換する
   * @param isFirst 再帰の冒頭か、ユーザはデフォルト引数を使用する
   */
  std::string to_string(const_tree_iter_t iter, bool isFirst = true) const ;

  /** S式とってvalue_tに変換する */
  value_t to_value(const_tree_iter_t iter) const ;

  /** get_tree_iterator()のエイリアス */
  const_tree_iter_t root() const; 

  /** リスト操作を模した関数 */
  const_tree_iter_t car(const_tree_iter_t iter) const; 
  const_tree_iter_t cadr(const_tree_iter_t iter) const; 
  const_tree_iter_t caddr(const_tree_iter_t iter) const; 
  const_tree_iter_t cadddr(const_tree_iter_t iter) const; 

  /** ASTのイテレータを返す */
  const_tree_iter_t get_tree_iterator() const; 

  static std::string get_string_from_tree(const_tree_iter_t var_iter);

private:
  /** 初期化には必ずinput_str又はSExpParseTreeが必要 */
  SExpParseTree();

  typedef enum{
    NODE_PLUS,
    NODE_SUBTRACT,
    NODE_TIMES,
    NODE_DIVIDE,
    NODE_POWER,
    NODE_DIFFERENTIAL,
    NODE_PREVIOUS,
    NODE_SQRT,
    NODE_NEGATIVE,
  }nodeType;

  typedef node_sptr (function_for_node)(const_tree_iter_t iter, const nodeType &) const ;
  /** メンバ関数ポインタの型 */
  typedef node_sptr (SExpParseTree::*p_function_for_node)(const_tree_iter_t iter, const nodeType &) const ; 
  typedef struct tag_function_and_node{
    p_function_for_node function;
    nodeType node;
    tag_function_and_node(p_function_for_node func, nodeType nod):function(func), node(nod){}
  }function_and_node;
  typedef std::map<std::string, function_and_node> string_map_t;

  /** 各ノードに対応する処理．（注：関数）*/
  function_for_node for_derivative;
  function_for_node for_unary_node;
  function_for_node for_binary_node;

  /** to_string()の再帰部分 */
  std::string to_string_recursive(const_tree_iter_t iter) const ;

  /** 再帰で呼び出していく方 */
  node_sptr to_symbolic_tree(const_tree_iter_t iter) const ;

  /** Mathematica文字列と処理&ノードの対応関係 */
  static string_map_t string_map_;

  /** エンティティの識別子, input_str */
  std::string identifier_;
  SExpParser::tree_info_t ast_tree_;

};

//std::ostream& operator<<(std::ostream& s, const SExpParseTree::const_tree_iter_t& iter);

} // namespace vcs
} // namespace hydla

#endif //_INCLUDED_HYDLA_PARSER_S_EXP_PARSE_TREE_H_

