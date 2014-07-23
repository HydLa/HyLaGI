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

/**
 * support functions
 */
bool Parser::is_COMMA(Token token){ return token == COMMA; }
bool Parser::is_WEAKER(Token token){ return token == WEAKER; }
bool Parser::is_LOGICAL_OR(Token token){ return token == LOGICAL_OR || token == VERTICAL_BAR; }
bool Parser::is_LOGICAL_AND(Token token){ return token == LOGICAL_AND || token == AMPERSAND; }
bool Parser::is_COMPARE(Token token){ return token == LESS || token == LESS_EQUAL || token == GREATER || token == GREATER_EQUAL || token == EQUAL || token == NOT_EQUAL; }

/**
 * get list element
 * RET = NAME[num]
 * NAME[num] parsed by FUNCTION
 */
#define list_element(LIST_TYPE, RET, TYPE, FUNCTION, BOUND_VARS, ADDITIONAL){       \
  RET = TYPE();                                                          \
  boost::shared_ptr<Number> LIST_INDEX;                                  \
  position_t LEXER_POSITION = lexer.get_current_position();              \
  list_t LIST = LIST_TYPE("", BOUND_VARS);                                    \
  if(!LIST.empty()){                                                     \
    if(lexer.get_token() == LEFT_BOX_BRACKETS){                          \
      if((LIST_INDEX = non_variable_expression(BOUND_VARS))){            \
        if(lexer.get_token() == RIGHT_BOX_BRACKETS){                     \
          int INT_INDEX = (int)std::stof(LIST_INDEX->get_number());      \
          list_t::iterator LIST_IT = LIST.begin();                       \
          for(int LIST_I = 0; LIST_I < INT_INDEX-1; LIST_I++, LIST_IT++);\
          Parser ELEMENT_PARSER((*(LIST_IT))+ADDITIONAL);                \
          ELEMENT_PARSER.set_list(expression_list_map, program_list_map);                             \
          RET = ELEMENT_PARSER.FUNCTION;                                 \
          if(!ELEMENT_PARSER.parse_ended()){                             \
            RET = TYPE();                                                \
          }                                                              \
        }                                                                \
      }                                                                  \
    }                                                                    \
  }                                                                      \
  if(!RET) lexer.set_current_position(LEXER_POSITION);                   \
}
/**
 * make binary node (lhs operator operand operator operand ...)
 * lhs (operator operand)*
 */
#define ZERO_OR_MORE_OPERATOR_OPERAND(CLASS, LHS, IS_OPERATOR, IS_OPERAND) \
{                                                                          \
  position_t zero_position = lexer.get_current_position();         \
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
  if(!error_info.empty()){
    for(auto info : error_info){
      std::cout << "parse error : " << info.first.first+1 << " : ";
      std::cout << info.second << std::endl;
      std::cout << "    " << lexer.get_string(info.first.first) << std::endl;
      std::cout << "    ";
      for(int i = 0; i < info.first.second; i++) std::cout << " ";
      std::cout << "~" << std::endl;
    }
    // TODO : throw Error
    std::exit(1);
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

  // list_definition "."
  if(list_definition()){
    position_t tmp_position = lexer.get_current_position();
    if(lexer.get_token() == PERIOD){
      return node_sptr(new True());
    }
    lexer.set_current_position(tmp_position);
    error_occurred(lexer.get_current_position(), "expected \".\" after variable list definition");
    return node_sptr(new True());
  }
  lexer.set_current_position(position);

  // def_statement(constraint_def) "."
  boost::shared_ptr<ConstraintDefinition> cd;
  if((cd = constraint_def())){
    position_t tmp_position = lexer.get_current_position();
    if(lexer.get_token() == PERIOD){
      constraint_definitions.push_back(cd);
      return cd;
    }
    lexer.set_current_position(tmp_position);
    error_occurred(lexer.get_current_position(), "expected \".\" after constraint definition");
    return cd;
  }
  lexer.set_current_position(position);

  // def_statement(program_def) "."
  boost::shared_ptr<ProgramDefinition> pd;
  if((pd = program_def())){
    position_t tmp_position = lexer.get_current_position();
    if(lexer.get_token() == PERIOD){
      program_definitions.push_back(pd);
      return pd;
    }
    lexer.set_current_position(tmp_position);
    error_occurred(lexer.get_current_position(), "expected \".\" after program definition");
    return pd;
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


  return node_sptr();
}

/// program := program_priority ( "," program_priority )*
node_sptr Parser::program(){
  position_t position = lexer.get_current_position();
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
  position_t position = lexer.get_current_position();
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

/// constraint_def := constraint_callee "<=>" constraint
boost::shared_ptr<ConstraintDefinition> Parser::constraint_def(){
  position_t position = lexer.get_current_position();
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
  position_t position = lexer.get_current_position();
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

  std::string name;
  // program_list
  list_t p_list;
  if(!((p_list=program_list("",std::map<std::string,std::string>())).empty())){
    boost::shared_ptr<Parallel> parallel;
    node_sptr lhs,rhs;
    for(auto elem : p_list){
      Parser program_parser(elem+".");
      program_parser.set_list(expression_list_map, program_list_map);
      if((rhs = program_parser.program())){
        if(lhs){
          parallel = boost::shared_ptr<Parallel>(new Parallel(lhs,rhs));
          lhs = parallel;
        }else{
          lhs = rhs;
        }
      }else{
        lhs = node_sptr();
        break;
      }
    }
    if(lhs) return lhs;
  }
  lexer.set_current_position(position);

  // list_element
  std::map<std::string,std::string> null_map;
  list_element(program_list, ret,node_sptr,program(),null_map,".");
  if(ret) return ret;
  lexer.set_current_position(position);

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
    ZERO_OR_MORE_OPERATOR_OPERAND(LogicalOr, tmp_l, is_LOGICAL_OR, logical_and);
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
    ZERO_OR_MORE_OPERATOR_OPERAND(LogicalAnd, tmp_l, is_LOGICAL_AND, always);
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

#define UPDATE_COMPARE_NODE(NAME, FIRST, CLASS, LHS, RHS)           \
  case NAME:                                                        \
    LHS = boost::shared_ptr<CLASS>(new CLASS(LHS,RHS));             \
    break;

/// compare_expression := expression (("<"|"<="|">"|">="|"="|"!=") expression)+
node_sptr Parser::compare_expression(){
  node_sptr ret;
  node_sptr lhs;
  position_t position = lexer.get_current_position();
  // expression
  if((lhs = expression())){
    Token token = lexer.get_token();
    // ("<"|"<="|">"|">="|"="|"!=")
    if(is_COMPARE(token)){
      node_sptr rhs;
      do{
        // expression
        if((rhs = expression())){
          switch(token){
            UPDATE_COMPARE_NODE(LESS, first, Less, lhs, rhs)
            UPDATE_COMPARE_NODE(LESS_EQUAL, first, LessEqual, lhs, rhs)
            UPDATE_COMPARE_NODE(GREATER, first, Greater, lhs, rhs)
            UPDATE_COMPARE_NODE(GREATER_EQUAL, first, GreaterEqual, lhs, rhs)
            UPDATE_COMPARE_NODE(EQUAL, first, Equal, lhs, rhs)
            UPDATE_COMPARE_NODE(NOT_EQUAL, first, UnEqual, lhs, rhs)
            default: break;
          }
          if(!ret) ret = lhs;
          else ret = boost::shared_ptr<LogicalAnd>(new LogicalAnd(ret,lhs));
          lhs = rhs;
        }else break;
        position = lexer.get_current_position();
        token = lexer.get_token();
      }while(is_COMPARE(token));
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
    ZERO_OR_MORE_OPERATOR_OPERAND(LogicalOr, tmp_l, is_LOGICAL_OR, guard_term);
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
    ZERO_OR_MORE_OPERATOR_OPERAND(LogicalAnd, tmp_l, is_LOGICAL_AND, logical_not);
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
  std::string name;
  if((name = identifier()) != ""){
    if(name == "sum"){
      if(lexer.get_token() == LEFT_PARENTHESES){
        list_t tmp_list;
        std::map<std::string,std::string> null_map;
        if(!((tmp_list = expression_list("",null_map)).empty())){
          std::string content = "";
          for(auto l : tmp_list){
            if(content != "") content += "+";
            content += "(" + l + ")";
          }
          Parser sum_expression_parser(content);
          sum_expression_parser.set_list(expression_list_map, program_list_map);
          node_sptr sum_expression = sum_expression_parser.expression();
          if(sum_expression_parser.parse_ended()){
            if(lexer.get_token() == RIGHT_PARENTHESES){
              return sum_expression;
            }
          }
        }
      }
    }
  }
  lexer.set_current_position(position);
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
  std::map<std::string,std::string> null_map;
  list_element(expression_list, ret, node_sptr, expression(), null_map, ""); 
  if(ret) return ret;

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
  boost::shared_ptr<Variable> ret;
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
  if(token == VERTICAL_BAR){
    std::string name;
    boost::shared_ptr<Number> ret;
    list_t l;
    if(!((l=expression_list("",std::map<std::string,std::string>())).empty())){
      if(lexer.get_token() == VERTICAL_BAR){
        return boost::shared_ptr<Number>(new Number(std::to_string(l.size())));
      }
    }
    if(!((l=program_list("",std::map<std::string,std::string>())).empty())){
      if(lexer.get_token() == VERTICAL_BAR){
        return boost::shared_ptr<Number>(new Number(std::to_string(l.size())));
      }
    }
  }
  lexer.set_current_position(position);
  return node_sptr();
}

boost::shared_ptr<Number> Parser::non_variable_expression(std::map<std::string, std::string> bound_vars){
  double num;
  boost::shared_ptr<Number> ret;
  boost::shared_ptr<Number> tmp;
  position_t position = lexer.get_current_position();
  if((ret = non_variable_term(bound_vars))){
    position_t tmp_position = lexer.get_current_position();
    Token token = lexer.get_token();
    while(token == PLUS || token == MINUS){
      if((tmp = non_variable_term(bound_vars))){
        switch(token){
          case PLUS:
            num = std::stof(ret->get_number())+std::stof(tmp->get_number());
            break;
          case MINUS:
            num = std::stof(ret->get_number())-std::stof(tmp->get_number());
            break;
          default:
            break;
        }
        ret = boost::shared_ptr<Number>(new Number(std::to_string(num)));
      }else{
        lexer.set_current_position(tmp_position);
        break;
      }
      tmp_position = lexer.get_current_position();
      token = lexer.get_token();
    }
    lexer.set_current_position(tmp_position);
    return ret;
  } 
  lexer.set_current_position(position);
  return boost::shared_ptr<Number>();
}
boost::shared_ptr<Number> Parser::non_variable_term(std::map<std::string,std::string> bound_vars){
  double num;
  boost::shared_ptr<Number> ret;
  boost::shared_ptr<Number> tmp;
  position_t position = lexer.get_current_position();
  if((ret = non_variable_factor(bound_vars))){
    position_t tmp_position = lexer.get_current_position();
    Token token = lexer.get_token();
    while(token == MUL || token == DIVIDE){
      if((tmp = non_variable_factor(bound_vars))){
        switch(token){
          case MUL:
            num = std::stof(ret->get_number())*std::stof(tmp->get_number());
            break;
          case DIVIDE:
            num = std::stof(ret->get_number())/std::stof(tmp->get_number());
            break;
          default:
            break;
        }
        ret = boost::shared_ptr<Number>(new Number(std::to_string(num)));
      }else{
        lexer.set_current_position(tmp_position);
        break;
      }
      tmp_position = lexer.get_current_position();
      token = lexer.get_token();
    }
    lexer.set_current_position(tmp_position);
    return ret;
  }
  lexer.set_current_position(position);
  return boost::shared_ptr<Number>();
}

boost::shared_ptr<Number> Parser::non_variable_factor(std::map<std::string, std::string> bound_vars){
  position_t position = lexer.get_current_position();
  Token token = lexer.get_token();
  boost::shared_ptr<Number> ret;
  if(token == LEFT_PARENTHESES){
    if((ret = non_variable_expression(bound_vars))){
      if(token == RIGHT_PARENTHESES){
        return ret;
      }
    }
  }
  if(token == NUMBER || token == INTEGER){
    ret = boost::shared_ptr<Number>(new Number(lexer.get_current_token_string())); 
    return ret;
  }
  if(token == VERTICAL_BAR){
    std::string name;
    list_t l;
    if(!((l=expression_list("",std::map<std::string,std::string>())).empty())){
      if(lexer.get_token() == VERTICAL_BAR){
        return boost::shared_ptr<Number>(new Number(std::to_string(l.size())));
      }
    }
    if(!((l=program_list("",std::map<std::string,std::string>())).empty())){
      if(lexer.get_token() == VERTICAL_BAR){
        return boost::shared_ptr<Number>(new Number(std::to_string(l.size())));
      }
    }
  }
  if(token == IDENTIFIER || token == ALPHABET){
    std::string name = lexer.get_current_token_string();
    if(bound_vars.find(name) != bound_vars.end()){
      Lexer num_lexer(bound_vars[name]);
      token = num_lexer.get_token();
      if(token == NUMBER || token == INTEGER){
        if(num_lexer.get_token() == END_OF_FILE){
          return boost::shared_ptr<Number>(new Number(num_lexer.get_current_token_string())); 
        }
      }
    }
  }
  lexer.set_current_position(position);
  return boost::shared_ptr<Number>();
}

/**
 * bound_variable "in" list
 * bound_variable "=!=" bound_variable
 */
list_t Parser::list_conditions(
    std::map<std::string, std::string> bound_vars,
    std::string elements_of_list)
{
  list_t ret;
  position_t position = lexer.get_current_position();
  std::string name;

  boost::shared_ptr<Variable> var;
  if((var = variable(bound_vars))){
    if(lexer.get_token() == DIFFERENT_VARIABLE){
      std::string first_var = var->get_name();
      if((var = variable(bound_vars))){
        std::string second_var = var->get_name();
        if(bound_vars.find(first_var) != bound_vars.end()) first_var = bound_vars[first_var];
        if(bound_vars.find(second_var) != bound_vars.end()) second_var = bound_vars[second_var];
        if(first_var != second_var){
          Token token;
          token = lexer.get_token();
          if(token == COMMA){
            list_t tmp = list_conditions(bound_vars, elements_of_list);
            for(auto var : tmp) ret.insert(var);
          }else if(token == RIGHT_BRACES){
            ret.insert(replace_string_by_bound_variables(bound_vars, elements_of_list));
          }
          return ret;
        }else{
          Token token = lexer.get_token();
          if(token == COMMA){
            list_conditions(bound_vars, elements_of_list);
            return ret;
          }else if(token == RIGHT_BRACES){
            return ret;
          }
          if(token != COMMA && token != RIGHT_BRACES){
            error_occurred(lexer.get_current_position(), "expected \",\" or \"}\" after =!=");
            return ret;
          }
        }
      }
    }
  }
  lexer.set_current_position(position);

  if((name = identifier()) != ""){
    position_t tmp_position = lexer.get_current_position();
    lexer.get_token();
    if(lexer.get_current_token_string() == "in"){
      list_t tmp_list;
      if(!((tmp_list = list(ALL,"",bound_vars)).empty())){
        if(bound_vars.find(name) != bound_vars.end()){
          std::cout << "cannot decided " << name << " value" << std::endl;
          std::exit(1);
        }
        Token token;
        token = lexer.get_token();
        tmp_position = lexer.get_current_position();
        for(auto element : tmp_list){
          bound_vars[name] = replace_string_by_bound_variables(bound_vars, element);
          lexer.set_current_position(tmp_position);
          if(token == COMMA){
            list_t tmp = list_conditions(bound_vars, elements_of_list);
            for(auto var : tmp) ret.insert(var);
          }else if(token == RIGHT_BRACES){
            ret.insert(replace_string_by_bound_variables(bound_vars, elements_of_list));
          }else{
            error_occurred(lexer.get_current_position(), "expected \",\" or \"}\" after =!=");
            return ret;
          }
        }
        return ret;
      }else{
        std::cout << "expected list after \"in\"" << std::endl;
        std::exit(1);
      }
    }
  }
  lexer.set_current_position(position);

  return list_t();
}

std::string Parser::replace_string_by_bound_variables(
    std::map<std::string, std::string> bound_vars,
    std::string origin)
{
  std::string ret = "";
  Lexer replace_lexer(origin);
  while(replace_lexer.get_token() != END_OF_FILE){
    if(bound_vars.find(replace_lexer.get_current_token_string()) != bound_vars.end()){
      ret += bound_vars[replace_lexer.get_current_token_string()];
    }else{
      ret += replace_lexer.get_current_token_string();
    }
  }
  return ret;
}

/**
 * list_factor := identifier
 *              | "{" string ( "|" conditions )? "}"
 *              | expression".."expression
 *              | "[" expression "]"
 *              | "(" list ")"
 */
list_t Parser::list_factor(ListType type, std::string list_name, std::map<std::string, std::string> bound_vars){
  list_t ret;
  std::string name;
  position_t position = lexer.get_current_position();

  // identifier
  if((name = identifier()) != ""){
    // defined list
    if(expression_list_map.find(name) != expression_list_map.end() &&
        (type == EXPRESSION || type == ALL)){
      return expression_list_map[name];
    }
    if(program_list_map.find(name) != program_list_map.end() &&
        (type == PROGRAM || type == ALL)){
      return program_list_map[name];
    }
    // var num..var num
    int from,to;
    std::string head = "";
    for(int i = 0; i < name.size(); i++){
      Parser tmp_parser(name.substr(i));
      tmp_parser.set_list(expression_list_map, program_list_map);
      boost::shared_ptr<Number> num = tmp_parser.non_variable_expression(bound_vars);
      if(num){
        if(tmp_parser.parse_ended()){
          from = (int)std::stof(num->get_number());
          head = name.substr(0,i);
          break;
        }
      }
    }
    if(head != ""){
      if(lexer.get_token() == TWO_PERIOD){
        if((name = identifier()) != ""){
          Parser tmp_parser(name.substr(head.size()));
          tmp_parser.set_list(expression_list_map, program_list_map);
          boost::shared_ptr<Number> num = tmp_parser.non_variable_expression(bound_vars);
          if(num){
            if(tmp_parser.parse_ended()){
              to = (int)std::stof(num->get_number());
              for(int i = from; i <= to; i++){
                ret.insert(head+std::to_string(i));
              }
              return ret;
            }
          }
        }
      }
    }
  }
  lexer.set_current_position(position);


  // non_expression ".." non_variable_expression
  if(type == EXPRESSION || type == ALL){
    boost::shared_ptr<Number> num1, num2;
    if((num1 = non_variable_expression(bound_vars))){
      if(lexer.get_token() == TWO_PERIOD){
        if((num2 = non_variable_expression(bound_vars))){
          int from = (int)std::stof(num1->get_number());
          int to = (int)std::stof(num2->get_number());
          for(int i = from; i <= to; i++){
            ret.insert(std::to_string(i));
          }
          return ret;
        }
      }
    }
  }
  lexer.set_current_position(position);

  // "{" string ( "|" conditions )? "}"
  if(lexer.get_token() == LEFT_BRACES){
    position_t tmp_position = lexer.get_current_position();
    std::vector<std::string> contents_of_list;
    std::vector<position_t> positions;
    std::string content = "";
    Token token = lexer.get_token();
    while(token != RIGHT_BRACES){
      if(token == VERTICAL_BAR){
        positions.push_back(lexer.get_current_position());
        contents_of_list.push_back(content);
        content = "";
      }else{
        content += lexer.get_current_token_string();
      }
      token = lexer.get_token();
    }
    contents_of_list.push_back(content);
    positions.push_back(lexer.get_current_position());
    std::map<std::string, std::string> var_map;
    for(int i = 0; i < positions.size()-1; i++){
      content = "";
      for(int j = 0; j <= i; j++){
        if(content != "") content += "|";
        content += contents_of_list[j];
      }
      list_t tmp_list;
      lexer.set_current_position(positions[i]);
      if(!((tmp_list = list_conditions(var_map, content)).empty())) return tmp_list;
    }
    content = "";
    for(auto cont : contents_of_list){
      if(content != "") content += "|";
      content += cont;
    }
    lexer.set_current_position(positions.back());
    if(token == RIGHT_BRACES){
      std::string var_tmp = "";
      int p = 0;
      for(auto ch : content){
        if(ch == '(') p++;
        if(ch == ')') p--;
        if(ch != ',' || p > 0){
          var_tmp += ch;
        }else{
          ret.insert(var_tmp);
          var_tmp = "";
        }
      }
      ret.insert(var_tmp);
      return ret;
    }
  }
  lexer.set_current_position(position);

  // "(" list ")"
  if(lexer.get_token() == LEFT_PARENTHESES){
    if(!((ret = list(type, list_name, bound_vars)).empty())){
      if(lexer.get_token() == RIGHT_PARENTHESES){
        return ret;
      }
    }
  }

  return list_t();
}

list_t Parser::list(ListType type, std::string list_name, std::map<std::string, std::string> bound_vars){
  list_t ret;
  list_t tmp;
  position_t position = lexer.get_current_position();
  if(!((ret = list_term(type, list_name, bound_vars)).empty())){
    position_t tmp_position = lexer.get_current_position();
    Token token = lexer.get_token();
    while(lexer.get_current_token_string() == "or"){
      if(!((tmp = list_term(type, list_name,bound_vars)).empty())){
        for(auto element : tmp) ret.insert(element);
      }else{
        lexer.set_current_position(tmp_position);
        break;
      }
      tmp_position = lexer.get_current_position();
      token = lexer.get_token();
    }
    lexer.set_current_position(tmp_position);
    return ret;
  } 
  lexer.set_current_position(position);
  return list_t();
}

list_t Parser::list_term(ListType type, std::string list_name, std::map<std::string, std::string> bound_vars){
  list_t ret;
  list_t tmp;
  position_t position = lexer.get_current_position();
  if(!((ret = list_factor(type, list_name, bound_vars)).empty())){
    position_t tmp_position = lexer.get_current_position();
    Token token = lexer.get_token();
    while(lexer.get_current_token_string() == "and"){
      if(!((tmp = list_factor(type, list_name,bound_vars)).empty())){
        for(list_t::iterator element = ret.begin(); element != ret.end(); ){
          bool in_tmp = false;
          for(auto tmp_element : tmp){
            if((*element) == tmp_element){
              in_tmp = true;
              break;
            }
          }
          if(!in_tmp) element = ret.erase(element);
          else element++;
        }
      }else{
        lexer.set_current_position(tmp_position);
        break;
      }
      tmp_position = lexer.get_current_position();
      token = lexer.get_token();
    }
    lexer.set_current_position(tmp_position);
    return ret;
  } 
  lexer.set_current_position(position);
  return list_t();
}

bool Parser::list_definition(){
  std::string name;
  position_t position = lexer.get_current_position();
  if((name = identifier()) != ""){
    Token token = lexer.get_token();
    if(token == DEFINITION){
      if(expression_list_map.find(name) != expression_list_map.end()){
        std::cout << "redefinition of " << name << std::endl;
        std::exit(1);
      }
      list_t tmp_list;
      tmp_list = list(EXPRESSION,name,std::map<std::string, std::string>());
      if(!tmp_list.empty()){
        expression_list_map[name] = tmp_list;
        return true;
      }
    }
    if(token == EQUIVALENT){
      if(program_list_map.find(name) != program_list_map.end()){
        std::cout << "redefinition of " << name << std::endl;
        std::exit(1);
      }
      list_t tmp_list;
      tmp_list = list(PROGRAM, name,std::map<std::string, std::string>());
      if(!tmp_list.empty()){
        program_list_map[name] = tmp_list;
        return true;
      }
    }
  }
  lexer.set_current_position(position);
  return false;
}

list_t Parser::expression_list(std::string list_name, std::map<std::string, std::string> bound_vars){
  return list(EXPRESSION,list_name,bound_vars);
}
list_t Parser::program_list(std::string list_name, std::map<std::string, std::string> bound_vars){
  return list(PROGRAM,list_name,bound_vars);
}

} // namespace parser
} // namespace hydla
