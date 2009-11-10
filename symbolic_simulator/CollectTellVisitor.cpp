#include "CollectTellVisitor.h"

#include <iostream>

using namespace hydla::parse_tree;

namespace hydla {
namespace symbolic_simulator {


CollectTellVisitor::CollectTellVisitor(ParseTree& parse_tree, MathLink& ml) :
  parse_tree_(parse_tree),
  ml_(ml)
{}

CollectTellVisitor::~CollectTellVisitor()
{}

  // ���
  void CollectTellVisitor::visit(boost::shared_ptr<ConstraintDefinition> node)  {}
  void CollectTellVisitor::visit(boost::shared_ptr<ProgramDefinition> node)     {}

  // �ƤӽФ�
  void CollectTellVisitor::visit(boost::shared_ptr<ConstraintCaller> node)      
  {
    node_sptr chnode(node->get_child_node());
    chnode->accept(chnode, this);
  }

  void CollectTellVisitor::visit(boost::shared_ptr<ProgramCaller> node)         
  {
    node_sptr chnode(node->get_child_node());
    chnode->accept(chnode, this);
  }

  // ����
  void CollectTellVisitor::visit(boost::shared_ptr<Constraint> node)            
  {
    //ml_MLPutFunction("unit", 1);

    node_sptr chnode(node->get_child_node());
    chnode->accept(chnode, this);
  }

  // Ask����
  void CollectTellVisitor::visit(boost::shared_ptr<Ask> node)                   
  {
    //ml_MLPutFunction("ask", 2);
    
  }

  // Tell����
  void CollectTellVisitor::visit(boost::shared_ptr<Tell> node)                  
  {
    //ml_.MLPutFunction("tell", 1);
    ml_.MLPutFunction("Join", 2);
    ml_.MLPutFunction("List", 1);

    node_sptr chnode(node->get_child_node());
    chnode->accept(chnode, this);
    std::cout << std::endl;
  }

  // ��ӱ黻��
  void CollectTellVisitor::visit(boost::shared_ptr<Equal> node)                 
  {
    ml_.MLPutFunction("Equal", 2);
        
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    std::cout << "=";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  }

  void CollectTellVisitor::visit(boost::shared_ptr<UnEqual> node)               
  {
    ml_.MLPutFunction("UnEqual", 2);
    
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    std::cout << "!=";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
    }

  void CollectTellVisitor::visit(boost::shared_ptr<Less> node)                  
  {
    ml_.MLPutFunction("Less", 2);
    
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    std::cout << "<";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  }

  void CollectTellVisitor::visit(boost::shared_ptr<LessEqual> node)             
  {
    ml_.MLPutFunction("LessEqual", 2);
    
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    std::cout << "<=";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  }

  void CollectTellVisitor::visit(boost::shared_ptr<Greater> node)               
  {
    ml_.MLPutFunction("Greater", 2);
    
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    std::cout << ">";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  }

  void CollectTellVisitor::visit(boost::shared_ptr<GreaterEqual> node)          
  {
    ml_.MLPutFunction("GreaterEqual", 2);
    
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    std::cout << ">=";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  }

  // �����黻��
  void CollectTellVisitor::visit(boost::shared_ptr<LogicalAnd> node)            
  {
    //ml_.MLPutFunction("And, 2);

    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  }

  void CollectTellVisitor::visit(boost::shared_ptr<LogicalOr> node)             
  {
    //ml_.MLPutFunction("Or", 2);
    
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  }
  
  // �������黻��
  void CollectTellVisitor::visit(boost::shared_ptr<Plus> node)                  
  {
    ml_.MLPutFunction("Plus", 2);
    
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    std::cout << "+";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  }

  void CollectTellVisitor::visit(boost::shared_ptr<Subtract> node)              
  {
    ml_.MLPutFunction("Subtract", 2);
    
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    std::cout << "-";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  }

  void CollectTellVisitor::visit(boost::shared_ptr<Times> node)                 
  {
    ml_.MLPutFunction("Times", 2);

    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    std::cout << "*";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  }

  void CollectTellVisitor::visit(boost::shared_ptr<Divide> node)                
  {
    ml_.MLPutFunction("Divide", 2);
    
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    std::cout << "/";
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  }
  
  // ����ñ��黻��
  void CollectTellVisitor::visit(boost::shared_ptr<Negative> node)              
  {       
    ml_.MLPutFunction("Minus", 1);
    std::cout << "-";

    node_sptr chnode(node->get_child_node());
    chnode->accept(chnode, this);
  }

  void CollectTellVisitor::visit(boost::shared_ptr<Positive> node)              
  {
    node_sptr chnode(node->get_child_node());
    chnode->accept(chnode, this);
  }

  // ����������黻��
  void CollectTellVisitor::visit(boost::shared_ptr<Weaker> node)                
  {
    // �դˤ�������
    //ml_.MLPutFunction("order", 2);
    
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  }
  
  void CollectTellVisitor::visit(boost::shared_ptr<Parallel> node)              
  {
    //ml_.MLPutFunction("group", 2);

    node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
    node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  }

  // ����黻��
  void CollectTellVisitor::visit(boost::shared_ptr<Always> node)                
  {
    node_sptr chnode(node->get_child_node());
    chnode->accept(chnode, this);
  }

  // ��ʬ
  void CollectTellVisitor::visit(boost::shared_ptr<Differential> node)          
  {
    ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
    ml_.MLPutArgCount(1);      // this 1 is for the 'f'
    ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
    ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
    ml_.MLPutSymbol("Derivative");
    ml_.MLPutInteger(1);

    node_sptr chnode(node->get_child_node());
    chnode->accept(chnode, this);
    std::cout << "'";
  }

  // ���˸�
  void CollectTellVisitor::visit(boost::shared_ptr<Previous> node)              
  {
    //ml_.MLPutFunction("prev", 1);
    node_sptr chnode(node->get_child_node());
    chnode->accept(chnode, this);
    std::cout << "-";
  }
  
  // �ѿ�
  void CollectTellVisitor::visit(boost::shared_ptr<Variable> node)              
  {
    ml_.MLPutSymbol(node->get_name().c_str());
    vars_.insert(node->get_name().c_str());
    std::cout << node->get_name().c_str();
  }

  // ����
  void CollectTellVisitor::visit(boost::shared_ptr<Number> node)                
  {    
    ml_.MLPutInteger(atoi(node->get_number().c_str()));
    std::cout << node->get_number().c_str();
  }


bool CollectTellVisitor::is_consistent()
{

/*
  ml.MLPutFunction("List", 3);
  ml.MLPutFunction("Equal", 2);
  ml.MLPutSymbol("x");
  ml.MLPutSymbol("y");
  ml.MLPutFunction("Equal", 2);
  ml.MLPutSymbol("x");
  ml.MLPutInteger(1);
  ml.MLPutFunction("Equal", 2);
  ml.MLPutSymbol("y");
  ml.MLPutInteger(2);
  ml.MLPutFunction("List", 2);
  ml.MLPutSymbol("x");
  ml.MLPutSymbol("y");
  ml.MLEndPacket();
*/


  // isConsistent[expr, vars]���Ϥ�����
  ml_.MLPutFunction("isConsistent", 2);


  // �ѡ����ĥ꡼����expr������Mathematica���Ϥ�
  parse_tree_.dispatch(this);

  // �Ǹ�˶��Υꥹ�Ȥ򤯤äĤ���
  ml_.MLPutFunction("List", 0);


  // vars���Ϥ�
  int var_num = vars_.size();
  ml_.MLPutFunction("List", var_num);
  std::set<std::string>::iterator it = vars_.begin();
  const char* sym;
  std::cout << "vars: ";
  while(it!=vars_.end()){
    sym = (*it).c_str();
    ml_.MLPutSymbol(sym);
    std::cout << sym << " ";
    it++;
  }

  std::cout << std::endl << std::endl;

  int p;
  while ((p = ml_.MLNextPacket()) && p != RETURNPKT) ml_.MLNewPacket();

  int num;
  ml_.MLGetInteger(&num);
  std::cout << "#num:" << num << std::endl;
  
  // Mathematica����1��True��ɽ���ˤ��֤��true��0��False��ɽ���ˤ��֤��false���֤�
  if(num==1) return true;
  return false;
}


} //namespace symbolic_simulator
} // namespace hydla
