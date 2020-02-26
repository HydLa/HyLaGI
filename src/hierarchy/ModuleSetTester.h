#pragma once

#include "ModuleSet.h"

namespace hydla {
namespace hierarchy {

class ModuleSetTester {
public:
  ModuleSetTester() {}
  virtual ~ModuleSetTester() {}

  virtual bool test_module_set(module_set_sptr ms) = 0;
};

} // namespace hierarchy
} // namespace hydla
