
#include "StdProfilePrinter.h"
#include "PhaseResult.h"
#include <iostream>

using namespace std;

namespace hydla{
namespace io{


void StdProfilePrinter::print_profile(const entire_profile_t& result) const
{
  cout << "#------Simulation Time------\n";
  for(unsigned int i = 0; i < result.size(); i++){
    cout << "#------Simulation Phase " << result[i]->id << "------\n";
    for(profile_t::const_iterator it = result[i]->profile.begin(); it != result[i]->profile.end(); it++){
      cout << it->first << "\t: " << it->second << endl;
    }
  }
}


} // output
} // hydla
