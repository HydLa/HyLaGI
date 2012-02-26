#include "ContinuityMapMaker.h"
#include "Logger.h"

#include <assert.h>
#include <iostream>

namespace hydla {
namespace simulator {
using namespace hydla::logger;
  
ContinuityMapMaker::ContinuityMapMaker()
{}

ContinuityMapMaker::~ContinuityMapMaker()
{}



// Ask§–ñ
void ContinuityMapMaker::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{
}


// •Ï”
void ContinuityMapMaker::visit(boost::shared_ptr<hydla::parse_tree::Variable> node)
{
  if(!differential_count_) return;
  continuity_map_t::iterator find = variables_.find(node->get_name());
  if(find == variables_.end() || find->second < differential_count_){
    if(negative_)
      variables_[node->get_name()] = -differential_count_ + 1;
    else
      variables_[node->get_name()] = differential_count_;
  }
}


// ”÷•ª
void ContinuityMapMaker::visit(boost::shared_ptr<hydla::parse_tree::Differential> node)
{
  differential_count_++;
  accept(node->get_child());
  differential_count_--;
}


// ¶‹ÉŒÀ
void ContinuityMapMaker::visit(boost::shared_ptr<hydla::parse_tree::Previous> node)
{
  if(in_interval_){
    accept(node->get_child());
  }
}


} //namespace simulator
} //namespace hydla 
