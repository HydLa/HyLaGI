/*
#include "CMMap.h"
#include "TreeInfixPrinter.h"
#include <iostream>

namespace hydla{
  namespace hierarchy{
    CMMap::CMMap():searched_(false){}
    CMMap::CMMap(hydla::symbolic_expression::node_sptr& cond):condition_(cond),searched_(false){}
    CMMap::~CMMap(){}

    module_set_list_t CMMap::get_module_set_list(){
      return ms_list_;
    }
    
    hydla::symbolic_expression::node_sptr& CMMap::get_condition(){ return condition_; }

    bool CMMap::is_searched(){
      return searched_;
    }

    cm_map_list_t CMMap::get_parents(){
      return parents_;
    }

    void CMMap::add_parents(const cm_map_sptr& cm){
      parents_.push_back(cm);
    }

    void CMMap::add_children(const cm_map_sptr& cm){
      children_.push_back(cm);
    }

    bool CMMap::is_super_cm(const cm_map_sptr& cm){
      for(cm_map_list_t::iterator it = children_.begin(); it != children_.end(); it++){
	if((*it) == cm) return true;
      }
      return false;
    }

    bool CMMap::is_super_cm(const module_set_sptr& ms){
      for(module_set_list_t::iterator it = ms_list_.begin(); it != ms_list_.end(); it++){
	if((*it)->is_super_set(*ms)) return true;
      }
      return false;
    }

    void CMMap::set_condition(const hydla::symbolic_expression::node_sptr& cond){
      condition_ = cond;
    }

    void CMMap::add_module_set(const module_set_sptr& ms){
      ms_list_.push_back(ms);
    }

    void CMMap::unsearched(){
      searched_ = false;
    }

    void CMMap::mark_children(){
      if(searched_) return;
      for(cm_map_list_t::iterator it = children_.begin(); it != children_.end(); it++){
        (*it)->mark_children();
      }
      searched_ = true;
    }

    void CMMap::reset(){
      condition_ = hydla::symbolic_expression::node_sptr();
      ms_list_.clear();
      parents_.clear();
      children_.clear();
      searched_ = false;
    }

    std::ostream& CMMap::dump(std::ostream& s) const{
      s << "condition : " << hydla::symbolic_expression::TreeInfixPrinter().get_infix_string(condition_) << std::endl;
      //      s << "*****" << condition_ << "*****" << std::endl;
      //      s << "  parent" << std::endl;
            for(cm_map_list_t::const_iterator it = parents_.begin(); it != parents_.end(); it++){
	s << "    " << hydla::symbolic_expression::TreeInfixPrinter().get_infix_string((*it)->get_condition()) << std::endl;
	//	s << "    " << (*it)->get_condition() << std::endl;
      }
      s << "  children" << std::endl;
      for(cm_map_list_t::const_iterator it = children_.begin(); it != children_.end(); it++){
	s << "    " << hydla::symbolic_expression::TreeInfixPrinter().get_infix_string((*it)->get_condition()) << std::endl;
	//	s << "    " << (*it)->get_condition() << std::endl;
      }
      
      s << "module sets : " << std::endl;
      for(module_set_list_t::const_iterator it = ms_list_.begin(); it != ms_list_.end(); it++){
	s << "    " << (*it)->get_name() << std::endl;
      }
      //      s << "  searched : " << (searched_ ? "True" : "False") << std::endl;
      return s;
    }

    std::ostream& operator<<(std::ostream& s, const CMMap& c){
      return c.dump(s);
    }
  }
}
*/
