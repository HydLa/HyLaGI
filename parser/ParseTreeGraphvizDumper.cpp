#include "ParseTreeGraphvizDumper.h"

#include <utility>
#include <algorithm>

using namespace hydla::parse_tree;
using namespace hydla::parse_error;

using std::make_pair;

namespace hydla { 
namespace parser {

ParseTreeGraphvizDumper::ParseTreeGraphvizDumper()
{}

ParseTreeGraphvizDumper::~ParseTreeGraphvizDumper()
{}

namespace {

  struct NodeDumper {
    NodeDumper(std::ostream& s) :
      s_(s)
    {}

    template<typename T>
    void operator()(const T& n) {
      s_ << "  "
         << "n" << n.first 
         << " [label = \"" << n.second << "\"];\n";      
    }

    std::ostream& s_;
  };
  
  struct EdgeDumper {
    EdgeDumper(std::ostream& s) :
      s_(s)
    {}

    template<typename T>
    void operator()(const T& n) {
      s_ << "  "
         << "n" << n.first << " -> "
         << "n" << n.second << ";\n";      
    }

    std::ostream& s_;
  };
}

std::ostream& ParseTreeGraphvizDumper::dump(std::ostream& s, const node_sptr& node)
{
  node_id_ = 0;
  nodes_.clear();
  edges_.clear();

  accept(node);

  s << "digraph g {\n";
  s << "  node [shape = record];\n";
  s << "\n";

  std::for_each(nodes_.begin(), nodes_.end(), NodeDumper(s));
  s << "\n";
  
  std::for_each(edges_.begin(), edges_.end(), EdgeDumper(s));
  s << "}" << std::endl;

  return s;
}

void ParseTreeGraphvizDumper::dump_node(boost::shared_ptr<UnaryNode> node)
{
  graph_node_id_t own_id = node_id_;
  nodes_.insert(make_pair(own_id, node->get_node_type_name()));

  edges_.insert(make_pair(own_id, ++node_id_));
  accept(node->get_child());
}

void ParseTreeGraphvizDumper::dump_node(boost::shared_ptr<BinaryNode> node)
{
  graph_node_id_t own_id = node_id_;
  nodes_.insert(make_pair(own_id, node->get_node_type_name()));

  edges_.insert(make_pair(own_id, ++node_id_));
  accept(node->get_lhs());

  edges_.insert(make_pair(own_id, ++node_id_));
  accept(node->get_rhs());
}

// �����`
void ParseTreeGraphvizDumper::visit(boost::shared_ptr<ConstraintDefinition> node)  
{
  dump_node(node);
}

// �v���O������`
void ParseTreeGraphvizDumper::visit(boost::shared_ptr<ProgramDefinition> node)     
{
  dump_node(node);
}

// ����Ăяo��
void ParseTreeGraphvizDumper::visit(boost::shared_ptr<ConstraintCaller> node)      
{
  dump_node(node);
}

// �v���O�����Ăяo��
void ParseTreeGraphvizDumper::visit(boost::shared_ptr<ProgramCaller> node)         
{
  dump_node(node);
}

// ����
void ParseTreeGraphvizDumper::visit(boost::shared_ptr<Constraint> node)            
{
  dump_node(node);
}

// Ask����
void ParseTreeGraphvizDumper::visit(boost::shared_ptr<Ask> node)                   
{
  dump_node(node);
}

// Tell����
void ParseTreeGraphvizDumper::visit(boost::shared_ptr<Tell> node)                  
{
  dump_node(node);
}

// �Z�p�P�����Z�q
void ParseTreeGraphvizDumper::visit(boost::shared_ptr<Negative> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(boost::shared_ptr<Positive> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(boost::shared_ptr<Equal> node)                 
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(boost::shared_ptr<UnEqual> node)               
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(boost::shared_ptr<Less> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(boost::shared_ptr<LessEqual> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(boost::shared_ptr<Greater> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(boost::shared_ptr<GreaterEqual> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(boost::shared_ptr<Plus> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(boost::shared_ptr<Subtract> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(boost::shared_ptr<Times> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(boost::shared_ptr<Divide> node)
{
  dump_node(node);
}


void ParseTreeGraphvizDumper::visit(boost::shared_ptr<Power> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(boost::shared_ptr<LogicalAnd> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(boost::shared_ptr<LogicalOr> node)
{
  dump_node(node);
}  

void ParseTreeGraphvizDumper::visit(boost::shared_ptr<Weaker> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(boost::shared_ptr<Parallel> node)
{
  dump_node(node);
}
  
// �������Z�q
void ParseTreeGraphvizDumper::visit(boost::shared_ptr<Always> node)
{
  dump_node(node);
}
  
// ����
void ParseTreeGraphvizDumper::visit(boost::shared_ptr<Differential> node)
{
  dump_node(node);
}

// ���Ɍ�
void ParseTreeGraphvizDumper::visit(boost::shared_ptr<Previous> node)
{
  dump_node(node);
}

// �ȑO��PP�̒l
void ParseTreeGraphvizDumper::visit(boost::shared_ptr<PreviousPoint> node)
{
  dump_node(node);
}
  
// �ϐ�
void ParseTreeGraphvizDumper::visit(boost::shared_ptr<Variable> node)
{
  graph_node_id_t own_id = node_id_;
  nodes_.insert(make_pair(own_id, 
    "{" + node->get_node_type_name() + " | " + node->get_name() + "}"));
}

// ����
void ParseTreeGraphvizDumper::visit(boost::shared_ptr<Number> node)
{
  graph_node_id_t own_id = node_id_;
  nodes_.insert(make_pair(own_id, 
    "{" + node->get_node_type_name() + " | " + node->get_number() + "}"));
}


} //namespace parser
} //namespace hydla
