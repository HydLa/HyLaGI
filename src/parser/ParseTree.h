#pragma once

#include <ostream>
#include <string>
#include <map>
#include <cassert>
#include <set>

#include <memory>
#include <boost/make_shared.hpp>

#include "ParseError.h"
#include "Node.h"

namespace hydla { 
namespace parse_tree {


class ParseTree {
public: 
  // 変数表
  typedef std::map<std::string, int>     variable_map_t;
  typedef variable_map_t::const_iterator variable_map_const_iterator;

    
  ParseTree();
  ParseTree(const ParseTree& pt);

  virtual ~ParseTree();

  /**
   * ParseTreeを構築する
   */
  void parse_string(std::string str)
  {
    std::istringstream stream(str);
    parse(stream);
  }
    
  void parse(std::istream& s);

  /**
   * 変数を登録する
   * すでに登録済みの同一変数の微分回数よりも
   * 大きかったら変数のリストに登録される
   *
   * @param name 変数名
   * @param differential_count 微分回数
   *
   * @return 登録されたかどうか
   */
  bool register_variable(const std::string& name, int differential_count);

  /**
   * 指定した変数の最大微分回数を求める
   */
  int get_differential_count(const std::string& name) const;

  /**
   * 変数表の先頭の要素を返す
   */
  variable_map_const_iterator variable_map_begin() const
  {
    return variable_map_.begin();
  }

  /**
   * 変数表の最後の次の要素を返す
   */
  variable_map_const_iterator variable_map_end() const
  {
    return variable_map_.end();
  }
  
  
  variable_map_t get_variable_map() const
  {
    return variable_map_;
  }


  /**
   * パースされたノードツリーの設定
   * 設定された後，意味解析等の前処理は自動でおこなわれる
   */
  //void set_tree(const symbolic_expression::node_sptr& tree);
 
  /**
   * ノードツリーを交換する
   * 新しいノードツリーは意味解析等の前処理をおこなった後である必要がある
   */
  symbolic_expression::node_sptr swap_tree(const symbolic_expression::node_sptr& tree);

  bool is_same_struct(const ParseTree& pt, bool exactly_same) const {
    return node_tree_->is_same_struct(*pt.node_tree_, exactly_same);
  }

  /**
   * dot言語形式での表示
   */
  std::string to_graphviz() const
  {
    std::stringstream str;
    to_graphviz(str);
    return str.str();
  }

  std::ostream& to_graphviz(std::ostream& s) const;

  /**
   * 構文木を JSON 形式で出力する
   */
  std::ostream& dump_in_json(std::ostream& s) const;

  /**
   * ノードツリーに対してビジターを適用する
   */
  void dispatch(symbolic_expression::TreeVisitor* visitor)
  {
    if(node_tree_) node_tree_->accept(node_tree_, visitor);
  }

  /**
   * ノードツリーに対してビジターを適用する
   */
  void dispatch(symbolic_expression::BaseNodeVisitor* visitor)
  {
    if(node_tree_) node_tree_->accept(node_tree_, visitor);
  }

  /**
   * トップノードを得る
   */  
  symbolic_expression::node_sptr get_node() const
  {
    return node_tree_;
  }  
  
  /**
   * assertノードを得る
   */
  symbolic_expression::node_sptr get_assertion_node() const
  {
    return assertion_node_tree_;
  }

  
  /**
   * すべてのデータを破棄し、初期状態に戻す
   */
  void clear();

  /**
   * ParseTreeの状態の出力
   */
  std::ostream& dump(std::ostream& s) const;

private:  
  ParseTree& operator=(const ParseTree& pt);

  symbolic_expression::node_sptr            node_tree_;
  symbolic_expression::node_sptr            assertion_node_tree_;
  
  variable_map_t       variable_map_;

};

std::ostream& operator<<(std::ostream& s, const ParseTree& pt);


} //namespace parse_tree
} //namespace hydla

