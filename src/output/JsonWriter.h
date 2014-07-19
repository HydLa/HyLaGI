#ifndef _HYDLA_OUTPUT_JSON_WRITER_H_
#define _HYDLA_OUTPUT_JSON_WRITER_H_

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include "PhaseResult.h"
#include "Simulator.h"
#include "picojson.h"

namespace hydla{
namespace output{

class JsonWriter{

  public:

  typedef hydla::simulator::PhaseResult                                       phase_result_t;
  typedef boost::shared_ptr<phase_result_t>                                   phase_result_sptr_t;
  typedef boost::shared_ptr<const phase_result_t>                             phase_result_const_sptr_t;
  typedef hydla::simulator::variable_map_t variable_map_t;
  typedef hydla::simulator::variable_set_t variable_set_t;
  typedef hydla::simulator::parameter_map_t parameter_map_t;
  typedef hydla::simulator::Simulator       simulator_t;
  typedef hydla::simulator::ValueRange      value_range_t;

  void write(const simulator_t &simulator, std::string name);
  virtual void set_epsilon_mode(hydla::simulator::backend_sptr_t back, bool flag);
  hydla::simulator::backend_sptr_t backend_;
  bool epsilon_mode_flag = false;
  private:
  picojson::value for_phase(const phase_result_const_sptr_t &phase);
  picojson::value for_vm(const variable_map_t &vm);
  picojson::value for_range(const value_range_t &range);
  picojson::value for_vs(const variable_set_t &vs);
  picojson::value for_pm(const parameter_map_t &pm);
  picojson::value make_children(const phase_result_const_sptr_t &phase);
  picojson::value for_range_diff(const value_range_t &range);
};


}// output
}// hydla

#endif // include guard
