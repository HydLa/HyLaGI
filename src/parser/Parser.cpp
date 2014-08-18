#include <vector>
#include <cstdlib>

#include "Lexer.h"
#include "Parser.h"

namespace hydla{
  namespace parser{

using namespace symbolic_expression;

Parser::Parser(std::string str):lexer(str){}
Parser::Parser(std::istream& stream):lexer(stream){}
Parser::~Parser(){}

bool Parser::parse_ended(){
  position_t position = lexer.get_current_position();
  if(lexer.get_token() == END_OF_FILE) return true;
  lexer.set_current_position(position);
  return false;
}

node_sptr Parser::parse(node_sptr& an, DefinitionContainer<ConstraintDefinition> &cd, DefinitionContainer<ProgramDefinition> &pd){
  parse();
  if(!error_info.empty()){
    std::cout << "error occured while parsing" << std::endl;
    for(auto info : error_info){
      std::cout << "parse error - " << info.first.first+1 << " : ";
      std::cout << info.second << std::endl;
      std::cout << "    " << lexer.get_string(info.first.first) << std::endl;
      std::cout << "    ";
      for(int i = 0; i < info.first.second; i++) std::cout << " ";
      std::cout << "~" << std::endl;
    }
    // TODO : throw Error
    std::exit(1);
  }
  an = assertion_node;
  for(auto constraint_definition : constraint_definitions){
    cd.add_definition(constraint_definition);
  }
  for(auto program_definition : program_definitions){
    pd.add_definition(program_definition);
  }
  return parsed_program;
}

node_sptr Parser::parse(){
  position_t position = lexer.get_current_position();
  while(!parse_ended()){
    hydla_program();
    if(position == lexer.get_current_position()){
      lexer.get_token();
    }
    position = lexer.get_current_position();
  }
  return node_sptr();
}

/// hydla_program := statements
node_sptr Parser::hydla_program(){
  return statements();
}

/// statements := statement*
node_sptr Parser::statements(){
  node_sptr ret;
  std::vector<node_sptr> nodes;
  while((ret = statement())){}
  return node_sptr();
}

/**
 * statement := assert "."
 *            | def_statement "."
 *            | program "."
 */
node_sptr Parser::statement(){
  position_t position = lexer.get_current_position();
  node_sptr ret;
  if(lexer.get_token() == END_OF_FILE){
    return node_sptr();
  }
  lexer.set_current_position(position);

  // assert "."
  if((ret = assertion())){
    position_t tmp_position = lexer.get_current_position();
    if(lexer.get_token() == PERIOD){
      assertion_node = ret;
      return ret;
    }
    lexer.set_current_position(tmp_position);
    error_occurred(lexer.get_current_position(), "expected \".\" after assert statement");
    return ret; 
  }
  lexer.set_current_position(position);

  // def_statement(constraint_def) "."
  boost::shared_ptr<ConstraintDefinition> cd;
  // constraint_callee
  if((cd = constraint_callee())){
    // "<=>"
    if(lexer.get_token() == EQUIVALENT){
      position_t tmp_position = lexer.get_current_position();
      node_sptr tmp;
      // constraint
      if((tmp = constraint())){
        cd->set_child(tmp);
        tmp_position = lexer.get_current_position();
        if(lexer.get_token() == PERIOD){
          constraint_definitions.push_back(cd);
          return cd;
        }
        error_occurred(tmp_position, "expected \".\" after constraint definition");
        return node_sptr();
      }
      error_occurred(tmp_position, "expected constraint after \"<=>\"");
      return node_sptr();
    }
  }
  lexer.set_current_position(position);

  // def_statement(program_def) "."
  boost::shared_ptr<ProgramDefinition> pd;
  // constraint_callee
  if((pd = program_callee())){
    // "{"
    if(lexer.get_token() == LEFT_BRACES){
      position_t tmp_position = lexer.get_current_position();
      node_sptr tmp;
      // program
      if((tmp = program())){
        pd->set_child(tmp);
        tmp_position = lexer.get_current_position();
        if(lexer.get_token() == RIGHT_BRACES){
          tmp_position = lexer.get_current_position();
          if(lexer.get_token() == PERIOD){
            program_definitions.push_back(pd);
            return pd;
          }
          error_occurred(tmp_position, "expected \".\" after program definition");
          return node_sptr();
        }
        error_occurred(tmp_position, "expected \"}\" after defined program");
        return node_sptr();
      }
      error_occurred(tmp_position, "expected program after \"{\"");
      return node_sptr();
    }
  }
  lexer.set_current_position(position);
  
  // program "."
  if((ret = program())){
    position_t tmp_position = lexer.get_current_position();
    if(lexer.get_token() == PERIOD){
      if(!parsed_program) parsed_program = ret;
      else parsed_program = boost::shared_ptr<Parallel>(new Parallel(parsed_program,ret));
      return ret;
    }
    lexer.set_current_position(tmp_position);
    error_occurred(lexer.get_current_position(), "expected \".\" after program");
    return ret;
  }
  lexer.set_current_position(position);

  error_occurred(lexer.get_current_position(), "parse error");
  return node_sptr();
}

/// program := program_priority ( "," program_priority )*
node_sptr Parser::program(){
  position_t position = lexer.get_current_position();
  node_sptr tmp_l;
  // program_priority
  if((tmp_l = program_priority())){
    // ( "," program_priority )*
    position_t zero_position = lexer.get_current_position();
    node_sptr rhs;
    while(lexer.get_token() == COMMA){
      if((rhs = program_priority())){
        boost::shared_ptr<Parallel> tmp(new Parallel());
        tmp->set_lhs(tmp_l);
        tmp->set_rhs(rhs);
        tmp_l = tmp;
      }else break;
      zero_position = lexer.get_current_position();
    }
    lexer.set_current_position(zero_position);

    return tmp_l;
  }

  return node_sptr();
}

/// program_priority := program_factor ( "<<" program_factor )*
node_sptr Parser::program_priority(){
  position_t position = lexer.get_current_position();
  node_sptr tmp_l;
  // program_factor
  if((tmp_l = program_factor())){
    // ( "<<" program_factor )*
    position_t zero_position = lexer.get_current_position();
    node_sptr rhs;
    while(lexer.get_token() == WEAKER){
      if((rhs = program_factor())){
        boost::shared_ptr<Weaker> tmp(new Weaker());
        tmp->set_lhs(tmp_l);
        tmp->set_rhs(rhs);
        tmp_l = tmp;
      }else break;
      zero_position = lexer.get_current_position();
    }
    lexer.set_current_position(zero_position);

    return tmp_l;
  }
  return node_sptr();
}

/// program_factor := module
node_sptr Parser::program_factor(){
  return module();
}

/// constraint_callee := constraint_name formal_args
boost::shared_ptr<ConstraintDefinition> Parser::constraint_callee(){
  boost::shared_ptr<ConstraintDefinition> ret(new ConstraintDefinition());
  std::vector<std::string> args;
  std::string name;
  // constraint_name
  if((name = constraint_name()) != ""){
    ret->set_name(name);
    // formal_args
    args = formal_args();
    if(!args.empty()){
      for(auto arg : args) ret->add_bound_variable(arg);
    }
    return ret;
  }

  return boost::shared_ptr<ConstraintDefinition>();
}

/// constraint_caller := constraint_name actual_args
boost::shared_ptr<ConstraintCaller> Parser::constraint_caller(){
  boost::shared_ptr<ConstraintCaller> ret(new ConstraintCaller());
  std::vector<node_sptr> args;
  std::string name;
  // constraint_name
  if((name = constraint_name()) != ""){
    ret->set_name(name);
    // actual_args
    args = actual_args();
    if(!args.empty()){
      for(auto arg : args) ret->add_actual_arg(arg);
    }
    return ret;
  }

  return boost::shared_ptr<ConstraintCaller>();
}

/// program_callee := program_name formal_args
boost::shared_ptr<ProgramDefinition> Parser::program_callee(){
  boost::shared_ptr<ProgramDefinition> ret(new ProgramDefinition());
  std::vector<std::string> args;
  std::string name;
  // program_name
  if((name = program_name()) != ""){
    ret->set_name(name);
    // formal_args
    args = formal_args();
    if(!args.empty()){
      for(auto arg : args) ret->add_bound_variable(arg);
    }
    return ret;
  }

  return boost::shared_ptr<ProgramDefinition>(); 
}

/// program_caller := program_name actual_args
boost::shared_ptr<ProgramCaller> Parser::program_caller(){
  boost::shared_ptr<ProgramCaller> ret(new ProgramCaller());
  std::vector<node_sptr> args;
  std::string name;
  // program_name
  if((name = program_name()) != ""){
    ret->set_name(name);
    // actual_args
    args = actual_args();
    if(!args.empty()){
      for(auto arg : args){
        ret->add_actual_arg(arg);
      }
    }
    return ret;
  }

  return boost::shared_ptr<ProgramCaller>();
}

node_sptr Parser::parenthesis_program(){
  node_sptr ret;
  position_t position = lexer.get_current_position();
  if(lexer.get_token() == LEFT_PARENTHESES){
    if((ret = program())){
      if(lexer.get_token() == RIGHT_PARENTHESES){
        position_t tmp_position = lexer.get_current_position(); 
        Token token;
        // after "(" program ")" must be ")"* ("," ||  "<<" || "}" || ".")
        while((token = lexer.get_token()) == RIGHT_PARENTHESES);
        if(token == COMMA || token == WEAKER || token == RIGHT_BRACES || token == PERIOD){
          // but don't skip these token
          lexer.set_current_position(tmp_position);
          return ret;
        }
      }
    }
  }
  lexer.set_current_position(position);
  return node_sptr();
}

/**
 * module := program_caller
 *         | "(" program ")"
 *         | constraint
 */
node_sptr Parser::module(){
  node_sptr ret;
  position_t position = lexer.get_current_position();

  // program_caller
  if((ret = program_caller())){
    position_t tmp_position = lexer.get_current_position(); 
    Token token;
    // after program_caller must be ")"* ("," ||  "<<" || "}" || ".")
    while((token = lexer.get_token()) == RIGHT_PARENTHESES);
    if(token == COMMA || token == WEAKER || token == RIGHT_BRACES || token == PERIOD){
      // but don't skip these token
      lexer.set_current_position(tmp_position);
      return ret;
    }
  }
  lexer.set_current_position(position);

  // "(" program ")"
  if((ret = parenthesis_program())){ return ret; }

  // constraint
  if((ret = constraint())){ return ret;}

  return node_sptr();
}

/// constraint := logical_or
boost::shared_ptr<Constraint> Parser::constraint(){
  node_sptr ret;
  if((ret = logical_or())) return boost::shared_ptr<Constraint>(new Constraint(ret));
  return boost::shared_ptr<Constraint>();
}

/// logical_or := logical_and ( ("|" | "\/") logical_and )*
node_sptr Parser::logical_or(){ 
  position_t position = lexer.get_current_position();
  node_sptr tmp_l;
  // logical_and
  if((tmp_l = logical_and())){
    // ( ("|" | "\/") logical_and )*
    position_t zero_position = lexer.get_current_position();
    node_sptr rhs;
    Token token = lexer.get_token();
    while(token == VERTICAL_BAR || token == LOGICAL_OR){
      if((rhs = logical_and())){
        boost::shared_ptr<LogicalOr> tmp(new LogicalOr());
        tmp->set_lhs(tmp_l);
        tmp->set_rhs(rhs);
        tmp_l = tmp;
      }else break;
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
  position_t position = lexer.get_current_position();
  node_sptr tmp_l;
  // always
  if((tmp_l = always())){
    // ( ("&" | "/\") always )*
    position_t zero_position = lexer.get_current_position();
    node_sptr rhs;
    Token token = lexer.get_token();
    while(token == AMPERSAND || token == LOGICAL_AND){
      if((rhs = always())){
        boost::shared_ptr<LogicalAnd> tmp(new LogicalAnd());
        tmp->set_lhs(tmp_l);
        tmp->set_rhs(rhs);
        tmp_l = tmp;
      }else break;
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
  boost::shared_ptr<Always> always(new Always());
  position_t position = lexer.get_current_position();
  // "[]"
  if(lexer.get_token() == ALWAYS){
    // conditional_constraint
    if((ret = conditional_constraint())){
      always->set_child(ret);
      return always;
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
    boost::shared_ptr<Ask> ask(new Ask());
    node_sptr c;
    // "=>"
    if(lexer.get_token() == IMPLIES){
      // constraint
      if((c = constraint())){
        ask->set_guard(ret);
        ask->set_child(c);
        return ask;
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
      if(lexer.get_token() == RIGHT_PARENTHESES){
        return ret;
      }
    }
  }
  lexer.set_current_position(position);

  return node_sptr();
}

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
        }else break;
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
 * atomic_constraint := compare_expression
 *                    | command
 *                    | tautology
 */
node_sptr Parser::atomic_constraint(){
  node_sptr ret;
  position_t position = lexer.get_current_position();
  // command
  if((ret = command())){ return ret;}
  // tautology
  if((ret = tautology())){ return ret;}
  // compare_expression
  if((ret = compare_expression())){ return ret; }

  return node_sptr();
}

/// guard := guard_term (("|" | "\/") guard_term)*
node_sptr Parser::guard(){
  position_t position = lexer.get_current_position();
  node_sptr tmp_l;
  // guard_term
  if((tmp_l = guard_term())){
    // (("|" | "\.") guard_term)*
    position_t zero_position = lexer.get_current_position();
    node_sptr rhs;
    Token token = lexer.get_token();
    while(token == VERTICAL_BAR || token == LOGICAL_OR){
      if((rhs = guard_term())){
        boost::shared_ptr<LogicalOr> tmp(new LogicalOr());
        tmp->set_lhs(tmp_l);
        tmp->set_rhs(rhs);
        tmp_l = tmp;
      }else break;
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
  position_t position = lexer.get_current_position();
  node_sptr tmp_l;
  // logical_not
  if((tmp_l = logical_not())){
    // (( "&" | "/\") logical_not)*
    position_t zero_position = lexer.get_current_position();
    node_sptr rhs;
    Token token = lexer.get_token();
    while(token == AMPERSAND || token == LOGICAL_AND){
      if((rhs = logical_not())){
        boost::shared_ptr<LogicalAnd> tmp(new LogicalAnd());
        tmp->set_lhs(tmp_l);
        tmp->set_rhs(rhs);
        tmp_l = tmp;
      }else break;
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
      return boost::shared_ptr<Not>(new Not(ret));
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
      if(lexer.get_token() == RIGHT_PARENTHESES){
        return ret;
      }
    }
  }
  lexer.set_current_position(position);

  return node_sptr();
}

/// command := "@" identifier "(" (identifier ("," identifier)* )? ")"
node_sptr Parser::command(){
  // TODO: implement
  return node_sptr();
}

/// expression := arithmetic
node_sptr Parser::expression(){ 
  position_t position = lexer.get_current_position();
  return arithmetic(); 
}

/// arithmetic := arith_term (("+"|"-") arith_term)*
node_sptr Parser::arithmetic(){
  node_sptr ret;
  position_t position = lexer.get_current_position();
  // arith_term
  if((ret = arith_term())){
    position_t position = lexer.get_current_position();
    Token token = lexer.get_token();
    // ("+"|"-")
    while(token == PLUS || token == MINUS){
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
      }else break;
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
      }else break;
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
      node_sptr tmp;
      // power
      if((tmp = power())){ return boost::shared_ptr<Power>(new Power(ret,tmp));}
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
 * factor := (fanction | unsupported_function) "(" (expression ("," expression)* )? ")"
 *         | PI
 *         | E
 *         | parameter
 *         | variable
 *         | system_variable
 *         | number
 *         | "(" expression ")"
 */
node_sptr Parser::factor(){
  node_sptr ret;
  position_t position = lexer.get_current_position();
  std::string name;
  // Pi | E
  if((name = identifier()) != ""){
    if(lexer.get_current_token_string() == "PI"){
      return boost::shared_ptr<Pi>(new Pi());
    }
    if(lexer.get_current_token_string() == "E"){
      return boost::shared_ptr<E>(new E());
    }
  }
  lexer.set_current_position(position);

  boost::shared_ptr<ArbitraryNode> func;
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
      }
    }
  }
  lexer.set_current_position(position);
  // parameter
  if((ret = parameter())){ return ret;}
  // variable
  if((ret = variable(std::map<std::string,std::string>()))){ return ret;}
  // system_variable
  if((ret = system_variable())){ return ret;}
  // number
  if((ret = number())){ return ret;}
  // "(" expression ")"
  if(lexer.get_token() == LEFT_PARENTHESES){
    // expression
    if((ret = expression())){
      // ")"
      if(lexer.get_token() == RIGHT_PARENTHESES){ return ret;}
    }
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
    std::string name = identifier();
    // t
    if(name == "t"){ return boost::shared_ptr<SymbolicT>(new SymbolicT()); }
    // timer
    if(name == "timer"){ return boost::shared_ptr<SVtimer>(new SVtimer()); }
  }
  lexer.set_current_position(position);

  return node_sptr();
}

// unsupported_function := " " " [a-z,A-Z]+ " " "
boost::shared_ptr<UnsupportedFunction> Parser::unsupported_function(){
  position_t position = lexer.get_current_position();
  // " " "
  if(lexer.get_token() == DOUBLE_QUOTATION){
    // " [a-z, A-Z]+ "
    if(lexer.get_token() == ALPHABET){
      std::string name = lexer.get_current_token_string();
      // " " " 
      if(lexer.get_token() == DOUBLE_QUOTATION){
        return boost::shared_ptr<UnsupportedFunction>(new UnsupportedFunction(name));
      }
    }
  }
  lexer.set_current_position(position);

  return boost::shared_ptr<UnsupportedFunction>(); 
}

/// function := [a-z,A-Z]+
boost::shared_ptr<Function> Parser::function(){
  position_t position = lexer.get_current_position();
  // [a-z,A-Z]+
  if(lexer.get_token() == ALPHABET){ return boost::shared_ptr<Function>(new Function(lexer.get_current_token_string()));}
  lexer.set_current_position(position);

  return boost::shared_ptr<Function>();
}

/**
 * variable := identifier
 */
boost::shared_ptr<Variable> Parser::variable(std::map<std::string, std::string> bound_vars){
  std::string name;
  // identifier
  position_t position = lexer.get_current_position();
  if((name = identifier()) != ""){
    position_t tmp_position = lexer.get_current_position();
    if(lexer.get_token () != LEFT_BOX_BRACKETS){
      lexer.set_current_position(tmp_position);
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
    if(lexer.get_token() == LEFT_BOX_BRACKETS){
      boost::shared_ptr<Variable> var;
      int first,second;
      // variable
      if((var = variable(std::map<std::string,std::string>()))){
        // ","
        if(lexer.get_token() == COMMA){
          // integer
          if(lexer.get_token() == INTEGER){
            first = std::atoi(lexer.get_current_token_string().c_str());
            // ","
            if(lexer.get_token() == COMMA){
              // integer
              if(lexer.get_token() == INTEGER){
                second = std::atoi(lexer.get_current_token_string().c_str());
                // "]"
                if(lexer.get_token() == RIGHT_BOX_BRACKETS){
                  return boost::shared_ptr<Parameter>(new Parameter(var->get_name(),first,second));
                }
              }
            }
          }
        }
      }
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
    if(lexer.get_token() == LEFT_PARENTHESES){
      // guard
      if((ret = guard())){
        // ")"
        if(lexer.get_token() == RIGHT_PARENTHESES){
          return ret;
        }
      }
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
    if(lexer.get_token() == ALPHABET){
      // "TRUE"
      if(lexer.get_current_token_string() == "TRUE"){ return boost::shared_ptr<True>(new True());}
      if(lexer.get_current_token_string() == "FALSE"){ return boost::shared_ptr<False>(new False());}
    }
  }
  lexer.set_current_position(position);
  
  return node_sptr();
}

/// bound_variable := identifier
boost::shared_ptr<Variable> Parser::bound_variable(){
  std::string name;
  // identifier
  if((name=identifier()) != ""){ return boost::shared_ptr<Variable>(new Variable(name));}
  return boost::shared_ptr<Variable>();
}

/// program_name := identifier
std::string Parser::program_name(){
  std::string name;
  // identifier
  if((name=identifier()) != ""){ return name; }
  return std::string(); 
}

/// constraint_name := identifier
std::string Parser::constraint_name(){
  std::string name;
  // identifier
  if((name=identifier()) != ""){ return name; }
  return std::string(); 
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
    }
    if(lexer.get_token() == RIGHT_PARENTHESES){ return ret; }
  }
  lexer.set_current_position(position);

  return std::vector<node_sptr>();
}

/**
 * program_list_element := program_list "[" expression "]"
 */
node_sptr Parser::program_list_element(){
  position_t position = lexer.get_current_position();
  node_sptr list;
  if((list = program_list())){
    if(lexer.get_token() == LEFT_BOX_BRACKETS){
      node_sptr expr;
      if((expr = expression())){
        if(lexer.get_token() == RIGHT_BOX_BRACKETS){
          boost::shared_ptr<ProgramListElement> e(new ProgramListElement());
          e->set_lhs(list);
          e->set_rhs(expr);
          return e;
        }
      }
    }
  }
  lexer.set_current_position(position);
  return node_sptr();
}
/**
 * expression_list_element := expression_list "[" expression "]"
 */
node_sptr Parser::expression_list_element(){
  position_t position = lexer.get_current_position();
  node_sptr list;
  if((list = expression_list())){
    if(lexer.get_token() == LEFT_BOX_BRACKETS){
      node_sptr expr;
      if((expr = expression())){
        if(lexer.get_token() == RIGHT_BOX_BRACKETS){
          boost::shared_ptr<ExpressionListElement> e(new ExpressionListElement());
          e->set_lhs(list);
          e->set_rhs(expr);
          return e;
        }
      }
    }
  }
  lexer.set_current_position(position);
  return node_sptr();
}

/**
 * conditional_program_list := "{" program "|" ( list_condition ("," list_condition)* )?
 */
node_sptr Parser::conditional_program_list(){
  position_t position = lexer.get_current_position();
  node_sptr tmp;
  if(lexer.get_token() == LEFT_BRACES){
    if((tmp = program())){
      if(lexer.get_token() == VERTICAL_BAR){
        boost::shared_ptr<ConditionalProgramList> ret(new ConditionalProgramList());
        ret->set_program(tmp);
        if((tmp = list_condition())){
          ret->add_argument(tmp);
          position_t tmp_position;
          while(lexer.get_token() == COMMA){
            if((tmp = list_condition())){
              ret->add_argument(tmp);
            }else break;
            tmp_position = lexer.get_current_position();
          }
          lexer.set_current_position(tmp_position);
          if(lexer.get_token() == RIGHT_BRACES){
            return ret;
          }
        }
      }
    }
  }
  lexer.set_current_position(position);
  return node_sptr();
}
/**
 * program_list := program_list_term ("or" program_list_term)*
 */
node_sptr Parser::program_list(){
  position_t position = lexer.get_current_position();
  node_sptr ret;
  if((ret = program_list_term())){
    node_sptr list;
    position_t tmp_position = lexer.get_current_position();
    while(lexer.get_token() == ALPHABET && lexer.get_current_token_string() == "or"){
      if((list = program_list_term())){
        boost::shared_ptr<Union> uni(new Union(ret,list));
        ret = uni;
      }else break;
      tmp_position = lexer.get_current_position();
    }
    lexer.set_current_position(tmp_position);
    return ret;
  }
  lexer.set_current_position(position);
  return node_sptr();
}
/**
 * program_list_term := program_list_factor ("and" program_list_factor )*
 */
node_sptr Parser::program_list_term(){
  position_t position = lexer.get_current_position();
  node_sptr ret;
  if((ret = program_list_factor())){
    node_sptr list;
    position_t tmp_position = lexer.get_current_position();
    while(lexer.get_token() == ALPHABET && lexer.get_current_token_string() == "and"){
      if((list = program_list_factor())){
        boost::shared_ptr<Intersection> in(new Intersection(ret,list));
        ret = in;
      }else break;
      tmp_position = lexer.get_current_position();
    }
    lexer.set_current_position(tmp_position);
    return ret;
  }
  lexer.set_current_position(position);
  return node_sptr();
}
/**
 * program_list_factor := conditional_program_list
 *                      | "{"program ("," program)* "}"
 *                      | identifier number ".." identifier number
 */
node_sptr Parser::program_list_factor(){
  node_sptr ret;
  // conditional_program_list
  position_t position = lexer.get_current_position();
  // "{" program ("," program)* "}"
  if((ret = conditional_program_list())) return ret;
  if(lexer.get_token() == LEFT_BRACES){
    boost::shared_ptr<ProgramList> list(new ProgramList());
    if((ret = program())){
      list->add_argument(ret);
      position_t tmp_position = lexer.get_current_position();
      while(lexer.get_token() == COMMA){
        if((ret = program())){
          list->add_argument(ret);
        }else break;
        tmp_position = lexer.get_current_position();
      }
      lexer.set_current_position(position);
      if(lexer.get_token() == RIGHT_BRACES){
        return list;
      }
    }
  }
  lexer.set_current_position(position);
  if(lexer.get_token() == LEFT_PARENTHESES){
    if((ret = program_list())){
      if(lexer.get_token() == RIGHT_PARENTHESES){
        return ret;
      }
    }
  }
  lexer.set_current_position(position);
  // identifier number ".." identifier number
  std::string str;
  if((str = identifier()) != ""){
    if(lexer.get_token() == TWO_PERIOD){
      std::string str2;
      if((str2 = identifier()) != ""){
        int num1;
        int num2;
        for(num1 = str.length()-1; num1 >= 0; num1--){
          if(str[num1] < '0' || '9' < str[num1]) break;
        }
        for(num2 = str2.length()-1; num2 >= 0; num2--){
          if(str2[num2] < '0' || '9' < str2[num2]) break;
        }
        if(str.substr(0,num1+1) == str2.substr(0,num2+1)){
          boost::shared_ptr<Range> range(
            new Range(boost::shared_ptr<Number>(new Number(str.substr(num1+1))),
                      boost::shared_ptr<Number>(new Number(str2.substr(num2+1)))));
            range->set_string(str.substr(0,num1+1));
            return range;
        }
      }
    }
  }
  lexer.set_current_position(position);
  if((str = identifier()) != ""){
    boost::shared_ptr<ProgramListCaller> caller(new ProgramListCaller());
    caller->set_name(str);
    return caller;
  }
  lexer.set_current_position(position);
  return node_sptr();
}
/**
 * conditional_expression_list := "{" expression "|" ( list_condition ("," list_condition)* )? "}"
 */
node_sptr Parser::conditional_expression_list(){
  position_t position = lexer.get_current_position();
  node_sptr tmp;
  if(lexer.get_token() == LEFT_BRACES){
    if((tmp = expression())){
      if(lexer.get_token() == VERTICAL_BAR){
        boost::shared_ptr<ConditionalExpressionList> ret(new ConditionalExpressionList());
        ret->set_expression(tmp);
        if((tmp = list_condition())){
          ret->add_argument(tmp);
          position_t tmp_position;
          while(lexer.get_token() == COMMA){
            if((tmp = list_condition())){
              ret->add_argument(tmp);
            }else break;
            tmp_position = lexer.get_current_position();
          }
          lexer.set_current_position(tmp_position);
          if(lexer.get_token() == RIGHT_BRACES){
            return ret;
          }
        }
      }
    }
  }
  lexer.set_current_position(position);
  return node_sptr();
}
/**
 * expression_list := expression_list_term ("or" expression_list_term )*
 */
node_sptr Parser::expression_list(){
  position_t position = lexer.get_current_position();
  node_sptr ret;
  if((ret = expression_list_term())){
    node_sptr list;
    position_t tmp_position = lexer.get_current_position();
    while(lexer.get_token() == ALPHABET && lexer.get_current_token_string() == "or"){
      if((list = expression_list_term())){
        boost::shared_ptr<Union> uni(new Union(ret,list));
        ret = uni;
      }else break;
      tmp_position = lexer.get_current_position();
    }
    lexer.set_current_position(tmp_position);
    return ret;
  }
  lexer.set_current_position(position);
  return node_sptr();
}
/**
 * program_list_term := expression_list_factor ("and" expression_list_factor )*
 */
node_sptr Parser::expression_list_term(){
  position_t position = lexer.get_current_position();
  node_sptr ret;
  if((ret = expression_list_factor())){
    node_sptr list;
    position_t tmp_position = lexer.get_current_position();
    while(lexer.get_token() == ALPHABET && lexer.get_current_token_string() == "and"){
      if((list = expression_list_factor())){
        boost::shared_ptr<Intersection> in(new Intersection(ret,list));
        ret = in;
      }else break;
      tmp_position = lexer.get_current_position();
    }
    lexer.set_current_position(tmp_position);
    return ret;
  }
  lexer.set_current_position(position);
  return node_sptr();
}
/**
 * expression_list_factor := conditional_expression_list
 *                         | "{"expression ("," expression)* "}"
 *                         | identifier number ".." identifier number
 *                         | number ".." number
 *                         | "(" expression_list ")"
 *                         | identifier
 */
node_sptr Parser::expression_list_factor(){
  node_sptr ret;
  // conditional_expression_list
  position_t position = lexer.get_current_position();
  // "{" expression ("," expression)* "}"
  if((ret = conditional_expression_list())) return ret;
  if(lexer.get_token() == LEFT_BRACES){
    boost::shared_ptr<ExpressionList> list(new ExpressionList());
    if((ret = expression())){
      list->add_argument(ret);
      position_t tmp_position = lexer.get_current_position();
      while(lexer.get_token() == COMMA){
        if((ret = expression())){
          list->add_argument(ret);
        }else break;
        tmp_position = lexer.get_current_position();
      }
      lexer.set_current_position(position);
      if(lexer.get_token() == RIGHT_BRACES){
        return list;
      }
    }
  }
  lexer.set_current_position(position);
  // number ".." number
  if((ret = number())){
    if(lexer.get_token() == TWO_PERIOD){
      node_sptr num2;
      if((num2 = number())){
        return boost::shared_ptr<Range>(new Range(ret,num2));
      }
    }
  }
  lexer.set_current_position(position);
  if(lexer.get_token() == LEFT_PARENTHESES){
    if((ret = expression_list())){
      if(lexer.get_token() == RIGHT_PARENTHESES){
        return ret;
      }
    }
  }
  lexer.set_current_position(position);
  // identifier number ".." identifier number
  std::string str;
  if((str = identifier()) != ""){
    if(lexer.get_token() == TWO_PERIOD){
      std::string str2;
      if((str2 = identifier()) != ""){
        int num1;
        int num2;
        for(num1 = str.length()-1; num1 >= 0; num1--){
          if(str[num1] < '0' || '9' < str[num1]) break;
        }
        for(num2 = str2.length()-1; num2 >= 0; num2--){
          if(str2[num2] < '0' || '9' < str2[num2]) break;
        }
        if(str.substr(0,num1+1) == str2.substr(0,num2+1)){
          boost::shared_ptr<Range> range(
            new Range(boost::shared_ptr<Number>(new Number(str.substr(num1+1))),
                      boost::shared_ptr<Number>(new Number(str2.substr(num2+1)))));
            range->set_string(str.substr(0,num1+1));
            return range;
        }
      }
    }
  }
  lexer.set_current_position(position);
  if((str = identifier()) != ""){
    boost::shared_ptr<ExpressionListCaller> caller(new ExpressionListCaller());
    caller->set_name(str);
    return caller;
  }
  lexer.set_current_position(position);
  return node_sptr();
}
/**
 * list_condition := bound_variable "in" (expression_list | program_list)
 *                 | (expression_list_element | program_list_element | bound_variable) "=!=" (expression_list_element | program_list_element | bound_variable)
 */
node_sptr Parser::list_condition(){
  node_sptr lhs;
  position_t position = lexer.get_current_position();
  // bound_variable "in" (expression_list | program_list)
  if((lhs = bound_variable())){
    if(lexer.get_token() == ALPHABET && lexer.get_current_token_string() == "in"){
      node_sptr list;
      if((list = expression_list())){
        return boost::shared_ptr<EachElement>(new EachElement(lhs,list));
      }
      if((list = program_list())){
        return boost::shared_ptr<EachElement>(new EachElement(lhs,list));
      }
    }
  }
  lexer.set_current_position(position);
// (expression_list_element | program_list_element | bound_variable) "=!=" (expression_list_element | program_list_element | bound_variable)
  if((lhs = expression_list_element()) ||
     (lhs = program_list_element()) ||
     (lhs = bound_variable())){
    if(lexer.get_token() == DIFFERENT_VARIABLE){
      node_sptr rhs;
      if((rhs = expression_list_element()) ||
         (rhs = program_list_element()) ||
         (rhs = bound_variable())){
        return boost::shared_ptr<DifferentVariable>(new DifferentVariable(lhs,rhs)); 
      }
    }
  }
  lexer.set_current_position(position);
  return node_sptr();
}

/// formal_args := ( "(" bound_variable ("," bound_variable)* ")" )?
std::vector<std::string> Parser::formal_args(){
  std::vector<std::string> ret;
  position_t position = lexer.get_current_position();
  // "("
  if(lexer.get_token() == LEFT_PARENTHESES){
    boost::shared_ptr<Variable> var;
    // bound_variable
    if((var = bound_variable())){
      ret.push_back(var->get_name());
      position_t tmp_position = lexer.get_current_position();
      // ("," bound_variable)*
      while(lexer.get_token() == COMMA){
        if((var = bound_variable())){
          ret.push_back(var->get_name());
        }else{
          lexer.set_current_position(position);
          return std::vector<std::string>();
        }
        tmp_position = lexer.get_current_position();
      }
      lexer.set_current_position(tmp_position);
    }
    if(lexer.get_token() == RIGHT_PARENTHESES){ return ret; }
  }
  lexer.set_current_position(position);

  return std::vector<std::string>(); 
}

// identifier := [a-z,A-Z,"_"] [a-z,A-Z,"_",0-9]*
std::string Parser::identifier(){
  position_t position = lexer.get_current_position();
  Token token = lexer.get_token();
  if(token == IDENTIFIER || token == ALPHABET){ return lexer.get_current_token_string();}
  lexer.set_current_position(position);
  return std::string();
}

// number := [0-9]+ ("." [0-9]+)?
node_sptr Parser::number(){
  position_t position = lexer.get_current_position();
  Token token = lexer.get_token();
  if(token == IDENTIFIER || token == ALPHABET){
    node_sptr ret;
    std::string name = lexer.get_current_token_string();
  }

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
