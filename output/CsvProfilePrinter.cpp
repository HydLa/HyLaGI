
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
      simulator::SimulationTodo& todo = *result[i];
      if(todo.phase == simulator::PointPhase)
      {
        output_stream_ << "Point";
      }
      else
      {
        output_stream_ << "Interval";
      }
      output_stream_ << "Phase " << todo.id << ", ";
      for(profile_t::const_iterator it = todo.profile.begin(); it != todo.profile.end(); it++){
        label_set.insert(it->first);
        //フェーズによってシミュレーションしないモジュール集合などもあるため，必要なラベルをすべて列挙しておく．
      }
    }
    output_stream_ << "Sum";
    output_stream_ << "\n";
    for(std::set<std::string>::const_iterator it = label_set.begin(); it != label_set.end(); it++){
      output_stream_ << *it << ", ";
      int sum = 0;
      for(unsigned int i = 0; i < result.size(); i++){
        simulator::SimulationTodo& todo = *result[i];
        output_stream_ << todo.profile[*it] << ", ";
        sum += todo.profile[*it];
      }
      output_stream_ << sum << "\n";
    }
  }
}


} // output
} // hydla
