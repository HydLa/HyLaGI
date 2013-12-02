#include "JsonWriter.h"
#include "Dumpers.h"
#include <sstream>

using namespace boost::property_tree;
using namespace std;

namespace hydla{
namespace output{

void JsonWriter::write(const simulator_t &simulator)
{
  stringstream sstr;
  parse_tree_.add_child("vars", for_vs(simulator.get_variable_set()));
  parse_tree_.add_child("pars", for_pm(simulator.get_parameter_map()));

  phase_result_const_sptr_t root = simulator.get_result_root();
  ptree children;
  for(vector<phase_result_sptr_t>::const_iterator it = root->children.begin(); it != root->children.end(); it++)
  {
    children.push_back(std::make_pair("", for_phase(*it)));
  }
  parse_tree_.add_child("traj", make_children(root));
  write_json("hoge.json", parse_tree_);
}


JsonWriter::ptree_t JsonWriter::for_phase(const phase_result_const_sptr_t &phase)
{
  ptree_t ret;
  ret.put("id", phase->id);
  if(phase->phase == simulator::PointPhase)
  {
    ret.put("phase_type", "PP");
    ret.put("time", *phase->current_time);
  }
  else if(phase->phase == simulator::IntervalPhase)
  {
    ret.put("phase_type", "IP");
    ret.put("start_time", *phase->current_time);
    ret.put("end_time", *phase->end_time);
  }

  ret.add_child("vm", for_vm(phase->variable_map));
  ret.add_child("pm", for_pm(phase->parameter_map));
  ret.add_child("children", make_children(phase));
  return ret;
}


JsonWriter::ptree_t JsonWriter::make_children(const phase_result_const_sptr_t &phase)
{
  ptree_t children;
  for(vector<phase_result_sptr_t>::const_iterator it = phase->children.begin(); it != phase->children.end(); it++)
  {
    children.push_back(std::make_pair("", for_phase(*it)));
  }
  return children;
}


JsonWriter::ptree_t JsonWriter::for_vm(const variable_map_t &vm)
{
  ptree_t ret;
  for(variable_map_t::const_iterator it = vm.begin(); it != vm.end(); it++)
  {
    std::string key = it->first.get_string();
    ret.put(key, it->second);
  }
  return ret;
}

JsonWriter::ptree_t JsonWriter::for_vs(const variable_set_t &vs)
{
  ptree_t ret;
  for(variable_set_t::const_iterator it = vs.begin(); it != vs.end(); it++)
  {
    ret.push_back(std::make_pair("", it->get_string()));
  }
  return ret;
}

JsonWriter::ptree_t JsonWriter::for_pm(const parameter_map_t &pm)
{
  ptree_t ret;
  for(parameter_map_t::const_iterator it = pm.begin(); it != pm.end(); it++)
  {
    std::string key = it->first.get_name();
    ret.put(key, it->second);
  }
  return ret;
}


}
}
