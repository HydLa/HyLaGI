#include "PacketErrorHandler.h"

#include "mathlink_helper.h"
#include "Logger.h"
#include "TimeOutError.h"
#include "../SolveError.h"

#include <iostream>

namespace hydla {
namespace vcs {
namespace mathematica {

bool PacketErrorHandler::handle(MathLink* ml) 
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  ml->get_next();
  ml->get_next();
  int ret_code = ml->get_integer();
  if(ret_code == 0) {
    HYDLA_LOGGER_LOCATION(VCS);
    throw SolveError("input:\n" + ml->get_input_print() + "\n\ntrace:\n" + ml->get_debug_print());
    return true;
  }
  if(ret_code == -1) {
    HYDLA_LOGGER_LOCATION(VCS);
    throw hydla::timeout::TimeOutError("input:\n" + ml->get_input_print() + "\n\ntrace:\n" + ml->get_debug_print());
    return true;
  }
  ml->get_next();
  HYDLA_LOGGER_FUNC_END(VCS);
  return false;
}


} // namespace mathematica
} // namespace simulator
} // namespace hydla 

