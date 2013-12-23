#include "JsonWriter.h"
#include <sstream>

using namespace boost::property_tree;
using namespace std;

namespace hydla{
namespace output{

void JsonWriter::write(const simulator_t &simulator, std::string name)
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
 
  std::ofstream ofs;
  ofs.open(name.c_str());
  write_json(ofs, parse_tree_);
  ofs.close(); 
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
  if(phase->children.size() == 0){
    std::string cot;
    switch(phase->cause_of_termination){
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
    ret.put("cot", cot);
  }
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

JsonWriter::ptree_t JsonWriter::for_range(const value_range_t &range)
{
  ptree_t ret;
  if(range.unique())
  {
    ret.put("", range.get_unique()->get_string());
  }
  else
  {
    if(range.get_lower_cnt() > 0)
    {
      ptree_t lbs;
      for(uint i = 0; i < range.get_lower_cnt(); i++)
      {
        const value_range_t::bound_t &bound = range.get_lower_bound(i);
        if(bound.include_bound)
        {
          lbs.put(bound.value->get_string(), "[");
        }
        else
        {
          lbs.put(bound.value->get_string(), "(");
        }
      }
      ret.add_child("lb", lbs);
    }
    if(range.get_upper_cnt() > 0)
    {
      ptree_t ubs;
      for(uint i = 0; i < range.get_upper_cnt(); i++)
      {
        const value_range_t::bound_t &bound = range.get_upper_bound(i);
        if(bound.include_bound)
        {
          ubs.put(bound.value->get_string(), "]");
        }
        else
        {
          ubs.put(bound.value->get_string(), ")");
        }
      }
      ret.add_child("ub", ubs);
    }
  }
  return ret;
}


JsonWriter::ptree_t JsonWriter::for_vm(const variable_map_t &vm)
{
  ptree_t ret;
  for(variable_map_t::const_iterator it = vm.begin(); it != vm.end(); it++)
  {
    const std::string &key = it->first.get_string();
    const value_range_t &range = it->second;
    ret.add_child(key, for_range(range) );
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
    std::string key = it->first.to_string();
    ret.add_child(key, for_range(it->second));
  }
  return ret;
}


}
}
