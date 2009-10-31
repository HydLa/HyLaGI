#ifndef _INCLUDED_HYDLA_INTERLANGUAGE_SENDER_H_
#define _INCLUDED_HYDLA_INTERLANGUAGE_SENDER_H_

#include "mathlink_helper.h"
#include "Node.h"
#include "TreeVisitor.h"

using namespace hydla::parse_tree;

namespace hydla {
namespace symbolic_simulator {

class InterlanguageSender : public TreeVisitor {
public:
  InterlanguageSender(ParseTree& parse_tree, MathLink& ml) :
    parse_tree_(parse_tree),
    ml_(ml)
  {}

  virtual ~InterlanguageSender()
  {}

  // ’è‹`
  virtual void visit(boost::shared_ptr<ConstraintDefinition> node)  {}
  virtual void visit(boost::shared_ptr<ProgramDefinition> node)     {}

  // ŒÄ‚Ño‚µ
  virtual void visit(boost::shared_ptr<ConstraintCaller> node)      
  {
    node_sptr chnode(node->get_child_node());
    chnode->accept(chnode, this);
  }

  virtual void visit(boost::shared_ptr<ProgramCaller> node)         
  {
    node_sptr chnode(node->get_child_node());
    chnode->accept(chnode, this);
  }

  // §–ñ®
  virtual void visit(boost::shared_ptr<Constraint> node)            
  {
    //ml_.MLPutFunction("unit", 1);
    inter_str_ += "unit[";
    node_sptr chnode(node->get_child_node());
    chnode->accept(chnode, this);
    inter_str_ += "]";
  }

  // Ask§–ñ
  virtual void visit(boost::shared_ptr<Ask> node)                   
  {
    inter_str_ += "ask[";
    //ml_.MLPutFunction("ask", 2);
    in_guard_ = true;
    node_sptr gunode(node->get_guard_node());
    gunode->accept(gunode, this);

    inter_str_ += ",";

    in_guard_ = false;
    node_sptr chnode(node->get_child_node());
    chnode->accept(chnode, this);
    inter_str_ += "]";
  }

  // Tell§–ñ
  virtual void visit(boost::shared_ptr<Tell> node)                  
  {
    //ml_.MLPutFunction("tell", 1);
    inter_str_ += "tell[";
    node_sptr chnode(node->get_child_node());
    chnode->accept(chnode, this);
    inter_str_ += "]";
  }

  // ”äŠr‰‰Zq
  virtual void visit(boost::shared_ptr<Equal> node)                 
  {
    //ml_.MLPutFunction("Equal", 2);
        
    inter_str_ += "Equal[";
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    inter_str_ += ",";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
    inter_str_ += "]";
  }

  virtual void visit(boost::shared_ptr<UnEqual> node)               
  {
    //ml_.MLPutFunction("UnEqual", 2);
    
    inter_str_ += "UnEqual[";
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    inter_str_ += ",";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
    inter_str_ += "]";
  }

  virtual void visit(boost::shared_ptr<Less> node)                  
  {
    //ml_.MLPutFunction("Less", 2);
    
    inter_str_ += "Less[";
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    inter_str_ += ",";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
    inter_str_ += "]";
  }

  virtual void visit(boost::shared_ptr<LessEqual> node)             
  {
    //ml_.MLPutFunction("LessEqual", 2);
    
    inter_str_ += "LessEqual[";
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    inter_str_ += ",";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
    inter_str_ += "]";
  }

  virtual void visit(boost::shared_ptr<Greater> node)               
  {
    //ml_.MLPutFunction("Greater", 2);
    
    inter_str_ += "Greater[";
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    inter_str_ += ",";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
    inter_str_ += "]";
  }

  virtual void visit(boost::shared_ptr<GreaterEqual> node)          
  {
    //ml_.MLPutFunction("GreaterEqual", 2);
    
    inter_str_ += "GreaterEqual[";
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    inter_str_ += ",";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
    inter_str_ += "]";
  }

  // ˜_—‰‰Zq
  virtual void visit(boost::shared_ptr<LogicalAnd> node)            
  {
    //ml_.MLPutFunction("And", 2);

    if(in_guard_) {
      inter_str_ += "And[";
      node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
      inter_str_ += ",";
      node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
      inter_str_ += "]";
    } else {
      node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
      inter_str_ += ",";
      node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
    }
  }

  virtual void visit(boost::shared_ptr<LogicalOr> node)             
  {
    //ml_.MLPutFunction("Or", 2);
    
    inter_str_ += "Or[";
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    inter_str_ += ",";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
    inter_str_ += "]";
  }
  
  // Zp“ñ€‰‰Zq
  virtual void visit(boost::shared_ptr<Plus> node)                  
  {
    //ml_.MLPutFunction("Plus", 2);
    
    inter_str_ += "Plus[";
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    inter_str_ += ",";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
    inter_str_ += "]";
  }

  virtual void visit(boost::shared_ptr<Subtract> node)              
  {
    //ml_.MLPutFunction("Subtract", 2);
    
    inter_str_ += "Subtract[";
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    inter_str_ += ",";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
    inter_str_ += "]";
  }

  virtual void visit(boost::shared_ptr<Times> node)                 
  {
    //ml_.MLPutFunction("Times", 2);

    inter_str_ += "Times[";
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    inter_str_ += ",";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
    inter_str_ += "]";
  }

  virtual void visit(boost::shared_ptr<Divide> node)                
  {
    //ml_.MLPutFunction("Divide", 2);
    
    inter_str_ += "Divide[";
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    inter_str_ += ",";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
    inter_str_ += "]";
  }
  
  // Zp’P€‰‰Zq
  virtual void visit(boost::shared_ptr<Negative> node)              
  {       
    //ml_.MLPutFunction("Minus", 1);
    
    inter_str_ += "Minus[";
    node_sptr chnode(node->get_child_node());
    chnode->accept(chnode, this);
    inter_str_ += "]";
  }

  virtual void visit(boost::shared_ptr<Positive> node)              
  {
    node_sptr chnode(node->get_child_node());
    chnode->accept(chnode, this);
  }
  
  // §–ñŠK‘w’è‹`‰‰Zq
  virtual void visit(boost::shared_ptr<Weaker> node)                
  {
    // ‹t‚É‚µ‚Ä‘—M
    //ml_.MLPutFunction("order", 2);
    
    inter_str_ += "order[";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
    inter_str_ += ",";
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    inter_str_ += "]";
  }
  
  virtual void visit(boost::shared_ptr<Parallel> node)              
  {
    //ml_.MLPutFunction("group", 2);

    inter_str_ += "group[";
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    inter_str_ += ",";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
    inter_str_ += "]";
  }


  // ‘Š‰‰Zq
  virtual void visit(boost::shared_ptr<Always> node)                
  {
    //ml_.MLPutFunction("always", 1);
    inter_str_ += "always[";
    node_sptr chnode(node->get_child_node());
    chnode->accept(chnode, this);
    inter_str_ += "]";
  }
  
  // ”÷•ª
  virtual void visit(boost::shared_ptr<Differential> node)          
  {
//     ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
//     ml_.MLPutArgCount(1);      // this 1 is for the 'f'
//     ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
//     ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
//     ml_.MLPutSymbol("Derivative");
//     ml_.MLPutInteger(1);

    inter_str_ += "Derivative[1][";
    node_sptr chnode(node->get_child_node());
    chnode->accept(chnode, this);
    inter_str_ += "]";
  }

  // ¶‹ÉŒÀ
  virtual void visit(boost::shared_ptr<Previous> node)              
  {
    //ml_.MLPutFunction("prev", 1);
    inter_str_ += "prev[";
    node_sptr chnode(node->get_child_node());
    chnode->accept(chnode, this);
    inter_str_ += "]";
  }
  
  // •Ï”
  virtual void visit(boost::shared_ptr<Variable> node)              
  {
    //ml_.MLPutSymbol(node->get_name().c_str());
    inter_str_ += node->get_name();
  }

  // ”š
  virtual void visit(boost::shared_ptr<Number> node)                
  {    
    //ml_.MLPutInteger(atoi(node->get_number().c_str()));
    inter_str_ += node->get_number();
  }

  std::string get_interlanguage() 
  {
    return inter_str_;
  }

  void create_interlanguage(std::string max_time)
  {
    in_guard_ = false;
    inter_str_.clear();

    parse_tree_.dispatch(this);

    std::string tmpstr;

    tmpstr += "HydLaMain[";
    tmpstr += inter_str_;
    tmpstr += ", {";
    variable_map_t::const_iterator it  = parse_tree_.get_variable_map().begin();
    variable_map_t::const_iterator end = parse_tree_.get_variable_map().end();
    while(it!=end) {
      tmpstr += it->first;
      if(++it != end) tmpstr += ",";
    }
    tmpstr += "}, ";
    tmpstr += max_time;
    tmpstr += "]";


    inter_str_ = tmpstr;   
  }

private:
  MathLink& ml_;
  ParseTree& parse_tree_;

  int         in_guard_;
  std::string inter_str_;
  
};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_INTERLANGUAGE_SENDER_H_
