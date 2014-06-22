#include <vector>
#include <cstdlib>

#include "Lexer.h"
#include "Parser.h"

namespace hydla{
  namespace parser{

using namespace symbolic_expression;

Parser::Parser(std::string file_name):lexer(file_name){}
Parser::Parser(std::istream& stream):lexer(stream){}
Parser::~Parser(){}

/**
 * support functions
 */
bool Parser::is_COMMA(Token token){ return token == COMMA; }
bool Parser::is_WEAKER(Token token){ return token == WEAKER; }
bool Parser::is_LOGICAL_OR(Token token){ return token == LOGICAL_OR || token == VERTICAL_BAR; }
bool Parser::is_LOGICAL_AND(Token token){ return token == LOGICAL_AND || token == AMPERSAND; }
bool Parser::is_COMPARE(Token token){ return token == LESS || token == LESS_EQUAL || token == GREATER || token == GREATER_EQUAL || token == EQUAL || token == NOT_EQUAL; }

/**
 * make binary node (lhs operator operand operator operand ...)
 * lhs (operator operand)*
 */
#define ZERO_OR_MORE_OPERATOR_OPERAND(CLASS, LHS, IS_OPERATOR, IS_OPERAND) \
{                                                                          \
  std::pair<int,int> zero_position = lexer.get_current_position();         \
  node_sptr rhs;                                                           \
  while(IS_OPERATOR(lexer.get_token())){                                   \
    if((rhs = IS_OPERAND())){                                              \
      boost::shared_ptr<CLASS> zero_tmp(new CLASS());                      \
      zero_tmp->set_lhs(LHS);                                              \
      zero_tmp->set_rhs(rhs);                                              \
      LHS = zero_tmp;                                                      \
    }else break;                                                           \
    zero_position = lexer.get_current_position();                          \
  }                                                                        \
  lexer.set_current_position(zero_position);                               \
}

node_sptr Parser::parse(node_sptr an, DefinitionContainer<ConstraintDefinition> &cd, DefinitionContainer<ProgramDefinition> &pd){
  parse();
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
  return hydla_program();
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
  std::pair<int,int> position = lexer.get_current_position();
  node_sptr ret;
  if(lexer.get_token() == END_OF_FILE){
    return node_sptr();
  }
  lexer.set_current_position(position);

  // assert "."
  if((ret = assertion())){
    if(lexer.get_token() == PERIOD){
      assertion_node = ret;
      return ret;
    }
  }
  lexer.set_current_position(position);

  // def_statement(constraint_def) "."
  boost::shared_ptr<ConstraintDefinition> cd;
  if((cd = constraint_def())){
    if(lexer.get_token() == PERIOD){
      constraint_definitions.push_back(cd);
      return cd;
    }
  }
  lexer.set_current_position(position);

  // def_statement(program_def) "."
  boost::shared_ptr<ProgramDefinition> pd;
  if((pd = program_def())){
    if(lexer.get_token() == PERIOD){
      program_definitions.push_back(pd);
      return pd;
    }
  }
  lexer.set_current_position(position);

  // program "."
  if((ret = program())){
    if(lexer.get_token() == PERIOD){
      parsed_program = ret;
      return ret;
    }
  }
  lexer.set_current_position(position);

  std::exit(1);
  return node_sptr();
}

/// program := program_priority ( "," program_priority )*
node_sptr Parser::program(){
  std::pair<int,int> position = lexer.get_current_position();
  node_sptr tmp_l;
  // program_priority
  if((tmp_l = program_priority())){
    // ( "," program_priority )*
    ZERO_OR_MORE_OPERATOR_OPERAND(Parallel, tmp_l, is_COMMA, program_priority);
    return tmp_l;
  }

  return node_sptr();
}

/// program_priority := program_factor ( "<<" program_factor )*
node_sptr Parser::program_priority(){
  std::pair<int,int> position = lexer.get_current_position();
  node_sptr tmp_l;
  // program_factor
  if((tmp_l = program_factor())){
    // ( "<<" program_factor )*
    ZERO_OR_MORE_OPERATOR_OPERAND(Weaker, tmp_l, is_WEAKER, program_factor);
    return tmp_l;
  }
  return node_sptr();
}

/// program_factor := module
node_sptr Parser::program_factor(){
  return module();
}

/**
 * def_statement := constraint_def
 *                | program_def
 */
node_sptr Parser::def_statement(){
  node_sptr ret;
  // contraint_def
  if((ret = constraint_def())) return ret;
  // program_def
  if((ret = program_def())) return ret;
  return node_sptr();
}

/// constraint_def := constraint_callee "<=>" constraint
boost::shared_ptr<ConstraintDefinition> Parser::constraint_def(){
  std::pair<int,int> position = lexer.get_current_position();
  boost::shared_ptr<ConstraintDefinition> ret;
  node_sptr tmp;
  // constraint_callee
  if((ret = constraint_callee())){
    // "<=>"
    if(lexer.get_token() == EQUIVALENT){
      // constraint
      if((tmp = constraint())){
        ret->set_child(tmp);
        return ret;
      }
    }
  }
  lexer.set_current_position(position);

  return boost::shared_ptr<ConstraintDefinition>();
}

/// program_def := program_callee "{" program "}"
boost::shared_ptr<ProgramDefinition> Parser::program_def(){
  std::pair<int,int> position = lexer.get_current_position();
  boost::shared_ptr<ProgramDefinition> ret;
  node_sptr tmp;
  // program_callee
  if((ret = program_callee())){
    // "{"
    if(lexer.get_token() == LEFT_BRACES){
      // program
      if((tmp = program())){
        // "}"
        if(lexer.get_token() == RIGHT_BRACES){
          ret->set_child(tmp);
          return ret;
        }
      }
    }
  }
  lexer.set_current_position(position);

  return boost::shared_ptr<ProgramDefinition>();
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
      for(auto arg : args) ret->add_actual_arg(arg);
    }
    return ret;
  }

  return boost::shared_ptr<ProgramCaller>();
}

/**
 * module := program_caller
 *         | "(" program ")"
 *         | constraint
 */
node_sptr Parser::module(){
  node_sptr ret;
  std::pair<int,int> position = lexer.get_current_position();
  // program_caller
  if((ret = program_caller())){
    std::pair<int,int> tmp_position = lexer.get_current_position(); 
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
  if(lexer.get_token() == LEFT_PARENTHESES){
    if((ret = program())){
      if(lexer.get_token() == RIGHT_PARENTHESES){
        std::pair<int,int> tmp_position = lexer.get_current_position(); 
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
  std::pair<int,int> position = lexer.get_current_position();
  node_sptr tmp_l;
  // logical_and
  if((tmp_l = logical_and())){
    // ( ("|" | "\/") logical_and )*
    ZERO_OR_MORE_OPERATOR_OPERAND(LogicalOr, tmp_l, is_LOGICAL_OR, logical_and);
    return tmp_l;
  }

  return node_sptr();
}

/// logical_and := always ( ("&" | "/\") always )*
node_sptr Parser::logical_and(){
  std::pair<int,int> position = lexer.get_current_position();
  node_sptr tmp_l;
  // always
  if((tmp_l = always())){
    // ( ("&" | "/\") always )*
    ZERO_OR_MORE_OPERATOR_OPERAND(LogicalAnd, tmp_l, is_LOGICAL_AND, always);
    return tmp_l;
  }

  return node_sptr();
}

/// always := "[]"? conditional_constraint
node_sptr Parser::always(){
  node_sptr ret;
  boost::shared_ptr<Always> always(new Always());
  std::pair<int,int> position = lexer.get_current_position();
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
  std::pair<int,int> position = lexer.get_current_position();

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

#define UPDATE_COMPARE_NODE(NAME, CLASS, LHS, RHS)        \
  case NAME:                                              \
    LHS = boost::shared_ptr<CLASS>(new CLASS(LHS,RHS));   \
    break;

/// compare_expression := expression (("<"|"<="|">"|">="|"="|"!=") expression)+
node_sptr Parser::compare_expression(){
  node_sptr ret;
  bool first = true;
  std::pair<int,int> position = lexer.get_current_position();
  // expression
  if((ret = expression())){
    Token token = lexer.get_token();
    // ("<"|"<="|">"|">="|"="|"!=")
    if(is_COMPARE(token)){
      node_sptr tmp;
      do{
        // expression
        if((tmp = expression())){
          switch(token){
            UPDATE_COMPARE_NODE(LESS, Less, ret, tmp)
            UPDATE_COMPARE_NODE(LESS_EQUAL, LessEqual, ret, tmp)
            UPDATE_COMPARE_NODE(GREATER, Greater, ret, tmp)
            UPDATE_COMPARE_NODE(GREATER_EQUAL, GreaterEqual, ret, tmp)
            UPDATE_COMPARE_NODE(EQUAL, Equal, ret, tmp)
            UPDATE_COMPARE_NODE(NOT_EQUAL, UnEqual, ret, tmp)
            default: break;
          }
          first = false;
        }else break;
        position = lexer.get_current_position();
        token = lexer.get_token();
      }while(is_COMPARE(token));
      lexer.set_current_position(position);
      if(!first){return ret;}
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
  std::pair<int,int> position = lexer.get_current_position();
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
  std::pair<int,int> position = lexer.get_current_position();
  node_sptr tmp_l;
  // guard_term
  if((tmp_l = guard_term())){
    // (("|" | "\.") guard_term)*
    ZERO_OR_MORE_OPERATOR_OPERAND(LogicalOr, tmp_l, is_LOGICAL_OR, guard_term);
    return tmp_l;
  }
  return node_sptr();
}

/// guard_term := logical_not (( "&" | "/\") logical_not)*
node_sptr Parser::guard_term(){
  std::pair<int,int> position = lexer.get_current_position();
  node_sptr tmp_l;
  // logical_not
  if((tmp_l = logical_not())){
    // (( "&" | "/\") logical_not)*
    ZERO_OR_MORE_OPERATOR_OPERAND(LogicalAnd, tmp_l, is_LOGICAL_AND, logical_not);
    return tmp_l;
  }

  return node_sptr();
}

/// logical_not := "!"? comparison
node_sptr Parser::logical_not(){
  node_sptr ret;
  std::pair<int,int> position = lexer.get_current_position();
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
  std::pair<int,int> position = lexer.get_current_position();
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
  return arithmetic(); 
}

/// arithmetic := arith_term (("+"|"-") arith_term)*
node_sptr Parser::arithmetic(){
  node_sptr ret;
  // arith_term
  if((ret = arith_term())){
    std::pair<int,int> position = lexer.get_current_position();
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
    std::pair<int,int> position = lexer.get_current_position();
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
  std::pair<int,int> position = lexer.get_current_position();
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
    std::pair<int,int> position = lexer.get_current_position();
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
    std::pair<int,int> position = lexer.get_current_position();
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
    std::pair<int,int> position = lexer.get_current_position();
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
  std::pair<int,int> position = lexer.get_current_position();
  // Pi | E
  if(lexer.get_token() == ALPHABET){
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
        std::pair<int,int> tmp_position = lexer.get_current_position();
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
  if((ret = variable())){ return ret;}
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
  std::pair<int,int> position = lexer.get_current_position();
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
  std::pair<int,int> position = lexer.get_current_position();
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
  std::pair<int,int> position = lexer.get_current_position();
  // [a-z,A-Z]+
  if(lexer.get_token() == ALPHABET){ return boost::shared_ptr<Function>(new Function(lexer.get_current_token_string()));}
  lexer.set_current_position(position);

  return boost::shared_ptr<Function>();
}

/// variable := identifier
boost::shared_ptr<Variable> Parser::variable(){
  std::string name;
  // identifier
  if((name = identifier()) != ""){ return boost::shared_ptr<Variable>(new Variable(name));}
  return boost::shared_ptr<Variable>();
}

/// parameter := "p" "[" variable "," integer "," integer "]"
node_sptr Parser::parameter(){
  std::pair<int,int> position = lexer.get_current_position();
  lexer.get_token();
  // "p"
  if(lexer.get_current_token_string() == "p"){
    // "["
    if(lexer.get_token() == LEFT_BOX_BRACKETS){
      boost::shared_ptr<Variable> var;
      int first,second;
      // variable
      if((var = variable())){
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
  std::pair<int,int> position = lexer.get_current_position();
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
  std::pair<int,int> position = lexer.get_current_position();
  // "$"
  if(lexer.get_token() == SYSTEM){
    if(lexer.get_token() == ALPHABET){
      // "TRUE"
      if(lexer.get_current_token_string() == "TRUE"){ return boost::shared_ptr<True>(new True());}
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
  std::pair<int,int> position = lexer.get_current_position();
  // "("
  if(lexer.get_token() == LEFT_PARENTHESES){
    node_sptr var;
    // expression
    if((var = expression())){
      ret.push_back(var);
      std::pair<int,int> tmp_position = lexer.get_current_position();
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

/// formal_args := ( "(" bound_variable ("," bound_variable)* ")" )?
std::vector<std::string> Parser::formal_args(){
  std::vector<std::string> ret;
  std::pair<int,int> position = lexer.get_current_position();
  // "("
  if(lexer.get_token() == LEFT_PARENTHESES){
    boost::shared_ptr<Variable> var;
    // bound_variable
    if((var = bound_variable())){
      ret.push_back(var->get_name());
      std::pair<int,int> tmp_position = lexer.get_current_position();
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
  std::pair<int,int> position = lexer.get_current_position();
  Token token = lexer.get_token();
  if(token == IDENTIFIER || token == ALPHABET){ return lexer.get_current_token_string();}
  lexer.set_current_position(position);
  return std::string();
}

// number := [0-9]+ ("." [0-9]+)?
node_sptr Parser::number(){
  std::pair<int,int> position = lexer.get_current_position();
  Token token = lexer.get_token();
  if(token == NUMBER || token == INTEGER){ return boost::shared_ptr<Number>(new Number(lexer.get_current_token_string()));}
  lexer.set_current_position(position);
  return node_sptr();
}

} // namespace parser
} // namespace hydla
