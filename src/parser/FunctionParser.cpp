#include <vector>
#include <cstdlib>
#include <exception>

#include "Lexer.h"
#include "Parser.h"

namespace hydla{
  namespace parser{

using namespace symbolic_expression;

/// function_name := [a-z,A-Z]+
std::string Parser::function_name(){
  position_t position = lexer.get_current_position();
  Token token = lexer.get_token();
  if(token == LOWER_IDENTIFIER || token == UPPER_IDENTIFIER){
    std::string name = lexer.get_current_token_string();
    bool alpha = true;
    for(auto c : name){
      if(!(('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z'))){
        alpha = false;
        break;
      }
      if(alpha) return name;
    }
  }
  lexer.set_current_position(position);
  return std::string();
}

/// unsupported_function := " " " function_name " " "
boost::shared_ptr<UnsupportedFunction> Parser::unsupported_function(){
  position_t position = lexer.get_current_position();
  std::string name;
  // " " "
  if(lexer.get_token() == DOUBLE_QUOTATION){
    // " [a-z, A-Z]+ "
    if((name = function_name()) != ""){
      // " " " 
      if(lexer.get_token() == DOUBLE_QUOTATION){
        return boost::shared_ptr<UnsupportedFunction>(new UnsupportedFunction(name));
      }
    }
  }
  lexer.set_current_position(position);

  return boost::shared_ptr<UnsupportedFunction>(); 
}

/// function := function_name 
boost::shared_ptr<Function> Parser::function(){
  position_t position = lexer.get_current_position();
  std::string name;
  // [a-z,A-Z]+
  if((name = function_name()) != ""){ 
    return boost::shared_ptr<Function>(new Function(name));
  }
  lexer.set_current_position(position);

  return boost::shared_ptr<Function>();
}

/// actual_args := ( "("expression ("," expression)*")" )?
std::vector<node_sptr> Parser::actual_args(){
  std::vector<node_sptr> ret;
  position_t position = lexer.get_current_position();
  // "("
  if(lexer.get_token() == LEFT_PARENTHESES){
    node_sptr var;
    // expression
    if((var = expression())){
      ret.push_back(var);
      position_t tmp_position = lexer.get_current_position();
      // ("," expression)*
      while(lexer.get_token() == COMMA){
        if((var = expression())){
          ret.push_back(var);
        }else{
          lexer.set_current_position(position);
          return std::vector<node_sptr>();
        }
        tmp_position = lexer.get_current_position();
      }
      lexer.set_current_position(tmp_position);
    }else{ 
      error_occurred(lexer.get_current_position(), "expected expression");
    }
    position_t right_position = lexer.get_current_position();
    if(lexer.get_token() == RIGHT_PARENTHESES){ return ret; }
    else{
      error_occurred(right_position, "expected \")\"");
    }
  }
  lexer.set_current_position(position);

  return std::vector<node_sptr>();
}

/// formal_args := ( "(" variable ("," variable)* ")" )?
std::vector<std::string> Parser::formal_args(){
  std::vector<std::string> ret;
  position_t position = lexer.get_current_position();
  // "("
  if(lexer.get_token() == LEFT_PARENTHESES){
    boost::shared_ptr<Variable> var;
    // variable
    if((var = variable())){
      ret.push_back(var->get_name());
      position_t tmp_position = lexer.get_current_position();
      // ("," variable)*
      while(lexer.get_token() == COMMA){
        if((var = variable())){
          ret.push_back(var->get_name());
        }else{
          error_occurred(lexer.get_current_position(), "expected variable after \",\"");
          lexer.set_current_position(position);
          return std::vector<std::string>();
        }
        tmp_position = lexer.get_current_position();
      }
      lexer.set_current_position(tmp_position);
    }
    position_t right_position = lexer.get_current_position();
    if(lexer.get_token() == RIGHT_PARENTHESES){ return ret; }
    else{
      error_occurred(right_position, "expected \")\"");
    }
  }
  lexer.set_current_position(position);

  return std::vector<std::string>(); 
}

} // namespace parser
} // namespace hydla
