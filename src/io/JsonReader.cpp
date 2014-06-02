#include "JsonReader.h"
#include "Logger.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include "Utility.h"
#include "Constants.h"

using namespace std;
using namespace picojson;
using namespace hydla::utility;

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
  phase_result_sptr_t phase(new phase_result_t());
  phase->id = json_object["id"].get<int>();
  string phase_type_str = json_object["type"].get<string>();
  if(phase_type_str == "PP")phase->phase_type = simulator::PointPhase;
  else phase->phase_type = simulator::IntervalPhase;
  if(phase->phase_type == simulator::PointPhase)
  {
/*    object time_object;
    
    time_object["time_point"] =
      value(phase->current_time.get_string());
    phase_object["time"] = value(time_object);
*/
  }
  else if(phase->phase_type == simulator::IntervalPhase)
  {
/*
    object time_object;
    time_object["start_time"] = 
      value(phase->current_time.get_string());
    if(!phase->end_time.undefined())
    {
      time_object["end_time"] = 
        value(phase->end_time.get_string());
    }
    phase_object["time"] = value(time_object);
*/
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
  return value_range_t();
}


JsonReader::variable_map_t JsonReader::read_vm(object &o)
{
  variable_map_t vm;
  for(auto it : o)
  {
    const std::string &key = it.first;
    const value_range_t &range = read_range(it.second.get<object>());
    //TODO: 変数の名前と微分回数読む
    //vm[key] = range;
  }
  return vm;
}

JsonReader::parameter_map_t JsonReader::read_pm(object &o)
{
/*  object pm_obj;
  for(parameter_map_t::const_iterator it = pm.begin(); it != pm.end(); it++)
  {
    std::string key = it->first.to_string();
    pm_obj[key] = for_range(it->second);
  }
  return value(pm_obj);
*/
  return parameter_map_t();
}


}
}
