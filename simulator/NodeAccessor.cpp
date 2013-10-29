#include "NodeAccessor.h"
#include "PhaseSimulator.h"
#include <string>

using namespace std;

namespace hydla {
  namespace simulator {

GuardGetter::GuardGetter(){}
GuardGetter::~GuardGetter(){}

void GuardGetter::accept(const boost::shared_ptr<parse_tree::Node>& n)
{
  n->accept(n, this);
}

void GuardGetter::visit(boost::shared_ptr<parse_tree::Ask> node)
{
  asks.insert(node);
}

VaribleGetter::VaribleGetter(){}
VaribleGetter::~VaribleGetter(){}

void VaribleGetter::accept(const boost::shared_ptr<parse_tree::Node>& n)
{
  n->accept(n, this);
}

void VaribleGetter::visit(boost::shared_ptr<parse_tree::Differential> node)
{
  tmp_diff_cnt++;
  cout << "DIFF:" << tmp_diff_cnt << endl;
  node->get_child()->accept(node->get_child(),this);
  tmp_diff_cnt--;
}
void VaribleGetter::visit(boost::shared_ptr<parse_tree::Variable> node)
{
  guard_variable_t gv;
  gv.diff_cnt = tmp_diff_cnt;
  tmp_diff_cnt = 0;
  gv.name = node->get_name();
  vec_variable.push_back(gv);
}

}//namespace hydla
}//namespace simulator 
