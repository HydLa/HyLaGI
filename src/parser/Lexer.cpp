#include "Lexer.h"

namespace hydla {
namespace parser {

Lexer::Lexer() : line(0), column(0) {}
Lexer::Lexer(std::string file, std::istream &stream) : line(0), column(0) {
  file_info.push_back(file_info_t(file, 0));
  std::string tmp;
  while (std::getline(stream, tmp)) {
    strs.push_back(tmp);
  }
}

Lexer::Lexer(std::istream &stream) : line(0), column(0) {
  std::string tmp;
  while (std::getline(stream, tmp)) {
    strs.push_back(tmp);
  }
}

Lexer::Lexer(std::string str) : line(0), column(0) { strs.push_back(str); }
Lexer::Lexer(std::string file, std::vector<std::string> strings)
    : strs(strings), line(0), column(0) {
  file_info.push_back(file_info_t(file, 0));
}

Lexer::Lexer(std::vector<std::string> strings)
    : strs(strings), line(0), column(0) {}
Lexer::~Lexer() {}

void Lexer::add_file(std::string file, std::istream &stream) {
  file_info.push_back(file_info_t(file, strs.size()));
  std::string tmp;
  while (std::getline(stream, tmp)) {
    strs.push_back(tmp);
  }
}

Token Lexer::identifier() {
  std::string identifier;
  do {
    identifier += get_current_char();
  } while (next_char() &&
           (is_digit(get_current_char()) || is_alphabet(get_current_char()) ||
            get_current_char() == '_'));
  current_token_string = identifier;
  if ('A' <= current_token_string[0] && current_token_string[0] <= 'Z')
    return UPPER_IDENTIFIER;
  return LOWER_IDENTIFIER;
}

Token Lexer::number() {
  bool is_real = false;
  std::string number;
  do {
    if (get_current_char() == '.') {
      if (is_real) {
        break;
      } else {
        int now_line = line;
        int now_column = column;
        if (next_char() && is_digit(get_current_char())) {
          is_real = true;
          number += ".";
        } else {
          line = now_line;
          column = now_column;
          current_token_string = number;
          return INTEGER;
        }
      }
    }
    number += get_current_char();
  } while (next_char() &&
           (is_digit(get_current_char()) || get_current_char() == '.'));
  current_token_string = number;
  if (is_real)
    return NUMBER;
  else
    return INTEGER;
}

Token Lexer::get_token() {
  skip_space();
  if (strs.size() <= line)
    return END_OF_FILE;
  char current = get_current_char();
  // identifier | Pi | E
  if (is_alphabet(current) || current == '_') {
    return identifier();
  }
  // number
  if (is_digit(current)) {
    return number();
  }

  // else
  current_token_string = current;
  next_char();
  switch (current) {
  case '~':
    return TILDE;
  case '^':
    return POWER;
  case ']':
    return RIGHT_BOX_BRACKETS;
  case '(':
    return LEFT_PARENTHESES;
  case ')':
    return RIGHT_PARENTHESES;
  case '{':
    return LEFT_BRACES;
  case '}':
    return RIGHT_BRACES;
  case ',':
    return COMMA;
  case '&':
    return AMPERSAND;
  case '|':
    return VERTICAL_BAR;
  case '\'':
    return DIFFERENTIAL;
  case ';':
    return SEMICOLON;
  case '+':
    return PLUS;
  case '-':
    return MINUS;
  case '@':
    return COMMAND;
  case '$':
    return SYSTEM;
  case '\"':
    return DOUBLE_QUOTATION;
  }
  // 2 or more charactors
  switch (current) {
  case ':':
    if (get_current_char() == '=') {
      next_char();
      current_token_string = ":=";
      return DEFINITION;
    }
    return COLON;
  case '.':
    if (get_current_char() == '.') {
      next_char();
      current_token_string = "..";
      return TWO_PERIOD;
    }
    return PERIOD;
  case '!':
    if (get_current_char() == '=') {
      next_char();
      current_token_string = "!=";
      return NOT_EQUAL;
    }
    return NOT;
  case '*':
    if (get_current_char() == '*') {
      next_char();
      current_token_string = "**";
      return POWER;
    }
    return MUL;
  case '<':
    if (get_current_char() == '=') {
      next_char();
      if (get_current_char() == '>') {
        next_char();
        current_token_string = "<=>";
        return EQUIVALENT;
      } else {
        current_token_string = "<=";
        return LESS_EQUAL;
      }
    }
    if (get_current_char() == '<') {
      next_char();
      current_token_string = "<<";
      return WEAKER;
    }
    return LESS;
  case '>':
    if (get_current_char() == '=') {
      next_char();
      current_token_string = ">=";
      return GREATER_EQUAL;
    }
    return GREATER;
  case '=':
    if (get_current_char() == '>') {
      next_char();
      current_token_string = "=>";
      return IMPLIES;
    }
    return EQUAL;
  case '[':
    if (get_current_char() == ']') {
      next_char();
      current_token_string = "[]";
      return ALWAYS;
    }
    return LEFT_BOX_BRACKETS;
  case '/':
    current = get_current_char();
    if (current == '\\') {
      next_char();
      current_token_string = "/\\";
      return LOGICAL_AND;
    }
    if (current == '*') {
      next_char();
      skip_space();
      while (true) {
        if (get_current_char() == '*') {
          next_char();
          if (get_current_char() == '/') {
            next_char();
            return get_token();
          }
        } else {
          next_char();
          skip_space();
        }
      }
    }
    if (current == '/') {
      next_line();
      return get_token();
    }
    current_token_string = "/";
    return DIVIDE;
    break;
  case '\\':
    if (get_current_char() == '/') {
      next_char();
      current_token_string = "\\/";
      return LOGICAL_OR;
    }
    return BACKSLASH;
    break;
  }

  return UNKNOWN;
}

void Lexer::skip_space() {
  while (line < strs.size()) {
    while (column < strs[line].size()) {
      if (is_space(get_current_char()))
        column++;
      else
        return;
    }
    column = 0;
    line++;
  }
}

bool Lexer::next_line() {
  column = 0;
  line++;
  return line < strs.size();
}

bool Lexer::next_char() {
  column++;
  if (line < strs.size() && column < strs[line].size())
    return true;
  else
    return false;
}

} // namespace parser
} // namespace hydla
