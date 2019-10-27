#include "boost_algo_str.h"

namespace hydla{
  namespace debug{
    void replace_all(std::string &str, const std::string &src, const std::string &dst){
      if(src.empty()) return;
      
      std::string::size_type pos = 0;
      while((pos = str.find(src, pos)) != std::string::npos){
	str.replace(pos, src.size(), dst);
	pos += dst.size();
      }
    }
  }
}
