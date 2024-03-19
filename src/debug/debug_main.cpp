#include "debug_main.h"
#include "ParseTree.h"

// constraint hierarchy
#include "IncrementalModuleSet.h"
#include "ModuleSetContainerCreator.h"

#include "json_parser.h"
#include "solve_sym.h"

#include <iostream>

using namespace hydla::debug;
using namespace std;
using namespace hydla::parse_tree;
using namespace hydla::hierarchy;

namespace hydla {
namespace debug {
Debug::Debug() {}

void Debug::dump_debug(Debug *db, std::string string,
                       std::shared_ptr<ParseTree> pt,
                       std::shared_ptr<IncrementalModuleSet> msc) {
  std::stringstream sout1, sout2;
  std::vector<std::vector<std::vector<std::string>>> constraint_map;
  std::vector<std::vector<std::string>> v_map, h_map;

  msc->dump_priority_data_for_graphviz(sout2);
  pt->dump_in_json(sout1); // json形式の取得
  std::tie(constraint_map, h_map) = Json_p::json_p(sout1, sout2.str());
  v_map = Json_p::make_v_map(pt);
  db->debug_call(constraint_map, v_map, h_map);
}

void Debug::debug_call(
    std::vector<std::vector<std::vector<std::string>>> constraint_map,
    std::vector<std::vector<std::string>> v_map,
    std::vector<std::vector<std::string>> h_map) // ここで構文木をパースする
{
  Solve_sym::solve_main(constraint_map, v_map, h_map);
}
} // namespace debug
} // namespace hydla
