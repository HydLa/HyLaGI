#include "JsonReader.h"
#include "Logger.h"
#include <fstream>
#include "Utility.h"
#include "Constants.h"

using namespace std;
using namespace picojson;
using namespace hydla::utility;
using namespace hydla::parser;

namespace hydla{
namespace io{

simulator::phase_result_sptr_t JsonReader::read_phase(const std::string &name)
{
  ifstream ifs;
  ifs.open(name.c_str());
  value json;
  ifs >> json;
  ifs.close();
  
  return read_phase(json.get<object>() );
}

JsonReader::phase_result_sptr_t JsonReader::read_phase(object &json_object)
{ 
  //TODO: positive_asksとかnegative_asksとかも読む
  phase_result_sptr_t phase(new phase_result_t());
  phase->id = json_object["id"].get<long>();
  string phase_type_str = json_object["type"].get<string>();
  if(phase_type_str == "PP")phase->phase_type = simulator::PointPhase;
  else phase->phase_type = simulator::IntervalPhase;
  if(phase->phase_type == simulator::PointPhase)
  {
    object time_object = json_object["time"].get<object>();
    string time_str = time_object["time_point"].get<string>();
    phase->current_time =
      ast.parse_generate(time_str, HydLaAST::ARITHMETIC_EXPRESSION);
  }
  else if(phase->phase_type == simulator::IntervalPhase)
  {

    object time_object = json_object["time"].get<object>();
    string start_str = time_object["start_time"].get<string>();
    phase->current_time =
      ast.parse_generate(start_str, HydLaAST::ARITHMETIC_EXPRESSION);
    string end_str = time_object["end_time"].get<string>();
    phase->end_time = ast.parse_generate(end_str, HydLaAST::ARITHMETIC_EXPRESSION);
  }
  phase->variable_map = read_vm(json_object["variable_map"].get<object>());
  phase->parameter_map = read_pm(json_object["parameter_map"].get<object>());
  picojson::array children = json_object["children"].get<picojson::array>();
  for(auto child : children)
  {
    phase->children.push_back(read_phase(child.get<object>()));
  }
  if(json_object.find("cause_for_termination") != json_object.end())
  {
    string cause_str = json_object["cause_for_termination"].get<string>();
    phase->cause_for_termination = get_cause_for_string(cause_str);
  }
  return phase; 
}

JsonReader::value_range_t JsonReader::read_range(object &o)
{
  value_range_t range;
  if(o.find("unique_value") != o.end())
  {
    string value_str = o["unique_value"].get<string>();
    range.set_unique_value(ast.parse_generate(value_str, HydLaAST::ARITHMETIC_EXPRESSION));
  }
  else
  {
    if(o.find("lower_bounds") != o.end())
    {
      picojson::array bound_array = o["lower_bounds"].get<picojson::array>();
      for(auto bound : bound_array)
      {
        object bound_object = bound.get<object>();
        bool closed = bound_object["closed"].get<bool>();
        simulator::value_t value = 
          ast.parse_generate(bound_object["value"].get<string>(), HydLaAST::ARITHMETIC_EXPRESSION);
        range.add_lower_bound(value, closed);
      }
    }

    if(o.find("upper_bounds") != o.end())
    {
      picojson::array bound_array = o["upper_bounds"].get<picojson::array>();
      for(auto bound : bound_array)
      {
        object bound_object = bound.get<object>();
        bool closed = bound_object["closed"].get<bool>();
        simulator::value_t value = 
          ast.parse_generate(bound_object["value"].get<string>(), HydLaAST::ARITHMETIC_EXPRESSION);
        range.add_lower_bound(value, closed);
      }
    }
  }
  return range;
}


JsonReader::variable_map_t JsonReader::read_vm(object &o)
{
  variable_map_t vm;
  for(auto it : o)
  {
    const std::string &str = it.first;
    const value_range_t &range = read_range(it.second.get<object>());
    simulator::variable_t var(str);
    vm[var] = range;
  }
  return vm;
}

JsonReader::parameter_map_t JsonReader::read_pm(object &o)
{
  parameter_map_t pm;
  for(auto it :o)
  {
    const std::string &str = it.first;
    const value_range_t &range = read_range(it.second.get<object>());
    simulator::parameter_t par(str);
    pm[par] = range; 
  }
  return pm;
}


}
}
