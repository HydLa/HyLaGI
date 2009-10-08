#ifndef _INCLUDED_HTDLA_HYDLA_GRAMMAR_H_
#define _INCLUDED_HTDLA_HYDLA_GRAMMAR_H_

#include <boost/bind.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_functor_parser.hpp>
#include <boost/spirit/include/classic_escape_char.hpp>
#include <boost/spirit/include/classic_parse_tree.hpp>
#include <boost/spirit/include/classic_ast.hpp>

namespace hydla {
using namespace boost::spirit::classic;

struct HydLaGrammar : public grammar<HydLaGrammar> {
  typedef enum _RuleID {
    RI_Number = 1, 
    RI_PrevVariable,
    RI_Variable,
    RI_Identifier,
    RI_ModuleName,

    RI_LogicalAnd,
    RI_Ask_LogicalAnd,
    RI_LogicalOr,

    RI_Less,
    RI_LessEqual,
    RI_Greater,
    RI_GreaterEqual,
    RI_Equal,
    RI_Unequal,
    RI_AskEqual,
    RI_AskUnequal,

    RI_Negative,
    RI_Positive,

    RI_Plus,
    RI_Subtract,
    RI_Times,
    RI_Divide,

    RI_Entailment,
    RI_Globally,
    RI_Equivalent,
    RI_Weaker,
    RI_Parallel,
    RI_Differential,
    RI_Previous,
    RI_Prev,

    RI_FuncCall,
    RI_Function,

    RI_Expression,
    RI_Factor,
    RI_Limit,
    RI_Unary,
    RI_Arith_term,
    RI_Arithmetic,
    RI_Comparison,
    RI_Always,
    RI_Ask,
    RI_Tell,
    RI_Logical_term,
    RI_Logical,

    RI_Ask_Factor,
    RI_Ask_Unary,
    RI_Ask_Arith_Term,
    RI_Ask_Arithmetic,
    RI_Ask_Comparison,
    RI_Ask_Logical_Term,
    RI_Ask_Logical,

    RI_Program,
    RI_ProgramParallel,
    RI_ProgramOrdered,
    RI_ProgramFactor,

    RI_Module,
    RI_Constraint,
    RI_ConstraintName,

    RI_DefStatement,
    RI_ProgramDef,
    RI_ModuleDef,
    RI_ConstraintDef,

    RI_Statements,

  } RuleID;

  template<typename S> 
  struct definition {

#define defRuleID(ID) rule<S, parser_context<>, parser_tag<ID> >

    defRuleID(RI_Identifier)   identifier; 
    defRuleID(RI_Number)       number; 
    defRuleID(RI_PrevVariable) prev_val;
    defRuleID(RI_Variable)     variable; 
    defRuleID(RI_ModuleName)   module_name; 
    
    defRuleID(RI_Globally)  globally; 
    defRuleID(RI_Entailment)  entailment; 
    defRuleID(RI_Equivalent)  equivalent; 
    defRuleID(RI_Weaker)  weaker; 
    defRuleID(RI_Parallel)  parallel; 
    defRuleID(RI_Differential)  differential; 
    defRuleID(RI_Previous)  previous;     

    defRuleID(RI_Plus)     add; 
    defRuleID(RI_Subtract) sub; 
    defRuleID(RI_Times)    mul; 
    defRuleID(RI_Divide)   div;

    defRuleID(RI_Positive)   positive; 
    defRuleID(RI_Negative)   negative;

    defRuleID(RI_Less)    less; 
    defRuleID(RI_LessEqual)  less_eq; 
    defRuleID(RI_Greater) greater; 
    defRuleID(RI_GreaterEqual)  greater_eq; 
    defRuleID(RI_Equal)      equal; 
    defRuleID(RI_Unequal)    not_equal; 
    defRuleID(RI_AskEqual)      ask_equal; 
    defRuleID(RI_AskUnequal)    ask_not_equal; 

    defRuleID(RI_LogicalAnd)  logical_and; 
    defRuleID(RI_Ask_LogicalAnd)  ask_logical_and; 
    defRuleID(RI_LogicalOr)  logical_or; 

    defRuleID(RI_Program)          program; 
    defRuleID(RI_ProgramParallel)  program_parallel; 
    defRuleID(RI_ProgramOrdered)   program_ordered; 
    defRuleID(RI_ProgramFactor)    program_factor; 

    defRuleID(RI_DefStatement)    def_statement; 
    defRuleID(RI_ProgramDef)    program_def; 
    defRuleID(RI_ConstraintDef) constraint_def; 
    defRuleID(RI_ModuleDef)     module_def; 

    defRuleID(RI_Constraint)     constraint; 
    defRuleID(RI_ConstraintName) constraint_name; 
    defRuleID(RI_Module)    module;           

    defRuleID(RI_Always)  always; 
    defRuleID(RI_Ask)     ask; 
    defRuleID(RI_Tell)    tell; 

    defRuleID(RI_Expression)    expression; 
    defRuleID(RI_Factor)        factor; 
    defRuleID(RI_Limit)         limit; 
    defRuleID(RI_Unary)         unary; 
    defRuleID(RI_Arithmetic)    arithmetic;
    defRuleID(RI_Arith_term)    arith_term;
    defRuleID(RI_Logical)        logical; 
    defRuleID(RI_Logical_term)  logical_term; 
    defRuleID(RI_Comparison)    comparison; 

    defRuleID(RI_Ask_Factor)       ask_factor; 
    defRuleID(RI_Ask_Unary)         ask_unary; 
    defRuleID(RI_Ask_Arith_Term)   ask_arith_term; 
    defRuleID(RI_Ask_Arithmetic)   ask_arithmetic; 
    defRuleID(RI_Ask_Comparison)   ask_comparison; 
    defRuleID(RI_Ask_Logical_Term) ask_logical_term; 
    defRuleID(RI_Ask_Logical)       ask_logical;

    defRuleID(RI_Statements)       statements;

    // 構文定義
    definition(const HydLaGrammar& self) {
            
      //開始
      statements  = *((def_statement | program) 
                       >> discard_node_d[ch_p('.')]) >> end_p;

      //プログラム
      program = program_parallel;

      program_parallel = program_ordered % root_node_d[parallel];
      program_ordered  = program_factor  % root_node_d[weaker];
      program_factor   = module
                       | inner_node_d['(' >> program_parallel >> ')'];

      // 定義
      def_statement = constraint_def | program_def;

      //program定義
      program_def = module_name >> inner_node_d['{' >> program >> '}'];
      
      //constraint定義
      constraint_def = constraint_name >> root_node_d[equivalent] >> gen_pt_node_d[constraint];

      //module定義
      module = gen_pt_node_d[constraint];
      
      //制約式
      constraint = logical_term;

      //論理積
      logical_term = always % root_node_d[logical_and];

      //always
      always = !(root_node_d[globally]) >> ask;

      //ask
      ask = ask_logical >> root_node_d[entailment] >> logical_term
          | gen_pt_node_d[tell]
          | constraint_name
          | inner_node_d['(' >> logical_term >> ')']; 

      //tell制約
      tell = comparison;

      //比較
      comparison = arithmetic 
              >> root_node_d[less_eq
                             | less
                             | greater_eq
                             | greater
                             | equal
                             | not_equal]
              >> arithmetic;

      //算術式
      arithmetic =
        arith_term % root_node_d[add | sub];

      //算術項
      arith_term =  
        unary % root_node_d[mul | div];
      
      //単項演算子
      unary = !(root_node_d[positive | negative]) >> limit;

      //極限
      limit = factor >> !(root_node_d[differential]) >> !(root_node_d[previous]);

      //因子
      factor =  
          variable
        | number
        | inner_node_d['(' >> arithmetic >> ')'];

      // ---- ask ----      
      //論理和
      ask_logical = ask_logical_term % root_node_d[logical_or];

      //論理積
      ask_logical_term = ask_comparison % root_node_d[ask_logical_and]
              | comparison
              | constraint_name
              | inner_node_d['(' >> ask_logical >> ')'];


      //比較
      ask_comparison = ask_arithmetic 
              >> root_node_d[less_eq
                             | less
                             | greater_eq
                             | greater
                             | ask_equal
                             | ask_not_equal]
              >> ask_arithmetic;

      //算術式
      ask_arithmetic =
        ask_arith_term % root_node_d[add | sub];

      //算術項
      ask_arith_term =  
        ask_unary % root_node_d[mul | div];
      
      //単項演算子
      ask_unary = !(root_node_d[positive | negative]) >> ask_factor;

      //因子
      ask_factor = 
          prev_val
        | number
        | inner_node_d['(' >> ask_arithmetic >> ')'];

      //数字
      number = 
        lexeme_d[leaf_node_d[
          +digit_p >> !('.' >> +digit_p)]]; 

      //変数
      variable = lexeme_d[leaf_node_d[identifier]];

      //制約名
      constraint_name = lexeme_d[leaf_node_d[identifier]];

      //モジュール名
      module_name = lexeme_d[leaf_node_d[identifier]];

      //識別子
      identifier = lexeme_d[leaf_node_d[
                     (alpha_p | '_') >> *(alpha_p | digit_p | '_')]];

      //特殊記号定義
      equivalent   = str_p("<=>");
      entailment   = str_p("=>"); //implies entails
      globally     = str_p("[]"); 
      differential = +ch_p('\'');
      previous     = ch_p('-');

      //半順序定義演算子
      weaker       = str_p("<<");
      parallel     = ch_p(',');

      //比較演算子
      less        = ch_p('<'); 
      less_eq     = str_p("<="); 
      greater     = ch_p('>'); 
      greater_eq  = str_p(">=");
      equal       = ch_p("="); 
      ask_equal   = ch_p("="); 
      not_equal   = str_p("!="); 
      ask_not_equal   = str_p("!="); 

      //論理演算子
      logical_and     = str_p("&") | str_p("/\\");
      ask_logical_and = str_p("&") | str_p("/\\");
      logical_or      = str_p("|") | str_p("\\/");

      //算術演算子
      add          = ch_p('+');
      sub          = ch_p('-');
      mul          = ch_p('*');
      div          = ch_p('/');

      //単項演算子
      positive    = ch_p('+');
      negative    = ch_p('-');
    }

    // 開始ルール
    defRuleID(RI_Statements) const& start() const {
      return statements;
    }
  };
};

}
#endif

