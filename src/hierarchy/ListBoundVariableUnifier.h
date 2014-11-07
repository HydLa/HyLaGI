#pragma once

#include <map>
#include <string>

#include "DefaultTreeVisitor.h"

namespace hydla{
namespace hierarchy{

class ListBoundVariableUnifier : public symbolic_expression::DefaultTreeVisitor
{
public:
  ListBoundVariableUnifier();
  ~ListBoundVariableUnifier();

  void unify(symbolic_expression::node_sptr node);
  void apply_change(symbolic_expression::node_sptr node);

  virtual void visit(boost::shared_ptr<symbolic_expression::Variable> node);
private:
  int bv_count_ = 0;
  bool applying = false;
  std::map<std::string, std::string> replace_name_map_;
};


} // hierarchy
} // hydla
