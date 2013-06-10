#include "ParameterReplacer.h"
#include "SymbolicValue.h"
#include "Logger.h"

namespace hydla {
namespace simulator {

ParameterReplacer::ParameterReplacer()
{}

ParameterReplacer::~ParameterReplacer()
{}

void ParameterReplacer::replace_value(value_t& val)
{
  val->accept(*this);
}


void ParameterReplacer::visit(hydla::simulator::symbolic::SymbolicValue& val)
{
  accept(val.get_node());
}

void ParameterReplacer::add_mapping(const std::string& name,
      const int& derivative_count, const int& id)
{
  parameter_id_map_.insert(std::make_pair(std::make_pair(name, derivative_count), id));
}


void ParameterReplacer::visit(boost::shared_ptr<hydla::parse_tree::Parameter> node)
{
  if(node->get_phase_id() == -1)
  {
    parameter_id_map_t::iterator it = parameter_id_map_.find(std::make_pair(node->get_name(), node->get_derivative_count()));
    if(it != parameter_id_map_.end())
    {
      node->set_phase_id(it->second);
    }
  }
}



} //namespace simulator
} //namespace hydla 
