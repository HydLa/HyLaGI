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
  virtual void visit(ConstraintDefinition* node)  {}
  virtual void visit(ProgramDefinition* node)     {}

  // ŒÄ‚Ño‚µ
  virtual void visit(ConstraintCaller* node)      
  {
    node->get_child_node()->accept(this);
  }

  virtual void visit(ProgramCaller* node)         
  {
    node->get_child_node()->accept(this);
  }

  // §–ñŽ®
  virtual void visit(Constraint* node)            
  {
    //ml_.MLPutFunction("unit", 1);
    inter_str_ += "unit[";
    node->get_child_node()->accept(this);
    inter_str_ += "]";
  }

  // Ask§–ñ
  virtual void visit(Ask* node)                   
  {
    inter_str_ += "ask[";
    //ml_.MLPutFunction("ask", 2);
    in_guard_ = true;
    node->get_guard_node()->accept(this);

    inter_str_ += ",";

    in_guard_ = false;
    node->get_child_node()->accept(this);
    inter_str_ += "]";
  }

  // Tell§–ñ
  virtual void visit(Tell* node)                  
  {
    //ml_.MLPutFunction("tell", 1);
    inter_str_ += "tell[";
    node->get_child_node()->accept(this);
    inter_str_ += "]";
  }

  // ”äŠr‰‰ŽZŽq
  virtual void visit(Equal* node)                 
  {
    //ml_.MLPutFunction("Equal", 2);
        
    inter_str_ += "Equal[";
    node->get_lhs()->accept(this);
    inter_str_ += ",";
    node->get_rhs()->accept(this);
    inter_str_ += "]";
  }

  virtual void visit(UnEqual* node)               
  {
    //ml_.MLPutFunction("UnEqual", 2);
    
    inter_str_ += "UnEqual[";
    node->get_lhs()->accept(this);
    inter_str_ += ",";
    node->get_rhs()->accept(this);
    inter_str_ += "]";
  }

  virtual void visit(Less* node)                  
  {
    //ml_.MLPutFunction("Less", 2);
    
    inter_str_ += "Less[";
    node->get_lhs()->accept(this);
    inter_str_ += ",";
    node->get_rhs()->accept(this);
    inter_str_ += "]";
  }

  virtual void visit(LessEqual* node)             
  {
    //ml_.MLPutFunction("LessEqual", 2);
    
    inter_str_ += "LessEqual[";
    node->get_lhs()->accept(this);
    inter_str_ += ",";
    node->get_rhs()->accept(this);
    inter_str_ += "]";
  }

  virtual void visit(Greater* node)               
  {
    //ml_.MLPutFunction("Greater", 2);
    
    inter_str_ += "Greater[";
    node->get_lhs()->accept(this);
    inter_str_ += ",";
    node->get_rhs()->accept(this);
    inter_str_ += "]";
  }

  virtual void visit(GreaterEqual* node)          
  {
    //ml_.MLPutFunction("GreaterEqual", 2);
    
    inter_str_ += "GreaterEqual[";
    node->get_lhs()->accept(this);
    inter_str_ += ",";
    node->get_rhs()->accept(this);
    inter_str_ += "]";
  }

  // ˜_—‰‰ŽZŽq
  virtual void visit(LogicalAnd* node)            
  {
    //ml_.MLPutFunction("And", 2);

    if(in_guard_) {
      inter_str_ += "And[";
      node->get_lhs()->accept(this);
      inter_str_ += ",";
      node->get_rhs()->accept(this);
      inter_str_ += "]";
    } else {
      node->get_lhs()->accept(this);
      inter_str_ += ",";
      node->get_rhs()->accept(this);
    }
  }

  virtual void visit(LogicalOr* node)             
  {
    //ml_.MLPutFunction("Or", 2);
    
    inter_str_ += "Or[";
    node->get_lhs()->accept(this);
    inter_str_ += ",";
    node->get_rhs()->accept(this);
    inter_str_ += "]";
  }
  
  // ŽZp“ñ€‰‰ŽZŽq
  virtual void visit(Plus* node)                  
  {
    //ml_.MLPutFunction("Plus", 2);
    
    inter_str_ += "Plus[";
    node->get_lhs()->accept(this);
    inter_str_ += ",";
    node->get_rhs()->accept(this);
    inter_str_ += "]";
  }

  virtual void visit(Subtract* node)              
  {
    //ml_.MLPutFunction("Subtract", 2);
    
    inter_str_ += "Subtract[";
    node->get_lhs()->accept(this);
    inter_str_ += ",";
    node->get_rhs()->accept(this);
    inter_str_ += "]";
  }

  virtual void visit(Times* node)                 
  {
    //ml_.MLPutFunction("Times", 2);

    inter_str_ += "Times[";
    node->get_lhs()->accept(this);
    inter_str_ += ",";
    node->get_rhs()->accept(this);
    inter_str_ += "]";
  }

  virtual void visit(Divide* node)                
  {
    //ml_.MLPutFunction("Divide", 2);
    
    inter_str_ += "Divide[";
    node->get_lhs()->accept(this);
    inter_str_ += ",";
    node->get_rhs()->accept(this);
    inter_str_ += "]";
  }
  
  // ŽZp’P€‰‰ŽZŽq
  virtual void visit(Negative* node)              
  {       
    //ml_.MLPutFunction("Minus", 1);
    
    inter_str_ += "Minus[";
    node->get_child_node()->accept(this);
    inter_str_ += "]";
  }

  virtual void visit(Positive* node)              
  {
    node->get_child_node()->accept(this);
  }
  
  // §–ñŠK‘w’è‹`‰‰ŽZŽq
  virtual void visit(Weaker* node)                
  {
    // ‹t‚É‚µ‚Ä‘—M
    //ml_.MLPutFunction("order", 2);
    
    inter_str_ += "order[";
    node->get_rhs()->accept(this);
    inter_str_ += ",";
    node->get_lhs()->accept(this);
    inter_str_ += "]";
  }
  
  virtual void visit(Parallel* node)              
  {
    //ml_.MLPutFunction("group", 2);

    inter_str_ += "group[";
    node->get_lhs()->accept(this);
    inter_str_ += ",";
    node->get_rhs()->accept(this);
    inter_str_ += "]";
  }


  // Žž‘Š‰‰ŽZŽq
  virtual void visit(Always* node)                
  {
    //ml_.MLPutFunction("always", 1);
    inter_str_ += "always[";
    node->get_child_node()->accept(this);
    inter_str_ += "]";
  }
  
  // ”÷•ª
  virtual void visit(Differential* node)          
  {
//     ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
//     ml_.MLPutArgCount(1);      // this 1 is for the 'f'
//     ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
//     ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
//     ml_.MLPutSymbol("Derivative");
//     ml_.MLPutInteger(1);

    inter_str_ += "Derivative[1][";
    node->get_child_node()->accept(this);
    inter_str_ += "]";
  }

  // ¶‹ÉŒÀ
  virtual void visit(Previous* node)              
  {
    //ml_.MLPutFunction("prev", 1);
    inter_str_ += "prev[";
    node->get_child_node()->accept(this);
    inter_str_ += "]";
  }
  
  // •Ï”
  virtual void visit(Variable* node)              
  {
    //ml_.MLPutSymbol(node->get_name().c_str());
    inter_str_ += node->get_name();
  }

  // ”Žš
  virtual void visit(Number* node)                
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
