#include "IntermediateCodeGenerator.h"

using namespace hydla::parse_tree;

namespace hydla {
namespace symbolic_legacy_simulator {

IntermediateCodeGenerator::IntermediateCodeGenerator()
{}

IntermediateCodeGenerator::~IntermediateCodeGenerator()
{}


// �Ăяo��
void IntermediateCodeGenerator::visit(boost::shared_ptr<ConstraintCaller> node)      
{
  accept(node->get_child());
}

void IntermediateCodeGenerator::visit(boost::shared_ptr<ProgramCaller> node)         
{
  accept(node->get_child());
}

// ����
void IntermediateCodeGenerator::visit(boost::shared_ptr<Constraint> node)            
{
  //ml_.MLPutFunction("unit", 1);
  inter_str_ += "unit[";
  accept(node->get_child());
  inter_str_ += "]";
}

// Ask����
void IntermediateCodeGenerator::visit(boost::shared_ptr<Ask> node)                   
{
  inter_str_ += "ask[";
  //ml_.MLPutFunction("ask", 2);
  in_guard_ = true;
  accept(node->get_guard());

  inter_str_ += ",";

  in_guard_ = false;
  accept(node->get_child());
  inter_str_ += "]";
}

// Tell����
void IntermediateCodeGenerator::visit(boost::shared_ptr<Tell> node)                  
{
  //ml_.MLPutFunction("tell", 1);
  inter_str_ += "tell[";
  accept(node->get_child());
  inter_str_ += "]";
}

// ��r���Z�q
void IntermediateCodeGenerator::visit(boost::shared_ptr<Equal> node)                 
{
  //ml_.MLPutFunction("Equal", 2);

  inter_str_ += "Equal[";
  accept(node->get_lhs());
  inter_str_ += ",";
  accept(node->get_rhs());
  inter_str_ += "]";
}

void IntermediateCodeGenerator::visit(boost::shared_ptr<UnEqual> node)               
{
  //ml_.MLPutFunction("UnEqual", 2);
    
  inter_str_ += "UnEqual[";
  accept(node->get_lhs());
  inter_str_ += ",";
  accept(node->get_rhs());
  inter_str_ += "]";
}

void IntermediateCodeGenerator::visit(boost::shared_ptr<Less> node)                  
{
  //ml_.MLPutFunction("Less", 2);
    
  inter_str_ += "Less[";
  accept(node->get_lhs());
  inter_str_ += ",";
  accept(node->get_rhs());
  inter_str_ += "]";
}

void IntermediateCodeGenerator::visit(boost::shared_ptr<LessEqual> node)             
{
  //ml_.MLPutFunction("LessEqual", 2);
    
  inter_str_ += "LessEqual[";
  accept(node->get_lhs());
  inter_str_ += ",";
  accept(node->get_rhs());
  inter_str_ += "]";
}

void IntermediateCodeGenerator::visit(boost::shared_ptr<Greater> node)               
{
  //ml_.MLPutFunction("Greater", 2);
    
  inter_str_ += "Greater[";
  accept(node->get_lhs());
  inter_str_ += ",";
  accept(node->get_rhs());
  inter_str_ += "]";
}

void IntermediateCodeGenerator::visit(boost::shared_ptr<GreaterEqual> node)          
{
  //ml_.MLPutFunction("GreaterEqual", 2);
    
  inter_str_ += "GreaterEqual[";
  accept(node->get_lhs());
  inter_str_ += ",";
  accept(node->get_rhs());
  inter_str_ += "]";
}

// �_�����Z�q
void IntermediateCodeGenerator::visit(boost::shared_ptr<LogicalAnd> node)            
{
  //ml_.MLPutFunction("And", 2);

  if(in_guard_) {
    inter_str_ += "And[";
    accept(node->get_lhs());
    inter_str_ += ",";
    accept(node->get_rhs());
    inter_str_ += "]";
  } else {
    accept(node->get_lhs());
    inter_str_ += ",";
    accept(node->get_rhs());
  }
}

void IntermediateCodeGenerator::visit(boost::shared_ptr<LogicalOr> node)             
{
  //ml_.MLPutFunction("Or", 2);
    
  inter_str_ += "Or[";
  accept(node->get_lhs());
  inter_str_ += ",";
  accept(node->get_rhs());
  inter_str_ += "]";
}
  
// �Z�p�񍀉��Z�q
void IntermediateCodeGenerator::visit(boost::shared_ptr<Plus> node)                  
{
  //ml_.MLPutFunction("Plus", 2);
    
  inter_str_ += "Plus[";
  accept(node->get_lhs());
  inter_str_ += ",";
  accept(node->get_rhs());
  inter_str_ += "]";
}

void IntermediateCodeGenerator::visit(boost::shared_ptr<Subtract> node)              
{
  //ml_.MLPutFunction("Subtract", 2);
    
  inter_str_ += "Subtract[";
  accept(node->get_lhs());
  inter_str_ += ",";
  accept(node->get_rhs());
  inter_str_ += "]";
}

void IntermediateCodeGenerator::visit(boost::shared_ptr<Times> node)                 
{
  //ml_.MLPutFunction("Times", 2);

  inter_str_ += "Times[";
  accept(node->get_lhs());
  inter_str_ += ",";
  accept(node->get_rhs());
  inter_str_ += "]";
}

void IntermediateCodeGenerator::visit(boost::shared_ptr<Divide> node)                
{
  //ml_.MLPutFunction("Divide", 2);
    
  inter_str_ += "Divide[";
  accept(node->get_lhs());
  inter_str_ += ",";
  accept(node->get_rhs());
  inter_str_ += "]";
}
  
// �Z�p�P�����Z�q
void IntermediateCodeGenerator::visit(boost::shared_ptr<Negative> node)              
{       
  //ml_.MLPutFunction("Minus", 1);
    
  inter_str_ += "Minus[";
  accept(node->get_child());
  inter_str_ += "]";
}

void IntermediateCodeGenerator::visit(boost::shared_ptr<Positive> node)              
{
  accept(node->get_child());
}
  
// ����K�w��`���Z�q
void IntermediateCodeGenerator::visit(boost::shared_ptr<Weaker> node)                
{
  // �t�ɂ��đ��M
  //ml_.MLPutFunction("order", 2);
    
  inter_str_ += "order[";
  accept(node->get_rhs());
  inter_str_ += ",";
  accept(node->get_lhs());
  inter_str_ += "]";
}
  
void IntermediateCodeGenerator::visit(boost::shared_ptr<Parallel> node)              
{
  //ml_.MLPutFunction("group", 2);

  inter_str_ += "group[";
  accept(node->get_lhs());
  inter_str_ += ",";
  accept(node->get_rhs());
  inter_str_ += "]";
}


// �������Z�q
void IntermediateCodeGenerator::visit(boost::shared_ptr<Always> node)                
{
  //ml_.MLPutFunction("always", 1);
  inter_str_ += "always[";
  accept(node->get_child());
  inter_str_ += "]";
}
  
// ����
void IntermediateCodeGenerator::visit(boost::shared_ptr<Differential> node)          
{
//     ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
//     ml_.MLPutArgCount(1);      // this 1 is for the 'f'
//     ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
//     ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
//     ml_.MLPutSymbol("Derivative");
//     ml_.MLPutInteger(1);

  inter_str_ += "Derivative[1][";
  accept(node->get_child());
  inter_str_ += "]";
}

// ���Ɍ�
void IntermediateCodeGenerator::visit(boost::shared_ptr<Previous> node)              
{
  //ml_.MLPutFunction("prev", 1);
  inter_str_ += "prev[";
  accept(node->get_child());
  inter_str_ += "]";
}
  
// �ϐ�
void IntermediateCodeGenerator::visit(boost::shared_ptr<Variable> node)              
{
  //ml_.MLPutSymbol(node->get_name().c_str());
  inter_str_ += node->get_name();
}

// ����
void IntermediateCodeGenerator::visit(boost::shared_ptr<Number> node)                
{    
  //ml_.MLPutInteger(atoi(node->get_number().c_str()));
  inter_str_ += node->get_number();
}

std::string IntermediateCodeGenerator::create(parse_tree_sptr parse_tree, 
                                       std::string max_time)
{
  parse_tree_ = parse_tree;

  in_guard_ = false;
  inter_str_.clear();

  parse_tree_->dispatch(this);

  std::string tmpstr;

  tmpstr += "HydLaMain[";
  tmpstr += inter_str_;
  tmpstr += ", {";
  hydla::parse_tree::ParseTree::variable_map_t::const_iterator it  = 
    parse_tree_->variable_map_begin();
  hydla::parse_tree::ParseTree::variable_map_t::const_iterator end = 
    parse_tree_->variable_map_end();
  while(it!=end) {
    tmpstr += it->first;
    if(++it != end) tmpstr += ",";
  }
  tmpstr += "}, ";
  tmpstr += max_time;
  tmpstr += "]";


  inter_str_ = tmpstr;   
  return inter_str_;
}


} // namespace symbolic_legacy_simulator
} // namespace hydla
