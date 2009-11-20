#ifndef _INCLUDED_HYDLA_PARSE_TREE_H_
#define _INCLUDED_HYDLA_PARSE_TREE_H_

#include <string>
#include <vector>
#include <map>

#include <boost/shared_ptr.hpp>

#include "ParseError.h"
#include "Node.h"

namespace hydla { 
namespace parse_tree {

class ParseTree {
public:
  ParseTree();
  virtual ~ParseTree();

  void addConstraintDefinition(boost::shared_ptr<ConstraintDefinition> d);
  void addProgramDefinition(boost::shared_ptr<ProgramDefinition> d);
  void setTree(node_sptr& tree) {node_tree_ = tree;}

  std::string to_string();
  void preprocess();

  void dispatch(parse_tree::TreeVisitor* visitor)
  {
    if(node_tree_) node_tree_->accept(node_tree_, visitor);
  }

  const variable_map_t& get_variable_map() const 
  {
    return variable_map_;
  }

  /**
   * すべてのデータを破棄し、初期状態に戻す
   */
  void clear()
  {
    node_tree_.reset();
    cons_def_map_.clear();
    prog_def_map_.clear();
  }

private:
  difinition_type_t create_definition_key(boost::shared_ptr<Definition> d);

  node_sptr            node_tree_;
  constraint_def_map_t cons_def_map_;
  program_def_map_t    prog_def_map_;
  variable_map_t       variable_map_;
};

} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_H_
