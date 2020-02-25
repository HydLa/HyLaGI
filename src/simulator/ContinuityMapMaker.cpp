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



// Ask制約
void ContinuityMapMaker::visit(std::shared_ptr<hydla::symbolic_expression::Ask> node)
{
  accept(node->get_child());
}


// 変数
void ContinuityMapMaker::visit(std::shared_ptr<hydla::symbolic_expression::Variable> node)
{
  if(!differential_count_) return;
  continuity_map_t::iterator find = variables_.find(node->get_name());
  if(find == variables_.end() || find->second < differential_count_){
    if(negative_)
      variables_[node->get_name()] = -differential_count_;
    else
      variables_[node->get_name()] = differential_count_;
  }
}


// 微分
void ContinuityMapMaker::visit(std::shared_ptr<hydla::symbolic_expression::Differential> node)
{
  differential_count_++;
  accept(node->get_child());
  differential_count_--;
}


// 左極限
void ContinuityMapMaker::visit(std::shared_ptr<hydla::symbolic_expression::Previous> node)
{
  if(in_interval_){
    accept(node->get_child());
  }
}


std::ostream& operator<<(std::ostream& s, const continuity_map_t& continuity_map)
{
  for(continuity_map_t::const_iterator it = continuity_map.begin(); it != continuity_map.end(); it++){
    s << it->first << "," << it->second << "\n";
  }
  return s;
}

} //namespace simulator
} //namespace hydla 
