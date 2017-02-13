#pragma once

#include "Node.h"
#include "ParseTree.h"


// constraint hierarchy
#include "ModuleSetContainerCreator.h"
#include "IncrementalModuleSet.h"

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
      static void dump_debug(Debug *db, std::string string, boost::shared_ptr<parse_tree::ParseTree> pt, boost::shared_ptr<hierarchy::IncrementalModuleSet> msc);
      void debug_call();

    };
  }
}
