%{
#include <stdio.h>
#include <stdlib.h>
#include "PropertyNode.h"
#include "NeverClaim.h"
#include "ParseTreeGraphvizDumper.h"
#include "Parser.h"
#define YYDEBUG 1

extern "C"
{
int yyparse(void);
int yylex(void);
int yywrap(void)
{
  return 1;
}

int yyerror(char const *str)
{
  extern char *yytext;
  fprintf(stderr, "Parsing Error occured at \"%s\"\n", yytext);
  return 0;
}

}

using ConstraintBinding = std::pair<std::string, hydla::never_claim::NeverClaim::GuardCondition>;
hydla::simulator::Automaton * buchiAutomaton;
hydla::never_claim::NeverClaim nc;
std::string gLabelFrom;
std::map<std::string, hydla::never_claim::NeverClaim::GuardCondition> propositionMap;

%}

%code requires {
#include "Node.h"
}

%union {
  char        *string_value;
  hydla::symbolic_expression::Node *node;
}

%token <double_value>      DOUBLE_LITERAL
%token <char>              ATOMIC_PROPOSITION
%token CR
%token NEVER SKIP
%token COLON SEMICOLON D_COLON DEFINITION
%token IF FI GOTO TRUE ONE
%left OR
%left AND
%right NOT
%token ALWAYS EVENTUALLY BLANK
%token L_CRULY R_CRULY L_PAREN R_PAREN R_ARROW
%token <string_value> IDENTIFIER
%token <string_value> CONSTRAINT_STRING
%token COMMENT
%type <string_value> label from_label
%type <node> guard_condition proposition proposition_symbol

%%

never_claim_with_definitions
: proposition_definitions never_claim
;

proposition_definitions 
: proposition_definition
| proposition_definitions proposition_definition
;

proposition_definition
: IDENTIFIER CONSTRAINT_STRING CR {
  using namespace hydla;
  using namespace hydla::never_claim;

  std::string constraint(&($2[2])); // "remove ":="
  parser::ParseTreeGraphvizDumper ptDumper;
  parser::Parser parser(constraint);
  NeverClaim::GuardCondition cons_tree = parser.constraint()->get_child();
  /*
  std::cout << "==============" << std::endl;
  ptDumper.dump(std::cout, cons_tree);
  */
  propositionMap[$1] = cons_tree;
}
;

never_claim
: NEVER L_CRULY CR labelled_contents R_CRULY CR
;

labelled_contents
: labelled_statements
| labelled_contents labelled_statements
;

labelled_statements
: from_label COLON CR if_statement
| from_label COLON CR SKIP CR {
  using hydla::never_claim::NeverClaim;
  using namespace hydla::symbolic_expression;
  nc.addEdge(gLabelFrom, gLabelFrom, NeverClaim::GuardCondition(new True()));
}
;

from_label
: label {
  $$ = $1;
  nc.addNewLabel(std::string($1));
  gLabelFrom = std::string($1);
}

label
: IDENTIFIER {
  $$ = $1;
}
;

if_statement
: IF CR case_statements FI SEMICOLON CR
;

case_statements
: case_statement
| case_statements case_statement
;

case_statement
: D_COLON guard_condition R_ARROW GOTO label CR {
  using hydla::never_claim::NeverClaim;
  using namespace hydla::symbolic_expression;

  std::string nextLabel($5);
  NeverClaim::GuardCondition guard = NeverClaim::GuardCondition($2);

  nc.addNewLabel(nextLabel);
  nc.addEdge(gLabelFrom, nextLabel, guard);
}
;

guard_condition
: proposition
;

proposition
: proposition_symbol { $$ = $1; }
| L_PAREN proposition R_PAREN { $$ = $2; }
| proposition OR proposition {
  using namespace hydla::symbolic_expression;
  using namespace hydla::never_claim;
  $$ = new LogicalOr(NeverClaim::GuardCondition($1), NeverClaim::GuardCondition($3));
}
| proposition AND proposition {
  using namespace hydla::symbolic_expression;
  using namespace hydla::never_claim;
  $$ = new LogicalAnd(NeverClaim::GuardCondition($1), NeverClaim::GuardCondition($3));
}
| NOT proposition {
  using namespace hydla::symbolic_expression;
  using namespace hydla::never_claim;
  $$ = new Not(NeverClaim::GuardCondition($2));
}
;

proposition_symbol
: TRUE {
  using namespace hydla::symbolic_expression;
  $$ = new True();
}
| ONE {
  using namespace hydla::symbolic_expression;
  $$ = new True();
}
| IDENTIFIER {
  // using namespace hydla::symbolic_expression;
  using namespace hydla;
  using namespace hydla::never_claim;

/*
  parser::ParseTreeGraphvizDumper ptDumper;
  std::cout << "======" << std::endl;
  std::cout << $1 << std::endl;
  */
  if (propositionMap[$1] == nullptr)
  {
    std::cout << "\nnever claim: Proposition \"" << $1 << "\" is not defined." << std::endl;
    exit(1);
  }
  // ptDumper.dump(std::cout, propositionMap[$1]);

  $$ = propositionMap[$1].get();
  // $$ = new Variable($1);
}
;

%%
using namespace hydla;
using namespace hydla::simulator;
using namespace hydla::never_claim;
using namespace symbolic_expression;
using namespace std;

NeverClaim::BuchiAutomatonSptr nc_parse()
{
  // extern int yyparse(void);
  extern FILE *yyin;
  std::string error_str = "";

  //================================
  // An Example of a never claim 
  //================================
  // never { /* !(GFp && GF!p) */
  // T0_init :    /* init */
  //     if
  //     :: (1) -> goto T0_init
  //     :: (!p) -> goto accept_S2
  //     :: (p) -> goto accept_S3
  //     fi;
  // accept_S2 :    /* 1 */
  //     if
  //     :: (!p) -> goto accept_S2
  //     fi;
  // accept_S3 :    /* 2 */
  //     if
  //     :: (p) -> goto accept_S3
  //     fi;
  // }
  //================================

  yyin = stdin;
  if (yyparse()) {
    fprintf(stderr, "never claim: Sorry, failed in parsing =_=;\n");
    exit(1);
  }
  printf("never claim: Successfully parsed! :)\n");

  /*
  nc.printAllLabels();
  nc.printAllEdges();
  */
  auto ba = nc.createBA();
  return ba;
}

#include "lex.yy.c"
