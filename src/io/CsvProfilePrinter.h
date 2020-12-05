#pragma once

#include "ProfilePrinter.h"
#include <iostream>

namespace hydla {
namespace io {

class CsvProfilePrinter {
public:
  virtual void print_profile(const entire_profile_t &) const;
  CsvProfilePrinter(std::ostream &stream = std::cout)
      : output_stream_(stream) {}

private:
  std::ostream &output_stream_;
};

} // namespace io
} // namespace hydla
