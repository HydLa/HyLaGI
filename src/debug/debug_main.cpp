#pragma once

#include "debug_main.h"
#include "ParseTree.h"

// constraint hierarchy
#include "ModuleSetContainerCreator.h"
#include "IncrementalModuleSet.h"


#include "solve_sym.h"
#include "json_parser.h"

#include <iostream>

using namespace hydla::debug;
using namespace std;
using namespace hydla::parse_tree;
using namespace hydla::hierarchy;

namespace hydla{
  namespace debug{
    Debug::Debug(){}

    void Debug::dump_debug(Debug *db, std::string string, boost::shared_ptr<ParseTree> pt, boost::shared_ptr<IncrementalModuleSet> msc)
    {
      std::stringstream sout;
      std::stringstream sout1, sout2;
      std::vector<std::vector<std::vector<std::string>>> constraint_map;
      std::vector<std::vector<std::string>> v_map, h_map;

      cout << "huga" << std::endl;
      msc->dump_priority_data_for_graphviz(sout2);
      cout << sout2.str() << std::endl;
      msc->dump_module_sets_for_graphviz(sout);
      //cout << sout.str() << std::endl;
      //Json_p::make_h_map(sout);
      cout << "huga1" << std::endl;
      //h_map = make_candidate_set(msc);
      cout << "hugaaa1" << std::endl;
      pt->dump_in_json(sout1);//json形式の取得
      cout << sout1.str() << std::endl;
      std::tie(constraint_map, h_map) = Json_p::json_p(sout1, sout2.str());
      v_map = Json_p::make_v_map(pt);
      db -> debug_call(constraint_map, v_map, h_map);
    }

    void Debug::debug_tree(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree)
    {
      cout << "debug_tree" << std::endl;
      for (auto entry : parse_tree->get_variable_map())
      {
        cout << entry.first << std::endl;
        cout << entry.second << std::endl;
      }
      //auto def = parse_tree->get_constraint_definition();
    }

    void Debug::debug_call(std::vector<std::vector<std::vector<std::string>>> constraint_map, std::vector<std::vector<std::string>> v_map, std::vector<std::vector<std::string>> h_map) //ここで構文木をパースする
    {
      Solve_sym::solve_main(constraint_map, v_map, h_map);

      cout << "debug...." << std::endl;
    }

    std::vector<std::vector<std::string>> Debug::make_candidate_set(boost::shared_ptr<IncrementalModuleSet> msc){
      std::vector<std::vector<std::string>> ret;
      ModuleSet module;
      std::stringstream sout;

      //msc->dump_module_sets_for_graphviz(sout);




      msc->reset();
      module = msc->get_max_module_set();
      hierarchy::IncrementalModuleSet::module_set_set_t mss;
      //cout << "  \"" << msc->get_max_module_set().get_name() << "\"" << std::endl;
      while(msc->has_next())
      {
        ModuleSet tmp = msc->get_max_module_set();
        tmp.erase(msc->unadopted_module_set());
        msc->generate_new_ms(mss, tmp);
        //cout << tmp.get_name() << std::endl;
        for (auto ms : tmp)
        {
          //cout << &ms << std::endl;
        //cout << tmp.get_name() << std::endl;
          ModuleSet child = msc->get_max_module_set();
          child.erase(ms);
          if (tmp.including(child))
          {
            //cout << "  \"" << tmp.get_name() << "\" -> \"" << child.get_name() << "\"" << std::endl;
          }
        }
      }
      msc->reset();

      return ret;
    }
  }
}
