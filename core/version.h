#ifndef _INCLUDED_HYDLA_VERSION_H_
#define _INCLUDED_HYDLA_VERSION_H_

#include <sstream>
#include <string>
#include <algorithm>

/*
 * To enable subversion propset
 * svn propset svn:keywords "LastChangedDate Revision" version.h
 */

namespace hydla {
class Version {
 public:
  Version(){}
  ~Version(){}

  static const char* version()  {
    return "0.2.5";
  }
  
  static std::string revision() {
    std::string rev("$Revision$"); //template for subversion
    return rev.substr(11, rev.size() - 11 - 2);
  }
  
  static std::string last_change_date() {
    std::string date("$LastChangedDate$"); //template for subversion
    return date.substr(18, date.find(' ', 18)-18);
  }

  static const char* copyright() {
    return "Copyright (C) 2008-2009 uedalab HydLa project";
  }
  
  static const char* description() {
    std::stringstream s;
    s << "hydla v" << version() 
      << " (" << last_change_date()
      << " r" << revision() << ")\n\n" 
      << copyright();

    return s.str().c_str();
  }
};
}
 
#endif //_INCLUDED_HYDLA_VERSION_H_
