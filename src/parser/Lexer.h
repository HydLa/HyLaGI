#pragma once

#include<string>
#include<vector>
#include<fstream>
#include<iostream>
#include<istream>
#include<utility>

namespace hydla{
  namespace parser{

enum Token{
  ALPHABET,            //  [a-z, A-Z]+
  IDENTIFIER,          //  [a-z, A-Z, "_"] [a-z, A-Z, "_", 0-9]*
  NUMBER,              //  ([0-9]*) [. [0-9]* ]?
  INTEGER,             //  ([0-9]*)
  EQUIVALENT,          //  "<=>"
  IMPLIES,             //  "=>"
  ALWAYS,              //  "[]"
  WEAKER,              //  "<<"
  COMMAND,             //  "@"
  SYSTEM,              //  "$"
  LEFT_PARENTHESES,    //  "("
  RIGHT_PARENTHESES,   //  ")"
  LEFT_BRACES,         //  "{"
  RIGHT_BRACES,        //  "}"
  LEFT_BOX_BRACKETS,   //  "["
  RIGHT_BOX_BRACKETS,  //  "]"
  PERIOD,              //  "."
  COMMA,               //  ","
  PLUS,                //  "+"
  MINUS,               //  "-"
  MUL,                 //  "*"
  DIVIDE,              //  "/"
  POWER,               //  "^" | "**"
  EQUAL,               //  "="
  NOT_EQUAL,           //  "!="
  NOT,                 //  "!"
  LESS,                //  "<"
  LESS_EQUAL,          //  "<="
  GREATER,             //  ">"
  GREATER_EQUAL,       //  ">="
  LOGICAL_AND,         //  "/\"
  AMPERSAND,           //  "&"
  LOGICAL_OR,          //  "\/"
  VERTICAL_BAR,        //  "|"
  END_OF_FILE,         //  EOF
  DIFFERENTIAL,        //  "'"
  DOUBLE_QUOTATION,    //  """
  UNKNOWN,             //  other
  ERROR                //  error
};

class Lexer{
public:
  Lexer(std::string);
  Lexer(std::istream&);
  Lexer(std::vector<std::string>);
  ~Lexer();
  Token get_token();
 
  std::string get_string(int line){ return strs[line]; }
  std::string get_current_token_string(){ return current_token_string; }
  std::pair<int,int> get_current_position(){ return std::pair<int,int>(line,column); }
  void set_current_position(std::pair<int,int> p){ line = p.first; column = p.second; }

private:

  Token identifier();
  Token number();

  // support functions
  bool is_digit(char c){ return '0'<=c && c<='9'; }
  bool is_alphabet(char c){ return (is_capital_alphabet(c) || is_small_alphabet(c)); }
  bool is_capital_alphabet(char c){ return 'A'<=c && c<='Z'; }
  bool is_small_alphabet(char c){ return 'a'<=c && c<='z'; }
  bool is_space(char c){ return c==' ' || c=='\n' || c=='\t' || c=='\r'; }
  char get_current_char(){ return strs[line][column]; }
  bool next_line();
  bool next_char();
  void skip_space();

  // all strings of input file.
  std::vector<std::string> strs;

  // previous Token
  Token previous_token;
  // current token string
  std::string current_token_string;
  // line number that Lexer is reading now
  int line;
  // column number than Lexer is reading now
  int column;
};

  } // namespace parser
} // namespace hydla
