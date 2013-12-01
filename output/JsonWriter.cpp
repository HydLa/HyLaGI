#include "JsonWriter.h"
#include "Dumpers.h"
#include <sstream>

using namespace boost::property_tree;
using namespace std;

namespace hydla{
namespace output{

void JsonWriter::write(const phase_result_const_sptr_t &root)
{
  stringstream sstr;
  parse_tree_ = for_phase(root->children[0]);
/*
  sstr = stringstream();
  sstr << root->parameter_map;
  pt.put("pm", sstr.str());
  sstr = stringstream();
  sstr << root->positive_asks;
  pt.put("pa", sstr.str());
  sstr = stringstream();
  sstr << root->negative_asks;
  pt.put("na", sstr.str());
  sstr = stringstream();
  sstr << root->expanded_always;
  pt.put("ea", sstr.str());
*/
  write_json("hoge.json", parse_tree_);
}


ptree JsonWriter::for_phase(const phase_result_const_sptr_t &phase)
{
  ptree ret;
  ret.put("id", phase->id);
  for(vector<phase_result_sptr_t>::const_iterator it = phase->children.begin();
      it != phase->children.end(); it++)
  {
    ret.add_child("children", for_phase(*it));
  }
  return ret;
}

void JsonWriter::for_vm(const variable_map_t &vm)
{
/*
  for(variable_map_t::const_iterator it = vm.begin(); it != vm.end(); it++)
  {
    std::string key = prefix + "vm." + it->first.get_string();
    parse_tree_.put(key, it->second);
  }
*/
}


}
}
