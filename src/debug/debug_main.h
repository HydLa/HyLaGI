#pragma once

#include "Node.h"

namespace hydla { 
  namespace debug {
    class Debug{
      
      /**
       * 構文木を JSON 形式で出力する
       */
      std::ostream& dump_debug(std::ostream& s) const;

    };
  }
}