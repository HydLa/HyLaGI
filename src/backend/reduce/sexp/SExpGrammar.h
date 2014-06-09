#pragma once

#include <boost/spirit/include/classic_escape_char.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_ast.hpp>
#include <boost/spirit/include/classic_lists.hpp>

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
  static const int RI_Header      = 7;

  template<typename S>
  struct definition {

    #define defRuleID(ID) rule<S, parser_context<>, parser_tag<ID> >

    defRuleID(RI_Identifier)     identifier;
    defRuleID(RI_Number)         number;
    defRuleID(RI_String)         string;
    defRuleID(RI_List)           list;
    defRuleID(RI_SExpression)    s_expression;
    defRuleID(RI_Data)           data; 
    defRuleID(RI_Header)         header;

    // 構文定義
    definition(const SExpGrammar& self) {
      // 開始
      data = +(s_expression);
      // S式
      s_expression = no_node_d[!+ch_p(' ')] >> (number | identifier | string | list) >> no_node_d[!+ch_p(' ')];
      // 数字
      number = leaf_node_d[!(ch_p('-') | ch_p('+')) >> +digit_p];
      // 識別子
      identifier = leaf_node_d[alpha_p >> lexeme_d[*(alpha_p | int_p | ch_p('!') >> ch_p('!') | ch_p('-') | ch_p('_') | ch_p(':'))]];
      // 文字列
      string = leaf_node_d[inner_node_d[lexeme_d[confix_p(ch_p('"'), *c_escape_ch_p, ch_p('"'))]]];

      // リストのヘッダ
      header = identifier;

      // リスト構造
      list =
        no_node_d[ch_p('[')] >> root_node_d[header] >> list_p(*s_expression, space_p) >> no_node_d[ch_p(']')];

    }

    // 開始ルール
    defRuleID(RI_Data) const& start() const {
      return data;
    }
  };
};

} // namespace backend
} // namespace hydla

