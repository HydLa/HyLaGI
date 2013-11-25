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
  typedef boost::shared_ptr<const phase_result_t>                             phase_result_const_sptr_t;
  typedef hydla::simulator::variable_map_t variable_map_t;
  
  void write(const phase_result_const_sptr_t &root);  
  void read_vm(const variable_map_t &vm); 
  private:
  boost::property_tree::ptree parse_tree_;
};


}// output
}// hydla

#endif // include guard
