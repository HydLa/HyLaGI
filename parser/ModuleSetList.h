#ifndef _INCLUDED_HTDLA_CH_MODULE_SET_LIST_H_
#define _INCLUDED_HTDLA_CH_MODULE_SET_LIST_H_

#include <vector>

#include "ModuleSet.h"

namespace hydla {
namespace ch {

class ModuleSetList {
public:
  typedef std::vector<module_set_sptr> module_set_list_t;

  ModuleSetList();
  ModuleSetList(module_set_sptr m);
  ~ModuleSetList();

  void add_parallel(ModuleSetList& parallel_module_set_list);
  void add_weak(ModuleSetList& weak_module_set_list);
  std::ostream& dump(std::ostream& s);

private:
  module_set_list_t module_set_list_;
};

std::ostream& operator<<(std::ostream& s, ModuleSetList& m);

} // namespace ch
} // namespace hydla

#endif //_INCLUDED_HTDLA_CH_MODULE_SET_LIST_H_