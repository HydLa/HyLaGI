
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
    std::set<std::string> label_set;
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
      for(profile_t::const_iterator it = result[i]->profile.begin(); it != result[i]->profile.end(); it++){
        label_set.insert(it->first);
        //フェーズによってシミュレーションしないモジュール集合などもあるため，必要なラベルをすべて列挙しておく．
      }
    }
    cout << "\n";
    for(std::set<std::string>::const_iterator it = label_set.begin(); it != label_set.end(); it++){ 
      cout << *it << ", ";
      for(unsigned int i = 0; i < result.size(); i++){
        cout << result[i]->profile[*it] << ", ";
      }
      cout << "\n";
    }
  }
}


} // output
} // hydla
