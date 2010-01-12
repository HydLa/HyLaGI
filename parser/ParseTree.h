#ifndef _INCLUDED_HYDLA_PARSE_TREE_H_
#define _INCLUDED_HYDLA_PARSE_TREE_H_

#include <ostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cassert>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/bimap/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/bimap/support/lambda.hpp>

#include "ParseError.h"
#include "Node.h"
#include "NodeFactory.h"

namespace hydla { 
namespace parse_tree {

class ParseTree {
public: 
  typedef hydla::parser::NodeFactory        node_factory_t;
  typedef boost::shared_ptr<node_factory_t> node_factory_sptr;

  // 変数表
  typedef std::map<std::string, int>     variable_map_t;
  typedef variable_map_t::const_iterator variable_map_const_iterator;

    
  // ノード表
  typedef boost::bimaps::bimap<
            boost::bimaps::unordered_set_of<node_id_t>, 
            boost::bimaps::unordered_set_of<node_sptr> > node_map_t;
  typedef node_map_t::value_type                         node_map_value_t;

  ParseTree();
  ParseTree(const ParseTree& pt);

  virtual ~ParseTree();

  /**
   * ParseTreeを構築する
   */
  template<typename NodeFactoryT>
  void parse(std::istream& s)
  {
    parse(s, boost::make_shared<NodeFactoryT>());
  }

  template<typename NodeFactoryT>
  void parse_string(std::string str)
  {
    std::istringstream stream(str);
    parse<NodeFactoryT>(stream);
  }
    
  void parse(std::istream& s, node_factory_sptr node_factory);


  /**
   * ノードのIDの更新をおこなう
   */
  void uptate_node_id();

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

  /**
   * パースされたノードツリーの設定
   * 設定された後，意味解析等の前処理は自動でおこなわれる
   */
  //void set_tree(const node_sptr& tree);
 
  /**
   * ノードツリーを交換する
   * 新しいノードツリーは意味解析等の前処理をおこなった後である必要がある
   */
  node_sptr swap_tree(const node_sptr& tree);

  bool is_same_struct(const ParseTree& pt, bool exactly_same) const {
    return node_tree_->is_same_struct(*pt.node_tree_, exactly_same);
  }

  /**
   * dot言語形式での表示
   */
  std::ostream& to_graphviz(std::ostream& s) const;

  /**
   * ノードツリーに対してビジターを適用する
   */
  void dispatch(parse_tree::TreeVisitor* visitor)
  {
    if(node_tree_) node_tree_->accept(node_tree_, visitor);
  }

  /**
   * ノードツリーに対してビジターを適用する
   */
  void dispatch(parse_tree::BaseNodeVisitor* visitor)
  {
    if(node_tree_) node_tree_->accept(node_tree_, visitor);
  }

  /**
   * 新しいノードを追加する
   */
  node_id_t register_node(const node_sptr& n);

  /**
   * IDに対応付けられているノードの変更
   */
  void update_node(node_id_t id, const node_sptr& n);
    
  /**
   * ノードに対応付けられているIDの変更
   */
  void update_node_id(node_id_t id, const node_sptr& n);

  /**
   * 指定されたIDに対応するノードを得る
   */
  node_sptr get_node(node_id_t id)
  {
    node_map_t::left_iterator it = node_map_.left.find(id);
    if(it != node_map_.left.end()) {
      return it->second;
    }
    return node_sptr();
  }
  
  /**
   * 指定されたノードに対応するIDを得る
   */
  node_id_t get_node_id(const node_sptr& n)
  {
    node_map_t::right_iterator it = node_map_.right.find(n);
    if(it != node_map_.right.end()) {
      return it->second;
    }
    return node_id_t();
  }

  /**
   * 登録されているNodeFactoryを元に指定された型のノードを生成する
   */
  template<typename NodeType>
  boost::shared_ptr<NodeType> create_node() const
  {
    return node_factory_->create<NodeType>();
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
  node_factory_sptr    node_factory_;

  node_sptr            node_tree_;
  variable_map_t       variable_map_;

  node_map_t           node_map_;
  node_id_t            max_node_id_;
};

std::ostream& operator<<(std::ostream& s, const ParseTree& pt);


} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_H_
