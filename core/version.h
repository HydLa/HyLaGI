#ifndef _INCLUDED_HYDLA_VERSION_H_
#define _INCLUDED_HYDLA_VERSION_H_

#include <sstream>

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
  
  static const char* revision() {
    return "$Revision$"; //template for subversion
  }
  
  static const char* last_change_date() {
    return "$LastChangedDate$"; //template for subversion
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
