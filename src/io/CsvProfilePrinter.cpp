
#include "CsvProfilePrinter.h"
#include "PhaseResult.h"

#include <iomanip>

using namespace std;

namespace hydla{
namespace io{


void CsvProfilePrinter::print_profile(const entire_profile_t& result) const
{
  if(!result.empty()){
    output_stream_ << "Simulation Time, ";
    set<string> label_set;
    // collect necessary labels
    for(auto phase : result)
    {
      if(phase->phase_type == simulator::POINT_PHASE)
      {
        output_stream_ << "P";
      }
      else
      {
        output_stream_ << "I";
      }
      output_stream_ << "P " << phase->id << ", ";
      for(auto entry : phase->profile){
        label_set.insert(entry.first);
      }
    }

    output_stream_ << "Sum";
    output_stream_ << "\n";
    for(auto label : label_set)
    {
      output_stream_ << label << ", ";
      double sum = 0;
      for(auto phase : result){
        output_stream_ << phase->profile[label] << ", ";
        sum += phase->profile[label];
      }
      output_stream_ << sum << "\n";
    }
  }
}


} // output
} // hydla
