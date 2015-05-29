#include "IntervalTreeVisitor.h"
#include "Logger.h"
#include <exception>

namespace hydla
{
namespace interval
{

itvd IntervalTreeVisitor::pi = kv::constants<itvd>::pi();
itvd IntervalTreeVisitor::e = kv::constants<itvd>::e();

class IntervalException : public std::runtime_error
{
  public:
  IntervalException(const std::string& msg):
    std::runtime_error("error occured in interval calculation: " + msg){}
};


IntervalTreeVisitor::IntervalTreeVisitor()
{
}


itvd IntervalTreeVisitor::get_interval_value(const node_sptr& node, itvd *t, parameter_map_t *map)
{
  time_interval = t;
  parameter_map = map;
  accept(node);
  return interval_value;
}


void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Plus> node)
{
  accept(node->get_lhs());
  itvd lhs = interval_value;
  accept(node->get_rhs());
  itvd rhs = interval_value;
  interval_value = lhs + rhs;
  // debug_print("Plus : ", interval_value);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Subtract> node)
{
  accept(node->get_lhs());
  itvd lhs = interval_value;
  accept(node->get_rhs());
  itvd rhs = interval_value;
  interval_value = lhs - rhs;
  // debug_print("Subtract : ", interval_value);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Times> node)
{
  accept(node->get_lhs());
  itvd lhs = interval_value;
  accept(node->get_rhs());
  itvd rhs = interval_value;
  interval_value = lhs * rhs;
  // debug_print("Times : ", interval_value);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Divide> node)
{
  accept(node->get_lhs());
  itvd lhs = interval_value;
  accept(node->get_rhs());
  itvd rhs = interval_value;
  interval_value = lhs / rhs;
  // debug_print("Divide : ", interval_value);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Power> node)
{
  accept(node->get_lhs());
  itvd lhs = interval_value;
  accept(node->get_rhs());
  itvd rhs = interval_value;
  interval_value = pow(lhs, rhs);
  // debug_print("Power : ", interval_value);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Negative> node)
{
  accept(node->get_child());
  interval_value = -interval_value;
}


void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Pi> node)
{
  interval_value = pi;
  // debug_print("Pi : ", interval_value);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::E> node)
{
  interval_value = e;
  // debug_print("E : ", interval_value);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Number> node)
{
  interval_value = itvd(node->get_number());
  // debug_print("Number : ", interval_value);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Float> node)
{
  interval_value = itvd(node->get_number());
  // debug_print("Float : ", interval_value);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Function> node)
{
  std::string name = node->get_name();
  if(name == "Sin")
  {
    if(node->get_arguments_size() != 1)
    {
      invalid_node(*node);
    }
    accept(node->get_argument(0));
    interval_value = sin(interval_value);
    // debug_print("Sin : ", interval_value);
  }
  else if(name == "Cos")
  {
    if(node->get_arguments_size() != 1)
    {
      invalid_node(*node); 
    }
    accept(node->get_argument(0));
    interval_value = cos(interval_value);
    // debug_print("Cos : ", interval_value);
  }
  else if(name == "Tan")
  {
    if(node->get_arguments_size() != 1)
    {
      invalid_node(*node);
    }
    accept(node->get_argument(0));
    interval_value = tan(interval_value);
  }
  else if(name == "Log")
  {
    if(node->get_arguments_size() != 1)
    {
      invalid_node(*node);
    }
    accept(node->get_argument(0));
    interval_value = log(interval_value);
  }
  else
  {
    invalid_node(*node);
  }
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::SymbolicT> node)
{
  if(time_interval == nullptr)invalid_node(*node);
  interval_value = *time_interval;
  // debug_print("SymbolicT : ", interval_value);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Parameter> node)
{
  if(parameter_map == nullptr)invalid_node(*node);
  parameter_t param(node->get_name(),
                    node->get_differential_count(),
                    node->get_phase_id());
  auto param_it = parameter_map->find(param);
  if(param_it == parameter_map->end())invalid_node(*node);

  range_t range = param_it->second;;

  value_t lower_value = (range.get_lower_bound()).value;
  accept(lower_value.get_node());
  itvd lower_itvd = interval_value;

  value_t uppper_value = (range.get_upper_bound()).value;
  accept(uppper_value.get_node());
  itvd upper_itvd = interval_value;

  interval_value = itvd(lower_itvd.lower(), upper_itvd.upper());
}


void IntervalTreeVisitor::invalid_node(symbolic_expression::Node &node)
{
  throw IntervalException("invalid node: " + node.get_string());
}

void IntervalTreeVisitor::debug_print(std::string str, itvd x)
{
  std::cout << str << x << "\n";
}


} // namespace interval
} // namespace hydla
