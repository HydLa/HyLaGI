/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_Y_TAB_HPP_INCLUDED
# define YY_YY_Y_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 36 "nc.y" /* yacc.c:1909  */

#include "Node.h"

#line 48 "y.tab.hpp" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    DOUBLE_LITERAL = 258,
    ATOMIC_PROPOSITION = 259,
    CR = 260,
    NEVER = 261,
    SKIP = 262,
    COLON = 263,
    SEMICOLON = 264,
    D_COLON = 265,
    DEFINITION = 266,
    IF = 267,
    FI = 268,
    GOTO = 269,
    TRUE = 270,
    ONE = 271,
    OR = 272,
    AND = 273,
    NOT = 274,
    ALWAYS = 275,
    EVENTUALLY = 276,
    BLANK = 277,
    L_CRULY = 278,
    R_CRULY = 279,
    L_PAREN = 280,
    R_PAREN = 281,
    R_ARROW = 282,
    IDENTIFIER = 283,
    CONSTRAINT_STRING = 284,
    COMMENT = 285
  };
#endif
/* Tokens.  */
#define DOUBLE_LITERAL 258
#define ATOMIC_PROPOSITION 259
#define CR 260
#define NEVER 261
#define SKIP 262
#define COLON 263
#define SEMICOLON 264
#define D_COLON 265
#define DEFINITION 266
#define IF 267
#define FI 268
#define GOTO 269
#define TRUE 270
#define ONE 271
#define OR 272
#define AND 273
#define NOT 274
#define ALWAYS 275
#define EVENTUALLY 276
#define BLANK 277
#define L_CRULY 278
#define R_CRULY 279
#define L_PAREN 280
#define R_PAREN 281
#define R_ARROW 282
#define IDENTIFIER 283
#define CONSTRAINT_STRING 284
#define COMMENT 285

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 40 "nc.y" /* yacc.c:1909  */

  char        *string_value;
  // hydla::symbolic_expression::Node *node;
  hydla::symbolic_expression::Node *node;

#line 126 "y.tab.hpp" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_HPP_INCLUDED  */
