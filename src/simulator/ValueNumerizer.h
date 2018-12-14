#pragma once

#include <boost/optional.hpp>
#include "Node.h"
#include "DefaultTreeVisitor.h"

#include "Value.h"

namespace hydla {
namespace simulator {

class ValueNumerizer : public symbolic_expression::DefaultTreeVisitor
{
public:
  ValueNumerizer();

  /**
    numerize given expression
    @parameter exp expression to numerize(for input and output)
  */
  void numerize(Value &exp);
  
  virtual void visit(boost::shared_ptr<symbolic_expression::Plus> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Subtract> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Times> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Divide> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Power> node);

  virtual void visit(boost::shared_ptr<symbolic_expression::Number> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Float> node);

  virtual void visit(boost::shared_ptr<symbolic_expression::Parameter> node);

  virtual void visit(boost::shared_ptr<symbolic_expression::SymbolicT> node);

  virtual void visit(boost::shared_ptr<symbolic_expression::E> node);
  
  virtual void visit(boost::shared_ptr<symbolic_expression::Pi> node);

  virtual void visit(boost::shared_ptr<symbolic_expression::Negative> node);
  
  virtual void visit(boost::shared_ptr<symbolic_expression::Function> node);

  boost::optional<double> get_double()const
  {
    if(fully_numerized)
    {
      return current_double;
    }
    return boost::none;
  }

private:
  void invalid_node(symbolic_expression::Node &node);
  
  Value current_value;
  double current_double;
  bool fully_numerized = false;
};

} // namespace simulator
} // namespace hydla
