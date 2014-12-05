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
  void set_prefix(std::string);
  void reset_prefix();

  std::vector<std::string> get_list_variables();

  virtual void visit(boost::shared_ptr<symbolic_expression::Variable> node);

  virtual std::ostream& dump(std::ostream& s) const;
private:
  int bv_count_ = 0;
  bool applying_ = false;
  std::string prefix_ = "";
  std::map<std::string, std::string> replace_name_map_;
};


} // hierarchy
} // hydla
