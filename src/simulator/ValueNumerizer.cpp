#include "ValueNumerizer.h"
#include "HydLaError.h"
#include "Logger.h"
#include <cmath>
#include <exception>

using namespace hydla::symbolic_expression;
using namespace std;

namespace hydla {
namespace simulator {

ValueNumerizer::ValueNumerizer() {}

void ValueNumerizer::numerize(Value &val) {
  fully_numerized = true;
  accept(val.get_node());
  if (fully_numerized)
    val = Value(current_double);
  else
    val = current_value;
}

void ValueNumerizer::visit(std::shared_ptr<symbolic_expression::Plus> node) {
  accept(node->get_lhs());
  Value lhs = current_value;
  bool lhs_numerized = fully_numerized;
  double lhs_double = current_double;
  fully_numerized = true;
  accept(node->get_rhs());
  Value rhs = current_value;
  double rhs_double = current_double;
  bool rhs_numerized = fully_numerized;

  if (lhs_numerized && rhs_numerized) {
    current_double = lhs_double + rhs_double;
  } else {
    if (lhs_numerized) {
      current_value = Value(lhs_double);
      current_value += rhs;
    } else if (rhs_numerized) {
      current_value = lhs;
      current_value += Value(rhs_double);
    } else
      current_value = lhs + rhs;
    fully_numerized = false;
  }
  return;
}

void ValueNumerizer::visit(std::shared_ptr<symbolic_expression::Subtract> node) {
  accept(node->get_lhs());
  Value lhs = current_value;
  bool lhs_numerized = fully_numerized;
  double lhs_double = current_double;
  fully_numerized = true;
  accept(node->get_rhs());
  Value rhs = current_value;
  double rhs_double = current_double;
  bool rhs_numerized = fully_numerized;

  if (lhs_numerized && rhs_numerized) {
    current_double = lhs_double - rhs_double;
  } else {
    if (lhs_numerized) {
      current_value = Value(lhs_double);
      current_value -= rhs;
    } else if (rhs_numerized) {
      current_value = lhs;
      current_value -= Value(rhs_double);
    } else
      current_value = lhs - rhs;
    fully_numerized = false;
  }
  return;
}

void ValueNumerizer::visit(std::shared_ptr<symbolic_expression::Times> node) {

  accept(node->get_lhs());
  Value lhs = current_value;
  bool lhs_numerized = fully_numerized;
  double lhs_double = current_double;
  fully_numerized = true;
  accept(node->get_rhs());
  Value rhs = current_value;
  double rhs_double = current_double;
  bool rhs_numerized = fully_numerized;

  if (lhs_numerized && rhs_numerized) {
    current_double = lhs_double * rhs_double;
  } else {
    if (lhs_numerized) {
      current_value = Value(lhs_double);
      current_value *= rhs;
    } else if (rhs_numerized) {
      current_value = lhs;
      current_value *= Value(rhs_double);
    } else
      current_value = lhs * rhs;
    fully_numerized = false;
  }
  return;
}

void ValueNumerizer::visit(std::shared_ptr<symbolic_expression::Divide> node) {
  accept(node->get_lhs());
  Value lhs = current_value;
  bool lhs_numerized = fully_numerized;
  double lhs_double = current_double;
  fully_numerized = true;
  accept(node->get_rhs());
  Value rhs = current_value;
  double rhs_double = current_double;
  bool rhs_numerized = fully_numerized;
  if (lhs_numerized && rhs_numerized) {
    current_double = lhs_double / rhs_double;
  } else {
    if (lhs_numerized) {
      current_value = Value(lhs_double);
      current_value /= rhs;
    } else if (rhs_numerized) {
      current_value = lhs;
      current_value /= Value(rhs_double);
    } else
      current_value = lhs / rhs;
    fully_numerized = false;
  }
  return;
}

void ValueNumerizer::visit(std::shared_ptr<symbolic_expression::Power> node) {
  accept(node->get_lhs());
  Value lhs = current_value;
  bool lhs_numerized = fully_numerized;
  double lhs_double = current_double;
  fully_numerized = true;
  accept(node->get_rhs());
  Value rhs = current_value;
  double rhs_double = current_double;
  bool rhs_numerized = fully_numerized;

  if (lhs_numerized && rhs_numerized) {
    current_double = pow(lhs_double, rhs_double);
  } else {
    if (lhs_numerized) {
      current_value = Value(lhs_double);
      current_value ^= rhs;
    } else if (rhs_numerized) {
      current_value = lhs;
      current_value ^= Value(rhs_double);
    } else
      current_value = lhs ^ rhs;
    fully_numerized = false;
  }
  return;
}

void ValueNumerizer::visit(std::shared_ptr<symbolic_expression::Negative> node) {

  accept(node->get_child());
  if (!fully_numerized)
    current_value = -current_value;
  else
    current_double = -current_double;
}

void ValueNumerizer::visit(std::shared_ptr<symbolic_expression::Pi> node) {
  current_value = Value(M_PI);
  return;
}

void ValueNumerizer::visit(std::shared_ptr<symbolic_expression::E> node) {
  current_value = Value(M_E);
  return;
}

void ValueNumerizer::visit(std::shared_ptr<symbolic_expression::Number> node) {
  current_double = atof(node->get_number().c_str());
  return;
}

void ValueNumerizer::visit(std::shared_ptr<symbolic_expression::Float> node) {
  current_double = node->get_number();
  return;
}

void ValueNumerizer::visit(std::shared_ptr<symbolic_expression::Function> node) {
  std::string name = node->get_name();
  if (name == "sin") {
    if (node->get_arguments_size() != 1) {
      invalid_node(*node);
    }
    accept(node->get_argument(0));
    if (fully_numerized)
      current_double = sin(current_double);
    else {
      shared_ptr<Function> new_node(new Function(name));
      new_node->add_argument(current_value.get_node());
      current_value = Value(new_node);
    }
  } else if (name == "cos") {
    if (node->get_arguments_size() != 1) {
      invalid_node(*node);
    }
    accept(node->get_argument(0));
    if (fully_numerized)
      current_double = cos(current_double);
    else {
      shared_ptr<Function> new_node(new Function(name));
      new_node->add_argument(current_value.get_node());
      current_value = Value(new_node);
    }
  } else if (name == "tan") {
    if (node->get_arguments_size() != 1) {
      invalid_node(*node);
    }
    accept(node->get_argument(0));
    if (fully_numerized)
      current_double = tan(current_double);
    else {
      shared_ptr<Function> new_node(new Function(name));
      new_node->add_argument(current_value.get_node());
      current_value = Value(new_node);
    }
  } else if (name == "log") {
    if (node->get_arguments_size() != 1) {
      invalid_node(*node);
    }
    accept(node->get_argument(0));
    if (fully_numerized)
      current_double = log(current_double);
    else {
      shared_ptr<Function> new_node(new Function(name));
      new_node->add_argument(current_value.get_node());
      current_value = Value(new_node);
    }
  } else {
    invalid_node(*node);
  }
  return;
}

void ValueNumerizer::visit(std::shared_ptr<symbolic_expression::SymbolicT> node) {
  current_value = Value(node);
  fully_numerized = false;
  return;
}

void ValueNumerizer::visit(std::shared_ptr<symbolic_expression::Parameter> node) {
  current_value = Value(node);
  fully_numerized = false;
}

void ValueNumerizer::invalid_node(Node &node) {
  throw HYDLA_ERROR("invalid node: " + node.get_string());
}

} // namespace simulator
} // namespace hydla
