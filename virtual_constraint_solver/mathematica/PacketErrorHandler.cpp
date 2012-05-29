#include "PacketErrorHandler.h"

#include "mathlink_helper.h"
#include "Logger.h"

#include <iostream>

namespace hydla {
namespace vcs {
namespace mathematica {

bool PacketErrorHandler::handle(MathLink* ml) 
{
  HYDLA_LOGGER_VCS("#*** Begin PacketErrorHandler::handle ***");
  ml->get_next();
  ml->get_next();
  int ret_code = ml->get_integer();
  if(ret_code == 0) {
    HYDLA_LOGGER_VCS("#*** End PacketErrorHandler::handle with return true ***");
    return true;
  }
  ml->get_next();
  HYDLA_LOGGER_VCS("#*** End PacketErrorHandler::handle ***");
  return false;
}


} // namespace mathematica
} // namespace simulator
} // namespace hydla 

