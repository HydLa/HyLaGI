#ifndef _INCLUDED_HYDLA_TELL_COLLECTOR_H_
#define _INCLUDED_HYDLA_TELL_COLLECTOR_H_

#include <vector>
#include <set>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "ModuleSet.h"
#include "TreeVisitor.h"

namespace hydla {
namespace simulator {

class TellCollector : public parse_tree::TreeVisitor {
public:
  TellCollector();
  virtual ~TellCollector();
  
  void collect_tell(hydla::ch::module_set_sptr&                               ms,
                    std::vector<boost::shared_ptr<hydla::parse_tree::Tell> >* new_tells,
                    std::set<boost::shared_ptr<hydla::parse_tree::Tell> >*    collected_tells,
                    std::set<boost::shared_ptr<hydla::parse_tree::Ask> >*     entailed_asks);

  // ����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Constraint> node);

  // Ask����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // Tell����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);

  // �_����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);
  
  // �������Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Always> node);

private:
  std::vector<boost::shared_ptr<hydla::parse_tree::Tell> >* new_tells_;
  std::set<boost::shared_ptr<hydla::parse_tree::Tell> >*    collected_tells_;
  std::set<boost::shared_ptr<hydla::parse_tree::Ask> >*     entailed_asks_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_TELL_COLLECTOR_H_
