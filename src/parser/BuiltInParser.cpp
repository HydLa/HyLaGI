#include <vector>
#include <cstdlib>
#include <exception>

#include "Lexer.h"
#include "Parser.h"

namespace hydla{
  namespace parser{

using namespace symbolic_expression;

/**
 * constant := "Pi"
 *           | "Infinity"
 *           | "E"
 */
node_sptr Parser::constant(){
  position_t position = lexer.get_current_position();
  if(lexer.get_token() == UPPER_IDENTIFIER){
    std::string str = lexer.get_current_token_string();
    if(str == "Pi") return std::shared_ptr<Pi>(new Pi());
    if(str == "Infinity") return std::shared_ptr<Infinity>(new Infinity());
    if(str == "E") return std::shared_ptr<E>(new E());
  }
  lexer.set_current_position(position);
  return node_sptr();
}

/**
 * system_variable := "$t"
 *                  | "$timer
 */
node_sptr Parser::system_variable(){
  node_sptr ret;
  position_t position = lexer.get_current_position();
  // "$"
  if(lexer.get_token() == SYSTEM){
    if(lexer.get_token() == LOWER_IDENTIFIER){
      std::string str = lexer.get_current_token_string();
      // t
      if(str == "t"){ return std::shared_ptr<SymbolicT>(new SymbolicT()); }
      // timer
      if(str == "timer"){ return std::shared_ptr<SVtimer>(new SVtimer()); }
    }
  }
  lexer.set_current_position(position);

  return node_sptr();
}

/// assertion := "ASSERT" "(" guard ")"
node_sptr Parser::assertion(){
  node_sptr ret;
  position_t position = lexer.get_current_position();
  lexer.get_token();
  // "ASSERT"
  if(lexer.get_current_token_string() == "ASSERT"){
    // "("
    position_t tmp_position = lexer.get_current_position();
    if(lexer.get_token() == LEFT_PARENTHESES){
      // guard
      if((ret = guard())){
        position_t tmp_position = lexer.get_current_position();
        // ")"
        if(lexer.get_token() == RIGHT_PARENTHESES){
          return ret;
        }else{
          error_occurred(tmp_position, "expected \")\"");
        }
      }
    }else{
      error_occurred(tmp_position, "expected \"(\"");
    }
  }
  lexer.set_current_position(position);

  return node_sptr(); 
}

/// tautology := "$TRUE"
node_sptr Parser::tautology(){
  position_t position = lexer.get_current_position();
  // "$"
  if(lexer.get_token() == SYSTEM){
    if(lexer.get_token() == UPPER_IDENTIFIER){
      // "TRUE"
      if(lexer.get_current_token_string() == "TRUE"){ return std::shared_ptr<True>(new True());}
      if(lexer.get_current_token_string() == "FALSE"){ return std::shared_ptr<False>(new False());}
    }
  }
  lexer.set_current_position(position);
  
  return node_sptr();
}

} // namespace parser
} // namespace hydla
