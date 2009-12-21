#ifndef _INCLUDED_HTDLA_CH_MODULE_SET_GRAPH_H_
#define _INCLUDED_HTDLA_CH_MODULE_SET_GRAPH_H_

#include <vector>

#include <boost/bimap/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>

#include "ModuleSet.h"
#include "ModuleSetContainer.h"

namespace hydla {
namespace ch {

class ModuleSetGraph {
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

  struct Node {
    module_set_sptr mod;
    bool            visited;
  };

  struct NodeComp {
    bool operator()(const Node& lhs, const Node& rhs) const
    {      
      return ModuleSetComparator()(lhs.mod, rhs.mod);
    }
  };

  typedef std::vector<Node> nodes_t;

  ModuleSetGraph();
  ModuleSetGraph(module_set_sptr m);

  virtual ~ModuleSetGraph();
  
  /**
   * ���񍇐��Ƃ��ďW������������
   */
  void add_parallel(ModuleSetGraph& parallel_module_set_graph);
  
  /**
   * �㍇���Ƃ��ďW������������
   */
  void add_weak(ModuleSetGraph& weak_module_set_graph);

  /**
   * �W���̏W��(���̃N���X)�̖��O
   */ 
  std::string get_name() const;

  /**
   * �W���̏W���̃p�[�X�c���[�̓��e�o��
   */
  std::ostream& dump(std::ostream& s) const;

  /**
   * 
   */
  virtual bool dispatch(boost::function<bool (hydla::ch::module_set_sptr)> callback_func, 
                        int threads = 1);

  /**
   * �O���t�̕ӂ��\�z����
   */
  void build_edges();

private:

  /**
   * ��
   */
  edges_t edges_;

  /**
   * �m�[�h
   */
  nodes_t nodes_;
};


std::ostream& operator<<(std::ostream& s, const ModuleSetGraph& m);

} // namespace ch
} // namespace hydla

#endif // _INCLUDED_HTDLA_CH_MODULE_SET_GRAPH_H_
