#include "JsonWriter.h"
#include "Logger.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include "Utility.h"

using namespace std;
using namespace picojson;
using namespace hydla::utility;

namespace hydla{
namespace output{

void JsonWriter::write(const simulator_t &simulator, std::string name)
{
  stringstream sstr;
  object json_object;
  json_object["variables"] = for_vs(simulator.get_variable_set());
  json_object["parameters"] = for_pm(simulator.get_parameter_map());

  phase_result_const_sptr_t root = simulator.get_result_root();
  array children;
  json_object["first_phases"] = make_children(root);

  value json(json_object);
 
  std::ofstream ofs;
  ofs.open(name.c_str());
  ofs << json.serialize();
  ofs.close(); 
}


value JsonWriter::for_phase(const phase_result_const_sptr_t &phase)
{
  object phase_object;
  phase_object["id"] = value((long)phase->id);
  if(phase->phase_type == simulator::PointPhase)
  {
    phase_object["type"] = value(string("PP"));
    object time_object;
    time_object["time_point"] =
      value(phase->current_time.get_string());
    phase_object["time"] = value(time_object);
  }
  else if(phase->phase_type == simulator::IntervalPhase)
  {
    phase_object["type"] = value(string("IP"));

    object time_object;
    time_object["start_time"] = 
      value(phase->current_time.get_string());
    if(!phase->end_time.undefined())
    {
      time_object["end_time"] = 
        value(phase->end_time.get_string());
    }
    phase_object["time"] = value(time_object);
  }
  phase_object["variable_map"] = for_vm(phase->variable_map);
  phase_object["parameter_map"] = for_pm(phase->parameter_map);
  phase_object["children"] = make_children(phase);
  if(phase->children.size() == 0){
    std::string cot;
    switch(phase->cause_for_termination){
    case simulator::TIME_LIMIT:
      cot = "TIME_LIMIT";
      break;
    case simulator::STEP_LIMIT:
      cot = "STEP_LIMIT";
      break;
    case simulator::SOME_ERROR:
      cot = "SOME_ERROR";
      break;
    case simulator::INCONSISTENCY:
      cot = "INCONSISTENCY";
      break;
    case simulator::ASSERTION:
      cot = "ASSERTION";
      break;
    case simulator::OTHER_ASSERTION:
      cot = "OTHER_ASSERTION";
      break;
    case simulator::TIME_OUT_REACHED:
      cot = "TIME_OUT_REACHED";
      break;
    case simulator::NOT_UNIQUE_IN_INTERVAL:
      cot = "NOT_UNIQUE_IN_INTERVAL";
      break;
    case simulator::NOT_SELECTED:
      cot = "NOT_SELECTED";
      break;
    case simulator::NONE:
      cot = "NONE";
      break;
    default:
      cot = "ERROR";
    }
    phase_object["cause_for_termination"] = value(cot);
  }
  return value(phase_object);
}


value JsonWriter::make_children(const phase_result_const_sptr_t &phase)
{
  array children;
  for(vector<phase_result_sptr_t>::const_iterator it = phase->children.begin(); it != phase->children.end(); it++)
  {
    children.push_back(for_phase(*it));
  }
  return value(children);
}

value JsonWriter::for_range(const value_range_t &range)
{
  object range_object;

  if(range.unique())
  {
    range_object["unique_value"] = value(range.get_unique().get_string());
  }
  else
  {
    array lbs;
    for(uint i = 0; i < range.get_lower_cnt(); i++)
    {
      const value_range_t::bound_t &bound = range.get_lower_bound(i);
      object lb;
      lb["value"] = value(bound.value.get_string());
      lb["closed"] = value(bound.include_bound);
      lbs.push_back(value(lb));
    }
    range_object["lower_bounds"] = value(lbs);
 
    array ubs;
    for(uint i = 0; i < range.get_upper_cnt(); i++)
    {
      const value_range_t::bound_t &bound = range.get_upper_bound(i);
      object ub;
      ub["value"] = value(bound.value.get_string());
      ub["closed"] = value(bound.include_bound);
      ubs.push_back(value(ub));
    }
    range_object["upper_bounds"] = value(ubs);
  }
  return value(range_object);
}


value JsonWriter::for_vm(const variable_map_t &vm)
{
  object vm_obj;
  for(variable_map_t::const_iterator it = vm.begin(); it != vm.end(); it++)
  {
    const std::string &key = it->first.get_string();
    const value_range_t &range = it->second;
    vm_obj[key] = for_range(range);
  }
  return value(vm_obj);
}

value JsonWriter::for_vs(const variable_set_t &vs)
{
  array vs_array;
  for(variable_set_t::const_iterator it = vs.begin(); it != vs.end(); it++)
  {
    vs_array.push_back(value(it->get_string()));
  }
  return value(vs_array);
}

value JsonWriter::for_pm(const parameter_map_t &pm)
{
  object pm_obj;
  for(parameter_map_t::const_iterator it = pm.begin(); it != pm.end(); it++)
  {
    std::string key = it->first.to_string();
    pm_obj[key] = for_range(it->second);
  }
  return value(pm_obj);
}


}
}
