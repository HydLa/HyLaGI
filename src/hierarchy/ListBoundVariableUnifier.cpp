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

void ListBoundVariableUnifier::set_prefix(std::string str)
{
  prefix_ = str;
}

void ListBoundVariableUnifier::reset_prefix()
{
  prefix_ = "";
}

void ListBoundVariableUnifier::apply_change(symbolic_expression::node_sptr node)
{
  applying_ = true;
  accept(node);
  applying_ = false;
}

void ListBoundVariableUnifier::visit(boost::shared_ptr<symbolic_expression::Variable> node)
{
  std::string name = node->get_name();
  auto it = replace_name_map_.find(name);
  if(it != replace_name_map_.end())
  {
    node->set_name(it->second.substr(0,1)+prefix_+it->second.substr(1));
    return;
  }
  if(applying_) return;
  if(name.substr(0,3) == "$BV")
  {
    replace_name_map_[name] = "$UV"+std::to_string(bv_count_);
    bv_count_++;
    node->set_name(replace_name_map_[name].substr(0,1)+prefix_+replace_name_map_[name].substr(1));
  }
}

std::ostream& ListBoundVariableUnifier::dump(std::ostream& s) const
{
  for(auto m : replace_name_map_)
  {
    s << m.first << " -> " << m.second.substr(0,1) << prefix_ << m.second.substr(1) << std::endl;
  }
  return s;
}


}// hierarchy
}// hydla
