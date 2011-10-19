#include "Types.h"

#include <sstream>

namespace hydla {
namespace simulator {

namespace {
  struct NodeDumper {
      
    template<typename T>
    NodeDumper(T it, T end) 
    {
      for(; it!=end; ++it) {
        ss << **it << "\n";
      }
    }

    friend std::ostream& operator<<(std::ostream& s, const NodeDumper& nd)
    {
      s << nd.ss.str();
      return s;
    }

    std::stringstream ss;
  };
}

std::ostream& operator<<(std::ostream& s, const ask_set_t& a)
{
  s << NodeDumper(a.begin(), a.end());
  return s;
}

std::ostream& operator<<(std::ostream& s, const tells_t& a)
{
  s << NodeDumper(a.begin(), a.end());
  return s;
}

std::ostream& operator<<(std::ostream& s, const collected_tells_t& a)
{
  s << NodeDumper(a.begin(), a.end());
  return s;
}

std::ostream& operator<<(std::ostream& s, const expanded_always_t& a)
{
  s << NodeDumper(a.begin(), a.end());
  return s;
}

std::ostream& operator<<(std::ostream& s, const expanded_always_id_t& a)
{
  expanded_always_id_t::const_iterator it = a.begin();
  expanded_always_id_t::const_iterator end = a.end();

  for(; it!=end; ++it){
    s << *it << "\n";
  }
  return s;
}

std::ostream& operator<<(std::ostream& s, const continuity_map_t& continuity_map)
{
  for(continuity_map_t::const_iterator it = continuity_map.begin(); it != continuity_map.end(); it++){
    s << it->first << "," << it->second << "\n";
  }
  return s;
}

} //namespace simulator
} //namespace hydla 

