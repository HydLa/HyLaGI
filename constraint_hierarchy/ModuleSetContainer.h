#ifndef _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_H_
#define _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_H_

#include <boost/function.hpp>

#include "ModuleSet.h"

namespace hydla {
namespace ch {

class ModuleSetContainer {
public:
  ModuleSetContainer() 
  {}
  
  virtual ~ModuleSetContainer()
  {}

  /**
   * �W���̏W���̃_���v
   */
  virtual std::ostream& dump(std::ostream& s) const = 0;

  /**
   * �ɑ�Ȑ��񃂃W���[���W���𖳖����Ȃ��̂�������܂ł��߂�
   */
  virtual bool dispatch(boost::function<bool (hydla::ch::module_set_sptr)> callback_func, 
                        int threads = 1) = 0;
};

std::ostream& operator<<(std::ostream& s, const ModuleSetContainer& m);

} // namespace ch
} // namespace hydla

#endif // _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_H_
