#ifndef _INCLUDED_HYDLA_HYDLA_GRAMMAR_H_
#define _INCLUDED_HYDLA_HYDLA_GRAMMAR_H_

#include <boost/bind.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_functor_parser.hpp>
#include <boost/spirit/include/classic_escape_char.hpp>
#include <boost/spirit/include/classic_parse_tree.hpp>
#include <boost/spirit/include/classic_ast.hpp>

#include <string>

#include "HydLaGrammarRule.h"

namespace hydla {
namespace parser {

using namespace hydla::grammer_rule;
using namespace boost::spirit::classic;

struct HydLaGrammar : public grammar<HydLaGrammar> {
  template<typename S> 
  struct definition {
        
#define defRuleID(ID) rule<S, parser_context<>, parser_tag<ID> >

    defRuleID(RI_Identifier)     identifier; 
    defRuleID(RI_Number)         number;
    defRuleID(RI_Pi)              pi;
    defRuleID(RI_E)              e;
    defRuleID(RI_PrevVariable)   prev_val;
    defRuleID(RI_BoundVariable)  bound_variable;
    defRuleID(RI_Variable)       variable; 
    defRuleID(RI_ProgramName)    program_name; 
    
    defRuleID(RI_Always)        always; 
    defRuleID(RI_Implies)       implies; 
    defRuleID(RI_Equivalent)    equivalent; 
    defRuleID(RI_Weaker)        weaker; 
    defRuleID(RI_Parallel)      parallel; 
    defRuleID(RI_Differential)  differential; 
    defRuleID(RI_Previous)      previous;     

    defRuleID(RI_Plus)     add; 
    defRuleID(RI_Subtract) sub; 
    defRuleID(RI_Times)    mul; 
    defRuleID(RI_Divide)   div;
    defRuleID(RI_Power)    pow;

    defRuleID(RI_Positive)   positive; 
    defRuleID(RI_Negative)   negative;

    defRuleID(RI_Sin)        sin;
    defRuleID(RI_Cos)        cos;
    defRuleID(RI_Tan)        tan;
    defRuleID(RI_Asin)       asin;
    defRuleID(RI_Acos)        acos;
    defRuleID(RI_Atan)        atan;
    
    defRuleID(RI_Log)        log;
    defRuleID(RI_Ln)        ln;
    
    defRuleID(RI_ArbitraryBinary)        arbitrary_binary;
    defRuleID(RI_ArbitraryUnary)         arbitrary_unary;
    defRuleID(RI_ArbitraryFactor)        arbitrary_factor;

    defRuleID(RI_CompOp)       comp_op; 
    defRuleID(RI_Less)         less; 
    defRuleID(RI_LessEqual)    less_eq; 
    defRuleID(RI_Greater)      greater; 
    defRuleID(RI_GreaterEqual) greater_eq; 
    defRuleID(RI_Equal)        equal; 
    defRuleID(RI_UnEqual)      not_equal; 

    defRuleID(RI_LogicalAnd)   logical_and; 
    defRuleID(RI_LogicalOr)    logical_or; 
    defRuleID(RI_LogicalNot)    logical_not; 

    defRuleID(RI_Program)          program; 
    defRuleID(RI_ProgramParallel)  program_parallel; 
    defRuleID(RI_ProgramOrdered)   program_ordered; 
    defRuleID(RI_ProgramFactor)    program_factor; 

    defRuleID(RI_DefStatement)    def_statement;
    defRuleID(RI_ProgramDef)    program_def; 
    defRuleID(RI_ConstraintDef) constraint_def; 
    defRuleID(RI_ModuleDef)     module_def; 
        
    defRuleID(RI_Constraint)        constraint; 
    defRuleID(RI_ConstraintName)    constraint_name; 
    defRuleID(RI_ConstraintCallee)  constraint_callee;
    defRuleID(RI_ConstraintCaller)  constraint_caller;
    defRuleID(RI_ProgramCallee)     program_callee;
    defRuleID(RI_ProgramCaller)     program_caller;
    defRuleID(RI_FormalArgs)        formal_args;
    defRuleID(RI_ActualArgs)        actual_args;
    defRuleID(RI_Module)            module;           

    defRuleID(RI_AlwaysTerm)    always_term; 
    defRuleID(RI_Ask)           ask; 
    defRuleID(RI_Tell)          tell; 

    defRuleID(RI_Expression)    expression; 
    defRuleID(RI_Factor)        factor; 
    defRuleID(RI_Diff)          diff; 
    defRuleID(RI_Limit)         limit;
    defRuleID(RI_Unary)         unary; 
    defRuleID(RI_Arithmetic)    arithmetic;
    defRuleID(RI_Arith_term)    arith_term;
    defRuleID(RI_Power_term)    power_term;
    defRuleID(RI_Logical)       logical;
    defRuleID(RI_Logical_term)  logical_term; 


    defRuleID(RI_Ask_Logical_Literal)  ask_logical_literal; //!がつくもの
    defRuleID(RI_Ask_Logical_Term)  ask_logical_term; 
    defRuleID(RI_Ask_Logical)       ask_logical;
    defRuleID(RI_Comparison)        comparison; 
    defRuleID(RI_Assert)           assert;
    defRuleID(RI_Print)            print;
    defRuleID(RI_Print_PP)            print_pp;
    defRuleID(RI_Print_IP)            print_ip;
    defRuleID(RI_Scan)            scan;
    defRuleID(RI_Exit)            exit;
    defRuleID(RI_Abort)            abort;

    defRuleID(RI_Statements)       statements;
    defRuleID(RI_HydLaProgram)     hydla_program;

    // 構文定義
    definition(const HydLaGrammar& self) {

      //開始
      hydla_program = gen_pt_node_d[statements];
            
      //文の集合
      statements  = gen_ast_node_d[*(( assert | def_statement | program) 
                                     >> discard_node_d[ch_p('.')]) >> end_p];

      //プログラム
      program = program_parallel;

      program_parallel = program_ordered % root_node_d[parallel];
      program_ordered  = program_factor  % root_node_d[weaker];
      program_factor   = 
          gen_pt_node_d[program_caller] 
            >> eps_p(*ch_p(')') >> (parallel | weaker | ch_p('}') | ch_p('.')))
        | no_node_d[ch_p('(')] >> program_parallel >> no_node_d[ch_p(')')] // do not use inner_node_d
            >> eps_p(*ch_p(')') >> (parallel | weaker | ch_p('}') | ch_p('.')))
        | module;
      
      // 定義
      def_statement = gen_pt_node_d[constraint_def | program_def];
      
      // assert文
      assert = root_node_d[str_p("ASSERT")] >> no_node_d[ch_p('(')] >> ask_logical >> no_node_d[ch_p(')')];

      //print文
      //print = root_node_d[str_p("PRINT")] >> no_node_d[ch_p("(")] >> arbitrary_factor >>no_node_d[ch_p(")")];
      print = no_node_d[str_p("PRINT")] >>  no_node_d[ch_p("(")] >> no_node_d[ch_p('"')] 
             >> leaf_node_d[*(anychar_p - '"') >> ch_p('"') >> *(ch_p(',') | (anychar_p - ")"))]
             >> no_node_d[ch_p(")")];

      print_pp = no_node_d[str_p("PRINTPP")] >>  no_node_d[ch_p("(")] >> no_node_d[ch_p('"')] 
             >> leaf_node_d[*(anychar_p - '"') >> ch_p('"') >> *(ch_p(',') | (anychar_p - ")"))]
             >> no_node_d[ch_p(")")];
      print_ip = no_node_d[str_p("PRINTIP")] >>  no_node_d[ch_p("(")] >> no_node_d[ch_p('"')] 
             >> leaf_node_d[*(anychar_p - '"') >> ch_p('"') >> *(ch_p(',') | (anychar_p - ")"))]
             >> no_node_d[ch_p(")")];

      scan = no_node_d[str_p("SCAN")] >>  no_node_d[ch_p("(")] >> 
             leaf_node_d[*(ch_p(',') | (anychar_p - ")"))]
             >> no_node_d[ch_p(")")];

      exit = no_node_d[str_p("EXIT")] >>  no_node_d[ch_p("(")]  >> no_node_d[ch_p(")")];

      abort = no_node_d[str_p("ABORT")] >>  no_node_d[ch_p("(")]  >> no_node_d[ch_p(")")];
      

      //program定義
      program_def = 
        gen_pt_node_d[program_callee] 
          >> no_node_d[ch_p('{')] >> program >> no_node_d[ch_p('}')];
      
      //constraint定義
      constraint_def = 
        gen_pt_node_d[constraint_callee] >> no_node_d[equivalent] >> gen_pt_node_d[constraint];

      //constraint定義時の左辺
      constraint_callee = 
        gen_ast_node_d[constraint_name] >> gen_pt_node_d[formal_args];

      //constraint呼び出し
      constraint_caller = 
        gen_ast_node_d[constraint_name] >> gen_pt_node_d[actual_args];

      //program定義時の左辺
      program_callee = 
        gen_ast_node_d[program_name] >> gen_pt_node_d[formal_args];

      //program呼び出し
      program_caller= 
        gen_ast_node_d[program_name] >> gen_pt_node_d[actual_args];

      //module定義
      module = gen_pt_node_d[constraint];
      
      //制約式
      constraint = gen_ast_node_d[logical];

      //論理和
      logical      = logical_term % root_node_d[logical_or];

      //論理積
      logical_term = always_term % root_node_d[logical_and];

      //always
      always_term  = !(root_node_d[always]) >> ask;

      //ask
      ask = ask_logical >> root_node_d[implies] >> constraint
        | gen_pt_node_d[tell]
        | gen_pt_node_d[constraint_caller]
        | no_node_d[ch_p('(')] >> logical_term >> no_node_d[ch_p(')')];

      //tell
      tell = gen_ast_node_d[
        expression >> root_node_d[comp_op] >> expression] || print | print_pp | print_ip | scan | abort| exit;

      //式
      expression = arithmetic;

      //算術式
      arithmetic =
        arith_term % root_node_d[add | sub];
      
      //算術項
      arith_term =  
        unary % root_node_d[mul | div];
      
      
      //単項演算子
      unary = !(root_node_d[positive | negative]) >> power_term;
      
      //べき乗
      power_term = limit >> !(root_node_d[pow] >> power_term);

      //極限
      //factor以外の物が後ろにあったらprev
      limit = diff >> !(root_node_d[previous] >> eps_p(eps_p - factor));
      

      //微分
      diff = factor >> *(root_node_d[differential]);

      //因子
      factor =
          root_node_d[sin | cos | tan | asin | acos | atan | ln | arbitrary_unary] >>  no_node_d[ch_p('(')] >> expression  >> no_node_d[ch_p(')')]
        | root_node_d[log|arbitrary_binary] >>  no_node_d[ch_p('(')] >> expression >> no_node_d[ch_p(',')] >> expression >> no_node_d[ch_p(')')]
        | arbitrary_factor
        | pi
        | e
        | variable
        | number
        | no_node_d[ch_p('(')] >> expression >> no_node_d[ch_p(')')];

      // ---- ask ----
      //論理和
      ask_logical = ask_logical_term % root_node_d[logical_or];

      //論理積
      ask_logical_term = ask_logical_literal % root_node_d[logical_and];
      
      //否定
      ask_logical_literal = !(root_node_d[logical_not]) >> comparison;

      //比較
      comparison = 
        expression >> root_node_d[comp_op] >> expression
        | gen_pt_node_d[constraint_caller]
        | no_node_d[ch_p('(')] >> ask_logical >> no_node_d[ch_p(')')];
      
      //円周率
      pi = str_p("Pi");
      
      //自然対数の底
      e = str_p("E");
      
      arbitrary_binary = no_node_d[ch_p('"')] >> leaf_node_d[+alpha_p] >> no_node_d[ch_p('"')];
      arbitrary_unary = no_node_d[ch_p('"')] >> leaf_node_d[+alpha_p] >> no_node_d[ch_p('"')];
      //arbitrary_factor = no_node_d[ch_p('"')] >> leaf_node_d[+alpha_p] >> no_node_d[ch_p('"')];
      
      //数字
      number = 
        lexeme_d[leaf_node_d[
          +digit_p >> !('.' >> +digit_p)]]; 

      //変数
      variable = lexeme_d[leaf_node_d[identifier]];

      //束縛変数
      bound_variable = lexeme_d[leaf_node_d[identifier]];

      //制約名
      constraint_name = lexeme_d[leaf_node_d[identifier]];

      //モジュール名
      program_name = lexeme_d[leaf_node_d[identifier]];

      //識別子
      identifier = lexeme_d[leaf_node_d[
          (alpha_p | '_') >> *(alpha_p | digit_p | '_')]];

      //実引数
      actual_args = gen_ast_node_d[
        !(no_node_d[ch_p('(')] 
          >> !(expression >> *(no_node_d[ch_p(',')] >> expression)) 
          >> no_node_d[ch_p(')')])];

      //仮引数
      formal_args = gen_ast_node_d[
        !(no_node_d[ch_p('(')] 
          >> !(bound_variable >> *(no_node_d[ch_p(',')] >> bound_variable)) 
          >> no_node_d[ch_p(')')])];

      //特殊記号定義
      equivalent   = str_p("<=>");
      implies      = str_p("=>");
      always       = str_p("[]"); 
      differential = ch_p("'");
      previous     = ch_p('-');

      //半順序定義演算子
      weaker       = str_p("<<");
      parallel     = ch_p(',');

      //比較演算子
      comp_op     = less_eq
        | less
        | greater_eq
        | greater
        | equal
        | not_equal;

      less        = ch_p('<'); 
      less_eq     = str_p("<="); 
      greater     = ch_p('>'); 
      greater_eq  = str_p(">=");
      equal       = ch_p("="); 
      not_equal   = str_p("!="); 

      //論理演算子
      logical_and     = str_p("&") | str_p("/\\");
      logical_or      = str_p("|") | str_p("\\/");
      logical_not     = ch_p('!');

      //算術二項演算子
      add          = ch_p('+');
      sub          = ch_p('-');
      mul          = ch_p('*');
      div          = ch_p('/');
      pow          = ch_p('^') | str_p("**");


      //算術単項演算子
      positive    = ch_p('+');
      negative    = ch_p('-');
      
      //三角関数
      sin         = str_p("sin");
      cos         = str_p("cos");
      tan         = str_p("tan");
      asin        = str_p("arcsin");
      acos        = str_p("arccos");
      atan        = str_p("arctan");
      log         = str_p("log");
      ln          = str_p("ln");
    }

    // 開始ルール
    defRuleID(RI_HydLaProgram) const& start() const {
      return hydla_program;
    }
  };
};

} // namespace parser
} // namespace hydla

#endif //_INCLUDED_HYDLA_HYDLA_GRAMMAR_H_

