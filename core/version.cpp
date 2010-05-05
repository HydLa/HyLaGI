#include "version.h"

#include <sstream>

namespace hydla {
  Version::Version() {
  }
  
  Version::~Version() {
  }

  std::string Version::version()  {
    return "0.5.0";
  }
  
  std::string Version::revision() {
    return "$$Revision$$"; //template
  }
  
  std::string Version::copyright() {
    return "Copyright (C) 2008-2010 uedalab HydLa project";
  }
  
  std::string Version::description() {
    std::stringstream s;
    s << "hydla v" << version() << "\n\n" 
      //      << " (r" << revision() << ")\n\n" 
      << copyright();

    return s.str();
  }
}
