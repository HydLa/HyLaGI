#ifndef _INCLUDED_HYDLA_VERSION_H_
#define _INCLUDED_HYDLA_VERSION_H_

#include <string>

namespace hydla {
class Version {
 public:
  Version();
  ~Version();

  static std::string version();
  static std::string revision();
  static std::string copyright();
  static std::string description();
};
}
 
#endif //_INCLUDED_HYDLA_VERSION_H_
