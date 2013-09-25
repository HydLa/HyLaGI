#ifndef _INCLUDED_HYDLA_PARSER_S_EXP_AST_H_
#define _INCLUDED_HYDLA_PARSER_S_EXP_AST_H_

#include "../../../simulator/DefaultParameter.h"
#include "../../SymbolicVirtualConstraintSolver.h"
#include "SExpParser.h"

namespace hydla {
namespace parser {

/**
 * S式の抽象構文木を保持するクラス
 * DDDにおけるエンティティ
 */
class SExpAST{
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
  SExpAST(const std::string& input_str);
  ~SExpAST();
  SExpAST(const SExpAST& sp);

  bool operator==(const SExpAST& rhs);
  bool operator!=(const SExpAST& rhs);
  std::string get_id() const;

  /** ASTのイテレータを返す */
  const_tree_iter_t get_tree_iterator() const; 

  /** get_tree_iterator()のエイリアス */
  const_tree_iter_t root() const; 

  /** ASTの構造を出力する */
  std::ostream& dump_tree(std::ostream& outstream) const;

private:
  /** 初期化には必ずinput_str又はSExpASTが必要 */
  SExpAST();
  /** 再帰で回す */
  std::ostream& dump_tree(std::ostream& outstream, const_tree_iter_t iter, int nest = 0) const;

  /** エンティティの識別子, input_str */
  std::string identifier_;
  SExpParser::tree_info_t ast_tree_;

};

std::ostream& operator<<(std::ostream& s, const SExpAST& sexp_ast);

} // namespace vcs
} // namespace hydla

#endif //_INCLUDED_HYDLA_PARSER_S_EXP_AST_H_

