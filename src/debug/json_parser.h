#pragma once

#include <iostream>
#include "picojson.h"
#include "ParseTree.h"

namespace hydla { 
  namespace debug {
    class Json_p{
    public:
      Json_p(); 
      static std::tuple<std::vector<std::vector<std::vector<std::string>>>, std::vector<std::vector<std::string>>> json_p(std::stringstream& sout, std::string prio);
      static std::string json_search(picojson::value jstr, bool always, bool ask);
      static std::string make_equation(picojson::value jstr);
      static std::vector<std::vector<std::vector<std::string>>> make_vector(std::string serch_str);
      static std::vector<std::vector<std::string>> make_v_map(std::shared_ptr<hydla::parse_tree::ParseTree> parse_tree);
      static std::vector<std::vector<std::string>> make_h_map(std::vector<std::vector<std::vector<std::string>>> v_ret, std::map<std::string, std::vector<std::string>> p_ret);
      static std::map<std::string, std::vector<std::string>> make_prio_map(std::string prio);
    };
  }
}
