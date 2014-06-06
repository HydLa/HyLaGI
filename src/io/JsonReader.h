#pragma once
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include "PhaseResult.h"
#include "Simulator.h"
#include "picojson.h"
#include "HydLaAST.h"

namespace hydla{
namespace io{

class JsonReader{

  public:

  typedef hydla::simulator::PhaseResult                                       phase_result_t;
  typedef boost::shared_ptr<phase_result_t>                                   phase_result_sptr_t;
  typedef boost::shared_ptr<const phase_result_t>                             phase_result_const_sptr_t;
  typedef hydla::simulator::variable_map_t variable_map_t;
  typedef hydla::simulator::variable_set_t variable_set_t;
  typedef hydla::simulator::parameter_map_t parameter_map_t;
  typedef hydla::simulator::Simulator       simulator_t;
  typedef hydla::simulator::ValueRange      value_range_t;
  
  /**
   * read Json corresponding to one phase.
   */
  phase_result_sptr_t read_phase(const std::string &name);
  phase_result_sptr_t read_phase(picojson::object &o);
  private:
  parser::HydLaAST ast;
  variable_map_t read_vm(picojson::object &o); 
  value_range_t read_range(picojson::object &o); 
  parameter_map_t read_pm(picojson::object &o); 
};


}
}
