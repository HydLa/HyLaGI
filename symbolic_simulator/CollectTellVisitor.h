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

  // ÄêµÁ
  virtual void visit(boost::shared_ptr<ConstraintDefinition> node);
  virtual void visit(boost::shared_ptr<ProgramDefinition> node);

  // ¸Æ¤Ó½Ð¤·
  virtual void visit(boost::shared_ptr<ConstraintCaller> node);
  virtual void visit(boost::shared_ptr<ProgramCaller> node);     

  // À©Ìó¼°
  virtual void visit(boost::shared_ptr<Constraint> node);

  // AskÀ©Ìó
  virtual void visit(boost::shared_ptr<Ask> node);

  // TellÀ©Ìó
  virtual void visit(boost::shared_ptr<Tell> node);

  // Èæ³Ó±é»»»Ò
  virtual void visit(boost::shared_ptr<Equal> node);
  virtual void visit(boost::shared_ptr<UnEqual> node);
  virtual void visit(boost::shared_ptr<Less> node);
  virtual void visit(boost::shared_ptr<LessEqual> node);
  virtual void visit(boost::shared_ptr<Greater> node);
  virtual void visit(boost::shared_ptr<GreaterEqual> node);

  // ÏÀÍý±é»»»Ò
  virtual void visit(boost::shared_ptr<LogicalAnd> node);
  virtual void visit(boost::shared_ptr<LogicalOr> node);

  // »»½ÑÆó¹à±é»»»Ò
  virtual void visit(boost::shared_ptr<Plus> node);
  virtual void visit(boost::shared_ptr<Subtract> node);
  virtual void visit(boost::shared_ptr<Times> node);
  virtual void visit(boost::shared_ptr<Divide> node);

  // »»½ÑÃ±¹à±é»»»Ò
  virtual void visit(boost::shared_ptr<Negative> node);
  virtual void visit(boost::shared_ptr<Positive> node);

  // À©Ìó³¬ÁØÄêµÁ±é»»»Ò
  virtual void visit(boost::shared_ptr<Weaker> node);
  virtual void visit(boost::shared_ptr<Parallel> node);

  // »þÁê±é»»»Ò
  virtual void visit(boost::shared_ptr<Always> node);

  // ÈùÊ¬
  virtual void visit(boost::shared_ptr<Differential> node);

  // º¸¶Ë¸Â
  virtual void visit(boost::shared_ptr<Previous> node);
  
  // ÊÑ¿ô
  virtual void visit(boost::shared_ptr<Variable> node);

  // ¿ô»ú
  virtual void visit(boost::shared_ptr<Number> node);


private:
  MathLink& ml_;
  ParseTree& parse_tree_;
  std::set<std::string> vars_;

};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_COLLECT_TELL_VISITOR_H__

