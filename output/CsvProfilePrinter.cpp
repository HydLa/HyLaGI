
#include "CsvProfilePrinter.h"
#include "PhaseResult.h"
#include <iostream>

using namespace std;

namespace hydla{
namespace output{


void CsvProfilePrinter::print_profile(const entire_profile_t& result) const
{
  cout << "#------Simulation Time------\n";
  if(result.size() > 0){
    cout << "Simulation Time, ";
    for(unsigned int i = 0; i < result.size(); i++){
      phase_result_t& pr = *result[i]->phase_result;
      if(pr.phase == simulator::PointPhase)
      {
        cout << "Point";
      }
      else
      {
        cout << "Interval";
      }
      cout << "Phase " << result[i]->phase_result->id << ", ";
    }
    cout << "\n";
    for(profile_t::const_iterator it = result[0]->profile.begin(); it != result[0]->profile.end(); it++){ 
      cout << it->first << ", ";
      for(unsigned int i = 0; i < result.size(); i++){
        cout << result[i]->profile[it->first] << ", ";
      }
      cout << "\n";
    }
  }
}


} // output
} // hydla
