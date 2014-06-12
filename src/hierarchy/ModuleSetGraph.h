#pragma once
/*
#include <vector>

#include <boost/bimap/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>

#include "ModuleSet.h"
#include "ModuleSetContainer.h"

namespace hydla {
namespace hierarchy {

class ModuleSetGraph : public ModuleSetContainer {
public:

  struct superset {};
  struct subset {};
  
  typedef boost::bimaps::bimap<
    boost::bimaps::multiset_of<
      boost::bimaps::tags::tagged<module_set_sptr, superset>,
      ModuleSetComparator>,
    boost::bimaps::multiset_of<
      boost::bimaps::tags::tagged<module_set_sptr, subset>,
      ModuleSetComparator>
  > edges_t;
  

  ModuleSetGraph();
  ModuleSetGraph(module_set_sptr m);

  virtual ~ModuleSetGraph();
  
  /**
   * 並列合成として集合を合成する
   *
  void add_parallel(ModuleSetGraph& parallel_module_set_graph);

  /**
   * 並列合成として集合を合成する（required制約扱い）
   *
  void add_required_parallel(ModuleSetGraph& parallel_module_set_graph);
  
  /**
   * 弱合成として集合を合成する
   *
  void add_weak(ModuleSetGraph& weak_module_set_graph);

  /**
   * 集合の集合のダンプ
   *
  virtual std::ostream& dump(std::ostream& s) const;
  
  virtual void reset(const module_set_list_t &mss);

  /**
   *ノードの情報の名前表現によるダンプ
   *
  std::ostream& dump_node_names(std::ostream& s) const;

  /**
   * ノードの情報のツリー表現によるダンプ
   *
  std::ostream& dump_node_trees(std::ostream& s) const;

  /**
   * エッジの情報のダンプ
   *
  std::ostream& dump_edges(std::ostream& s) const;

  /**
   * graphvizで解釈可能な形式で出力をおこなう
   *
  std::ostream& dump_graphviz(std::ostream& s) const;


  /**
   * そのノードと子ノードをマーキングし，以降探索しないようにする
   *
  virtual void mark_nodes();
  

private:
  /**
   * 与えられたノードおよび，
   * それに包含されるノードに対して訪問フラグを立てる
   *
  void mark_visited_flag(const module_set_sptr& ms);

  /**
   * グラフの辺を構築する
   *
  void build_edges();

  /**
   * 辺
   *
  edges_t edges_;
  
  /**
   * 特定のモジュール集合を含むことによる枝刈と，
   * 極大性を満たさないことによる枝刈とを区別する必要がありそうなので，
   * 極大性を満たさない枝刈のためにms_to_visitとは別に使用する変数．
   *
  std::set<module_set_sptr> visited_module_sets_;

};



} // namespace hierarchy
} // namespace hydla
*/
