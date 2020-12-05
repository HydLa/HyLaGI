#include "ModuleSetContainer.h"
#include <iostream>

namespace hydla {
namespace hierarchy {

std::ostream &operator<<(std::ostream &s, const ModuleSetContainer &m) {
  return m.dump(s);
}

ModuleSetContainer::ModuleSetContainer(ModuleSet m) : maximal_module_set_(m) {
  full_module_set_set_.insert(m);
}

ModuleSet ModuleSetContainer::get_max_module_set() const {
  return maximal_module_set_;
}

ModuleSet ModuleSetContainer::get_module_set() const {
  return *ms_to_visit_.rbegin();
}

ModuleSetContainer::module_set_set_t
ModuleSetContainer::get_full_ms_list() const {
  return full_module_set_set_;
}

void ModuleSetContainer::reset() { reset(full_module_set_set_); }

void ModuleSetContainer::reset(const module_set_set_t &mss) {
  ms_to_visit_ = mss;
}

ModuleSet ModuleSetContainer::unadopted_module_set() {
  ModuleSet ret = maximal_module_set_;
  ret.erase(get_module_set());
  return ret;
}

void ModuleSetContainer::generate_new_ms(const module_set_set_t &mms,
                                         const ModuleSet &ms) {
  for (auto m : ms_to_visit_) {
    if (m.including(ms))
      ms_to_visit_.erase(m);
  }
}

void ModuleSetContainer::remove_included_ms_by_current_ms() {
  /// current は現在のモジュール集合
  ModuleSet current = get_module_set();
  module_set_set_t::iterator lit = ms_to_visit_.begin();
  while (lit != ms_to_visit_.end()) {
    /**
     * ms_to_visit_内のモジュール集合で
     * currentが包含するモジュール集合を削除
     */
    if (current.including(*lit)) {
      lit = ms_to_visit_.erase(lit);
    } else
      lit++;
  }
}

} // namespace hierarchy
} // namespace hydla
