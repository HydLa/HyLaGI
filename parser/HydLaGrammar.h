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


    defRuleID(RI_Ask_Logical_Literal)  ask_logical_literal; //!��������
    defRuleID(RI_Ask_Logical_Term)  ask_logical_term; 
    defRuleID(RI_Ask_Logical)       ask_logical;
    defRuleID(RI_Comparison)        comparison; 
    defRuleID(RI_Assert)           assert;

    defRuleID(RI_Statements)       statements;
    defRuleID(RI_HydLaProgram)     hydla_program;

    // �\����`
    definition(const HydLaGrammar& self) {

      //�J�n
      hydla_program = gen_pt_node_d[statements];
            
      //���̏W��
      statements  = gen_ast_node_d[*((assert | def_statement | program) 
                                     >> discard_node_d[ch_p('.')]) >> end_p];

      //�v���O����
      program = program_parallel;

      program_parallel = program_ordered % root_node_d[parallel];
      program_ordered  = program_factor  % root_node_d[weaker];
      program_factor   = 
          gen_pt_node_d[program_caller] 
            >> eps_p(*ch_p(')') >> (parallel | weaker | ch_p('}') | ch_p('.')))
        | no_node_d[ch_p('(')] >> program_parallel >> no_node_d[ch_p(')')] // do not use inner_node_d
            >> eps_p(*ch_p(')') >> (parallel | weaker | ch_p('}') | ch_p('.')))
        | module;
      
      // ��`
      def_statement = gen_pt_node_d[constraint_def | program_def];
      
      // assert��
      assert = root_node_d[str_p("ASSERT")] >> no_node_d[ch_p('(')] >> ask_logical >> no_node_d[ch_p(')')];

      //program��`
      program_def = 
        gen_pt_node_d[program_callee] 
          >> no_node_d[ch_p('{')] >> program >> no_node_d[ch_p('}')];
      
      //constraint��`
      constraint_def = 
        gen_pt_node_d[constraint_callee] >> no_node_d[equivalent] >> gen_pt_node_d[constraint];

      //constraint��`���̍���
      constraint_callee = 
        gen_ast_node_d[constraint_name] >> gen_pt_node_d[formal_args];

      //constraint�Ăяo��
      constraint_caller = 
        gen_ast_node_d[constraint_name] >> gen_pt_node_d[actual_args];

      //program��`���̍���
      program_callee = 
        gen_ast_node_d[program_name] >> gen_pt_node_d[formal_args];

      //program�Ăяo��
      program_caller= 
        gen_ast_node_d[program_name] >> gen_pt_node_d[actual_args];

      //module��`
      module = gen_pt_node_d[constraint];
      
      //����
      constraint = gen_ast_node_d[logical];

      //�_���a
      logical      = logical_term % root_node_d[logical_or];

      //�_����
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
        expression >> root_node_d[comp_op] >> expression];

      //��
      expression = arithmetic;

      //�Z�p��
      arithmetic =
        arith_term % root_node_d[add | sub];
      
      //�Z�p��
      arith_term =  
        unary % root_node_d[mul | div];
      
      
      //�P�����Z�q
      unary = !(root_node_d[positive | negative]) >> power_term;
      
      //�ׂ���
      power_term = limit >> !(root_node_d[pow] >> power_term);

      //�Ɍ�
      //factor�ȊO�̕������ɂ�������prev
      limit = diff >> !(root_node_d[previous] >> eps_p(eps_p - factor));
      

      //����
      diff = factor >> *(root_node_d[differential]);

      //���q
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
      //�_���a
      ask_logical = ask_logical_term % root_node_d[logical_or];

      //�_����
      ask_logical_term = ask_logical_literal % root_node_d[logical_and];
      
      //�ے�
      ask_logical_literal = !(root_node_d[logical_not]) >> comparison;

      //��r
      comparison = 
        expression >> root_node_d[comp_op] >> expression
        | gen_pt_node_d[constraint_caller]
        | no_node_d[ch_p('(')] >> ask_logical >> no_node_d[ch_p(')')];
      
      //�~����
      pi = str_p("Pi");
      
      //���R�ΐ��̒�
      e = str_p("E");
      
      arbitrary_binary = no_node_d[ch_p('"')] >> leaf_node_d[+alpha_p] >> no_node_d[ch_p('"')];
      arbitrary_unary = no_node_d[ch_p('"')] >> leaf_node_d[+alpha_p] >> no_node_d[ch_p('"')];
      arbitrary_factor = no_node_d[ch_p('"')] >> leaf_node_d[+alpha_p] >> no_node_d[ch_p('"')];
      
      //����
      number = 
        lexeme_d[leaf_node_d[
          +digit_p >> !('.' >> +digit_p)]]; 

      //�ϐ�
      variable = lexeme_d[leaf_node_d[identifier]];

      //�����ϐ�
      bound_variable = lexeme_d[leaf_node_d[identifier]];

      //����
      constraint_name = lexeme_d[leaf_node_d[identifier]];

      //���W���[����
      program_name = lexeme_d[leaf_node_d[identifier]];

      //���ʎq
      identifier = lexeme_d[leaf_node_d[
          (alpha_p | '_') >> *(alpha_p | digit_p | '_')]];

      //������
      actual_args = gen_ast_node_d[
        !(no_node_d[ch_p('(')] 
          >> !(expression >> *(no_node_d[ch_p(',')] >> expression)) 
          >> no_node_d[ch_p(')')])];

      //������
      formal_args = gen_ast_node_d[
        !(no_node_d[ch_p('(')] 
          >> !(bound_variable >> *(no_node_d[ch_p(',')] >> bound_variable)) 
          >> no_node_d[ch_p(')')])];

      //����L����`
      equivalent   = str_p("<=>");
      implies      = str_p("=>");
      always       = str_p("[]"); 
      differential = ch_p("'");
      previous     = ch_p('-');

      //��������`���Z�q
      weaker       = str_p("<<");
      parallel     = ch_p(',');

      //��r���Z�q
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

      //�_�����Z�q
      logical_and     = str_p("&") | str_p("/\\");
      logical_or      = str_p("|") | str_p("\\/");
      logical_not     = ch_p('!');

      //�Z�p�񍀉��Z�q
      add          = ch_p('+');
      sub          = ch_p('-');
      mul          = ch_p('*');
      div          = ch_p('/');
      pow          = ch_p('^') | str_p("**");


      //�Z�p�P�����Z�q
      positive    = ch_p('+');
      negative    = ch_p('-');
      
      //�O�p�֐�
      sin         = str_p("sin");
      cos         = str_p("cos");
      tan         = str_p("tan");
      asin        = str_p("arcsin");
      acos        = str_p("arccos");
      atan        = str_p("arctan");
      log         = str_p("log");
      ln          = str_p("ln");
    }

    // �J�n���[��
    defRuleID(RI_HydLaProgram) const& start() const {
      return hydla_program;
    }
  };
};

} // namespace parser
} // namespace hydla

#endif //_INCLUDED_HYDLA_HYDLA_GRAMMAR_H_

