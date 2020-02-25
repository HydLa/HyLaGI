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

    void split(std::vector<std::string> &dst, const std::string &src, const std::string &delim){
      std::string::size_type l = 0, r = 0;
      while((r = src.find_first_of(delim, l)) != std::string::npos){
	dst.push_back(src.substr(l,r-l));
	r++;
	l = r;
      }
      dst.push_back(src.substr(l,src.size()));
    }
  }
}
