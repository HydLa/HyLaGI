#include "IntervalTreeVisitor.h"
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
    std::runtime_error("error occured in interval caluculation: " + msg){}
};


IntervalTreeVisitor::IntervalTreeVisitor(itvd arg, parameter_map_t& map) : phase_map_(map)
{
  interval_value_ = itvd(0.,0.);
  interval_arg_ = arg;
}


itvd IntervalTreeVisitor::get_interval_value(const node_sptr& node)
{
  accept(node);
  return interval_value_;
}


void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Plus> node)
{
  accept(node->get_lhs());
  itvd lhs = interval_value_;
  accept(node->get_rhs());
  itvd rhs = interval_value_;
  interval_value_ = lhs + rhs;
  // debug_print("Plus : ", interval_value_);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Subtract> node)
{
  accept(node->get_lhs());
  itvd lhs = interval_value_;
  accept(node->get_rhs());
  itvd rhs = interval_value_;
  interval_value_ = lhs - rhs;
  // debug_print("Subtract : ", interval_value_);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Times> node)
{
  accept(node->get_lhs());
  itvd lhs = interval_value_;
  accept(node->get_rhs());
  itvd rhs = interval_value_;
  interval_value_ = lhs * rhs;
  // debug_print("Times : ", interval_value_);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Divide> node)
{
  accept(node->get_lhs());
  itvd lhs = interval_value_;
  accept(node->get_rhs());
  itvd rhs = interval_value_;
  // 怪しいことが起こる気がする
  // 例えば、0 を含む区間で何かを除算すると...
  interval_value_ = lhs / rhs;
  // debug_print("Divide : ", interval_value_);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Power> node)
{
  accept(node->get_lhs());
  itvd lhs = interval_value_;
  accept(node->get_rhs());
  itvd rhs = interval_value_;
  // これでいいのかわからないがとりあえず
  interval_value_ = pow(lhs, rhs);
  // debug_print("Power : ", interval_value_);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Negative> node)
{
  accept(node->get_child());
  interval_value_ = -interval_value_;
}


void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Pi> node)
{
  interval_value_ = pi;
  // debug_print("Pi : ", interval_value_);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::E> node)
{
  interval_value_ = e;
  // debug_print("E : ", interval_value_);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Number> node)
{
  interval_value_ = itvd(node->get_number());
  // debug_print("Number : ", interval_value_);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Float> node)
{
  interval_value_ = itvd(node->get_number());
  // debug_print("Float : ", interval_value_);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Function> node)
{
  std::string name = node->get_string();
  if(name == "sin")
  {
    if(node->get_arguments_size() != 1)
    {
      invalid_node(*node);
    }
    accept(node->get_argument(0));
    interval_value_ = sin(interval_value_);
    // debug_print("Sin : ", interval_value_);
  }
  else if(name == "cos")
  {
    if(node->get_arguments_size() != 1)
    {
      invalid_node(*node); 
    }
    accept(node->get_argument(0));
    interval_value_ = cos(interval_value_);
    // debug_print("Cos : ", interval_value_);
  }
  else if(name == "tan")
  {
    if(node->get_arguments_size() != 1)
    {
      invalid_node(*node);
    }
    accept(node->get_argument(0));
    interval_value_ = tan(interval_value_);
  }
  else if(name == "log")
  {
    if(node->get_arguments_size() != 1)
    {
      invalid_node(*node);
    }
    accept(node->get_argument(0));
    interval_value_ = log(interval_value_);
  }
  else
  {
    invalid_node(*node);
  }
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::SymbolicT> node)
{
  interval_value_ = interval_arg_;
  // debug_print("SymbolicT : ", interval_value_);
  return;
}

void IntervalTreeVisitor::visit(boost::shared_ptr<hydla::symbolic_expression::Parameter> node)
{
  parameter_t param(node->get_name(),
                    node->get_differential_count(),
                    node->get_phase_id());
  range_t range = phase_map_[param];

  value_t lower_value = (range.get_lower_bound()).value;
  accept(lower_value.get_node());
  itvd lower_itvd = interval_value_;

  value_t uppper_value = (range.get_upper_bound()).value;
  accept(uppper_value.get_node());
  itvd upper_itvd = interval_value_;

  interval_value_ = itvd(lower_itvd.lower(), upper_itvd.upper());
  debug_print("Parameter : ", interval_value_);
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
