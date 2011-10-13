#ifndef _INCLUDED_HYDLA_PARSE_TREE_H_
#define _INCLUDED_HYDLA_PARSE_TREE_H_

#include <ostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <cassert>
#include <set>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
//#include <boost/bimap/bimap.hpp>
//#include <boost/bimap/unordered_set_of.hpp>
//#include <boost/bimap/support/lambda.hpp>

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
  typedef std::map<node_id_t, node_sptr> node_map_t;
  typedef node_map_t::value_type         node_map_value_t;
  typedef node_map_t::const_iterator     node_map_const_iterator;

  // ノードID表
  typedef std::set<node_id_t>           node_id_list_t;
  typedef node_id_list_t::const_iterator node_id_list_const_iterator;

  /*
  struct tag_node_id {};
  struct tag_node_sptr {};

  typedef 
    boost::bimaps::bimap<
      boost::bimaps::unordered_set_of<
        boost::bimaps::tags::tagged<node_id_t, tag_node_id> >,
    boost::bimaps::unordered_set_of<
        boost::bimaps::tags::tagged<node_sptr, tag_node_sptr> > > node_map_t;

  typedef node_map_t::value_type                         node_map_value_t;
  */

  static const int INITIAL_MAX_NODE_ID = 0;

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
   * ノードIDの表の再構築をおこなう
   * 今までのノードIDは無効となる
   */
  void rebuild_node_id_list();

  /**
   * IDの割り当てられていないノードに対してIDを割り当てる
   * ノードが削除された場合は，ノード表から削除する
   */
  void update_node_id_list();

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
  std::string to_graphviz() const
  {
    std::stringstream str;
    to_graphviz(str);
    return str.str();
  }

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
   * ノード表から指定されたノードIDの情報を削除する
   */
  void remove_node(node_id_t id);
    
  /**
   * ノードに対応付けられているIDの変更
   */
  //void update_node_id(node_id_t id, const node_sptr& n);

  /**
   * 指定されたIDに対応するノードを得る
   */  
  node_sptr get_node(node_id_t id) const
  {
    node_map_t::const_iterator it = node_map_.find(id);
    if(it != node_map_.end()) {
      return it->second;
    }

    return node_sptr();
  }  
  
  /**
   * トップノードを得る
   */  
  node_sptr get_node() const
  {
    return node_tree_;
  }  
  
  /**
   * assertノードを得る
   */
  node_sptr get_assertion_node() const
  {
    return assertion_node_tree_;
  }
  
  /*
  node_sptr get_node(node_id_t id) const
  {
    node_map_t::map_by<tag_node_id>::const_iterator it = 
      node_map_.by<tag_node_id>().find(id);
    if(it != node_map_.by<tag_node_id>().end()) {
      return it->second;
    }
    return node_sptr();
  }
  */

  
  /**
   * 指定されたノードに対応するIDを得る
   */
  /*
  node_id_t get_node_id(const node_sptr& n) const
  {
    node_map_t::map_by<tag_node_sptr>::const_iterator it = 
      node_map_.by<tag_node_sptr>().find(n);
    if(it != node_map_.by<tag_node_sptr>().end()) {
      return it->second;
    }
    return node_id_t();
  }
  */

  /**
   * ノード表の最初の要素
   */
  node_map_const_iterator node_map_begin() const 
  {
    return node_map_.begin();
  }

  /**
   * ノード表の最後の次の要素
   */ 
  node_map_const_iterator node_map_end() const 
  {
    return node_map_.end();
  }

  /**
   * ノード表のサイズ
   */
  size_t node_map_size() const
  {
    return node_map_.size();
  }

  /**
   * ノード表中にあるノードIDのリストを作成する
   */
  void make_node_id_list();

  /**
   * ノードIDリストの最初の要素
   */
  node_id_list_const_iterator node_id_list_begin()
  {
    return node_id_list_.begin();
  }

  /**
   * ノードIDリストの最後の次の要素
   */
  node_id_list_const_iterator node_id_list_end()
  {
    return node_id_list_.end();
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
  ParseTree& operator=(const ParseTree& pt);

  node_factory_sptr    node_factory_;

  node_sptr            node_tree_;
  node_sptr            assertion_node_tree_;
  
  variable_map_t       variable_map_;

  node_map_t           node_map_;
  node_id_t            max_node_id_;
  node_id_list_t       node_id_list_;
};

std::ostream& operator<<(std::ostream& s, const ParseTree& pt);


} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_H_
