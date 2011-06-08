#ifndef _INCLUDED_HYDLA_S_EXP_GRAMMAR_H_
#define _INCLUDED_HYDLA_S_EXP_GRAMMAR_H_

#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_escape_char.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_ast.hpp>


namespace hydla {
namespace parser {

using namespace boost::spirit::classic;

struct SExpGrammar : public grammar<SExpGrammar> {

  static const int RI_Identifier  = 1;
  static const int RI_Number      = 2;
  static const int RI_String      = 3;
  static const int RI_List        = 4;
  static const int RI_SExpression = 5;
  static const int RI_Data        = 6;

  template<typename S>
  struct definition {

    #define defRuleID(ID) rule<S, parser_context<>, parser_tag<ID> >

    defRuleID(RI_Identifier)     identifier;
    defRuleID(RI_Number)         number;
    defRuleID(RI_String)         string;
    defRuleID(RI_List)           list;
    defRuleID(RI_SExpression)    s_expression;
    defRuleID(RI_Data)           data; 

    // �\����`
    definition(const SExpGrammar& self) {
      // �J�n
      data = +(s_expression);

      // S��
      s_expression = number | identifier | string | list;

      // ����
      number = int_p;

      // ���ʎq
      identifier = leaf_node_d[alpha_p >> lexeme_d[*(alpha_p | int_p | ch_p('-') | ch_p('_') | ch_p(':'))]];

      // ������
      string = leaf_node_d[inner_node_d[lexeme_d[confix_p('"', *c_escape_ch_p, '"')]]];

      // ���X�g�\��
      list =
        discard_node_d[ch_p('(')] >> +(s_expression) >> discard_node_d[ch_p(')')]
        | discard_node_d[ch_p('[')] >> +(s_expression) >> discard_node_d[ch_p(']')];
    }

    // �J�n���[��
    defRuleID(RI_Data) const& start() const {
      return data;
    }
  };
};

} // namespace parser
} // namespace hydla

#endif //_INCLUDED_HYDLA_S_EXP_GRAMMAR_H_
