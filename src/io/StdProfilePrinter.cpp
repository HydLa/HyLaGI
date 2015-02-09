
#include "StdProfilePrinter.h"
#include "PhaseResult.h"
#include <iostream>

using namespace std;

namespace hydla{
namespace io{


void StdProfilePrinter::print_profile(const entire_profile_t& result) const
{
  int max_characters = 0;
  int max_digits = 0;
  std::string profile_title = "Simulation Time";
  for(auto phase : result)
  {
    for(auto entry : phase->profile){
      if(max_characters < entry.first.length()) max_characters = entry.first.length();
      if(max_digits < std::to_string(entry.second).length()) max_digits = std::to_string(entry.second).length();
    }
  }
  cout << "#";
  for(int i = 0; i < (max_characters + max_digits + 2 - profile_title.length()) / 2; i++) cout << "=";
  cout << profile_title;
  for(int i = 0; i < (max_characters + max_digits + 2 - profile_title.length()) / 2; i++) cout << "=";
  cout << endl << endl;
  for(auto phase : result)
  {
    for(int i = 0; i < (max_characters + max_digits - 3) / 2; i++) cout << "-";
    cout << " ";
    cout << (phase->phase_type==simulator::POINT_PHASE ? "PP " : "IP ") << phase->id;
    cout << " ";
    for(int i = 0; i < (max_characters + max_digits - 3) / 2; i++) cout << "-";
    cout << endl;
    for(auto entry : phase->profile){
      cout << entry.first; 
      for(int i = entry.first.length(); i < max_characters; i++) cout << " "; 
      cout << " : ";
      for(int i = 0; i < max_digits - std::to_string(entry.second).length(); i++) cout << " ";
      cout << entry.second << endl;
    }
    cout << endl;
  }
}


} // output
} // hydla
