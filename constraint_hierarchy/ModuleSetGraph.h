#ifndef _INCLUDED_HTDLA_CH_MODULE_SET_GRAPH_H_
#define _INCLUDED_HTDLA_CH_MODULE_SET_GRAPH_H_

#include <vector>

#include <boost/bimap/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>

#include "ModuleSet.h"
#include "ModuleSetContainer.h"

namespace hydla {
namespace ch {

class ModuleSetGraph : public ModuleSetContainer {
public:
  struct Node {
    /// モジュール集合
    module_set_sptr mod;

    /// ノードの訪問フラグ
    bool            visited;
  };

  struct NodeComp {
    bool operator()(const Node& lhs, const Node& rhs) const
    {      
      return ModuleSetComparator()(lhs.mod, rhs.mod);
    }
     
    bool operator()(const Node* lhs, const Node* rhs) const
    {      
      return ModuleSetComparator()(lhs->mod, rhs->mod);
    }
  };

  typedef std::vector<Node> nodes_t;

  struct superset {};
  struct subset {};
  
  typedef boost::bimaps::bimap<
    boost::bimaps::multiset_of<
      boost::bimaps::tags::tagged<Node*, superset>,
      NodeComp>,
    boost::bimaps::multiset_of<
      boost::bimaps::tags::tagged<Node*, subset>,
      NodeComp>
  > edges_t;


  ModuleSetGraph();
  ModuleSetGraph(module_set_sptr m);

  virtual ~ModuleSetGraph();
  
  /**
   * 並列合成として集合を合成する
   */
  void add_parallel(ModuleSetGraph& parallel_module_set_graph);
  
  /**
   * 弱合成として集合を合成する
   */
  void add_weak(ModuleSetGraph& weak_module_set_graph);

  /**
   * モジュール名の集合の集合を返す
   */ 
  std::string get_name() const;

  /**
   * 集合の集合のパースツリーの内容出力
   */
  std::ostream& dump(std::ostream& s) const;

  std::ostream& to_graphviz(std::ostream& s) const;

  /**
   * 
   */
  virtual bool dispatch(boost::function<bool (hydla::ch::module_set_sptr)> callback_func, 
                        int threads = 1);

  /**
   * グラフの辺を構築する
   */
  void build_edges();

private:
  /**
   * 与えられたノードおよび，
   * それに包含されるノードに対して訪問フラグを立てる
   */
  void mark_visited_flag(Node* node);

  /**
   * 辺
   */
  edges_t edges_;

  /**
   * ノード
   */
  nodes_t nodes_;
};


std::ostream& operator<<(std::ostream& s, const ModuleSetGraph& m);

} // namespace ch
} // namespace hydla

#endif // _INCLUDED_HTDLA_CH_MODULE_SET_GRAPH_H_
