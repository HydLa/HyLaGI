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
    /// ���W���[���W��
    module_set_sptr mod;

    /// �m�[�h�̖K��t���O
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
   * ���񍇐��Ƃ��ďW������������
   */
  void add_parallel(ModuleSetGraph& parallel_module_set_graph);
  
  /**
   * �㍇���Ƃ��ďW������������
   */
  void add_weak(ModuleSetGraph& weak_module_set_graph);

  /**
   * �W���̏W���̃_���v
   */
  virtual std::ostream& dump(std::ostream& s) const;

  /**
   * �m�[�h�̏��̖��O�\���ɂ��_���v
   */
  std::ostream& dump_node_names(std::ostream& s) const;
  
  /**
   * �m�[�h�̏��̃c���[�\���ɂ��_���v
   */
  std::ostream& dump_node_trees(std::ostream& s) const;

  /**
   * �G�b�W�̏��̃_���v
   */
  std::ostream& dump_edges(std::ostream& s) const;

  /**
   * graphviz�ŉ��߉\�Ȍ`���ŏo�͂������Ȃ�
   */
  std::ostream& dump_graphviz(std::ostream& s) const;

  /**
   * 
   */
  virtual bool dispatch(boost::function<bool (hydla::ch::module_set_sptr)> callback_func, 
                        int threads = 1);

private:
  /**
   * �^����ꂽ�m�[�h����сC
   * ����ɕ�܂����m�[�h�ɑ΂��ĖK��t���O�𗧂Ă�
   */
  void mark_visited_flag(Node* node);

  /**
   * �O���t�̕ӂ��\�z����
   */
  void build_edges();

  /**
   * ��
   */
  edges_t edges_;

  /**
   * �m�[�h
   */
  nodes_t nodes_;
};



} // namespace ch
} // namespace hydla

#endif // _INCLUDED_HTDLA_CH_MODULE_SET_GRAPH_H_
