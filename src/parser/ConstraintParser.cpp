#include <vector>
#include <cstdlib>
#include <exception>

#include "Lexer.h"
#include "Parser.h"

namespace hydla{
  namespace parser{

using namespace symbolic_expression;

/// constraint_callee := name formal_args
std::shared_ptr<ConstraintDefinition> Parser::constraint_callee(){
  position_t position = lexer.get_current_position();
  std::shared_ptr<ConstraintDefinition> ret(new ConstraintDefinition());
  std::vector<std::string> args;
  std::string name;
  // name
  if((name = definition_name()) != ""){
    ret->set_name(name);
    // formal_args
    args = formal_args();
    if(!args.empty()){
      for(auto arg : args) ret->add_bound_variable(arg);
    }
    return ret;
  }
  lexer.set_current_position(position);
  return std::shared_ptr<ConstraintDefinition>();
}

/// constraint_caller := name actual_args
std::shared_ptr<ConstraintCaller> Parser::constraint_caller(){
  position_t position = lexer.get_current_position();
  std::shared_ptr<ConstraintCaller> ret(new ConstraintCaller());
  std::vector<node_sptr> args;
  std::string name;
  // name
  if((name = definition_name()) != ""){
    ret->set_name(name);
    // actual_args
    args = actual_args();
    if(!args.empty()){
      for(auto arg : args) ret->add_actual_arg(arg);
    }
    if(second_parse)
    {
      node_sptr defined;
      IS_DEFINED_AS(name,args.size(),tmp_constraint_definitions,defined);
      if((defined)) return ret;
      error_occurred(lexer.get_current_position(), "undefined constraint - " + std::to_string(args.size()) + " args constraint \"" + name + "\"");
    }
    else
    {
      return ret;
    }
  }
  lexer.set_current_position(position);
  return std::shared_ptr<ConstraintCaller>();
}

/// constraint := logical_or
std::shared_ptr<Constraint> Parser::constraint(){
  node_sptr ret;
  if((ret = logical_or())) return std::shared_ptr<Constraint>(new Constraint(ret));
  return std::shared_ptr<Constraint>();
}

/// logical_or := logical_and ( ("|" | "\/") logical_and )*
node_sptr Parser::logical_or(){ 
  node_sptr tmp_l;
  // logical_and
  if((tmp_l = logical_and())){
    // ( ("|" | "\/") logical_and )*
    position_t zero_position = lexer.get_current_position();
    node_sptr rhs;
    Token token = lexer.get_token();
    while(token == VERTICAL_BAR || token == LOGICAL_OR){
      std::string or_token = lexer.get_current_token_string();
      if((rhs = logical_and())){
        std::shared_ptr<LogicalOr> tmp(new LogicalOr());
        tmp->set_lhs(tmp_l);
        tmp->set_rhs(rhs);
        tmp_l = tmp;
      }else{
        error_occurred(lexer.get_current_position(), "expected constraint after \"" + or_token + "\"");
        break;
      }
      zero_position = lexer.get_current_position();
      token = lexer.get_token();
    }
    lexer.set_current_position(zero_position);
    return tmp_l;
  }
  return node_sptr();
}

/// logical_and := always ( ("&" | "/\") always )*
node_sptr Parser::logical_and(){
  node_sptr tmp_l;
  // always
  if((tmp_l = always())){
    // ( ("&" | "/\") always )*
    position_t zero_position = lexer.get_current_position();
    node_sptr rhs;
    Token token = lexer.get_token();
    while(token == AMPERSAND || token == LOGICAL_AND){
      std::string and_token = lexer.get_current_token_string();
      if((rhs = always())){
        std::shared_ptr<LogicalAnd> tmp(new LogicalAnd());
        tmp->set_lhs(tmp_l);
        tmp->set_rhs(rhs);
        tmp_l = tmp;
      }else{
        error_occurred(lexer.get_current_position(), "expected constraint after \"" + and_token + "\"");
        break;
      }
      zero_position = lexer.get_current_position();
      token = lexer.get_token();
    }
    lexer.set_current_position(zero_position);
    return tmp_l;
  }
  return node_sptr();
}

/// always := "[]"? conditional_constraint
node_sptr Parser::always(){
  node_sptr ret;
  std::shared_ptr<Always> always(new Always());
  position_t position = lexer.get_current_position();
  // "[]"
  if(lexer.get_token() == ALWAYS){
    // conditional_constraint
    if((ret = conditional_constraint())){
      always->set_child(ret);
      return always;
    }else{
      error_occurred(lexer.get_current_position(), "expected constraint after \"[]\"");
    }
  }
  lexer.set_current_position(position);

  // conditional_constraint();
  return conditional_constraint();
}

/**
 * conditional_constraint := guard "=>" constraint
 *                         | constraint_caller
 *                         | "(" logical_and ")"
 *                         | atomic_constraint
 */
node_sptr Parser::conditional_constraint(){
  node_sptr ret;
  position_t position = lexer.get_current_position();

  // guard "=>" constraint
  if((ret = guard())){
    std::shared_ptr<Ask> ask(new Ask());
    node_sptr c;
    // "=>"
    if(lexer.get_token() == IMPLIES){
      // constraint
      if((c = constraint())){
        ask->set_guard(ret);
        ask->set_child(c);
        return ask;
      }else{
        error_occurred(lexer.get_current_position(), "expected constraint after \"=>\"");
      }
    }
  }
  lexer.set_current_position(position);

  // atomic_constraint
  if((ret = atomic_constraint())){ return ret;}

  // constraint_caller
  if((ret = constraint_caller())){ return ret;}

  // "(" logical_and ")"
  if(lexer.get_token() == LEFT_PARENTHESES){
    // logical_and
    if((ret = logical_and())){
      // ")"
      position_t error_position = lexer.get_current_position();
      if(lexer.get_token() == RIGHT_PARENTHESES){
        return ret;
      }else{
        error_occurred(error_position, "expected \")\"");
      }
    }
  }
  lexer.set_current_position(position);

  return node_sptr();
}

/**
 * atomic_constraint := compare_expression
 *                    | tautology
 */
node_sptr Parser::atomic_constraint(){
  node_sptr ret;
  // tautology
  if((ret = tautology())){ return ret;}
  // compare_expression
  if((ret = compare_expression())){ return ret; }

  return node_sptr();
}

/// guard := guard_term (("|" | "\/") guard_term)*
node_sptr Parser::guard(){
  node_sptr tmp_l;
  // guard_term
  if((tmp_l = guard_term())){
    // (("|" | "\.") guard_term)*
    position_t zero_position = lexer.get_current_position();
    node_sptr rhs;
    Token token = lexer.get_token();
    while(token == VERTICAL_BAR || token == LOGICAL_OR){
      std::string or_token = lexer.get_current_token_string();
      if((rhs = guard_term())){
        std::shared_ptr<LogicalOr> tmp(new LogicalOr());
        tmp->set_lhs(tmp_l);
        tmp->set_rhs(rhs);
        tmp_l = tmp;
      }else{
        error_occurred(lexer.get_current_position(), "expected constraint after \""+or_token+"\"");
        break;
      }
      zero_position = lexer.get_current_position();
      token = lexer.get_token();
    }
    lexer.set_current_position(zero_position);
    return tmp_l;
  }
  return node_sptr();
}

/// guard_term := logical_not (( "&" | "/\") logical_not)*
node_sptr Parser::guard_term(){
  node_sptr tmp_l;
  // logical_not
  if((tmp_l = logical_not())){
    // (( "&" | "/\") logical_not)*
    position_t zero_position = lexer.get_current_position();
    node_sptr rhs;
    Token token = lexer.get_token();
    while(token == AMPERSAND || token == LOGICAL_AND){
      std::string and_token = lexer.get_current_token_string();
      if((rhs = logical_not())){
        std::shared_ptr<LogicalAnd> tmp(new LogicalAnd());
        tmp->set_lhs(tmp_l);
        tmp->set_rhs(rhs);
        tmp_l = tmp;
      }else{
        error_occurred(lexer.get_current_position(), "expected expression after \""+and_token+"\"");
        break;
      }
      zero_position = lexer.get_current_position();
      token = lexer.get_token();
    }
    lexer.set_current_position(zero_position);
    return tmp_l;
  }

  return node_sptr();
}

/// logical_not := "!"? comparison
node_sptr Parser::logical_not(){
  node_sptr ret;
  position_t position = lexer.get_current_position();
  // "!"
  if(lexer.get_token() == NOT){
    // comparison
    if((ret = comparison())){
      return std::shared_ptr<Not>(new Not(ret));
    }else{
      error_occurred(lexer.get_current_position(), "expected constraint after \"!\"");
    }
  }
  lexer.set_current_position(position);

  // comparison
  return comparison();
}

/**
 * comparison := compare_expression
 *             | "(" guard ")"
 *             | constraint_caller
 */
node_sptr Parser::comparison(){
  node_sptr ret;
  position_t position = lexer.get_current_position();
  // compare_expression
  if((ret = compare_expression())){ return ret; }
  // constraint_caller
  if((ret = constraint_caller())){ return ret;}
  // "(" guard ")"
  if(lexer.get_token() == LEFT_PARENTHESES){
    // guard
    if((ret = guard())){
      // ")"
      position_t right_position = lexer.get_current_position();
      if(lexer.get_token() == RIGHT_PARENTHESES){
        return ret;
      }else{
        error_occurred(right_position, "expected \")\"");
      }
    }
  }
  lexer.set_current_position(position);

  return node_sptr();
}

} // namespace parser
} // namespace hydla
