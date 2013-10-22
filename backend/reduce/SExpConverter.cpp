#include "SExpConverter.h"

using namespace hydla::parse_tree;

namespace hydla {
namespace vcs {
namespace reduce {

SExpConverter::SExpConverter() 
{}

SExpConverter::~SExpConverter(){}

//TODO init_var引数をなくす
node_sptr SExpConverter::make_equal(const variable_t &variable, const node_sptr& node, const bool& prev, const bool& init_var){
  HYDLA_LOGGER_FUNC_BEGIN(REST);
  node_sptr new_node(new Variable(variable.get_name()));
  for(int i=0;i<variable.get_derivative_count();i++){
    new_node = node_sptr(new Differential(new_node));
  }
  if(prev){
    new_node = node_sptr(new Previous(new_node));
  }
  HYDLA_LOGGER_FUNC_END(REST);
  return node_sptr(new Equal(new_node, node));
}

node_sptr SExpConverter::make_equal(hydla::simulator::DefaultParameter &variable, const node_sptr& node, const bool& prev, const bool& init_var){
  HYDLA_LOGGER_FUNC_BEGIN(REST);
  node_sptr new_node(new Variable(variable.get_name()));
  for(int i=0;i<variable.get_derivative_count();i++){
    new_node = node_sptr(new Differential(new_node));
  }
  if(prev){
    new_node = node_sptr(new Previous(new_node));
  }
  HYDLA_LOGGER_FUNC_END(REST);
  return node_sptr(new Equal(new_node, node));
}

void SExpConverter::set_range(const value_t &val, value_range_t &range, const int& relop){
  switch(relop){
    case 0://Equal
    range.set_unique(val);
    break;
    
    case 1://Less
    range.set_upper_bound(val, false);
    break;
    case 2://Greater
    range.set_lower_bound(val, false);
    break;
    case 3://LessEqual
    range.set_upper_bound(val, true);
    break;
    case 4://GreaterEqual
    range.set_lower_bound(val, true);
    break;
  }
}

void SExpConverter::set_parameter_on_value(value_t &val,const parameter_t &par){
  assert(0);
  //TODO
  //val.set(node_sptr(new hydla::parse_tree::Parameter(par.get_name())));
  return;
}

} // namespace reduce
} // namespace vcs
} // namespace hydla
