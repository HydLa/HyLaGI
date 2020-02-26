#pragma once

#include <memory>
#include "PhaseResult.h"
#include "Simulator.h"
#include "picojson.h"

namespace hydla {
namespace io {

class JsonReader
{
public:
  typedef simulator::PhaseResult                   phase_result_t;
  typedef std::shared_ptr<phase_result_t>        phase_result_sptr_t;
  typedef std::shared_ptr<const phase_result_t>  phase_result_const_sptr_t;
  typedef simulator::variable_map_t                variable_map_t;
  typedef simulator::variable_set_t                variable_set_t;
  typedef simulator::parameter_map_t               parameter_map_t;
  typedef simulator::Simulator                     simulator_t;
  typedef simulator::ValueRange                    value_range_t;
  
  /**
   * read Json corresponding to one phase.
   */
  phase_result_sptr_t read_phase(const std::string &name);
  phase_result_sptr_t read_phase(picojson::object &json_object);

private:
  variable_map_t read_vm(picojson::object &o); 
  value_range_t read_range(picojson::object &o); 
  parameter_map_t read_pm(picojson::object &o); 
};

} // namespace io
} // namespace hydla
