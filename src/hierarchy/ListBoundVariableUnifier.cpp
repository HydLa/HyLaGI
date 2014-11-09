#include "ListBoundVariableUnifier.h"

namespace hydla{
namespace hierarchy{

ListBoundVariableUnifier::ListBoundVariableUnifier(){}
ListBoundVariableUnifier::~ListBoundVariableUnifier(){}

void ListBoundVariableUnifier::unify(symbolic_expression::node_sptr node)
{
  replace_name_map_.clear();
  accept(node);
}

void ListBoundVariableUnifier::apply_change(symbolic_expression::node_sptr node)
{
  applying = true;
  accept(node);
  applying = false;
}

void ListBoundVariableUnifier::visit(boost::shared_ptr<symbolic_expression::Variable> node)
{
  std::string name = node->get_name();
  auto it = replace_name_map_.find(name);
  if(it != replace_name_map_.end())
  {
    node->set_name(it->second);
    return;
  }
  if(applying) return;
  if(name.substr(0,3) == "$BV")
  {
    replace_name_map_[name] = "$UV"+std::to_string(bv_count_);
    bv_count_++;
    node->set_name(replace_name_map_[name]);
  }
}

}// hierarchy
}// hydla
