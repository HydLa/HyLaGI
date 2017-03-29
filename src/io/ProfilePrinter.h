#pragma once

#include "Types.h"

namespace hydla{
namespace io{

class ProfilePrinter{
public:
  virtual void print_profile(const entire_profile_t&) const = 0;
};

}// output
}// hydla

