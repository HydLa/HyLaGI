#include "JsonWriter.h"
#include "Dumpers.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>

using namespace boost::property_tree;
using namespace std;

namespace hydla{
namespace output{

void JsonWriter::write(const phase_result_const_sptr_t &root)
{
  ptree pt;
  stringstream sstr;
  sstr << root->children[0]->variable_map;
  pt.put("vm", sstr.str());
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
  write_json("hoge.json", pt);
}

}
}
