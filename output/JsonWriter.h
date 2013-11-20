#ifndef _HYDLA_OUTPUT_JSON_WRITER_H_
#define _HYDLA_OUTPUT_JSON_WRITER_H_

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include "PhaseResult.h"
#include "Simulator.h"

namespace hydla{
namespace output{

class JsonWriter{

  public:

  typedef hydla::simulator::PhaseResult                                       phase_result_t;
  typedef boost::shared_ptr<const phase_result_t>                             phase_result_const_sptr_t;
  
  void write(const phase_result_const_sptr_t &root);  

};


}// output
}// hydla

#endif // include guard
