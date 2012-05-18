#include "PacketErrorHandler.h"

#include "mathlink_helper.h"
#include "Logger.h"

#include <iostream>

namespace hydla {
namespace vcs {
namespace mathematica {

bool PacketErrorHandler::handle(MathLink* ml, int ret_code) 
{  
  HYDLA_LOGGER_REST(
    "-- PacketErrorHandler::handle --\n", 
    "ret_code: ", ret_code);

  if(ret_code == 0) {
    //ml->MLGetNext();
    // TODO: エラーを取得し，表示する
    /*
     int error_size = ml->get_arg_count();
     HYDLA_LOGGER_DEBUG("error_size: ", error_size);
     ml->MLGetNext();
     for(int i=0; i<error_size; i++) {
       ml->MLGetNext();
       std::cout << ml->MLGetNext() << "\n";      
       HYDLA_LOGGER_ERROR("err msg:", ml->get_symbol());
       std::cout << ml->MLGetNext() << "\n";      
       HYDLA_LOGGER_ERROR("err msg:", ml->get_symbol());
     }*/
    //ml->MLNextPacket();
    ml->MLNewPacket();
    return true;
  }

  return false;
}

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

