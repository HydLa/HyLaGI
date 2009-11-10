#ifndef _INCLUDED_HYDLA_COLLECT_TELL_VISITOR_H_
#define _INCLUDED_HYDLA_COLLECT_TELL_VISITOR_H_

#include "Node.h"
#include "TreeVisitor.h"
#include "mathlink_helper.h"
#include "ParseTree.h"
#include <set>

using namespace hydla::parse_tree;

namespace hydla {
namespace symbolic_simulator {

class CollectTellVisitor : public parse_tree::TreeVisitor {
public:
  CollectTellVisitor(ParseTree& parse_tree, MathLink& ml);

  virtual ~CollectTellVisitor();

  bool is_consistent();

  // ���
  virtual void visit(boost::shared_ptr<ConstraintDefinition> node);
  virtual void visit(boost::shared_ptr<ProgramDefinition> node);

  // �ƤӽФ�
  virtual void visit(boost::shared_ptr<ConstraintCaller> node);
  virtual void visit(boost::shared_ptr<ProgramCaller> node);     

  // ����
  virtual void visit(boost::shared_ptr<Constraint> node);

  // Ask����
  virtual void visit(boost::shared_ptr<Ask> node);

  // Tell����
  virtual void visit(boost::shared_ptr<Tell> node);

  // ��ӱ黻��
  virtual void visit(boost::shared_ptr<Equal> node);
  virtual void visit(boost::shared_ptr<UnEqual> node);
  virtual void visit(boost::shared_ptr<Less> node);
  virtual void visit(boost::shared_ptr<LessEqual> node);
  virtual void visit(boost::shared_ptr<Greater> node);
  virtual void visit(boost::shared_ptr<GreaterEqual> node);

  // �����黻��
  virtual void visit(boost::shared_ptr<LogicalAnd> node);
  virtual void visit(boost::shared_ptr<LogicalOr> node);

  // �������黻��
  virtual void visit(boost::shared_ptr<Plus> node);
  virtual void visit(boost::shared_ptr<Subtract> node);
  virtual void visit(boost::shared_ptr<Times> node);
  virtual void visit(boost::shared_ptr<Divide> node);

  // ����ñ��黻��
  virtual void visit(boost::shared_ptr<Negative> node);
  virtual void visit(boost::shared_ptr<Positive> node);

  // ����������黻��
  virtual void visit(boost::shared_ptr<Weaker> node);
  virtual void visit(boost::shared_ptr<Parallel> node);

  // ����黻��
  virtual void visit(boost::shared_ptr<Always> node);

  // ��ʬ
  virtual void visit(boost::shared_ptr<Differential> node);

  // ���˸�
  virtual void visit(boost::shared_ptr<Previous> node);
  
  // �ѿ�
  virtual void visit(boost::shared_ptr<Variable> node);

  // ����
  virtual void visit(boost::shared_ptr<Number> node);


private:
  MathLink& ml_;
  ParseTree& parse_tree_;
  std::set<std::string> vars_;

};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_COLLECT_TELL_VISITOR_H__

