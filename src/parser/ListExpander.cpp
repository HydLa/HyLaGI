#include "ListExpander.h"

namespace hydla{
namespace parser{

using namespace symbolic_expression;

#define DEFINE_FACTOR_VISIT(TYPE) \
void ListExpander::visit(boost::shared_ptr<TYPE> node) \
{ \
  new_child = node; \
}

#define DEFINE_ARBITRARY_VISIT(TYPE) \
void ListExpander::visit(boost::shared_ptr<TYPE> node) \
{ \
  for(int i = 0; i < node->get_arguments_size(); i++){ \
    accept(node->get_argument(i)); \
    if(new_child){ \
      node->set_argument(new_child,i); \
      new_child.reset(); \
    } \
  } \
  new_child = node; \
}

#define DEFINE_UNARY_VISIT(TYPE) \
void ListExpander::visit(boost::shared_ptr<TYPE> node) \
{ \
  accept(node->get_child()); \
  if(new_child){ \
    node->set_child(new_child); \
    new_child.reset(); \
  } \
  new_child = node; \
} 

#define DEFINE_BINARY_VISIT(TYPE) \
void ListExpander::visit(boost::shared_ptr<TYPE> node) \
{ \
  accept(node->get_lhs()); \
  if(new_child){ \
    node->set_lhs(new_child); \
    new_child.reset(); \
  } \
  accept(node->get_rhs()); \
  if(new_child){ \
    node->set_rhs(new_child); \
    new_child.reset(); \
  } \
  new_child = node; \
} 

ListExpander::ListExpander(
    DefinitionContainer<symbolic_expression::ConstraintDefinition>& c,
    DefinitionContainer<symbolic_expression::ProgramDefinition>& pr,
    DefinitionContainer<symbolic_expression::ExpressionListDefinition>& e,
    DefinitionContainer<symbolic_expression::ProgramListDefinition>& p
) : constraint_def(c), program_def(pr), expression_list_def(e), program_list_def(p)
{}


symbolic_expression::node_sptr ListExpander::circular_check(
    referenced_definition_t def_type,
    boost::shared_ptr<symbolic_expression::Definition> definition)
{
  definition = boost::dynamic_pointer_cast<Definition>(definition->clone());
  
  //循環参照のチェック
  if (!referenced_definition.empty()){
    if (referenced_definition.top().find(def_type) != 
        referenced_definition.top().end()) {
  //    throw CircularReference(caller);
    }
  }
  std::set<referenced_definition_t> reference;
  reference.insert(def_type);
  referenced_definition.push(reference);

  // 循環参照検出用リストに登録
  referenced_definition.top().insert(def_type);

  accept(definition->get_child());
  if(new_child){
    definition->set_child(new_child);
    new_child.reset();
  }
  referenced_definition.pop();

  return definition->get_child();
}

node_sptr ListExpander::expand_list(boost::shared_ptr<SumOfList> node){
  accept(node);
  return boost::dynamic_pointer_cast<Plus>(new_child);
}

boost::shared_ptr<Number> ListExpander::expand_list(boost::shared_ptr<SizeOfList> node){
  accept(node);
  return boost::dynamic_pointer_cast<Number>(new_child);
}

node_sptr ListExpander::expand_list(boost::shared_ptr<ExpressionListElement> node){
  accept(node);
  node_sptr ret;
  if(new_child){
    ret = new_child;
    new_child.reset();
  }
  return ret;
}

node_sptr ListExpander::expand_list(boost::shared_ptr<ProgramListElement> node){
  accept(node);
  node_sptr ret;
  if(new_child){
    ret = new_child;
    new_child.reset();
  }
  return ret;
}

boost::shared_ptr<ExpressionList> ListExpander::expand_list(boost::shared_ptr<ConditionalExpressionList> node){
  boost::shared_ptr<ExpressionList> ret = boost::shared_ptr<ExpressionList>(new ExpressionList());
  expand_conditional_list(ret,node->get_expression(),node,0);
  return ret;
}
boost::shared_ptr<ProgramList> ListExpander::expand_list(boost::shared_ptr<ConditionalProgramList> node){
  boost::shared_ptr<ProgramList> ret = boost::shared_ptr<ProgramList>(new ProgramList());
  expand_conditional_list(ret,node->get_program(),node,0);
  return ret;
}

boost::shared_ptr<ArbitraryNode> ListExpander::expand_list(boost::shared_ptr<Range> node){
  bool original_in_list_element = in_list_element;
  in_list_element = true;
  boost::shared_ptr<ArbitraryNode> ret;
  node_sptr lhs, rhs;
  accept(node->get_lhs());
  if(new_child){
    lhs = new_child;
    new_child.reset();
  }else{
    lhs = node->get_lhs();
  }
  accept(node->get_rhs());
  if(new_child){
    rhs = new_child;
    new_child.reset();
  }else{
    rhs = node->get_rhs();
  }
  boost::shared_ptr<Number> lhs_num, rhs_num;
  lhs_num = boost::dynamic_pointer_cast<Number>(lhs->clone());
  rhs_num = boost::dynamic_pointer_cast<Number>(rhs->clone());
  if(lhs_num && rhs_num){
    if(node->get_string().length() == 0){
      ret = boost::shared_ptr<ExpressionList>(new ExpressionList());
      for(int i = std::stoi(lhs_num->get_number()); i <= std::stoi(rhs_num->get_number()); i++){
        ret->add_argument(boost::shared_ptr<Number>(new Number(std::to_string(i))));
      }
    }else if('A' <= node->get_string()[0] && node->get_string()[0] <= 'Z'){
      ret = boost::shared_ptr<ProgramList>(new ProgramList());
      for(int i = std::stoi(lhs_num->get_number()); i <= std::stoi(rhs_num->get_number()); i++){
        boost::shared_ptr<ConstraintCaller> caller = boost::shared_ptr<ConstraintCaller>(new ConstraintCaller());
        caller->set_name(node->get_string()+std::to_string(i));
        ret->add_argument(caller);
      }
    }else{
      ret = boost::shared_ptr<ExpressionList>(new ExpressionList());
      for(int i = std::stoi(lhs_num->get_number()); i <= std::stoi(rhs_num->get_number()); i++){
        boost::shared_ptr<Variable> variable = boost::shared_ptr<Variable>(new Variable(node->get_string()+std::to_string(i)));
        ret->add_argument(variable);
      }
    }
  }
  in_list_element = original_in_list_element;
  return ret;
}

boost::shared_ptr<ArbitraryNode> ListExpander::expand_list(boost::shared_ptr<Union> node){
  boost::shared_ptr<ArbitraryNode> ret;
  boost::shared_ptr<ExpressionList> el;
  accept(node->get_lhs());
  if(new_child){
    el = boost::dynamic_pointer_cast<ExpressionList>(new_child->clone());
    new_child.reset();
  }else{
    el = boost::dynamic_pointer_cast<ExpressionList>(node->get_lhs()->clone());
  }
  if(el){
    accept(node->get_rhs());
    boost::shared_ptr<ExpressionList> rhs;
    if(new_child){
      rhs = boost::dynamic_pointer_cast<ExpressionList>(new_child->clone());
      new_child.reset();
    }else{
      rhs = boost::dynamic_pointer_cast<ExpressionList>(node->get_rhs()->clone());
    }
    bool not_in = true;
    for(int i = 0; i < rhs->get_arguments_size(); i++){
      for(int j = 0; j < el->get_arguments_size(); j++){
        if(el->get_argument(j)->is_same_struct(*(rhs->get_argument(i)),false)){
          not_in = false;
          break;
        }
      }
      if(not_in){
        el->add_argument(rhs->get_argument(i));
      }
      not_in = true;
    }
    return el;
  }

  boost::shared_ptr<ProgramList> pl;
  if(new_child){
    pl = boost::dynamic_pointer_cast<ProgramList>(new_child->clone());
    new_child.reset();
  }else{
    pl = boost::dynamic_pointer_cast<ProgramList>(node->get_lhs()->clone());
  }
  if(pl){
    accept(node->get_rhs());
    boost::shared_ptr<ProgramList> rhs;
    if(new_child){
      rhs = boost::dynamic_pointer_cast<ProgramList>(new_child->clone());
      new_child.reset();
    }else{
      rhs = boost::dynamic_pointer_cast<ProgramList>(node->get_rhs()->clone());
    }
    bool not_in = true;
    for(int i = 0; i < rhs->get_arguments_size(); i++){
      for(int j = 0; j < pl->get_arguments_size(); j++){
        if(pl->get_argument(j)->is_same_struct(*(rhs->get_argument(i)),false)){
          not_in = false;
          break;
        }
      }
      if(not_in){
        pl->add_argument(rhs->get_argument(i));
      }
      not_in = true;
    }
    return pl;
  }
  return ret;
}

boost::shared_ptr<ArbitraryNode> ListExpander::expand_list(boost::shared_ptr<Intersection> node){
  boost::shared_ptr<ArbitraryNode> ret;
  boost::shared_ptr<ExpressionList> el;
  accept(node->get_lhs());
  if(new_child){
    el = boost::dynamic_pointer_cast<ExpressionList>(new_child->clone());
    new_child.reset();
  }else{
    el = boost::dynamic_pointer_cast<ExpressionList>(node->get_lhs()->clone());
  }
  if(el){
    ret = boost::shared_ptr<ExpressionList>(new ExpressionList());
    accept(node->get_rhs());
    boost::shared_ptr<ExpressionList> rhs;
    if(new_child){
      rhs = boost::dynamic_pointer_cast<ExpressionList>(new_child->clone());
      new_child.reset();
    }else{
      rhs = boost::dynamic_pointer_cast<ExpressionList>(node->get_rhs()->clone());
    }
    bool not_in = true;
    for(int i = 0; i < rhs->get_arguments_size(); i++){
      for(int j = 0; j < el->get_arguments_size(); j++){
        if(el->get_argument(j)->is_same_struct(*(rhs->get_argument(i)),false)){
          not_in = false;
          break;
        }
      }
      if(!not_in){
        ret->add_argument(rhs->get_argument(i));
      }
      not_in = true;
    }
    return ret;
  }

  boost::shared_ptr<ProgramList> pl;
  if(new_child){
    pl = boost::dynamic_pointer_cast<ProgramList>(new_child->clone());
    new_child.reset();
  }else{
    pl = boost::dynamic_pointer_cast<ProgramList>(node->get_lhs()->clone());
  }
  if(pl){
    ret = boost::shared_ptr<ProgramList>(new ProgramList());
    accept(node->get_rhs());
    boost::shared_ptr<ProgramList> rhs;
    if(new_child){
      rhs = boost::dynamic_pointer_cast<ProgramList>(new_child->clone());
      new_child.reset();
    }else{
      rhs = boost::dynamic_pointer_cast<ProgramList>(node->get_rhs()->clone());
    }
    bool not_in = true;
    for(int i = 0; i < rhs->get_arguments_size(); i++){
      for(int j = 0; j < pl->get_arguments_size(); j++){
        if(pl->get_argument(j)->is_same_struct(*(rhs->get_argument(i)),false)){
          not_in = false;
          break;
        }
      }
      if(!not_in){
        ret->add_argument(rhs->get_argument(i));
      }
      not_in = true;
    }
    return ret;
  }
  return ret;
}


void ListExpander::expand_conditional_list(boost::shared_ptr<ArbitraryNode> ret, node_sptr element, boost::shared_ptr<ArbitraryNode> list, int idx){

  boost::shared_ptr<BinaryNode> bn;
  if(list->get_arguments_size() == idx){
    accept(element->clone());
    if(new_child){
      ret->add_argument(new_child);
      new_child.reset();
    }else{
      ret->add_argument(element);
    }
    accept(ret);
    if(new_child) ret = boost::dynamic_pointer_cast<ArbitraryNode>(new_child);
    return;
  }

  // DifferenVariable
  bn = boost::dynamic_pointer_cast<DifferentVariable>(list->get_argument(idx)->clone());
  if(bn){
    node_sptr lhs,rhs;
    accept(bn->get_lhs());
    if(new_child){
      lhs = new_child;
      new_child.reset();
    }else{
      lhs = bn->get_lhs();
    }
    accept(bn->get_rhs());
    if(new_child){
      rhs = new_child;
      new_child.reset();
    }else{
      rhs = bn->get_rhs();
    }
    if(lhs->get_string() == rhs->get_string()){
      accept(ret);
      if(new_child) ret = boost::dynamic_pointer_cast<ArbitraryNode>(new_child);
      return;
    }
  }

  // EachElement
  bn = boost::dynamic_pointer_cast<EachElement>(list->get_argument(idx)->clone());
  if(bn){
    node_sptr local_var = bn->get_lhs();
    boost::shared_ptr<ArbitraryNode> each_list;
    accept(bn->get_rhs());
    if(new_child){
      each_list = boost::dynamic_pointer_cast<ExpressionList>(new_child->clone());
      new_child.reset();
    }else{
      each_list = boost::dynamic_pointer_cast<ExpressionList>(bn->get_rhs()->clone());
    }
    if(!each_list){
      if(new_child){
        each_list = boost::dynamic_pointer_cast<ProgramList>(new_child->clone());
        new_child.reset();
      }else{
        each_list = boost::dynamic_pointer_cast<ProgramList>(bn->get_rhs()->clone());
      }
    }
    if(each_list){
      // list
      node_sptr var = bn->get_lhs();
      for(int i = 0; i < each_list->get_arguments_size(); i++){
        local_variable_map[var] = each_list->get_argument(i);
        expand_conditional_list(ret,element,list,idx+1);
      }
      local_variable_map.erase(var);
    }
  }
}

void ListExpander::visit(boost::shared_ptr<symbolic_expression::Weaker> node){
  accept(node->get_lhs());
  if(new_child){
    node->set_lhs(new_child);
    new_child.reset();
  }
  accept(node->get_rhs());
  if(new_child){
    node->set_rhs(new_child);
    new_child.reset();
  }
  new_child = node;
}
void ListExpander::visit(boost::shared_ptr<symbolic_expression::Parallel> node){
  accept(node->get_lhs());
  if(new_child){
    node->set_lhs(new_child);
    new_child.reset();
  }
  accept(node->get_rhs());
  if(new_child){
    node->set_rhs(new_child);
    new_child.reset();
  }
  new_child = node;
}
void ListExpander::visit(boost::shared_ptr<symbolic_expression::ExpressionList> node){
  new_child = node;
}
void ListExpander::visit(boost::shared_ptr<symbolic_expression::ProgramList> node){
  new_child = node;
}
void ListExpander::visit(boost::shared_ptr<symbolic_expression::ConditionalExpressionList> node){
  new_child = expand_list(node);
}
void ListExpander::visit(boost::shared_ptr<symbolic_expression::ConditionalProgramList> node){
  new_child = expand_list(node);
}
void ListExpander::visit(boost::shared_ptr<symbolic_expression::Union> node){
  new_child = expand_list(node);
}
void ListExpander::visit(boost::shared_ptr<symbolic_expression::Intersection> node){
  new_child = expand_list(node);
}
void ListExpander::visit(boost::shared_ptr<symbolic_expression::Range> node){
  new_child = expand_list(node);
}

void ListExpander::visit(boost::shared_ptr<symbolic_expression::Number> node){
  new_child = node->clone();
}

void ListExpander::visit(boost::shared_ptr<symbolic_expression::Variable> node){
  for(auto map : local_variable_map){
    if(node->is_same_struct(*(map.first),true)){
      new_child = map.second;
      return;
    }
  }
  new_child = node;
}

void ListExpander::visit(boost::shared_ptr<symbolic_expression::ProgramCaller> node){
  boost::shared_ptr<ProgramCaller> caller = boost::shared_ptr<ProgramCaller>(new ProgramCaller());
  for(auto map : local_variable_map){
    if(node->is_same_struct(*(map.first),true)){
      node = boost::dynamic_pointer_cast<ProgramCaller>(map.second->clone());
      break;
    }
  }
  caller->set_name(node->get_name());
  for(int i = 0; i < node->actual_arg_size(); i++){
    accept(node->get_actual_arg(i)->clone());
    if(new_child){
      caller->add_actual_arg(new_child);
      new_child.reset();
    }else caller->add_actual_arg(node->get_actual_arg(i)->clone());
  }
  new_child = caller;
}

void ListExpander::visit(boost::shared_ptr<symbolic_expression::ConstraintCaller> node){
  boost::shared_ptr<ConstraintCaller> caller = boost::shared_ptr<ConstraintCaller>(new ConstraintCaller());
  for(auto map : local_variable_map){
    if(node->is_same_struct(*(map.first),true)){
      node = boost::dynamic_pointer_cast<ConstraintCaller>(map.second);
      break;
    }
  }
  caller->set_name(node->get_name());
  for(int i = 0; i < node->actual_arg_size(); i++){
    accept(node->get_actual_arg(i)->clone());
    if(new_child){
      caller->add_actual_arg(new_child);
      new_child.reset();
    }else caller->add_actual_arg(node->get_actual_arg(i)->clone());
  }
  new_child = caller;
}

void ListExpander::visit(boost::shared_ptr<symbolic_expression::Plus> node){
  boost::shared_ptr<Number> lhs, rhs;
  accept(node->get_lhs());
  if(new_child){
    node->set_lhs(new_child->clone());
    new_child.reset();
  }
  accept(node->get_rhs());
  if(new_child){
    node->set_rhs(new_child->clone());
    new_child.reset();
  }
  if(!in_list_element){
    new_child = node;
    return;
  }
  lhs = boost::dynamic_pointer_cast<Number>(node->get_lhs()->clone());
  rhs = boost::dynamic_pointer_cast<Number>(node->get_rhs()->clone());
  if(lhs && rhs){
    double num = std::stof(lhs->get_number()) + std::stof(rhs->get_number());
    std::string str = std::to_string(num);
    new_child = boost::shared_ptr<Number>(new Number(str));
  }else new_child = node;
}

void ListExpander::visit(boost::shared_ptr<symbolic_expression::Subtract> node){
  boost::shared_ptr<Number> lhs, rhs;
  accept(node->get_lhs());
  if(new_child){
    node->set_lhs(new_child->clone());
    new_child.reset();
  }
  accept(node->get_rhs());
  if(new_child){
    node->set_rhs(new_child->clone());
    new_child.reset();
  }
  if(!in_list_element){
    new_child = node;
    return;
  }
  lhs = boost::dynamic_pointer_cast<Number>(node->get_lhs()->clone());
  rhs = boost::dynamic_pointer_cast<Number>(node->get_rhs()->clone());
  if(lhs && rhs){
    double num = std::stof(lhs->get_number()) - std::stof(rhs->get_number());
    std::string str = std::to_string(num);
    new_child = boost::shared_ptr<Number>(new Number(str));
  }else new_child = node;
}

void ListExpander::visit(boost::shared_ptr<symbolic_expression::Times> node){
  boost::shared_ptr<Number> lhs, rhs;
  accept(node->get_lhs());
  if(new_child){
    node->set_lhs(new_child->clone());
    new_child.reset();
  }
  accept(node->get_rhs());
  if(new_child){
    node->set_rhs(new_child->clone());
    new_child.reset();
  }
  if(!in_list_element){
    new_child = node;
    return;
  }
  lhs = boost::dynamic_pointer_cast<Number>(node->get_lhs()->clone());
  rhs = boost::dynamic_pointer_cast<Number>(node->get_rhs()->clone());
  if(lhs && rhs){
    double num = std::stof(lhs->get_number()) * std::stof(rhs->get_number());
    std::string str = std::to_string(num);
    new_child = boost::shared_ptr<Number>(new Number(str));
  }else new_child = node;
}

void ListExpander::visit(boost::shared_ptr<symbolic_expression::Divide> node){
  boost::shared_ptr<Number> lhs, rhs;
  accept(node->get_lhs());
  if(new_child){
    node->set_lhs(new_child->clone());
    new_child.reset();
  }
  accept(node->get_rhs());
  if(new_child){
    node->set_rhs(new_child->clone());
    new_child.reset();
  }
  if(!in_list_element){
    new_child = node;
    return;
  }
  lhs = boost::dynamic_pointer_cast<Number>(node->get_lhs()->clone());
  rhs = boost::dynamic_pointer_cast<Number>(node->get_rhs()->clone());
  if(lhs && rhs){
    double num = std::stof(lhs->get_number()) / std::stof(rhs->get_number());
    std::string str = std::to_string(num);
    new_child = boost::shared_ptr<Number>(new Number(str));
  }else new_child = node;
}

void ListExpander::visit(boost::shared_ptr<symbolic_expression::Power> node){
  boost::shared_ptr<Number> lhs, rhs;
  accept(node->get_lhs());
  if(new_child){
    node->set_lhs(new_child->clone());
    new_child.reset();
  }
  accept(node->get_rhs());
  if(new_child){
    node->set_rhs(new_child->clone());
    new_child.reset();
  }
  if(!in_list_element){
    new_child = node;
    return;
  }
  lhs = boost::dynamic_pointer_cast<Number>(node->get_lhs()->clone());
  rhs = boost::dynamic_pointer_cast<Number>(node->get_rhs()->clone());
  if(lhs && rhs){
    double num = std::pow(std::stof(lhs->get_number()), std::stof(rhs->get_number()));
    std::string str = std::to_string(num);
    new_child = boost::shared_ptr<Number>(new Number(str));
  }else new_child = node;
}

void ListExpander::visit(boost::shared_ptr<symbolic_expression::ExpressionListElement> node){
  bool original_in_list_element = in_list_element;
  in_list_element = true;
  boost::shared_ptr<ArbitraryNode> list;
  boost::shared_ptr<Number> num;
  accept(node->get_lhs()->clone());
  if(new_child){
    list = boost::dynamic_pointer_cast<ArbitraryNode>(new_child->clone());
    new_child.reset();
  }else list = boost::dynamic_pointer_cast<ArbitraryNode>(node->get_lhs()->clone());
  accept(node->get_rhs()->clone());
  if(new_child){
    num = boost::dynamic_pointer_cast<Number>(new_child->clone());
    new_child.reset();
  }else num = boost::dynamic_pointer_cast<Number>(node->get_rhs()->clone());
  if(list && num){
    int idx = std::stoi(num->get_number());
    new_child = list->get_argument(idx-1);
  }else{
    new_child = node;
  }
  in_list_element = original_in_list_element;
}

void ListExpander::visit(boost::shared_ptr<symbolic_expression::ProgramListElement> node){
  in_list_element = true;
  boost::shared_ptr<ArbitraryNode> list;
  boost::shared_ptr<Number> num;
  accept(node->get_lhs()->clone());
  if(new_child){
    list = boost::dynamic_pointer_cast<ArbitraryNode>(new_child->clone());
    new_child.reset();
  }else list = boost::dynamic_pointer_cast<ArbitraryNode>(node->get_lhs()->clone());
  accept(node->get_rhs()->clone());
  if(new_child){
    num = boost::dynamic_pointer_cast<Number>(new_child->clone());
    new_child.reset();
  }else num = boost::dynamic_pointer_cast<Number>(node->get_rhs()->clone());
  if(list && num){
    int idx = std::stoi(num->get_number());
    new_child = list->get_argument(idx-1);
  }
  in_list_element = false;
}

void ListExpander::visit(boost::shared_ptr<symbolic_expression::ProgramListCaller> node){
  referenced_definition_t deftype(
    std::make_pair(node->get_name(),
                   node->actual_arg_size()));
  boost::shared_ptr<ProgramListDefinition> cons_def(
    program_list_def.get_definition(deftype));

  new_child = circular_check(deftype,cons_def);
  accept(new_child);
}

void ListExpander::visit(boost::shared_ptr<symbolic_expression::ExpressionListCaller> node){
  referenced_definition_t deftype(
    std::make_pair(node->get_name(),
                   node->actual_arg_size()));
  boost::shared_ptr<ExpressionListDefinition> cons_def(
    expression_list_def.get_definition(deftype));

  new_child = circular_check(deftype,cons_def);
  accept(new_child);
}

void ListExpander::visit(boost::shared_ptr<symbolic_expression::SizeOfList> node){
  accept(node->get_child());
  boost::shared_ptr<ArbitraryNode> list;
  if(new_child){
    list = boost::dynamic_pointer_cast<ArbitraryNode>(new_child->clone());
    new_child.reset();
  }else{
    list = boost::dynamic_pointer_cast<ArbitraryNode>(node->get_child()->clone());
  }
  new_child = boost::shared_ptr<Number>(new Number(std::to_string(list->get_arguments_size())));
}

void ListExpander::visit(boost::shared_ptr<symbolic_expression::SumOfList> node){
  node_sptr ret;
  accept(node->get_child());
  boost::shared_ptr<ExpressionList> list;
  if(new_child){
    list = boost::dynamic_pointer_cast<ExpressionList>(new_child->clone());
    new_child.reset();
  }else{
    list = boost::dynamic_pointer_cast<ExpressionList>(node->get_child()->clone());
  }
  for(int i = 0; i < list->get_arguments_size(); i++){
    if(ret){
      ret = boost::shared_ptr<Plus>(new Plus(ret,list->get_argument(i)));
    }else{
      ret = list->get_argument(i);
    }
  }
  new_child = ret;
}

DEFINE_UNARY_VISIT(Previous)
DEFINE_BINARY_VISIT(Equal)
DEFINE_FACTOR_VISIT(True)

DEFINE_UNARY_VISIT(Not)
DEFINE_UNARY_VISIT(Always)
DEFINE_UNARY_VISIT(Negative)
DEFINE_UNARY_VISIT(Tell)
DEFINE_UNARY_VISIT(Differential)
DEFINE_UNARY_VISIT(Constraint)
DEFINE_UNARY_VISIT(Positive)
DEFINE_UNARY_VISIT(ConstraintDefinition)
DEFINE_UNARY_VISIT(ProgramDefinition)
DEFINE_UNARY_VISIT(ExpressionListDefinition)
DEFINE_UNARY_VISIT(ProgramListDefinition)

DEFINE_BINARY_VISIT(LessEqual)
DEFINE_BINARY_VISIT(Less)
DEFINE_BINARY_VISIT(LogicalOr)
DEFINE_BINARY_VISIT(Greater)
DEFINE_BINARY_VISIT(GreaterEqual)
DEFINE_BINARY_VISIT(Ask)
DEFINE_BINARY_VISIT(LogicalAnd)
DEFINE_BINARY_VISIT(UnEqual)
DEFINE_BINARY_VISIT(EachElement)
DEFINE_BINARY_VISIT(DifferentVariable)

DEFINE_FACTOR_VISIT(Pi)
DEFINE_FACTOR_VISIT(SVtimer)
DEFINE_FACTOR_VISIT(SymbolicT)
DEFINE_FACTOR_VISIT(E)
DEFINE_FACTOR_VISIT(False)
DEFINE_FACTOR_VISIT(Float)
DEFINE_FACTOR_VISIT(Print)
DEFINE_FACTOR_VISIT(PrintIP)
DEFINE_FACTOR_VISIT(PrintPP)
DEFINE_FACTOR_VISIT(Scan)
DEFINE_FACTOR_VISIT(Exit)
DEFINE_FACTOR_VISIT(Abort)
DEFINE_FACTOR_VISIT(Parameter)
DEFINE_FACTOR_VISIT(Infinity)

DEFINE_ARBITRARY_VISIT(Function)
DEFINE_ARBITRARY_VISIT(UnsupportedFunction)

}
}
