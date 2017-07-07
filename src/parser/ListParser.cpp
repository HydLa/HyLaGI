#include <vector>
#include <cstdlib>
#include <exception>

#include "Lexer.h"
#include "Parser.h"

namespace hydla{
  namespace parser{

using namespace symbolic_expression;

void Parser::list_type_check(){
  bool finish = false;
  while(!finish){
    finish = true;
    for(auto i = program_list_definitions.begin(); i != program_list_definitions.end();)
    {
      bool is_expression_list = false;
      boost::shared_ptr<ProgramListCaller> def = boost::dynamic_pointer_cast<ProgramListCaller>((*i)->get_child());
      if(def)
      {
        for(auto ed : expression_list_definitions)
        {
          if(def->get_name() == ed->get_name() && def->actual_arg_size() == ed->bound_variable_size())
          {
            boost::shared_ptr<ExpressionListDefinition> eld(new ExpressionListDefinition()); 
            eld->set_name((*i)->get_name());
            for(auto var_it = (*i)->bound_variable_begin(); var_it != (*i)->bound_variable_end(); var_it++)
            {
              eld->add_bound_variable(*var_it);
            }
            boost::shared_ptr<ExpressionListCaller> elc(new ExpressionListCaller());
            elc->set_name(def->get_name());
            for(auto var_it = def->actual_arg_begin(); var_it != def->actual_arg_end(); var_it++)
            {
              elc->add_actual_arg(*var_it);
            }
            eld->set_child(elc);
            i = program_list_definitions.erase(i);
            expression_list_definitions.push_back(eld);
            is_expression_list = true;
            finish = false;
            break;
          }
        }
      }
      if(!is_expression_list) i++;
    }
  }
}

/// expression_list_callee := name
boost::shared_ptr<ExpressionListDefinition> Parser::expression_list_callee(){
  position_t position = lexer.get_current_position();
  boost::shared_ptr<ExpressionListDefinition> ret(new ExpressionListDefinition());
  std::vector<std::string> args;
  std::string name;
  // name
  if((name = definition_name()) != ""){
    ret->set_name(name);
    // formal_args
    return ret; 
  }
  lexer.set_current_position(position);
  return boost::shared_ptr<ExpressionListDefinition>();
}

/// program_list_callee := name
boost::shared_ptr<ProgramListDefinition> Parser::program_list_callee(){
  position_t position = lexer.get_current_position();
  boost::shared_ptr<ProgramListDefinition> ret(new ProgramListDefinition());
  std::vector<std::string> args;
  std::string name;
  // name
  if((name = definition_name()) != ""){
    ret->set_name(name);
    return ret;
  }
  lexer.set_current_position(position);
  return boost::shared_ptr<ProgramListDefinition>();
}

/**
 * sum_of_list := "sum" "(" expression_list ")"
 */
node_sptr Parser::sum_of_list(){
  boost::shared_ptr<SumOfList> ret(new SumOfList());
  position_t position = lexer.get_current_position();
  if(lexer.get_token() == LOWER_IDENTIFIER &&
     lexer.get_current_token_string() == "sum"){
    if(lexer.get_token() == LEFT_PARENTHESES){
      node_sptr tmp;
      if((tmp = expression_list())){
        ret->set_child(tmp);
        position_t tmp_position = lexer.get_current_position();
        if(lexer.get_token() == RIGHT_PARENTHESES){
          return ret;
        }else{
          error_occurred(tmp_position, "expected \")\" after expression list");
        }
      }else{
        error_occurred(lexer.get_current_position(), "expected expression list after \"(\"");
      }
    }
  }
  lexer.set_current_position(position);
  return node_sptr();
}

/**
 * size_of_list := "|" ( expression_list | program_list ) "|"
 */
node_sptr Parser::size_of_list(){
  boost::shared_ptr<SizeOfList> ret(new SizeOfList());
  position_t position = lexer.get_current_position();
  if(lexer.get_token() == VERTICAL_BAR){
    node_sptr tmp;
    if((tmp = expression_list()) ||
       (tmp = program_list())){
      ret->set_child(tmp);
      position_t tmp_position = lexer.get_current_position();
      if(lexer.get_token() == VERTICAL_BAR){
        return ret;
      }else{
        error_occurred(tmp_position, "expected \"|\" after list");
      }
    }else{
      error_occurred(lexer.get_current_position(), "expected expression list after \"|\"");
    }
  }
  lexer.set_current_position(position);
  return node_sptr();
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
        position_t tmp_position = lexer.get_current_position();
        if(lexer.get_token() == RIGHT_BOX_BRACKETS){
          boost::shared_ptr<ProgramListElement> e(new ProgramListElement());
          e->set_lhs(list);
          e->set_rhs(expr);
          return e;
        }else{
          error_occurred(tmp_position, "expected \"]\" after expression");
        }
      }else{
        error_occurred(lexer.get_current_position(), "expected expression after \"[\"");
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
        position_t tmp_position = lexer.get_current_position();
        if(lexer.get_token() == RIGHT_BOX_BRACKETS){
          boost::shared_ptr<ExpressionListElement> e(new ExpressionListElement());
          e->set_lhs(list);
          e->set_rhs(expr);
          return e;
        }else{
          error_occurred(tmp_position, "expected \"]\" after expression");
        }
      }else{
        error_occurred(lexer.get_current_position(), "expected expression after \"[\"");
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
  in_conditional_program_list_ = true;
  std::vector<boost::shared_ptr<ProgramCaller> > pc;
  local_program_caller_.push(pc);
  position_t position = lexer.get_current_position();
  node_sptr tmp;
  if(lexer.get_token() == LEFT_BRACES){
    if((tmp = program())){
      if(lexer.get_token() == VERTICAL_BAR){
        boost::shared_ptr<ConditionalProgramList> ret(new ConditionalProgramList());
        ret->set_program(tmp);
        if((tmp = list_condition())){
          ret->add_argument(tmp);
          position_t tmp_position = lexer.get_current_position();
          while(lexer.get_token() == COMMA){
            if((tmp = list_condition())){
              ret->add_argument(tmp);
            }else{
              error_occurred(lexer.get_current_position(), "expected list condition after \",\"");
              break;
            }
            tmp_position = lexer.get_current_position();
          }
          lexer.set_current_position(tmp_position);
          if(lexer.get_token() == RIGHT_BRACES){
            local_program_caller_.pop();
            in_conditional_program_list_ = false;
            return ret;
          }else{
            error_occurred(tmp_position, "expected \"}\"");
          }
        }else{
          error_occurred(lexer.get_current_position(), "expected list conditions after \"|\"");
        }
      }
    }
  }
  local_program_caller_.pop();
  in_conditional_program_list_ = false;
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
    while(lexer.get_token() == LOWER_IDENTIFIER && lexer.get_current_token_string() == "or"){
      if((list = program_list_term())){
        boost::shared_ptr<Union> uni(new Union(ret,list));
        ret = uni;
      }else{
        error_occurred(lexer.get_current_position(), "expected program list after \"or\"");
        break;
      }
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
    while(lexer.get_token() == LOWER_IDENTIFIER && lexer.get_current_token_string() == "and"){
      if((list = program_list_factor())){
        boost::shared_ptr<Intersection> in(new Intersection(ret,list));
        ret = in;
      }else{
        error_occurred(lexer.get_current_position(), "expected program list after \"and\"");
        break;
      }
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
 *                      | "{"program_priority ("," program_priority)* "}"
 *                      | identifier number ".." identifier number
 */
node_sptr Parser::program_list_factor(){
  node_sptr ret;
  // conditional_program_list
  position_t position = lexer.get_current_position();
  // "{" program_priority ("," program_priority)* "}"
  if((ret = conditional_program_list())) return ret;
  if(lexer.get_token() == LEFT_BRACES){
    position_t list_position = lexer.get_current_position();
    boost::shared_ptr<ProgramList> list(new ProgramList());
    if((ret = program_priority())){
      list->add_argument(ret);
      position_t tmp_position = lexer.get_current_position();
      while(lexer.get_token() == COMMA){
        if((ret = program_priority())){
          list->add_argument(ret);
        }else{
          error_occurred(lexer.get_current_position(), "expected program after \",\"");
          break;
        }
        tmp_position = lexer.get_current_position();
      }
      lexer.set_current_position(tmp_position);
      if(lexer.get_token() == RIGHT_BRACES){
        return list;
      }
    }
    lexer.set_current_position(list_position);
    // "{" identifier number ".." identifier number "}"
    std::string str;
    if((str = definition_name()) != ""){
      if(lexer.get_token() == TWO_PERIOD){
        std::string str2;
        if((str2 = definition_name()) != ""){
          int num1;
          int num2;
          for(num1 = str.length()-1; num1 >= 0; num1--){
            if(str[num1] < '0' || '9' < str[num1]) break;
          }
          for(num2 = str2.length()-1; num2 >= 0; num2--){
            if(str2[num2] < '0' || '9' < str2[num2]) break;
          }
          position_t right_position = lexer.get_current_position();
          if(str.substr(0,num1+1) == str2.substr(0,num2+1)){
            if(lexer.get_token() == RIGHT_BRACES){
              boost::shared_ptr<Range> range(
                new Range(boost::shared_ptr<Number>(new Number(str.substr(num1+1))),
                          boost::shared_ptr<Number>(new Number(str2.substr(num2+1)))));
                range->set_header(str.substr(0,num1+1));
                return range;
            }else{
              error_occurred(right_position, "expected \"}\"");
            }
          }else{
            error_occurred(right_position, str2 + " do not correspond to " + str);
          }
        }else{
          error_occurred(lexer.get_current_position(), "expected program corresponding to " + str);
        }
      }
    }
  }
  lexer.set_current_position(position);
  if(lexer.get_token() == LEFT_PARENTHESES){
    if((ret = program_list())){
      position_t right_position = lexer.get_current_position();
      if(lexer.get_token() == RIGHT_PARENTHESES){
        return ret;
      }else{
        error_occurred(right_position, "expected \")\"");
      }
    }
  }
  std::string str;
  lexer.set_current_position(position);
  if((str = definition_name()) != ""){
    if(second_parse)
    {
      node_sptr defined;
      IS_DEFINED_AS(str,0,tmp_program_list_definitions,defined);
      if((defined)){
        boost::shared_ptr<ProgramListCaller> caller(new ProgramListCaller());
        caller->set_name(str);
        return caller;
      }
      error_occurred(lexer.get_current_position(), "undefined program list \"" + str + "\"");  
    }
    else
    {
      boost::shared_ptr<ProgramListCaller> caller(new ProgramListCaller());
      caller->set_name(str);
      return caller;
    }
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
          position_t tmp_position = lexer.get_current_position();
          while(lexer.get_token() == COMMA){
            position_t error_position = lexer.get_current_position();
            if((tmp = list_condition())){
              ret->add_argument(tmp);
            }else{
              error_occurred(error_position, "expected list condition after \",\"");
              break;
            }
            tmp_position = lexer.get_current_position();
          }
          lexer.set_current_position(tmp_position);
          if(lexer.get_token() == RIGHT_BRACES){
            return ret;
          }else{
            error_occurred(tmp_position, "expected \"}\"");
          }
        }else{
          error_occurred(lexer.get_current_position(), "expected list condition after \"|\"");
        }
      }
    }
  }
  lexer.set_current_position(position);
  return node_sptr();
}

/**
 * nameless_list := "$" "List" "[" expression "]"
 */ 
node_sptr Parser::nameless_list(){
  node_sptr ret;
  position_t position = lexer.get_current_position();
  position_t tmp_position;
  if(lexer.get_token() == SYSTEM){
    if(lexer.get_token() == UPPER_IDENTIFIER){
      if(lexer.get_current_token_string() == "List"){
        tmp_position = lexer.get_current_position();
        if(lexer.get_token() == LEFT_BOX_BRACKETS){
          if((ret = expression())){
            tmp_position = lexer.get_current_position();
            if(lexer.get_token() == RIGHT_BOX_BRACKETS){
              boost::shared_ptr<ExpressionList> el(new ExpressionList());
              el->set_nameless_expression_arguments(ret);
              return el;
            }
            else
            {
              error_occurred(tmp_position, "expected \"]\" after expression");
            }
          }else error_occurred(lexer.get_current_position(), "expected expression after \"[\"");
        }
        else
        {
          error_occurred(tmp_position, "expected \"[\" after \"$List\"");
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
    while(lexer.get_token() == LOWER_IDENTIFIER && lexer.get_current_token_string() == "or"){
      if((list = expression_list_term())){
        boost::shared_ptr<Union> uni(new Union(ret,list));
        ret = uni;
      }else{
        error_occurred(lexer.get_current_position(), "expected expression list after \"or\"");
        break;
      }
      tmp_position = lexer.get_current_position();
    }
    lexer.set_current_position(tmp_position);
    return ret;
  }
  lexer.set_current_position(position);
  return node_sptr();
}
/**
 * expression_list_term := expression_list_factor ("and" expression_list_factor )*
 */
node_sptr Parser::expression_list_term(){
  position_t position = lexer.get_current_position();
  node_sptr ret;
  if((ret = expression_list_factor())){
    node_sptr list;
    position_t tmp_position = lexer.get_current_position();
    while(lexer.get_token() == LOWER_IDENTIFIER && lexer.get_current_token_string() == "and"){
      if((list = expression_list_factor())){
        boost::shared_ptr<Intersection> in(new Intersection(ret,list));
        ret = in;
      }else{
        error_occurred(lexer.get_current_position(), "expected expression list after \"and\"");
        break;
      }
      tmp_position = lexer.get_current_position();
    }
    lexer.set_current_position(tmp_position);
    return ret;
  }
  lexer.set_current_position(position);
  return node_sptr();
}
/**
 * expression_list_factor := nameless_list
 *                         | conditional_expression_list
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
  // nameless_list
  if((ret = nameless_list())) return ret;
  // "{" expression ("," expression)* "}"
  if((ret = conditional_expression_list())) return ret;
  if(lexer.get_token() == LEFT_BRACES){
    position_t list_position = lexer.get_current_position();
    boost::shared_ptr<ExpressionList> list(new ExpressionList());
    if((ret = expression())){
      list->add_argument(ret);
      position_t tmp_position = lexer.get_current_position();
      while(lexer.get_token() == COMMA){
        if((ret = expression())){
          list->add_argument(ret);
        }else{
          error_occurred(lexer.get_current_position(), "expected expression after \",\"");
          break;
        }
        tmp_position = lexer.get_current_position();
      }
      lexer.set_current_position(tmp_position);
      if(lexer.get_token() == RIGHT_BRACES){
        return list;
      }else{
        error_occurred(tmp_position, "expected \"}\"");
      }
    }
    lexer.set_current_position(list_position);
    // "{" identifier number ".." identifier number "}"
    std::string str;
    if((str = variable_name()) != ""){
      if(lexer.get_token() == TWO_PERIOD){
        std::string str2;
        if((str2 = variable_name()) != ""){
          int num1;
          int num2;
          for(num1 = str.length()-1; num1 >= 0; num1--){
            if(str[num1] < '0' || '9' < str[num1]) break;
          }
          for(num2 = str2.length()-1; num2 >= 0; num2--){
            if(str2[num2] < '0' || '9' < str2[num2]) break;
          }
          if(str.substr(0,num1+1) == str2.substr(0,num2+1)){
            position_t right_position = lexer.get_current_position();
            if(lexer.get_token() == RIGHT_BRACES){
              boost::shared_ptr<Range> range(
                new Range(boost::shared_ptr<Number>(new Number(str.substr(num1+1))),
                          boost::shared_ptr<Number>(new Number(str2.substr(num2+1)))));
                range->set_header(str.substr(0,num1+1));
                return range;
            }else{
              error_occurred(right_position, "expected \"}\"");
            }
          }else{
            error_occurred(lexer.get_current_position(), str2 + " do not correspond to " + str);
          }
        }else{
          error_occurred(lexer.get_current_position(), "expected variable corresponding to " + str);
        }
      }
    }
    lexer.set_current_position(list_position);
    // "{" expression ".." expression "}"
    if((ret = expression())){
      if(lexer.get_token() == TWO_PERIOD){
        node_sptr num2;
        if((num2 = expression())){
          position_t right_position = lexer.get_current_position();
          if(lexer.get_token() == RIGHT_BRACES){
            return boost::shared_ptr<Range>(new Range(ret,num2));
          }else{
            error_occurred(right_position, "expected \"}\"");
          }
        }else{
          error_occurred(lexer.get_current_position(), "expected expression after \"..\"");
        }
      }
    }
    lexer.set_current_position(list_position);
  }
  lexer.set_current_position(position);
  // "(" expression_list ")"
  if(lexer.get_token() == LEFT_PARENTHESES){
    if((ret = expression_list())){
      position_t right_position = lexer.get_current_position();
      if(lexer.get_token() == RIGHT_PARENTHESES){
        return ret;
      }else{
        error_occurred(right_position, "expected \")\"");
      }
    }
  }
  lexer.set_current_position(position);
  std::string str;
  if((str = definition_name()) != ""){
    if(second_parse)
    {
      node_sptr defined;
      IS_DEFINED_AS(str,0,tmp_expression_list_definitions,defined);
      if((defined)){
        boost::shared_ptr<ExpressionListCaller> caller(new ExpressionListCaller());
        caller->set_name(str);
        return caller;
      }
      error_occurred(lexer.get_current_position(), "undefined expression list \"" + str + "\"");
    }
    else
    {
      boost::shared_ptr<ExpressionListCaller> caller(new ExpressionListCaller());
      caller->set_name(str);
      return caller;
    }
  }
  lexer.set_current_position(position);
  return node_sptr();
}
/**
 * list_condition := variable "in" expression_list
 *                 | program "in" program_list
 *                 | (expression_list_element | program_list_element | variable) "!=" (expression_list_element | program_list_element | variable)
 */
node_sptr Parser::list_condition(){
  node_sptr lhs;
  position_t position = lexer.get_current_position();
  // variable_name "in" expression_list
  std::string name;
  if((name = variable_name()) != ""){
    if(lexer.get_token() == LOWER_IDENTIFIER && lexer.get_current_token_string() == "in"){
      node_sptr list;
      if((list = expression_list())){
        return boost::shared_ptr<EachElement>(new EachElement(boost::shared_ptr<Variable>(new Variable(name)),list));
      }else{
        error_occurred(lexer.get_current_position(), "expected expression list after \"in\"");
      }
    }
  }
  lexer.set_current_position(position);
  // definition_name "in" program_list
  if((name = definition_name()) != ""){
    if(lexer.get_token() == LOWER_IDENTIFIER && lexer.get_current_token_string() == "in"){
      node_sptr list;
      if((list = program_list())){
        boost::shared_ptr<ProgramCaller> lhs(new ProgramCaller());
        lhs->set_name(name);
        return boost::shared_ptr<EachElement>(new EachElement(lhs,list));
      }else{
        error_occurred(lexer.get_current_position(), "expected program list after \"in\"");
      }
    }
  }
  lexer.set_current_position(position);
// (expression_list_element | program_list_element | variable) "!=" (expression_list_element | program_list_element | variable)
  if((lhs = expression_list_element()) ||
     (lhs = program_list_element()) ||
     (lhs = variable())){
    if(lexer.get_token() == NOT_EQUAL){
      node_sptr rhs;
      if((rhs = expression_list_element()) ||
         (rhs = program_list_element()) ||
         (rhs = variable())){
        return boost::shared_ptr<DifferentVariable>(new DifferentVariable(lhs,rhs)); 
      }else{
        error_occurred(lexer.get_current_position(), "expected variable, expression list element or program list element after \"!=\"");
      }
    }
  }
  lexer.set_current_position(position);
  return node_sptr();
}

} // namespace parser
} // namespace hydla
