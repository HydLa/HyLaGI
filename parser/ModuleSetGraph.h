#ifndef _INCLUDED_HTDLA_CH_MODULE_SET_GRAPH_H_
#define _INCLUDED_HTDLA_CH_MODULE_SET_GRAPH_H_

#include "ModuleSet.h"

namespace hydla {
namespace ch {

class ModuleSetGraph {
public:
  ModuleSetGraph();
  ModuleSetGraph(module_set_sptr m);

  ~ModuleSetGraph();

  void add_parallel(ModuleSetGraph& parallel_module_set_graph);
  void add_weak(ModuleSetGraph& parallel_module_set_graph);
  std::ostream& dump(std::ostream& s);

private:

};

} // namespace ch
} // namespace hydla

#endif // _INCLUDED_HTDLA_CH_MODULE_SET_GRAPH_H_
