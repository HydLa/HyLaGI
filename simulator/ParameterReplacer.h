#ifndef _INCLUDED_HYDLA_PARAMETER_REPLACER_H_
#define _INCLUDED_HYDLA_PARAMETER_REPLACER_H_

#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "DefaultTreeVisitor.h"
#include "ValueVisitor.h"
#include "PhaseResult.h"

namespace hydla {
namespace simulator {

/**
 * VCSで仮にID = -1としたパラメータを，適切なものに置き換える（置き換えると言うかID変えるだけ）
 * TODO: このクラス自体が本来なら不要な気がする．
 * 数式のツリー作る際にどこでパラメータが必要になるかは原理的に分かりそうだから，どうにかしたい．
 * いずれにしろ，ID = -1 がマジックナンバーになってしまっている．
 */
class ParameterReplacer : public parse_tree::DefaultTreeVisitor, hydla::simulator::ValueVisitor{
public:

  typedef std::map< std::pair<std::string, int>, int > parameter_id_map_t;

  ParameterReplacer();

  virtual ~ParameterReplacer();
  
  void add_mapping(const std::string& name, const int& derivative_count, const int& id);
  
  virtual void visit(hydla::simulator::symbolic::SymbolicValue&);
  
  void replace_value(value_t& val);
  
  // 記号定数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parameter> node);
  
  private:
  parameter_id_map_t parameter_id_map_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_PARAMETER_REPLACER_H_
