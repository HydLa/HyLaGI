#include "StdProfilePrinter.h"
#include "PhaseResult.h"
#include <iostream>

using namespace std;

namespace hydla {
namespace io {

void StdProfilePrinter::print_profile(const entire_profile_t &result) const {
  size_t max_characters = 0;
  size_t max_digits = 0;
  std::string profile_title = "Simulation Time";
  for (auto phase : result) {
    for (auto entry : phase->profile) {
      if (max_characters < entry.first.length())
        max_characters = entry.first.length();
      if (max_digits < std::to_string(entry.second).length())
        max_digits = std::to_string(entry.second).length();
    }
  }
  cout << "#";
  for (size_t i = 0;
       i < (max_characters + max_digits + 2 - profile_title.length()) / 2; i++)
    cout << "=";
  cout << profile_title;
  for (size_t i = 0;
       i < (max_characters + max_digits + 2 - profile_title.length()) / 2; i++)
    cout << "=";
  cout << endl << endl;
  for (auto phase : result) {
    for (size_t i = 0; i < (max_characters + max_digits - 3) / 2; i++)
      cout << "-";
    cout << " ";
    cout << (phase->phase_type == simulator::POINT_PHASE ? "PP " : "IP ")
         << phase->id;
    cout << " ";
    for (size_t i = 0; i < (max_characters + max_digits - 3) / 2; i++)
      cout << "-";
    cout << endl;
    for (auto entry : phase->profile) {
      cout << entry.first;
      for (size_t i = entry.first.length(); i < max_characters; i++)
        cout << " ";
      cout << " : ";
      for (size_t i = 0; i < max_digits - std::to_string(entry.second).length();
           i++)
        cout << " ";
      cout << entry.second << endl;
    }
    cout << endl;
  }
}

} // namespace io
} // namespace hydla
