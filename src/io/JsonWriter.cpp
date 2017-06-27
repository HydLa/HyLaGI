#include "JsonWriter.h"
#include "Logger.h"
#include <fstream>
#include "Utility.h"
#include "Constants.h"
#include "Backend.h"

using namespace std;
using namespace picojson;
using namespace hydla::utility;

using namespace hydla::simulator;
using namespace hydla::backend;

namespace hydla {
namespace io {

void JsonWriter::write(const simulator_t &simulator, const std::string &name, const std::string &hydla_name)
{
  object json_object;
  json_object["variables"] = for_vs(simulator.get_variable_set());
  json_object["parameters"] = for_pm(simulator.get_parameter_map());

  phase_result_const_sptr_t root = simulator.get_result_root();
  picojson::array children;
  json_object["first_phases"] = make_children(root);
  json_object["name"] = value(hydla_name);

  value json(json_object);

  std::ofstream ofs;
  ofs.open(name.c_str());
  try
  {
    ofs.exceptions(ios::eofbit | ios::failbit | ios::badbit);
    ofs << json.serialize();
    ofs.close();
  }
  catch(ifstream::failure e)
  {
    cerr << "warning: destination hydat cannot be written." << endl;
  }
}

void JsonWriter::write_phase(const phase_result_const_sptr_t &phase, const std::string &name)
{
  std::ofstream ofs;
  ofs.open(name.c_str());
  ofs << for_phase(phase).serialize();
  ofs.close();
}

value JsonWriter::for_phase(const phase_result_const_sptr_t &phase)
{
  //TODO: positive_asksとかnegative_asksとかも書く
  object phase_object;
  phase_object["id"] = value((long)phase->id);
  if (phase->phase_type == simulator::POINT_PHASE)
  {
    phase_object["type"] = value(string("PP"));
    object time_object;
    time_object["time_point"] = value(phase->current_time.get_string());
    phase_object["time"] = value(time_object);
  }
  else if (phase->phase_type == simulator::INTERVAL_PHASE)
  {
    phase_object["type"] = value(string("IP"));

    object time_object;
    time_object["start_time"] = value(phase->current_time.get_string());
    if (!phase->end_time.undefined())
    {
      time_object["end_time"] = value(phase->end_time.get_string());
    }
    phase_object["time"] = value(time_object);
  }
  phase_object["variable_map"] = for_vm(phase->variable_map);
  std::vector<parameter_map_t> pms = phase->get_parameter_maps();
  //TODO: deal with multiple parameter_maps
  if (pms.size() == 0) 
  {
    // parameter_map is obsolete, only for HIDE
    phase_object["parameter_map"] = for_pm(parameter_map_t());
  }
  else
  {
    phase_object["parameter_map"] = for_pm(pms[0]);
  }

  picojson::array parameter_maps;
  for (auto pm : pms)
  {
    parameter_maps.push_back(for_pm(pm));
  }
  phase_object["parameter_maps"] = value(parameter_maps);

  phase_object["children"] = make_children(phase);
  phase_object["simulation_state"] = value(get_string_for_cause(phase->simulation_state));
  return value(phase_object);
}

value JsonWriter::make_children(const phase_result_const_sptr_t &phase)
{
  picojson::array children;
  for (vector<phase_result_sptr_t>::const_iterator it = phase->children.begin(); it != phase->children.end(); it++)
  {
    children.push_back(for_phase(*it));
  }
  return value(children);
}

value JsonWriter::for_range(const value_range_t &range)
{
  object range_object;

  if (range.unique())
  {
    range_object["unique_value"] = value(range.get_unique_value().get_string());
  }
  else
  {
    picojson::array lbs;
    for (uint i = 0; i < range.get_lower_cnt(); i++)
    {
      const value_range_t::bound_t &bound = range.get_lower_bound(i);
      object lb;
      lb["value"] = value(bound.value.get_string());
      lb["closed"] = value(bound.include_bound);
      lbs.push_back(value(lb));
    }
    range_object["lower_bounds"] = value(lbs);

    picojson::array ubs;
    for (uint i = 0; i < range.get_upper_cnt(); i++)
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
  if (epsilon_mode_flag)
  {
    for (variable_map_t::const_iterator it = vm.begin(); it != vm.end(); it++)
      {
        const std::string &key = it->first.get_string();
        const value_range_t &range = it->second;
        vm_obj[key] = for_range_diff(range);
      }
  }
  else
  {
    for (variable_map_t::const_iterator it = vm.begin(); it != vm.end(); it++)
      {
        const std::string &key = it->first.get_string();
        const value_range_t &range = it->second;
        vm_obj[key] = for_range(range);
      }
  }
  return value(vm_obj);
}

value JsonWriter::for_vs(const variable_set_t &vs)
{
  picojson::array vs_array;
  for (variable_set_t::const_iterator it = vs.begin(); it != vs.end(); it++)
  {
    vs_array.push_back(value(it->get_string()));
  }
  return value(vs_array);
}

value JsonWriter::for_pm(const parameter_map_t &pm)
{
  object pm_obj;
  for (parameter_map_t::const_iterator it = pm.begin(); it != pm.end(); it++)
  {
    std::string key = it->first.to_string();
    pm_obj[key] = for_range(it->second);
  }
  return value(pm_obj);
}

void JsonWriter::set_epsilon_mode(backend_sptr_t back, bool flag){
  backend = back;
  epsilon_mode_flag = flag;
}

value JsonWriter::for_range_diff(const value_range_t &range)
{
  simulator::value_t tmp;
  simulator::value_t ret;
  object range_object;

  if (range.unique())
  {
    tmp = range.get_unique_value();
    backend->call("diffEpsilon", true, 1, "vln", "vl", &tmp, &ret);
    range_object["unique_value"] = value(ret.get_string());
  }
  else
  {
    picojson::array lbs;
    for (uint i = 0; i < range.get_lower_cnt(); i++)
    {
      const value_range_t::bound_t &bound = range.get_lower_bound(i);
      object lb;
      tmp = bound.value;
      backend->call("diffEpsilon", true, 1, "vln", "vl", &tmp, &ret);
      lb["value"] = value(ret.get_string());
      lb["closed"] = value(bound.include_bound);
      lbs.push_back(value(lb));
    }
    range_object["lower_bounds"] = value(lbs);

    picojson::array ubs;
    for (uint i = 0; i < range.get_upper_cnt(); i++)
    {
      const value_range_t::bound_t &bound = range.get_upper_bound(i);
      object ub;
      tmp = bound.value;
      backend->call("diffEpsilon", true, 1, "vln", "vl", &tmp, &ret);
      ub["value"] = value(ret.get_string());
      ub["closed"] = value(bound.include_bound);
      ubs.push_back(value(ub));
    }
    range_object["upper_bounds"] = value(ubs);
  }
  return value(range_object);
}

} // namespace io
} // namespace hydla
