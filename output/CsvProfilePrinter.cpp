
#include "CsvProfilePrinter.h"
#include "PhaseResult.h"
#include <iostream>

using namespace std;

namespace hydla{
namespace output{


void CsvProfilePrinter::print_profile(const entire_profile_t& result) const
{
  if(result.size() > 0){
    output_stream_ << "Simulation Time, ";
    std::set<std::string> label_set;
    for(unsigned int i = 0; i < result.size(); i++){
      phase_result_t& pr = *result[i]->phase_result;
      if(pr.phase == simulator::PointPhase)
      {
        output_stream_ << "Point";
      }
      else
      {
        output_stream_ << "Interval";
      }
      output_stream_ << "Phase " << result[i]->phase_result->id << ", ";
      for(profile_t::const_iterator it = result[i]->profile.begin(); it != result[i]->profile.end(); it++){
        label_set.insert(it->first);
        //�t�F�[�Y�ɂ���ăV�~�����[�V�������Ȃ����W���[���W���Ȃǂ����邽�߁C�K�v�ȃ��x�������ׂė񋓂��Ă����D
      }
    }
    output_stream_ << "Sum";
    output_stream_ << "\n";
    for(std::set<std::string>::const_iterator it = label_set.begin(); it != label_set.end(); it++){ 
      output_stream_ << *it << ", ";
      int sum = 0;
      for(unsigned int i = 0; i < result.size(); i++){
        output_stream_ << result[i]->profile[*it] << ", ";
        sum += result[i]->profile[*it];
      }
      output_stream_ << sum << "\n";
    }
  }
}


} // output
} // hydla
