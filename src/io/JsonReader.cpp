#include "JsonReader.h"
#include "Constants.h"
#include "Logger.h"
#include "Parser.h"
#include "Utility.h"
#include <fstream>

using namespace std;
using namespace picojson;
using namespace hydla::utility;
using namespace hydla::parser;

namespace hydla {
namespace io {

simulator::phase_result_sptr_t JsonReader::read_phase(const std::string &name) {
  ifstream ifs;
  ifs.open(name.c_str());
  value json;
  ifs >> json;
  ifs.close();

  return read_phase(json.get<object>());
}

JsonReader::phase_result_sptr_t
JsonReader::read_phase(picojson::object &json_object) {
  // TODO: positive_asksとかnegative_asksとかparameter_mapとかも読む
  phase_result_sptr_t phase(new phase_result_t());
  phase->id = json_object["id"].get<long>();
  string phase_type_str = json_object["type"].get<string>();
  if (phase_type_str == "PP")
    phase->phase_type = simulator::POINT_PHASE;
  else
    phase->phase_type = simulator::INTERVAL_PHASE;
  if (phase->phase_type == simulator::POINT_PHASE) {
    object time_object = json_object["time"].get<object>();
    string time_str = time_object["time_point"].get<string>();
    phase->current_time = Parser(time_str).arithmetic();
  } else if (phase->phase_type == simulator::INTERVAL_PHASE) {
    object time_object = json_object["time"].get<object>();
    string start_str = time_object["start_time"].get<string>();
    phase->current_time = Parser(start_str).arithmetic();
    string end_str = time_object["end_time"].get<string>();
    phase->end_time = Parser(end_str).arithmetic();
  }
  phase->variable_map = read_vm(json_object["variable_map"].get<object>());
  picojson::array children = json_object["children"].get<picojson::array>();
  for (auto child : children) {
    phase->children.push_back(read_phase(child.get<object>()));
  }
  if (json_object.find("simulation_state") != json_object.end()) {
    string cause_str = json_object["simulation_state"].get<string>();
    phase->simulation_state = get_cause_for_string(cause_str);
  }
  return phase;
}

JsonReader::value_range_t JsonReader::read_range(object &o) {
  value_range_t range;
  if (o.find("unique_value") != o.end()) {
    string value_str = o["unique_value"].get<string>();
    range.set_unique_value(Parser(value_str).arithmetic());
  } else {
    if (o.find("lower_bounds") != o.end()) {
      picojson::array bound_array = o["lower_bounds"].get<picojson::array>();
      for (auto bound : bound_array) {
        object bound_object = bound.get<object>();
        bool closed = bound_object["closed"].get<bool>();
        simulator::value_t value =
            Parser(bound_object["value"].get<string>()).arithmetic();
        range.add_lower_bound(value, closed);
      }
    }

    if (o.find("upper_bounds") != o.end()) {
      picojson::array bound_array = o["upper_bounds"].get<picojson::array>();
      for (auto bound : bound_array) {
        object bound_object = bound.get<object>();
        bool closed = bound_object["closed"].get<bool>();
        simulator::value_t value =
            Parser(bound_object["value"].get<string>()).arithmetic();
        range.add_lower_bound(value, closed);
      }
    }
  }
  return range;
}

JsonReader::variable_map_t JsonReader::read_vm(object &o) {
  variable_map_t vm;
  for (auto it : o) {
    const std::string &str = it.first;
    const value_range_t &range = read_range(it.second.get<object>());
    simulator::variable_t var(str);
    vm[var] = range;
  }
  return vm;
}

JsonReader::parameter_map_t JsonReader::read_pm(object &o) {
  parameter_map_t pm;
  for (auto it : o) {
    const std::string &str = it.first;
    const value_range_t &range = read_range(it.second.get<object>());
    simulator::parameter_t par(str);
    pm[par] = range;
  }
  return pm;
}

} // namespace io
} // namespace hydla
