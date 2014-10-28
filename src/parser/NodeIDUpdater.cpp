#include "NodeIDUpdater.h"

#include <algorithm>
#include <cassert>

#include "Logger.h"

namespace hydla { 
namespace parser {
using namespace hydla::logger;

NodeIDUpdater::NodeIDUpdater()
{}

NodeIDUpdater::~NodeIDUpdater()
{}

void NodeIDUpdater::update(hydla::parse_tree::ParseTree* pt)
{
  HYDLA_LOGGER_DEBUG("#*** Begin ", __FUNCTION__ ," ***");

  parse_tree_ = pt;
  node_id_list_.clear();
  
  pt->dispatch(this);
  

  // 削除されていたノードを表から取り除く
  
  assert(node_id_list_.size() <= pt->node_map_size());
  std::vector<hydla::symbolic_expression::node_id_t> diff_id( pt->node_map_size());
  pt->make_node_id_list();

  std::set_symmetric_difference(
    node_id_list_.begin(),
    node_id_list_.end(),
    pt->node_id_list_begin(),
    pt->node_id_list_end(),
    diff_id.begin());
    
  std::vector<hydla::symbolic_expression::node_id_t>::const_iterator it  = diff_id.begin();
  std::vector<hydla::symbolic_expression::node_id_t>::const_iterator end = diff_id.end();
  for(; it!=end; ++it) {
    HYDLA_LOGGER_DEBUG("remove id: ", *it);
    pt->remove_node(*it);
  }
  HYDLA_LOGGER_DEBUG("#*** End ", __FUNCTION__ ," ***\n");
}

void NodeIDUpdater::visit(boost::shared_ptr<hydla::symbolic_expression::FactorNode> node)
{
  update_node_id(node);
}

void NodeIDUpdater::visit(boost::shared_ptr<hydla::symbolic_expression::UnaryNode> node)
{
  update_node_id(node);
  accept(node->get_child());
}

void NodeIDUpdater::visit(boost::shared_ptr<hydla::symbolic_expression::BinaryNode> node)
{    
  update_node_id(node);
  accept(node->get_lhs());
  accept(node->get_rhs());
}


void NodeIDUpdater::visit(boost::shared_ptr<hydla::symbolic_expression::ArbitraryNode> node)
{    
  update_node_id(node);
  for(int i=0;i<node->get_arguments_size();i++){
    accept(node->get_argument(i));
  }
}


} //namespace parser
} //namespace hydla