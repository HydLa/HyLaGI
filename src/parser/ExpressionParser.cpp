#include <vector>
#include <cstdlib>
#include <exception>

#include "Lexer.h"
#include "Parser.h"

namespace hydla{
  namespace parser{

using namespace symbolic_expression;

/// compare_expression := expression (("<"|"<="|">"|">="|"="|"!=") expression)+
node_sptr Parser::compare_expression(){
  node_sptr ret;
  node_sptr lhs;
  position_t position = lexer.get_current_position();
  // expression
  if((lhs = expression())){
    Token token = lexer.get_token();
    // ("<"|"<="|">"|">="|"="|"!=")
    if(token == LESS || token == LESS_EQUAL
    || token == GREATER || token == GREATER_EQUAL
    || token == EQUAL || token == NOT_EQUAL){
      node_sptr rhs;
      do{
        std::string op_token = lexer.get_current_token_string();
        // expression
        if((rhs = expression())){
          switch(token){
            case LESS:
              lhs = boost::shared_ptr<Less>(new Less(lhs,rhs));
              break;
            case LESS_EQUAL:
              lhs = boost::shared_ptr<LessEqual>(new LessEqual(lhs,rhs));
              break;
            case GREATER:
              lhs = boost::shared_ptr<Greater>(new Greater(lhs,rhs));
              break;
            case GREATER_EQUAL:
              lhs = boost::shared_ptr<GreaterEqual>(new GreaterEqual(lhs,rhs));
              break;
            case EQUAL:
              lhs = boost::shared_ptr<Equal>(new Equal(lhs,rhs));
              break;
            case NOT_EQUAL:
              lhs = boost::shared_ptr<UnEqual>(new UnEqual(lhs,rhs));
              break;
            default: break;
          }
          if(!ret) ret = lhs;
          else ret = boost::shared_ptr<LogicalAnd>(new LogicalAnd(ret,lhs));
          lhs = rhs;
        }else{
          error_occurred(lexer.get_current_position(), "expected expression after \""+op_token+"\"");
          break;
        }
        position = lexer.get_current_position();
        token = lexer.get_token();
      }while(token == LESS || token == LESS_EQUAL
          || token == GREATER || token == GREATER_EQUAL
          || token == EQUAL || token == NOT_EQUAL);
      lexer.set_current_position(position);
      if(ret){ return ret;}
    }
    lexer.set_current_position(position);
  }

  return node_sptr();
}

/**
 * expression := arithmetic
 *             | expression_list_element
 */
node_sptr Parser::expression(){ 
  node_sptr ret;
  if((ret = arithmetic())) return ret;
  return expression_list_element();
}

/// arithmetic := arith_term (("+"|"-") arith_term)*
node_sptr Parser::arithmetic(){
  node_sptr ret;
  // arith_term
  if((ret = arith_term())){
    position_t position = lexer.get_current_position();
    Token token = lexer.get_token();
    // ("+"|"-")
    while(token == PLUS || token == MINUS){
      std::string op_token = lexer.get_current_token_string();
      node_sptr tmp;
      // arith_term
      if((tmp = arith_term())){
        switch(token){
          case PLUS:
            ret = boost::shared_ptr<Plus>(new Plus(ret, tmp));
            break;
          case MINUS:
            ret = boost::shared_ptr<Subtract>(new Subtract(ret, tmp));
            break;
          default:
            break;
        }
      }else{
        error_occurred(lexer.get_current_position(), "expected expression after \""+op_token+"\"");
        break;
      }
      position = lexer.get_current_position();
      token = lexer.get_token();
    }
    lexer.set_current_position(position);
    return ret;
  }

  return node_sptr();
}

/// arith_term := unary (("*"|"/") unary)*
node_sptr Parser::arith_term(){
  node_sptr ret;
  // unary
  if((ret = unary())){
    position_t position = lexer.get_current_position();
    Token token = lexer.get_token();
    // ("*"|"/")
    while(token == MUL || token == DIVIDE){
      std::string op_token = lexer.get_current_token_string();
      node_sptr tmp;
      // unary
      if((tmp = unary())){
        switch(token){
          case MUL:
            ret = boost::shared_ptr<Times>(new Times(ret, tmp));
            break;
          case DIVIDE:
            ret = boost::shared_ptr<Divide>(new Divide(ret, tmp));
            break;
          default:
            break;
        }
      }else{
        error_occurred(lexer.get_current_position(), "expected expression after \""+op_token+"\"");
        break;
      }
      position = lexer.get_current_position();
      token = lexer.get_token();
    }
    lexer.set_current_position(position);
    return ret;
  }

  return node_sptr();
}

/// unary := ("+"|"-")? power
node_sptr Parser::unary(){
  node_sptr ret;
  position_t position = lexer.get_current_position();
  Token token = lexer.get_token();
  // ("+"|"-")
  if(token == PLUS || token == MINUS){
    std::string op_token = lexer.get_current_token_string();
    // power
    if((ret = power())){
      switch(token){
        case PLUS:
          return boost::shared_ptr<Positive>(new Positive(ret));
        case MINUS:
          return boost::shared_ptr<Negative>(new Negative(ret));
        default:
          break;
      }
    }else{
      error_occurred(lexer.get_current_position(), "expected expression after \""+op_token+"\"");
    }
  }
  lexer.set_current_position(position);

  // power
  return power();
}

/// power := prev (("**" | "^") power )?
node_sptr Parser::power(){
  node_sptr ret;
  // prev
  if((ret = prev())){
    position_t position = lexer.get_current_position();
    // ("**"|"^")
    if(lexer.get_token() == POWER){
      std::string op_token = lexer.get_current_token_string();
      node_sptr tmp;
      // power
      if((tmp = power())){ return boost::shared_ptr<Power>(new Power(ret,tmp));}
      else{
        error_occurred(lexer.get_current_position(), "expected expression after \""+op_token+"\"");
      }
    }
    lexer.set_current_position(position);
    return ret;
  }

  return node_sptr();
}

/// prev := diff ("-" [^factor])?
node_sptr Parser::prev(){
  node_sptr ret;
  // diff
  if((ret = diff())){
    position_t position = lexer.get_current_position();
    // "-"
    if(lexer.get_token() == MINUS){
      // [^factor]
      if(!(factor())){ return boost::shared_ptr<Previous>(new Previous(ret));}
    }
    lexer.set_current_position(position);
    return ret;
  }

  return node_sptr();
}

/// diff := factor "'"*
node_sptr Parser::diff(){
  node_sptr ret;
  // factor
  if((ret = factor())){
    position_t position = lexer.get_current_position();
    // "'"*
    while(lexer.get_token() == DIFFERENTIAL){
      ret = boost::shared_ptr<Differential>(new Differential(ret));
      position = lexer.get_current_position();
    }
    lexer.set_current_position(position);
    return ret;
  }

  return node_sptr();
}

/**
 * factor := constant 
 *         | sum_of_list
 *         | mul_of_list
 *         | (fanction | unsupported_function) "(" (expression ("," expression)* )? ")"
 *         | parameter
 *         | variable
 *         | system_variable
 *         | number
 *         | expression_list_element
 *         | "(" expression ")"
 */
node_sptr Parser::factor(){
  node_sptr ret;
  position_t position = lexer.get_current_position();
  std::string name;
  // Pi | E | Infinity
  if((ret = constant())) return ret;
  // sum_of_list
  if((ret = sum_of_list())){ return ret;}
  // mul_of_list
  if((ret = mul_of_list())){ return ret;}

  boost::shared_ptr<VariadicNode> func;
  // (function | unsupported_function) "(" (expression ("," expression)* )? ")"
  if((func = function()) || (func = unsupported_function())){
    // "("
    if(lexer.get_token() == LEFT_PARENTHESES){
      // expression
      if((ret = expression())){
        position_t tmp_position = lexer.get_current_position();
        func->add_argument(ret);
        // ("," expression)*
        while(lexer.get_token() == COMMA){
          if((ret = expression())) func->add_argument(ret);
          else break;
          tmp_position = lexer.get_current_position();
        }
        lexer.set_current_position(tmp_position);
        // ")"
        if(lexer.get_token() == RIGHT_PARENTHESES){ return func; }
        else{
          error_occurred(tmp_position, "expected \")\"");
        }
      }
    }
  }
  lexer.set_current_position(position);
  // parameter
  if((ret = parameter())){ return ret;}
  // variable
  if((ret = variable())){ return ret;}
  // system_variable
  if((ret = system_variable())){ return ret;}
  // number
  if((ret = number())){ return ret;}
  // expression_list_element
  if((ret = expression_list_element())){ return ret; }
  // "(" expression ")"
  if(lexer.get_token() == LEFT_PARENTHESES){
    // expression
    if((ret = expression())){
      // ")"
      position_t right_position = lexer.get_current_position();
      if(lexer.get_token() == RIGHT_PARENTHESES){ return ret;}
      else{
        error_occurred(right_position, "expected \")\"");
      }
    }
  }
  lexer.set_current_position(position);

  return node_sptr();
}

/**
 * variable_name := lower_identifier
 */
std::string Parser::variable_name(){
  position_t position = lexer.get_current_position();
  if(lexer.get_token() == LOWER_IDENTIFIER) return lexer.get_current_token_string();
  lexer.set_current_position(position);
  return std::string();
}


/**
 * variable := variable_name
 */
boost::shared_ptr<Variable> Parser::variable(){
  std::string name;
  // identifier
  position_t position = lexer.get_current_position();
  name = variable_name();
  if (name.find('_') != std::string::npos) {
    std::cout<<"ERROR: variable names containing under-scores are invalid in current HydLa ("<<name<<")\n";
	std::exit(1);
  }
  if(name != "") {
    position_t tmp_position = lexer.get_current_position();
    if(lexer.get_token () != LEFT_BOX_BRACKETS){
      lexer.set_current_position(tmp_position);
      if(name == "t") return boost::shared_ptr<Variable>(new SymbolicT());
      return boost::shared_ptr<Variable>(new Variable(name));
    }
  }
  lexer.set_current_position(position);
  return boost::shared_ptr<Variable>();
}

/// parameter := "p" "[" variable "," integer "," integer "]"
node_sptr Parser::parameter(){
  position_t position = lexer.get_current_position();
  lexer.get_token();
  // "p"
  if(lexer.get_current_token_string() == "p"){
    // "["
    position_t tmp_position = lexer.get_current_position();
    if(lexer.get_token() == LEFT_BOX_BRACKETS){
      tmp_position = lexer.get_current_position();
      boost::shared_ptr<Variable> var;
      int first,second;
      // variable
      if((var = variable())){
        tmp_position = lexer.get_current_position();
        // ","
        if(lexer.get_token() == COMMA){
          tmp_position = lexer.get_current_position();
          // integer
          if(lexer.get_token() == INTEGER){
            tmp_position = lexer.get_current_position();
            first = std::atoi(lexer.get_current_token_string().c_str());
            // ","
            if(lexer.get_token() == COMMA){
              tmp_position = lexer.get_current_position();
              // integer
              if(lexer.get_token() == INTEGER){
                second = std::atoi(lexer.get_current_token_string().c_str());
                tmp_position = lexer.get_current_position();
                // "]"
                if(lexer.get_token() == RIGHT_BOX_BRACKETS){
                  return boost::shared_ptr<Parameter>(new Parameter(var->get_name(),first,second));
                }else{
                  error_occurred(tmp_position, "expected \"]\"");
                }
              }else{
                error_occurred(tmp_position, "expected integer");
              }
            }else{
              error_occurred(tmp_position, "expected \",\"");
            }
          }else{
            error_occurred(tmp_position, "expected integer");
          }
        }else{
          error_occurred(tmp_position, "expected \",\"");
        }
      }else{
        error_occurred(tmp_position, "expected variable");
      }
    }else{
      error_occurred(tmp_position, "expected \"[\"");
    }
  }
  lexer.set_current_position(position);

  return node_sptr();
}

// number := [0-9]+ ("." [0-9]+)?
node_sptr Parser::number(){
  position_t position = lexer.get_current_position();
  // size_of_list
  node_sptr ret;
  if((ret = size_of_list())){ return ret;}
  Token token = lexer.get_token();
  if(token == NUMBER){
    std::string str = lexer.get_current_token_string();
    std::string num = "";
    std::string divided_by = "1";
    bool dot_appear = false;
    for(int i = 0; str.c_str()[i] != '\0'; i++){
      if(str.c_str()[i] == '.') dot_appear = true;
      else{
        num += str.c_str()[i];
        if(dot_appear)divided_by += "0";
      }
    }
    return boost::shared_ptr<Divide>(new Divide(boost::shared_ptr<Number>(new Number(num)), boost::shared_ptr<Number>(new Number(divided_by))));
  }
  if(token == INTEGER){ 
    return boost::shared_ptr<Number>(new Number(lexer.get_current_token_string()));
  }
  lexer.set_current_position(position);
  return node_sptr();
}

} // namespace parser
} // namespace hydla
