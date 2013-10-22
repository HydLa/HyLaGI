#include "PacketErrorHandler.h"

#include "MathLink.h"
#include "Logger.h"
#include "TimeOutError.h"
#include "LinkError.h"

#include <iostream>

namespace hydla {
namespace backend {
namespace mathematica {

bool PacketErrorHandler::handle(MathLink* ml) 
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  ml->get_next();
  ml->get_next();
  int ret_code = ml->get_integer();
  if(ret_code == 0) {
    throw LinkError("math", "input:\n" + ml->get_input_print() + "\n\ntrace:\n" + ml->get_debug_print(), 0, "");
    return true;
  }
  if(ret_code == -1) {
    throw hydla::timeout::TimeOutError("input:\n" + ml->get_input_print() + "\n\ntrace:\n" + ml->get_debug_print());
    return true;
  }
  ml->get_next();
  HYDLA_LOGGER_FUNC_END(VCS);
  return false;
}


} // namespace mathematica
} // namespace backend
} // namespace hydla 
