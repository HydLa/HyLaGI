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
   * ���񍇐��Ƃ��ďW������������
   */
  void add_parallel(ModuleSetGraph& parallel_module_set_graph);

  /**
   * ���񍇐��Ƃ��ďW������������irequired���񈵂��j
   */
  void add_required_parallel(ModuleSetGraph& parallel_module_set_graph);
  
  /**
   * �㍇���Ƃ��ďW������������
   */
  void add_weak(ModuleSetGraph& weak_module_set_graph);

  /**
   * �W���̏W���̃_���v
   */
  virtual std::ostream& dump(std::ostream& s) const;
  
  virtual void reset(const module_set_list_t &mss);

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
   * ���̃m�[�h�Ǝq�m�[�h���}�[�L���O���C�ȍ~�T�����Ȃ��悤�ɂ���
   */
  virtual void mark_nodes();
  

private:
  /**
   * �^����ꂽ�m�[�h����сC
   * ����ɕ�܂����m�[�h�ɑ΂��ĖK��t���O�𗧂Ă�
   */
  void mark_visited_flag(const module_set_sptr& ms);

  /**
   * �O���t�̕ӂ��\�z����
   */
  void build_edges();

  /**
   * ��
   */
  edges_t edges_;
  
  /**
   * ����̃��W���[���W�����܂ނ��Ƃɂ��}���ƁC
   * �ɑ含�𖞂����Ȃ����Ƃɂ��}���Ƃ���ʂ���K�v�����肻���Ȃ̂ŁC
   * �ɑ含�𖞂����Ȃ��}���̂��߂�ms_to_visit�Ƃ͕ʂɎg�p����ϐ��D
   */
  std::set<module_set_sptr> visited_module_sets_;

};



} // namespace ch
} // namespace hydla

#endif // _INCLUDED_HTDLA_CH_MODULE_SET_GRAPH_H_
