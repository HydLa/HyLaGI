#include "version.h"

#include <sstream>

namespace hydla {
Version::Version() {}

Version::~Version() {}

std::string Version::commit_id() { return COMMIT_ID; }

std::string Version::version() { return "0.9.5"; }

std::string Version::revision() {
  return "$$Revision$$"; // template
}

std::string Version::copyright() {
  return "Copyright (C) 2008-2017 UEDA Lab. HydLa project";
}

std::string Version::description() {
  std::stringstream s;
  s << "HyLaGI v" << version() << "(" << commit_id()
    << ")"
       "\n\n"
    //      << " (r" << revision() << ")\n\n"
    << copyright();

  return s.str();
}
} // namespace hydla
