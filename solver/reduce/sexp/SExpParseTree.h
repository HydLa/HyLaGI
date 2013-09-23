#ifndef _INCLUDED_HYDLA_PARSER_S_EXP_PARSE_TREE_H_
#define _INCLUDED_HYDLA_PARSER_S_EXP_PARSE_TREE_H_

#include "../../../simulator/DefaultParameter.h"
#include "../../SymbolicVirtualConstraintSolver.h"
#include "SExpParser.h"

namespace hydla {
namespace parser {

/**
 * S���̒��ۍ\���؂�ێ�����N���X
 * DDD�ɂ�����G���e�B�e�B
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
   * ��W����\��S�� "list"
   */
  static const std::string empty_list_s_exp;
 
  /**
   * @param input_str �p�[�X����S���̕�����
   */
  SExpParseTree(const std::string& input_str);
  ~SExpParseTree();
  SExpParseTree(const SExpParseTree& sp);

  /**
   * string_map_�̏�����
   * �m�[�h�ƕ�����̑Ή��֌W������Ă���
   */
  static void initialize();

  bool operator==(const SExpParseTree& rhs);
  bool operator!=(const SExpParseTree& rhs);
  std::string get_id() const;


  int get_derivative_count(const_tree_iter_t iter) const;

  void dump_tree(const_tree_iter_t iter, int nest = 0) const;

  /** 
   * S���Ƃ���string�ɕϊ�����
   * @param isFirst �ċA�̖`�����A���[�U�̓f�t�H���g�������g�p����
   */
  std::string to_string(const_tree_iter_t iter, bool isFirst = true) const ;

  /** S���Ƃ���value_t�ɕϊ����� */
  value_t to_value(const_tree_iter_t iter) const ;

  /** get_tree_iterator()�̃G�C���A�X */
  const_tree_iter_t root() const; 

  /** ���X�g�����͂����֐� */
  const_tree_iter_t car(const_tree_iter_t iter) const; 
  const_tree_iter_t cadr(const_tree_iter_t iter) const; 
  const_tree_iter_t caddr(const_tree_iter_t iter) const; 
  const_tree_iter_t cadddr(const_tree_iter_t iter) const; 

  /** AST�̃C�e���[�^��Ԃ� */
  const_tree_iter_t get_tree_iterator() const; 

  static std::string get_string_from_tree(const_tree_iter_t var_iter);

private:
  /** �������ɂ͕K��input_str����SExpParseTree���K�v */
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
  /** �����o�֐��|�C���^�̌^ */
  typedef node_sptr (SExpParseTree::*p_function_for_node)(const_tree_iter_t iter, const nodeType &) const ; 
  typedef struct tag_function_and_node{
    p_function_for_node function;
    nodeType node;
    tag_function_and_node(p_function_for_node func, nodeType nod):function(func), node(nod){}
  }function_and_node;
  typedef std::map<std::string, function_and_node> string_map_t;

  /** �e�m�[�h�ɑΉ����鏈���D�i���F�֐��j*/
  function_for_node for_derivative;
  function_for_node for_unary_node;
  function_for_node for_binary_node;

  /** to_string()�̍ċA���� */
  std::string to_string_recursive(const_tree_iter_t iter) const ;

  /** �ċA�ŌĂяo���Ă����� */
  node_sptr to_symbolic_tree(const_tree_iter_t iter) const ;

  /** Mathematica������Ə���&�m�[�h�̑Ή��֌W */
  static string_map_t string_map_;

  /** �G���e�B�e�B�̎��ʎq, input_str */
  std::string identifier_;
  SExpParser::tree_info_t ast_tree_;

};

//std::ostream& operator<<(std::ostream& s, const SExpParseTree::const_tree_iter_t& iter);

} // namespace vcs
} // namespace hydla

#endif //_INCLUDED_HYDLA_PARSER_S_EXP_PARSE_TREE_H_

