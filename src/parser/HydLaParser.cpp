#include <vector>
#include <cstdlib>
#include <exception>

#include "Lexer.h"
#include "Parser.h"

namespace hydla{
  namespace parser{

using namespace symbolic_expression;

Parser::Parser():lexer(){}
Parser::Parser(std::string str):lexer(str){}
Parser::Parser(std::vector<std::string> str):lexer(str){}
Parser::Parser(std::istream& stream):lexer(stream){}
Parser::Parser(std::string str, std::istream& stream):lexer(str,stream){}
Parser::Parser(std::string file, std::vector<std::string> str):lexer(file, str){}
Parser::~Parser(){}

bool Parser::parse_ended(){
  position_t position = lexer.get_current_position();
  if(lexer.get_token() == END_OF_FILE) return true;
  lexer.set_current_position(position);
  return false;
}

node_sptr Parser::is_defined(boost::shared_ptr<Definition> definition){
  node_sptr ret;
  std::string name = definition->get_name();
  int size = definition->bound_variable_size();
  IS_DEFINED_AS(name,size,tmp_constraint_definitions,ret);
  IS_DEFINED_AS(name,size,tmp_program_list_definitions,ret);
  IS_DEFINED_AS(name,size,tmp_expression_list_definitions,ret);
  IS_DEFINED_AS(name,size,tmp_program_definitions,ret);
  return ret; 
}

node_sptr Parser::parse(node_sptr& an, 
    DefinitionContainer<ConstraintDefinition> &cd, 
    DefinitionContainer<ProgramDefinition> &pd,
    DefinitionContainer<ExpressionListDefinition> &eld,
    DefinitionContainer<ProgramListDefinition> &pld
){
  parse();
  if(error_info.empty())
  {
    list_type_check();
    second_parse = true;
    lexer.set_current_position(position_t(0,0));
    error_tmp.clear();
    an.reset();
    parsed_program.reset();
    for(auto d : program_definitions) tmp_program_definitions.push_back(d);
    for(auto d : constraint_definitions) tmp_constraint_definitions.push_back(d);
    for(auto d : program_list_definitions) tmp_program_list_definitions.push_back(d);
    for(auto d : expression_list_definitions) tmp_expression_list_definitions.push_back(d);
    program_definitions.clear();
    constraint_definitions.clear();
    program_list_definitions.clear();
    expression_list_definitions.clear();
    parse();
  }
  if(!error_info.empty()){
    std::string error_message = "\n";
    for(auto info : error_info){
      error_message += "Parse error : " + info.second + "\n";
      error_message += lexer.get_error_position(info.first);
    }
    throw std::runtime_error(error_message);
  }
  an = assertion_node;
  for(auto constraint_definition : constraint_definitions){
    cd.add_definition(constraint_definition);
  }
  for(auto program_definition : program_definitions){
    pd.add_definition(program_definition);
  }
  for(auto expression_list_definition : expression_list_definitions){
    eld.add_definition(expression_list_definition);
  }
  for(auto program_list_definition : program_list_definitions){
    pld.add_definition(program_list_definition);
  }
  return parsed_program;
}

node_sptr Parser::parse(){
  while(!parse_ended()){
    hydla_program();
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
  if(!parse_ended()){
    assert(error_tmp.size() > 0);
    error_info_t deepest = error_info_t(position_t(0,0),"");
    for(auto m : error_tmp){
      if(deepest.first.first < m.first.first ||
        (deepest.first.first == m.first.first && 
         deepest.first.second <= m.first.second)){
        deepest = m;
      }
    }
    error_info.push_back(deepest);
    error_tmp.clear();

    lexer.set_current_position(deepest.first);
    while(!parse_ended()){
      position_t position = lexer.get_current_position();
      if((constraint_callee()) ||
         (program_callee()) ||
         (program_list_callee()) ||
         (expression_list_callee())){
        Token token = lexer.get_token();
        if(token == LEFT_BRACES ||
           token == EQUIVALENT ||
           token == DEFINITION){
          lexer.set_current_position(position);
          break;
        }
      }
      if(lexer.get_token() == PERIOD){
        break;
      }
    }
  }
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
    error_occurred(tmp_position, "expected \".\" after assert statement");
  }
  lexer.set_current_position(position);

  // def_statement(constraint_def) "."
  boost::shared_ptr<ConstraintDefinition> cd;
  // constraint_callee
  if((cd = constraint_callee())){
    // "<=>"
    position_t tmp_position = lexer.get_current_position();
    if(lexer.get_token() == EQUIVALENT){
      if((is_defined(cd)) && !second_parse){
        error_occurred(tmp_position, "redefinition of " + std::to_string(cd->bound_variable_size()) + " args definition \"" + cd->get_name() + "\"");
      }else{
        tmp_position = lexer.get_current_position();
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
  }
  lexer.set_current_position(position);

  // def_statement(program_def) "."
  boost::shared_ptr<ProgramDefinition> pd;
  // constraint_callee
  if((pd = program_callee())){
    // "{"
    position_t tmp_position = lexer.get_current_position();
    if(lexer.get_token() == LEFT_BRACES){
      if((is_defined(pd)) && !second_parse){
        error_occurred(lexer.get_current_position(), "redefinition of " + std::to_string(pd->bound_variable_size()) + " args definition \"" + pd->get_name() + "\"");
      }else{
        tmp_position = lexer.get_current_position();
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
  }
  lexer.set_current_position(position);

  // program_list_callee ":=" program_list
  boost::shared_ptr<ProgramListDefinition> pld;
  if((pld = program_list_callee())){
    position_t tmp_position = lexer.get_current_position();
    if(lexer.get_token() == DEFINITION){
      if((is_defined(pld)) && !second_parse){
        error_occurred(tmp_position, "redefinition of " + std::to_string(pld->bound_variable_size()) + " args definition \"" + pld->get_name() + "\"");
      }else{
        node_sptr tmp;
        tmp_position = lexer.get_current_position();
        if((tmp = program_list())){
          pld->set_child(tmp);
          tmp_position = lexer.get_current_position();
          if(lexer.get_token() == PERIOD){
            program_list_definitions.push_back(pld);
            return pld;
          }
          error_occurred(tmp_position, "expected \".\" after program list definition");
        }else{
          error_occurred(tmp_position, "expected list after \";=\"");
        }
      }
    }
  }
  lexer.set_current_position(position);

  // expression_list_callee ":=" expression_list
  boost::shared_ptr<ExpressionListDefinition> eld;
  if((eld = expression_list_callee())){
    position_t tmp_position = lexer.get_current_position();
    if(lexer.get_token() == DEFINITION){
      if((is_defined(eld)) && !second_parse){
        error_occurred(tmp_position, "redefinition of " + std::to_string(eld->bound_variable_size()) + " args definition \"" + eld->get_name() + "\"");
      }else{
        node_sptr tmp;
        tmp_position = lexer.get_current_position();
        if((tmp = expression_list())){
          eld->set_child(tmp);
          tmp_position = lexer.get_current_position();
          if(lexer.get_token() == PERIOD){
            expression_list_definitions.push_back(eld);
            return eld;
          }
          error_occurred(tmp_position, "expected \".\" after expression list definition");
        }else{
          error_occurred(tmp_position, "expected list after \":=\"");
        }
      }
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
  }
  lexer.set_current_position(position);

  error_occurred(lexer.get_current_position(), "expected definition statement, assertion statement or program");
  return node_sptr();
}

/// program := program_priority ( "," program_priority )*
node_sptr Parser::program(){
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
      }else{
        error_occurred(lexer.get_current_position(), "expected program after \",\"");
        break;
      }
      zero_position = lexer.get_current_position();
    }
    lexer.set_current_position(zero_position);
    return tmp_l;
  }
  return node_sptr();
}

/// program_priority := program_factor ( "<<" program_factor )*
node_sptr Parser::program_priority(){
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
      }else{
        error_occurred(lexer.get_current_position(), "expected program after \"<<\"");
        break;
      }
      zero_position = lexer.get_current_position();
    }
    lexer.set_current_position(zero_position);
    return tmp_l;
  }
  return node_sptr();
}

/**
 * program_factor := module
 */
node_sptr Parser::program_factor(){
  return module();
}

/// parenthesis_program := "(" program ")"
node_sptr Parser::parenthesis_program(){
  node_sptr ret;
  position_t position = lexer.get_current_position();
  if(lexer.get_token() == LEFT_PARENTHESES){
    if((ret = program())){
      position_t right_position = lexer.get_current_position();
      if(lexer.get_token() == RIGHT_PARENTHESES){
        position_t tmp_position = lexer.get_current_position(); 
        Token token;
        // after "(" program ")" must be ")"* ("," ||  "<<" || "}" || ".")
        while((token = lexer.get_token()) == RIGHT_PARENTHESES);
        if(token == COMMA || token == WEAKER || token == RIGHT_BRACES || token == PERIOD || token == VERTICAL_BAR){
          // but don't skip these token
          lexer.set_current_position(tmp_position);
          return ret;
        }else{
          error_occurred(tmp_position, "\",\", \"<<\", \"}\", \"|\" and \".\" must be after \")\"");
        }
      }else{
        error_occurred(right_position, "expected \")\"");
      }
    }
  }
  lexer.set_current_position(position);
  return node_sptr();
}

/**
 * module := program_caller
 *         | "(" program ")"
 *         | program_list_element
 *         | program_list
 *         | constraint
 */
node_sptr Parser::module(){
  node_sptr ret;
  position_t position = lexer.get_current_position();

  // constraint
  if((ret = constraint())){ return ret;}

  // program_caller
  if((ret = program_caller())){
    position_t tmp_position = lexer.get_current_position(); 
    Token token;
    // after program_caller must be ")"* ("," ||  "<<" || "}" || "." || "|")
    while((token = lexer.get_token()) == RIGHT_PARENTHESES);
    if(token == COMMA || token == WEAKER || token == RIGHT_BRACES || token == PERIOD || token == VERTICAL_BAR){
      // but don't skip these token
      lexer.set_current_position(tmp_position);
      return ret;
    }else{
      error_occurred(tmp_position, "\",\", \"<<\", \"}\" and \".\" must be after program_caller");
    }
  }
  lexer.set_current_position(position);

  // "(" program ")"
  if((ret = parenthesis_program())){ return ret; }

  // program_lists
  if((ret = program_list_element())){ return ret; }
  if((ret = program_list())){ return ret; }

  return node_sptr();
}

/// program_callee := name formal_args
boost::shared_ptr<ProgramDefinition> Parser::program_callee(){
  position_t position = lexer.get_current_position();
  boost::shared_ptr<ProgramDefinition> ret(new ProgramDefinition());
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
  return boost::shared_ptr<ProgramDefinition>(); 
}

/// program_caller := name actual_args
boost::shared_ptr<ProgramCaller> Parser::program_caller(){
  boost::shared_ptr<ProgramCaller> ret(new ProgramCaller());
  position_t position = lexer.get_current_position();
  std::vector<node_sptr> args;
  std::string name;
  // name
  if((name = definition_name()) != ""){
    ret->set_name(name);
    // actual_args
    args = actual_args();
    if(!args.empty()){
      for(auto arg : args){
        ret->add_actual_arg(arg);
      }
    }
    if(second_parse)
    {
      node_sptr defined;
      IS_DEFINED_AS(name,args.size(),tmp_program_definitions,defined);
      if((defined)) return ret;
      else if(in_conditional_program_list_){
        defined.reset();
        IS_DEFINED_AS(name,args.size(),tmp_constraint_definitions,defined);
        if(!(defined)){ 
          local_program_caller_.top().push_back(ret);
          return ret;
        }
      }
      error_occurred(lexer.get_current_position(), "undefined program - " + std::to_string(args.size()) + " args program \"" + name + "\"");
    }
    else return ret;
  }
  lexer.set_current_position(position);
  return boost::shared_ptr<ProgramCaller>();
}


/// definition_name := upper_identifier
std::string Parser::definition_name(){
  position_t position = lexer.get_current_position();
  if(lexer.get_token() == UPPER_IDENTIFIER) return lexer.get_current_token_string();
  lexer.set_current_position(position);
  return std::string(); 
}


} // namespace parser
} // namespace hydla
