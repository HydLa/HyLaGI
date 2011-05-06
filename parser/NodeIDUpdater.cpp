#include "NodeIDUpdater.h"

#include <algorithm>
#include <cassert>

#include "Logger.h"
#include <iostream>

namespace hydla { 
namespace parser {
using namespace hydla::logger;

NodeIDUpdater::NodeIDUpdater()
{}

NodeIDUpdater::~NodeIDUpdater()
{}

void NodeIDUpdater::update(hydla::parse_tree::ParseTree* pt)
{
	if(Logger::ptflag==5){
		HYDLA_LOGGER_AREA("#*** NodeIDUpdater::update ***");
	}
  HYDLA_LOGGER_DEBUG("#*** NodeIDUpdater::update ***");

  parse_tree_ = pt;
  node_id_list_.clear();
  
  pt->dispatch(this);

  // íœ‚³‚ê‚Ä‚¢‚½ƒm[ƒh‚ğ•\‚©‚çæ‚èœ‚­
  
  assert(node_id_list_.size() <= pt->node_map_size());
  std::vector<hydla::parse_tree::node_id_t> diff_id( pt->node_map_size());
  pt->make_node_id_list();

  std::set_symmetric_difference(
    node_id_list_.begin(),
    node_id_list_.end(),
    pt->node_id_list_begin(),
    pt->node_id_list_end(),
    diff_id.begin());

  std::vector<hydla::parse_tree::node_id_t>::const_iterator it  = diff_id.begin();
  std::vector<hydla::parse_tree::node_id_t>::const_iterator end = diff_id.end();
  for(; it!=end; ++it) {
	if(Logger::ptflag==5){
		HYDLA_LOGGER_AREA("remove id: ", *it);
	}
	HYDLA_LOGGER_DEBUG("remove id: ", *it);
    pt->remove_node(*it);
  }
}

void NodeIDUpdater::visit(boost::shared_ptr<hydla::parse_tree::FactorNode> node)
{
  update_node_id(node);
}

void NodeIDUpdater::visit(boost::shared_ptr<hydla::parse_tree::UnaryNode> node)
{
  update_node_id(node);
  accept(node->get_child());
}

void NodeIDUpdater::visit(boost::shared_ptr<hydla::parse_tree::BinaryNode> node)
{    
  update_node_id(node);
  accept(node->get_lhs());
  accept(node->get_rhs());
}


} //namespace parser
} //namespace hydla
