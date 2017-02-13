#pragma once

#include "Node.h"

namespace hydla { 
  namespace debug {
    class Debug{
     Debug(); 
      /**
       * 構文木を JSON 形式で出力する
       */
      public:  void dump_debug();

    };
  }
}
