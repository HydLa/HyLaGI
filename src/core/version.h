#pragma once

#include <string>

namespace hydla {
class Version {
public:
  Version();
  ~Version();

  static std::string commit_id();
  static std::string version();
  static std::string revision();
  static std::string copyright();
  static std::string description();
};
} // namespace hydla
