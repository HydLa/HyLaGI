#ifndef _HYDLA_OUTPUT_JSON_WRITER_H_
#define _HYDLA_OUTPUT_JSON_WRITER_H_

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include "PhaseResult.h"
#include "Simulator.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

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
  typedef boost::property_tree::ptree       ptree_t;
  typedef hydla::simulator::Simulator       simulator_t;
  
  void write(const simulator_t &simulator);  
  private:
  ptree_t for_phase(const phase_result_const_sptr_t &phase);
  ptree_t for_vm(const variable_map_t &vm); 
  ptree_t for_vs(const variable_set_t &vs); 
  ptree_t for_pm(const parameter_map_t &pm); 
  ptree_t make_children(const phase_result_const_sptr_t &phase);
  ptree_t parse_tree_;
};


}// output
}// hydla

#endif // include guard
