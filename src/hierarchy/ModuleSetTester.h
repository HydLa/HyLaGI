#ifndef _INCLUDED_HTDLA_CH_MODULE_SET_TESTER_H_
#define _INCLUDED_HTDLA_CH_MODULE_SET_TESTER_H_

#include "ModuleSet.h"

namespace hydla {
namespace hierarchy {

class ModuleSetTester {
public:
  ModuleSetTester(){}
  virtual ~ModuleSetTester(){}

  virtual bool test_module_set(module_set_sptr ms) = 0;
};

} // namespace hierarchy
} // namespace hydla


#endif //_INCLUDED_HTDLA_CH_MODULE_SET_TESTER_H_
