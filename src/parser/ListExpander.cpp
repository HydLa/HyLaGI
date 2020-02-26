#include "ListExpander.h"

namespace hydla {
namespace parser {

using namespace symbolic_expression;
using namespace boost;
using namespace std;

#define DEFINE_FACTOR_VISIT(TYPE)                                              \
  void ListExpander::visit(std::shared_ptr<symbolic_expression::TYPE> node) {  \
    new_child = node;                                                          \
  }

#define DEFINE_ARBITRARY_VISIT(TYPE)                                           \
  void ListExpander::visit(std::shared_ptr<symbolic_expression::TYPE> node) {  \
    for (int i = 0; i < node->get_arguments_size(); i++) {                     \
      accept(node->get_argument(i));                                           \
      if (new_child) {                                                         \
        node->set_argument(new_child, i);                                      \
        new_child.reset();                                                     \
      }                                                                        \
    }                                                                          \
    new_child = node;                                                          \
  }

#define DEFINE_UNARY_VISIT(TYPE)                                               \
  void ListExpander::visit(std::shared_ptr<symbolic_expression::TYPE> node) {  \
    accept(node->get_child());                                                 \
    if (new_child) {                                                           \
      node->set_child(new_child);                                              \
      new_child.reset();                                                       \
    }                                                                          \
    new_child = node;                                                          \
  }

#define DEFINE_BINARY_VISIT(TYPE)                                              \
  void ListExpander::visit(std::shared_ptr<symbolic_expression::TYPE> node) {  \
    accept(node->get_lhs());                                                   \
    if (new_child) {                                                           \
      node->set_lhs(new_child);                                                \
      new_child.reset();                                                       \
    }                                                                          \
    accept(node->get_rhs());                                                   \
    if (new_child) {                                                           \
      node->set_rhs(new_child);                                                \
      new_child.reset();                                                       \
    }                                                                          \
    new_child = node;                                                          \
  }

ListExpander::ListExpander(
    DefinitionContainer<symbolic_expression::ConstraintDefinition> &c,
    DefinitionContainer<symbolic_expression::ProgramDefinition> &pr,
    DefinitionContainer<symbolic_expression::ExpressionListDefinition> &e,
    DefinitionContainer<symbolic_expression::ProgramListDefinition> &p)
    : constraint_def(c), program_def(pr), expression_list_def(e),
      program_list_def(p) {}

symbolic_expression::node_sptr ListExpander::circular_check(
    referenced_definition_t def_type,
    std::shared_ptr<symbolic_expression::Definition> definition) {
  definition = std::dynamic_pointer_cast<Definition>(definition->clone());

  //循環参照のチェック
  if (!referenced_definition.empty()) {
    if (referenced_definition.top().find(def_type) !=
        referenced_definition.top().end()) {
      throw hydla::parser::error::CircularReference(definition);
    }
  }
  std::set<referenced_definition_t> reference;
  reference.insert(def_type);
  referenced_definition.push(reference);

  // 循環参照検出用リストに登録
  referenced_definition.top().insert(def_type);

  accept(definition->get_child());
  if (new_child) {
    definition->set_child(new_child);
    new_child.reset();
  }
  referenced_definition.pop();

  return definition->get_child();
}

node_sptr ListExpander::expand_list(
    std::shared_ptr<symbolic_expression::SumOfList> node) {
  accept(node);
  node_sptr ret = std::dynamic_pointer_cast<Plus>(new_child);
  // 要素が2つ以上のとき
  if (ret)
    return ret;
  ret = std::dynamic_pointer_cast<Number>(new_child);
  // 要素が1つのとき
  if (ret)
    return ret;
  // 要素が無いリストの総和は0とする
  return std::shared_ptr<Number>(new Number("0"));
}

node_sptr ListExpander::expand_list(
    std::shared_ptr<symbolic_expression::MulOfList> node) {
  accept(node);
  node_sptr ret = std::dynamic_pointer_cast<Times>(new_child);
  // 要素が2つ以上のとき
  if (ret)
    return ret;
  ret = std::dynamic_pointer_cast<Number>(new_child);
  // 要素が1つのとき
  if (ret)
    return ret;
  // 要素が無いリストの総積は0とする
  return std::shared_ptr<Number>(new Number("1"));
}

std::shared_ptr<Number> ListExpander::expand_list(
    std::shared_ptr<symbolic_expression::SizeOfList> node) {
  accept(node);
  return std::dynamic_pointer_cast<Number>(new_child);
}

node_sptr ListExpander::expand_list(
    std::shared_ptr<symbolic_expression::ExpressionListElement> node) {
  accept(node);
  node_sptr ret;
  if (new_child) {
    ret = new_child;
    new_child.reset();
  }
  return ret;
}

node_sptr ListExpander::expand_list(
    std::shared_ptr<symbolic_expression::ProgramListElement> node) {
  accept(node);
  node_sptr ret;
  if (new_child) {
    ret = new_child;
    new_child.reset();
  }
  return ret;
}

std::shared_ptr<ExpressionList> ListExpander::expand_list(
    std::shared_ptr<symbolic_expression::ConditionalExpressionList> node) {
  std::shared_ptr<ExpressionList> ret =
      std::shared_ptr<ExpressionList>(new ExpressionList());
  expand_conditional_list(ret, node->get_expression(), node, 0);
  return ret;
}
std::shared_ptr<ProgramList> ListExpander::expand_list(
    std::shared_ptr<symbolic_expression::ConditionalProgramList> node) {
  std::shared_ptr<ProgramList> ret =
      std::shared_ptr<ProgramList>(new ProgramList());
  expand_conditional_list(ret, node->get_program(), node, 0);
  return ret;
}

std::shared_ptr<VariadicNode>
ListExpander::expand_list(std::shared_ptr<symbolic_expression::Range> node) {
  bool original_in_list_element = in_list_element;
  in_list_element = true;
  std::shared_ptr<VariadicNode> ret;
  node_sptr lhs, rhs;
  accept(node->get_lhs());
  if (new_child) {
    lhs = new_child;
    new_child.reset();
  } else {
    lhs = node->get_lhs();
  }
  accept(node->get_rhs());
  if (new_child) {
    rhs = new_child;
    new_child.reset();
  } else {
    rhs = node->get_rhs();
  }
  std::shared_ptr<Number> lhs_num, rhs_num;
  lhs_num = std::dynamic_pointer_cast<Number>(lhs->clone());
  rhs_num = std::dynamic_pointer_cast<Number>(rhs->clone());
  if (lhs_num && rhs_num) {
    if (node->get_header().length() == 0) {
      ret = std::shared_ptr<ExpressionList>(new ExpressionList());
      for (int i = std::stoi(lhs_num->get_number());
           i <= std::stoi(rhs_num->get_number()); i++) {
        ret->add_argument(
            std::shared_ptr<Number>(new Number(std::to_string(i))));
      }
    } else if ('A' <= node->get_header()[0] && node->get_header()[0] <= 'Z') {
      ret = std::shared_ptr<ProgramList>(new ProgramList());
      for (int i = std::stoi(lhs_num->get_number());
           i <= std::stoi(rhs_num->get_number()); i++) {
        std::shared_ptr<ConstraintCaller> caller =
            std::shared_ptr<ConstraintCaller>(new ConstraintCaller());
        caller->set_name(node->get_header() + std::to_string(i));
        ret->add_argument(caller);
      }
    } else {
      ret = std::shared_ptr<ExpressionList>(new ExpressionList());
      for (int i = std::stoi(lhs_num->get_number());
           i <= std::stoi(rhs_num->get_number()); i++) {
        std::shared_ptr<Variable> variable = std::shared_ptr<Variable>(
            new Variable(node->get_header() + std::to_string(i)));
        ret->add_argument(variable);
      }
    }
  }
  in_list_element = original_in_list_element;
  return ret;
}

std::shared_ptr<VariadicNode>
ListExpander::expand_list(std::shared_ptr<symbolic_expression::Union> node) {
  std::shared_ptr<VariadicNode> ret;
  std::shared_ptr<ExpressionList> el;
  accept(node->get_lhs());
  if (new_child) {
    el = std::dynamic_pointer_cast<ExpressionList>(new_child->clone());
    new_child.reset();
  } else {
    el = std::dynamic_pointer_cast<ExpressionList>(node->get_lhs()->clone());
  }
  if (el) {
    accept(node->get_rhs());
    std::shared_ptr<ExpressionList> rhs;
    if (new_child) {
      rhs = std::dynamic_pointer_cast<ExpressionList>(new_child->clone());
      new_child.reset();
    } else {
      rhs = std::dynamic_pointer_cast<ExpressionList>(node->get_rhs()->clone());
    }
    bool not_in = true;
    for (int i = 0; i < rhs->get_arguments_size(); i++) {
      for (int j = 0; j < el->get_arguments_size(); j++) {
        if (el->get_argument(j)->is_same_struct(*(rhs->get_argument(i)),
                                                false)) {
          not_in = false;
          break;
        }
      }
      if (not_in) {
        el->add_argument(rhs->get_argument(i));
      }
      not_in = true;
    }
    return el;
  }

  std::shared_ptr<ProgramList> pl;
  if (new_child) {
    pl = std::dynamic_pointer_cast<ProgramList>(new_child->clone());
    new_child.reset();
  } else {
    pl = std::dynamic_pointer_cast<ProgramList>(node->get_lhs()->clone());
  }
  if (pl) {
    accept(node->get_rhs());
    std::shared_ptr<ProgramList> rhs;
    if (new_child) {
      rhs = std::dynamic_pointer_cast<ProgramList>(new_child->clone());
      new_child.reset();
    } else {
      rhs = std::dynamic_pointer_cast<ProgramList>(node->get_rhs()->clone());
    }
    bool not_in = true;
    for (int i = 0; i < rhs->get_arguments_size(); i++) {
      for (int j = 0; j < pl->get_arguments_size(); j++) {
        if (pl->get_argument(j)->is_same_struct(*(rhs->get_argument(i)),
                                                false)) {
          not_in = false;
          break;
        }
      }
      if (not_in) {
        pl->add_argument(rhs->get_argument(i));
      }
      not_in = true;
    }
    return pl;
  }
  return ret;
}

std::shared_ptr<VariadicNode> ListExpander::expand_list(
    std::shared_ptr<symbolic_expression::Intersection> node) {
  std::shared_ptr<VariadicNode> ret;
  std::shared_ptr<ExpressionList> el;
  accept(node->get_lhs());
  if (new_child) {
    el = std::dynamic_pointer_cast<ExpressionList>(new_child->clone());
    new_child.reset();
  } else {
    el = std::dynamic_pointer_cast<ExpressionList>(node->get_lhs()->clone());
  }
  if (el) {
    ret = std::shared_ptr<ExpressionList>(new ExpressionList());
    accept(node->get_rhs());
    std::shared_ptr<ExpressionList> rhs;
    if (new_child) {
      rhs = std::dynamic_pointer_cast<ExpressionList>(new_child->clone());
      new_child.reset();
    } else {
      rhs = std::dynamic_pointer_cast<ExpressionList>(node->get_rhs()->clone());
    }
    bool not_in = true;
    for (int i = 0; i < rhs->get_arguments_size(); i++) {
      for (int j = 0; j < el->get_arguments_size(); j++) {
        if (el->get_argument(j)->is_same_struct(*(rhs->get_argument(i)),
                                                false)) {
          not_in = false;
          break;
        }
      }
      if (!not_in) {
        ret->add_argument(rhs->get_argument(i));
      }
      not_in = true;
    }
    return ret;
  }

  std::shared_ptr<ProgramList> pl;
  if (new_child) {
    pl = std::dynamic_pointer_cast<ProgramList>(new_child->clone());
    new_child.reset();
  } else {
    pl = std::dynamic_pointer_cast<ProgramList>(node->get_lhs()->clone());
  }
  if (pl) {
    ret = std::shared_ptr<ProgramList>(new ProgramList());
    accept(node->get_rhs());
    std::shared_ptr<ProgramList> rhs;
    if (new_child) {
      rhs = std::dynamic_pointer_cast<ProgramList>(new_child->clone());
      new_child.reset();
    } else {
      rhs = std::dynamic_pointer_cast<ProgramList>(node->get_rhs()->clone());
    }
    bool not_in = true;
    for (int i = 0; i < rhs->get_arguments_size(); i++) {
      for (int j = 0; j < pl->get_arguments_size(); j++) {
        if (pl->get_argument(j)->is_same_struct(*(rhs->get_argument(i)),
                                                false)) {
          not_in = false;
          break;
        }
      }
      if (!not_in) {
        ret->add_argument(rhs->get_argument(i));
      }
      not_in = true;
    }
    return ret;
  }
  return ret;
}

void ListExpander::expand_conditional_list(std::shared_ptr<VariadicNode> ret,
                                           node_sptr element,
                                           std::shared_ptr<VariadicNode> list,
                                           int idx) {

  std::shared_ptr<BinaryNode> bn;
  if (list->get_arguments_size() == idx) {
    accept(element->clone());
    if (new_child) {
      ret->add_argument(new_child);
      new_child.reset();
    } else {
      ret->add_argument(element);
    }
    accept(ret);
    if (new_child)
      ret = std::dynamic_pointer_cast<VariadicNode>(new_child);
    return;
  }

  // DifferentVariable
  bn = std::dynamic_pointer_cast<DifferentVariable>(
      list->get_argument(idx)->clone());
  if (bn) {
    node_sptr lhs, rhs;
    accept(bn->get_lhs());
    if (new_child) {
      lhs = new_child;
      new_child.reset();
    } else {
      lhs = bn->get_lhs();
    }
    accept(bn->get_rhs());
    if (new_child) {
      rhs = new_child;
      new_child.reset();
    } else {
      rhs = bn->get_rhs();
    }
    if (lhs->get_string() != rhs->get_string()) {
      expand_conditional_list(ret, element, list, idx + 1);
      return;
    }
    return;
  }

  // EachElement
  bn = std::dynamic_pointer_cast<EachElement>(list->get_argument(idx)->clone());
  if (bn) {
    node_sptr local_var = bn->get_lhs();
    std::shared_ptr<VariadicNode> each_list;
    accept(bn->get_rhs());
    if (new_child) {
      each_list = std::dynamic_pointer_cast<ExpressionList>(new_child->clone());
      new_child.reset();
    } else {
      each_list =
          std::dynamic_pointer_cast<ExpressionList>(bn->get_rhs()->clone());
    }
    if (!each_list) {
      if (new_child) {
        each_list = std::dynamic_pointer_cast<ProgramList>(new_child->clone());
        new_child.reset();
      } else {
        each_list =
            std::dynamic_pointer_cast<ProgramList>(bn->get_rhs()->clone());
      }
    }
    if (each_list) {
      // list
      node_sptr var = bn->get_lhs();
      for (int i = 0; i < each_list->get_arguments_size(); i++) {
        local_variable_map[var] = each_list->get_argument(i);
        expand_conditional_list(ret, element, list, idx + 1);
      }
      local_variable_map.erase(var);
    }
  }
}

void ListExpander::visit(std::shared_ptr<symbolic_expression::Weaker> node) {
  accept(node->get_lhs());
  if (new_child) {
    node->set_lhs(new_child);
    new_child.reset();
  }
  accept(node->get_rhs());
  if (new_child) {
    node->set_rhs(new_child);
    new_child.reset();
  }
  new_child = node;
}
void ListExpander::visit(std::shared_ptr<symbolic_expression::Parallel> node) {
  accept(node->get_lhs());
  if (new_child) {
    node->set_lhs(new_child);
    new_child.reset();
  }
  accept(node->get_rhs());
  if (new_child) {
    node->set_rhs(new_child);
    new_child.reset();
  }
  new_child = node;
}
void ListExpander::visit(
    std::shared_ptr<symbolic_expression::ExpressionList> node) {
  new_child = node;
}
void ListExpander::visit(
    std::shared_ptr<symbolic_expression::ProgramList> node) {
  new_child = node;
}
void ListExpander::visit(
    std::shared_ptr<symbolic_expression::ConditionalExpressionList> node) {
  new_child = expand_list(node);
}
void ListExpander::visit(
    std::shared_ptr<symbolic_expression::ConditionalProgramList> node) {
  new_child = expand_list(node);
}
void ListExpander::visit(std::shared_ptr<symbolic_expression::Union> node) {
  new_child = expand_list(node);
}
void ListExpander::visit(
    std::shared_ptr<symbolic_expression::Intersection> node) {
  new_child = expand_list(node);
}
void ListExpander::visit(std::shared_ptr<symbolic_expression::Range> node) {
  new_child = expand_list(node);
}

void ListExpander::visit(std::shared_ptr<symbolic_expression::Number> node) {
  new_child = node->clone();
}

void ListExpander::visit(
    std::shared_ptr<symbolic_expression::ImaginaryUnit> node) {
  new_child = node->clone();
}

void ListExpander::visit(std::shared_ptr<symbolic_expression::Variable> node) {
  for (auto map : local_variable_map) {
    if (node->is_same_struct(*(map.first), true)) {
      new_child = map.second;
      return;
    }
  }
  new_child = node;
}

void ListExpander::visit(
    std::shared_ptr<symbolic_expression::ProgramCaller> node) {
  std::shared_ptr<ProgramCaller> caller =
      std::shared_ptr<ProgramCaller>(new ProgramCaller());
  for (auto map : local_variable_map) {
    if (node->is_same_struct(*(map.first), true)) {
      node = std::dynamic_pointer_cast<ProgramCaller>(map.second->clone());
      break;
    }
  }
  caller->set_name(node->get_name());
  for (int i = 0; i < node->actual_arg_size(); i++) {
    accept(node->get_actual_arg(i)->clone());
    if (new_child) {
      caller->add_actual_arg(new_child);
      new_child.reset();
    } else
      caller->add_actual_arg(node->get_actual_arg(i)->clone());
  }
  new_child = caller;
}

void ListExpander::visit(
    std::shared_ptr<symbolic_expression::ConstraintCaller> node) {
  std::shared_ptr<ConstraintCaller> caller =
      std::shared_ptr<ConstraintCaller>(new ConstraintCaller());
  for (auto map : local_variable_map) {
    if (node->is_same_struct(*(map.first), true)) {
      node = std::dynamic_pointer_cast<ConstraintCaller>(map.second);
      break;
    }
  }
  caller->set_name(node->get_name());
  for (int i = 0; i < node->actual_arg_size(); i++) {
    accept(node->get_actual_arg(i)->clone());
    if (new_child) {
      caller->add_actual_arg(new_child);
      new_child.reset();
    } else
      caller->add_actual_arg(node->get_actual_arg(i)->clone());
  }
  new_child = caller;
}

void ListExpander::visit(std::shared_ptr<symbolic_expression::Plus> node) {
  std::shared_ptr<Number> lhs, rhs;
  accept(node->get_lhs());
  if (new_child) {
    node->set_lhs(new_child->clone());
    new_child.reset();
  }
  accept(node->get_rhs());
  if (new_child) {
    node->set_rhs(new_child->clone());
    new_child.reset();
  }
  if (!in_list_element) {
    new_child = node;
    return;
  }
  lhs = std::dynamic_pointer_cast<Number>(node->get_lhs()->clone());
  rhs = std::dynamic_pointer_cast<Number>(node->get_rhs()->clone());
  if (lhs && rhs) {
    int num = std::stoi(lhs->get_number()) + std::stoi(rhs->get_number());
    std::string str = std::to_string(num);
    new_child = std::shared_ptr<Number>(new Number(str));
  } else
    new_child = node;
}

void ListExpander::visit(std::shared_ptr<symbolic_expression::Subtract> node) {
  std::shared_ptr<Number> lhs, rhs;
  accept(node->get_lhs());
  if (new_child) {
    node->set_lhs(new_child->clone());
    new_child.reset();
  }
  accept(node->get_rhs());
  if (new_child) {
    node->set_rhs(new_child->clone());
    new_child.reset();
  }
  if (!in_list_element) {
    new_child = node;
    return;
  }
  lhs = std::dynamic_pointer_cast<Number>(node->get_lhs()->clone());
  rhs = std::dynamic_pointer_cast<Number>(node->get_rhs()->clone());
  if (lhs && rhs) {
    int num = std::stoi(lhs->get_number()) - std::stoi(rhs->get_number());
    std::string str = std::to_string(num);
    new_child = std::shared_ptr<Number>(new Number(str));
  } else
    new_child = node;
}

void ListExpander::visit(std::shared_ptr<symbolic_expression::Times> node) {
  std::shared_ptr<Number> lhs, rhs;
  accept(node->get_lhs());
  if (new_child) {
    node->set_lhs(new_child->clone());
    new_child.reset();
  }
  accept(node->get_rhs());
  if (new_child) {
    node->set_rhs(new_child->clone());
    new_child.reset();
  }
  if (!in_list_element) {
    new_child = node;
    return;
  }
  lhs = std::dynamic_pointer_cast<Number>(node->get_lhs()->clone());
  rhs = std::dynamic_pointer_cast<Number>(node->get_rhs()->clone());
  if (lhs && rhs) {
    int num = std::stoi(lhs->get_number()) * std::stoi(rhs->get_number());
    std::string str = std::to_string(num);
    new_child = std::shared_ptr<Number>(new Number(str));
  } else
    new_child = node;
}

/// リストの添字に割り算がある場合例外を返す
void ListExpander::visit(std::shared_ptr<symbolic_expression::Divide> node) {
  accept(node->get_lhs());
  if (new_child) {
    node->set_lhs(new_child->clone());
    new_child.reset();
  }
  accept(node->get_rhs());
  if (new_child) {
    node->set_rhs(new_child->clone());
    new_child.reset();
  }
  if (!in_list_element) {
    new_child = node;
    return;
  }
  throw error::InvalidIndex(node);
}

void ListExpander::visit(std::shared_ptr<symbolic_expression::Power> node) {
  std::shared_ptr<Number> lhs, rhs;
  accept(node->get_lhs());
  if (new_child) {
    node->set_lhs(new_child->clone());
    new_child.reset();
  }
  accept(node->get_rhs());
  if (new_child) {
    node->set_rhs(new_child->clone());
    new_child.reset();
  }
  if (!in_list_element) {
    new_child = node;
    return;
  }
  lhs = std::dynamic_pointer_cast<Number>(node->get_lhs()->clone());
  rhs = std::dynamic_pointer_cast<Number>(node->get_rhs()->clone());
  if (lhs && rhs) {
    if (std::stoi(rhs->get_number()) < 0)
      throw error::InvalidIndex(node);
    int num =
        std::pow(std::stoi(lhs->get_number()), std::stoi(rhs->get_number()));
    std::string str = std::to_string(num);
    new_child = std::shared_ptr<Number>(new Number(str));
  } else
    new_child = node;
}

void ListExpander::visit(std::shared_ptr<symbolic_expression::Positive> node) {
  std::shared_ptr<Number> child;
  accept(node->get_child());
  if (new_child) {
    node->set_child(new_child->clone());
    new_child.reset();
  }
  if (!in_list_element) {
    new_child = node;
    return;
  }
  child = std::dynamic_pointer_cast<Number>(node->get_child()->clone());
  if (child) {
    int num = std::stoi(child->get_number());
    std::string str = std::to_string(num);
    new_child = std::shared_ptr<Number>(new Number(str));
  } else
    new_child = node;
}

void ListExpander::visit(std::shared_ptr<symbolic_expression::Negative> node) {
  std::shared_ptr<Number> child;
  accept(node->get_child());
  if (new_child) {
    node->set_child(new_child->clone());
    new_child.reset();
  }
  if (!in_list_element) {
    new_child = node;
    return;
  }
  child = std::dynamic_pointer_cast<Number>(node->get_child()->clone());
  if (child) {
    int num = std::stoi(child->get_number());
    std::string str = std::to_string(-num);
    new_child = std::shared_ptr<Number>(new Number(str));
  } else
    new_child = node;
}

void ListExpander::visit(
    std::shared_ptr<symbolic_expression::ExpressionListElement> node) {
  bool original_in_list_element = in_list_element;
  in_list_element = true;
  std::shared_ptr<ExpressionList> list;
  std::shared_ptr<Number> num;
  accept(node->get_lhs()->clone());
  if (new_child) {
    list = std::dynamic_pointer_cast<ExpressionList>(new_child->clone());
    new_child.reset();
  } else
    list = std::dynamic_pointer_cast<ExpressionList>(node->get_lhs()->clone());
  accept(node->get_rhs()->clone());
  if (new_child) {
    num = std::dynamic_pointer_cast<Number>(new_child->clone());
    new_child.reset();
  } else
    num = std::dynamic_pointer_cast<Number>(node->get_rhs()->clone());
  if (list && num) {
    int idx = std::stoi(num->get_number());
    if (idx <= 0 || idx > list->get_arguments_size())
      throw error::InvalidIndex(node);
    if ((list->get_nameless_expression_arguments())) {
      node->set_rhs(num->clone());
      accept(list->get_nameless_expression_arguments());
      num = std::dynamic_pointer_cast<Number>(new_child->clone());
      if (num) {
        list->set_nameless_arguments(std::stoi(num->get_number()));
      }
      node->set_lhs(list);
      new_child = node;
    } else
      new_child = list->get_argument(idx - 1);
  } else {
    if (!num)
      throw error::InvalidIndex(node);
    new_child = node;
  }
  in_list_element = original_in_list_element;
}

void ListExpander::visit(
    std::shared_ptr<symbolic_expression::ProgramListElement> node) {
  in_list_element = true;
  std::shared_ptr<VariadicNode> list;
  std::shared_ptr<Number> num;
  accept(node->get_lhs()->clone());
  if (new_child) {
    list = std::dynamic_pointer_cast<VariadicNode>(new_child->clone());
    new_child.reset();
  } else
    list = std::dynamic_pointer_cast<VariadicNode>(node->get_lhs()->clone());
  accept(node->get_rhs()->clone());
  if (new_child) {
    num = std::dynamic_pointer_cast<Number>(new_child->clone());
    new_child.reset();
  } else
    num = std::dynamic_pointer_cast<Number>(node->get_rhs()->clone());
  if (list && num) {
    int idx = std::stoi(num->get_number());
    new_child = list->get_argument(idx - 1);
  }
  in_list_element = false;
}

void ListExpander::visit(
    std::shared_ptr<symbolic_expression::ProgramListCaller> node) {
  referenced_definition_t deftype(
      std::make_pair(node->get_name(), node->actual_arg_size()));
  std::shared_ptr<ProgramListDefinition> cons_def(
      program_list_def.get_definition(deftype));
  if (!cons_def) {
    throw hydla::parser::error::UndefinedReference(node);
  }
  called_program_list_definition.insert(cons_def);

  new_child = circular_check(deftype, cons_def);
  accept(new_child);
}

void ListExpander::visit(
    std::shared_ptr<symbolic_expression::ExpressionListCaller> node) {
  referenced_definition_t deftype(
      std::make_pair(node->get_name(), node->actual_arg_size()));
  std::shared_ptr<ExpressionListDefinition> cons_def(
      expression_list_def.get_definition(deftype));
  if (!cons_def) {
    throw hydla::parser::error::UndefinedReference(node);
  }

  called_expression_list_definition.insert(cons_def);

  new_child = circular_check(deftype, cons_def);
  accept(new_child);

  std::shared_ptr<ExpressionList> el =
      std::dynamic_pointer_cast<ExpressionList>(new_child->clone());
  if (el) {
    el->set_list_name(node->get_name());
    new_child = el;
  }
}

void ListExpander::visit(
    std::shared_ptr<symbolic_expression::SizeOfList> node) {
  accept(node->get_child());
  std::shared_ptr<VariadicNode> list;
  if (new_child) {
    std::shared_ptr<ExpressionList> el =
        std::dynamic_pointer_cast<ExpressionList>(new_child);
    if (el) {
      if ((el->get_nameless_expression_arguments())) {
        bool tmp_in_list = in_list_element;
        in_list_element = true;
        accept(el->get_nameless_expression_arguments());
        in_list_element = tmp_in_list;
        std::shared_ptr<Number> num =
            std::dynamic_pointer_cast<Number>(new_child);
        if (num)
          return;
      }
    }
    list = std::dynamic_pointer_cast<VariadicNode>(new_child->clone());
    new_child.reset();
  } else {
    list = std::dynamic_pointer_cast<VariadicNode>(node->get_child()->clone());
  }
  new_child = std::shared_ptr<Number>(
      new Number(std::to_string(list->get_arguments_size())));
}

void ListExpander::visit(std::shared_ptr<symbolic_expression::SumOfList> node) {
  node_sptr ret;
  accept(node->get_child());
  std::shared_ptr<ExpressionList> list;
  if (new_child) {
    list = std::dynamic_pointer_cast<ExpressionList>(new_child->clone());
    new_child.reset();
  } else {
    list =
        std::dynamic_pointer_cast<ExpressionList>(node->get_child()->clone());
  }
  for (int i = 0; i < list->get_arguments_size(); i++) {
    if (ret) {
      ret = std::shared_ptr<Plus>(new Plus(ret, list->get_argument(i)));
    } else {
      ret = list->get_argument(i);
    }
  }
  new_child = ret;
}

void ListExpander::visit(std::shared_ptr<symbolic_expression::MulOfList> node) {
  node_sptr ret;
  accept(node->get_child());
  std::shared_ptr<ExpressionList> list;
  if (new_child) {
    list = std::dynamic_pointer_cast<ExpressionList>(new_child->clone());
    new_child.reset();
  } else {
    list =
        std::dynamic_pointer_cast<ExpressionList>(node->get_child()->clone());
  }
  for (int i = 0; i < list->get_arguments_size(); i++) {
    if (ret) {
      ret = std::shared_ptr<Times>(new Times(ret, list->get_argument(i)));
    } else {
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
// DEFINE_UNARY_VISIT(Negative)
DEFINE_UNARY_VISIT(Tell)
DEFINE_UNARY_VISIT(Differential)
DEFINE_UNARY_VISIT(Constraint)
// DEFINE_UNARY_VISIT(Positive)
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
DEFINE_BINARY_VISIT(Exists)
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

set<std::shared_ptr<ExpressionListDefinition>>
ListExpander::get_called_expression_list_definition() {
  return called_expression_list_definition;
}
set<std::shared_ptr<ProgramListDefinition>>
ListExpander::get_called_program_list_definition() {
  return called_program_list_definition;
}

} // namespace parser
} // namespace hydla
