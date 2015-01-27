
#include "StdProfilePrinter.h"
#include "PhaseResult.h"
#include <iostream>

using namespace std;

namespace hydla{
namespace io{


void StdProfilePrinter::print_profile(const entire_profile_t& result) const
{
  cout << "#------Simulation Time------\n";
  for(auto phase : result)
  {
    for(auto entry : phase->profile){
      cout << entry.first << "\t: " << entry.second << endl;
    }
  }
}


} // output
} // hydla
