#pragma once

#include "Node.h"

namespace hydla { 
  namespace debug {
    class Debug{
      public:
     Debug(); 
     Debug(const Debug& db); 
      /**
       * 構文木を JSON 形式で出力する
       */
      //public:  void dump_debug();
      static void dump_debug(Debug *db);
      void debug_call();

    };
  }
}
