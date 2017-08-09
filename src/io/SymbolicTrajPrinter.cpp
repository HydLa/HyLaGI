#include "SymbolicTrajPrinter.h"
#include <stack>
#include <iostream>
#include "Parameter.h"
#include "Logger.h"
#include "Backend.h"
#include "EpsilonMode.h"
#include "LTLNode.h"
#include "PropertyNode.h"
#include "IntervalTreeVisitor.h"
#include "HydLaError.h"

extern hydla::Opts opts;

namespace hydla {
namespace io {

using namespace simulator;
using namespace backend;
using namespace std;
using namespace interval;

SymbolicTrajPrinter::SymbolicTrajPrinter(backend_sptr_t b, std::ostream& ost, bool numerize_par):
  ostream(ost), backend(b), numerize_parameter(numerize_par){
}

string SymbolicTrajPrinter::get_state_output(const phase_result_t& result) const{
  stringstream sstr;
  if (result.phase_type == INTERVAL_PHASE)
  {
    sstr << "---------IP " << result.id << "---------" << endl;
    sstr << "unadopted modules: " << result.unadopted_ms.get_name() << endl;
    output_inconsistent_constraints(sstr, result);
    output_asks(sstr, result);
    if (!result.end_time.undefined())
    {
      sstr << "t\t: " << result.current_time << "->" << result.end_time << "\n";
    }
    else
    {
      sstr << "t\t: " << result.current_time << "->" << "???" << "\n";
    }
  }
  else
  {
    sstr << "---------PP " << result.id << "---------" << endl;
    sstr << "unadopted modules: " << result.unadopted_ms.get_name() << endl;
    output_inconsistent_constraints(sstr, result);
    output_asks(sstr, result);
    sstr << "t\t: " << result.current_time << "\n";
  }


  if (epsilon_mode_flag && backend.get())
  {
    output_limit_of_time(sstr,backend.get(),result);
  }

  output_variable_map(sstr, result);

  if (epsilon_mode_flag && backend.get())
  {
    output_limits_of_variable_map(sstr,backend.get(),result,result.variable_map);
  }
  return sstr.str();
}

void SymbolicTrajPrinter::output_asks(std::ostream &stream, const phase_result_t &phase)const
{
  stream << "positive \t: ";
  bool first = true;
  for (auto ask : phase.get_diff_positive_asks())
  {
    if (!first)stream << "\n\t\t  ";
    stream << get_infix_string(ask);
    first = false;
  }
  stream << endl;

  stream << "negative \t: ";
  first = true;
  for (auto ask : phase.get_diff_negative_asks())
  {
    if (!first)stream << "\n\t\t  ";
    stream << get_infix_string(ask);
    first = false;
  }
  stream << endl;
}

void SymbolicTrajPrinter::output_inconsistent_constraints(std::ostream &stream, const phase_result_t &phase)const
{
  if (!phase.inconsistent_module_sets.empty())
  {
    stream << "unsat mod\t: ";
    bool first = true;
    for (auto module_set : phase.inconsistent_module_sets)
    {
      if (!first)stream << "\t\t  ";
      stream << module_set.get_name() << endl;
      first = false;
    }
  }
  if (!phase.inconsistent_constraints.empty())
  {
    stream << "unsat cons\t: ";
    bool first = true;
    for (auto constraint: phase.inconsistent_constraints)
    {
      if (!first)stream << "\t\t  ";
      stream <<  constraint << endl;
      first = false;
    }
  }
}

void SymbolicTrajPrinter::output_parameter_map(const parameter_map_t& pm) const
{
  parameter_map_t::const_iterator it  = pm.begin();
  parameter_map_t::const_iterator end = pm.end();
  if (numerize_parameter)
  {
    for (; it!=end; ++it) {
      //ValueRange numerized_range = it->second.get_numerized_range();
      //ostream << it->first << "\t: " << numerized_range << "\n";
      ostream << it->first << "\t: ";
      it->second.dump(ostream);
      ostream << "\n";
    }
  }else
  {
    for (; it!=end; ++it) {
      ostream << it->first << "\t: " << it->second << "\n";
    }
  }
}

void SymbolicTrajPrinter::output_variable_map(std::
ostream &stream, const phase_result_t &result) const
{
  variable_map_t vm = result.variable_map;
  for (auto it = vm.begin(); it!=vm.end(); ++it) {

    // �����ѿ����ꤷ�����
    if (opts.output_mode != Opts::None)
    {
      bool hit = false;
      for (auto it2 = opts.output_vars.begin(); it2 != opts.output_vars.end(); ++it2)
      {
        if (it->first.get_string() == *it2)
        {
          hit = true;
          break;
        }
      }
      switch (opts.output_mode)
      {
        case Opts::Omit:
          if (hit) continue;
          break;
        case Opts::Output:
          if (!hit) continue;
          break;
        default :
          break;
      }
    }
    
    stream << it->first << "\t: " << it->second << "\n";
    if (opts.interval && it->second.unique() && result.phase_type == POINT_PHASE)
    {
      vector<parameter_map_t> par_maps = result.get_parameter_maps();
      if (par_maps.size() == 1)
      {
        IntervalTreeVisitor visitor;
        itvd dummy = itvd(0, 0);
        try
        {
          itvd itv = visitor.get_interval_value(it->second.get_unique_value().get_node(), &dummy, &par_maps[0]);
          stream << "width(" << it->first << "): " << width(itv) << endl;
        }
        catch(HydLaError)
        {
          //do nothing
        }
      }
    }
  }
}

void SymbolicTrajPrinter::output_one_phase(const phase_result_const_sptr_t& phase, const std::string& prefix) const
{
  ostream << prefix << endl;
  ostream << get_state_output(*phase);
  vector<parameter_map_t> par_maps = phase->get_parameter_maps();
  if (par_maps.size() > 0)
  {
    if (par_maps.size() == 1)
    {
      if (!par_maps[0].empty())
      {
        ostream << "---------parameter condition---------" << endl;
        output_parameter_map(par_maps[0]);
      }
    }
    else
    {
      int i = 0;
      for (auto it = par_maps.begin(); it != par_maps.end(); it++, i++)
      {
        ostream << "---------parameter condition(" << i+1 << ")---------" << endl;
        output_parameter_map(par_maps[i]);
      }
    }
  }
}

void SymbolicTrajPrinter::output_result_tree(const phase_result_const_sptr_t& root) const
{
  if (root->children.size() == 0)
  {
    ostream << "No Result." << endl;
    return;
  }
  int i = 1, j = 1;
  phase_result_sptrs_t::const_iterator it = root->children.begin(), end = root->children.end();
  for (;it!=end;it++)
  {
    vector<std::string> result;
    output_result_node(*it, result, i, j);
  }
}

void SymbolicTrajPrinter::output_result_node(const phase_result_const_sptr_t &node, vector<std::string> &result, int &case_num, int &phase_num) const{
  if (node->simulation_state == simulator::NOT_SIMULATED) return;
  if (node->children.size() == 0)
  {
    int current_case_num = case_num++;
    ostream << "---------Case " << current_case_num << "---------" << endl;
    vector<std::string>::const_iterator r_it = result.begin();
    for (;r_it != result.end(); r_it++)
    {
      ostream << *r_it;
    }

    if (node->simulation_state==simulator::ASSERTION ||
       node->simulation_state==simulator::INCONSISTENCY ||
      node->simulation_state==simulator::TIME_LIMIT ||
      node->simulation_state==simulator::NOT_SIMULATED ||
      node->simulation_state==simulator::NONE ||
       node->simulation_state==simulator::SIMULATED ||
      node->simulation_state==simulator::STEP_LIMIT ||
      node->simulation_state==simulator::SOME_ERROR ||
      node->simulation_state==simulator::INTERRUPTED)
    {
      ostream << get_state_output(*node);
    }

    {
      vector<parameter_map_t> par_maps = node->get_parameter_maps();
      if (par_maps.size() > 0)
      {
        if (par_maps.size() == 1)
        {
          if (!par_maps[0].empty())
          {
            ostream << "---------parameter condition(Case" << current_case_num << ")---------" << endl;
            output_parameter_map(par_maps[0]);
          }
        }
        else
        {
          int i = 0;
          for (auto it = par_maps.begin(); it != par_maps.end(); it++, i++)
          {
            ostream << "---------parameter condition(Case" << current_case_num << "_" << i+1 << ")---------" << endl;
            output_parameter_map(par_maps[i]);
          }
        }
      }
    }

    // print why the simulation was terminated
    switch(node->simulation_state)
    {
      case simulator::INCONSISTENCY:
        ostream << "# execution stuck\n";
        break;

      case simulator::SOME_ERROR:
        ostream << "# some error occurred\n" ;
        break;

      case simulator::ASSERTION:
        ostream << "# assertion failed\n" ;
        break;

      case simulator::TIME_LIMIT:
        ostream << "# time reached limit\n" ;
        break;

      case simulator::STEP_LIMIT:
        ostream << "# number of phases reached limit\n" ;
        break;

      case simulator::TIME_OUT_REACHED:
        ostream << "# time out\n" ;
        break;

      case simulator::NOT_UNIQUE_IN_INTERVAL:
        ostream << "# some variables are not unique in IP\n" ;
        break;

      case simulator::NOT_SIMULATED:
        ostream << "# following phases were not simulated\n" ;
        break;

      case simulator::INTERRUPTED:
        ostream << "# simulation was interrupted\n" ;
        break;

      default:
      case simulator::NONE:
        ostream << "# unknown error occured\n" ;
        break;
    }
    ostream << endl;
  }
  else
  {
    if (node->phase_type == simulator::POINT_PHASE)
    {
      stringstream sstr;
      sstr << "---------" << phase_num++ << "---------\n";
      result.push_back(sstr.str());
    }
    result.push_back(get_state_output(*node));

    phase_result_sptrs_t::const_iterator it = node->children.begin(), end = node->children.end();
    for (;it!=end;it++)
    {
      output_result_node(*it, result, case_num, phase_num);
    }
    result.pop_back();
    if (node->phase_type == POINT_PHASE)
    {
      result.pop_back();
      phase_num--;
    }
  }
}

void SymbolicTrajPrinter::set_epsilon_mode(backend_sptr_t back, bool flag)
{
  backend = back;
  epsilon_mode_flag = flag;
}

void SymbolicTrajPrinter::output_limit_of_time(std::ostream &stream, Backend* backend, const phase_result_t& result) const
{
  simulator::value_t ret_current_time,ret_end_time;
  symbolic_expression::node_sptr tmp_current_time,tmp_end_time;
  int check_result,check_current_time,check_end_time;

  if (result.phase_type == INTERVAL_PHASE)
  {
    if (!result.end_time.undefined() && !result.current_time.undefined())
    {
      tmp_current_time = result.current_time.get_node();
      tmp_end_time = result.end_time.get_node();
      backend->call("checkEpsilon", true, 1, "en", "i",&tmp_current_time, &check_current_time);
      backend->call("checkEpsilon", true, 1, "en", "i",&tmp_end_time, &check_end_time);
      check_result = check_current_time * check_end_time;
      if (check_result == 1)
      {
        backend->call("limitEpsilon", true, 1, "en", "vl",&tmp_current_time, &ret_current_time);
        backend->call("limitEpsilon", true, 1, "en", "vl",&tmp_end_time, &ret_end_time);
        stream << "Limit(t)\t: " << ret_current_time << "->" << ret_end_time << "\n";
      }
      else
      {
        backend->call("limitEpsilonP", true, 1, "en", "vl",&tmp_current_time, &ret_current_time);
        backend->call("limitEpsilonP", true, 1, "en", "vl",&tmp_end_time, &ret_end_time);
        stream << "Limit(t)\t+ : " << ret_current_time << "->" << ret_end_time << "\n";
        backend->call("limitEpsilonM", true, 1, "en", "vl",&tmp_current_time, &ret_current_time);
        backend->call("limitEpsilonM", true, 1, "en", "vl",&tmp_end_time, &ret_end_time);
        stream << "Limit(t)\t- : " << ret_current_time << "->" << ret_end_time << "\n";
      }
    }
    else if (!result.current_time.undefined())
    {
      tmp_current_time = result.current_time.get_node();
      backend->call("checkEpsilon", true, 1, "en", "i",&tmp_current_time, &check_result);
      if (check_result == 1)
      {
        backend->call("limitEpsilon", true, 1, "en", "vl",&tmp_current_time, &ret_current_time);
        stream << "Limit(t)\t: " << ret_current_time << "->" << "???" << "\n";
      }
      else
      {
        backend->call("limitEpsilonP", true, 1, "en", "vl",&tmp_current_time, &ret_current_time);
        stream << "Limit(t)\t+ : " << ret_current_time << "->" << "???" << "\n";
        backend->call("limitEpsilonM", true, 1, "en", "vl",&tmp_current_time, &ret_current_time);
        stream << "Limit(time)\t- : " << ret_current_time << "->" << "???" << "\n";
      }
    }
  }
  else
  {
    if (!result.current_time.undefined())
    {
      tmp_current_time = result.current_time.get_node();
      backend->call("checkEpsilon", true, 1, "en", "i",&tmp_current_time, &check_result);
      if (check_result == 1)
      {
        backend->call("limitEpsilon", true, 1, "en", "vl",&tmp_current_time, &ret_current_time);
        stream << "Limit(t)\t: " << ret_current_time << "\n";
      }
      else
      {
        backend->call("limitEpsilonP", true, 1, "en", "vl",&tmp_current_time, &ret_current_time);
        stream << "Limit(t)\t+ : " << ret_current_time << "\n";
        backend->call("limitEpsilonM", true, 1, "en", "vl",&tmp_current_time, &ret_current_time);
        stream << "Limit(t)\t- : " << ret_current_time << "\n";
      }
    }
  }
}

void SymbolicTrajPrinter::output_limits_of_variable_map(std::ostream &stream, Backend* backend, const phase_result_t& result, const variable_map_t& vm) const
{
  variable_map_t::const_iterator it  = vm.begin();
  variable_map_t::const_iterator end = vm.end();
  simulator::value_t ret;
  simulator::value_t tmp;
  int check_result;
  it  = vm.begin();
  for (; it!=end; ++it)
  {
    if (it->second.unique())
    {
      tmp = it->second.get_unique_value();
      backend->call("checkEpsilon", true, 1, "vln", "i",&tmp, &check_result);
      if (check_result == 1)
      {
        backend->call("limitEpsilon", true, 1, "vln", "vl", &tmp, &ret);
        stream << "Limit(" << it->first << ")\t  : " << ret << "\n";
      }
      else
      {
        backend->call("limitEpsilonP", true, 1, "vln", "vl", &tmp, &ret);
        stream << "Limit(" << it->first << ")\t+ : " << ret << "\n";
        backend->call("limitEpsilonM", true, 1, "vln", "vl", &tmp, &ret);
        stream << "Limit(" << it->first << ")\t- : " << ret << "\n";
      }
    }
    else
    {
      stream << "Limit(" << it->first << ")\t: " << it->second << "\n";
    }
  }
}
void SymbolicTrajPrinter::output_property_automaton(PropertyNode* node)
{
  // node->write_reset();
  // ostream << "digraph g{" << "\n";
  // ostream << "\"init\"[shape=\"point\"];" << "\n";
  // ostream << "\"init\"" << " -> " << "\"Property" << to_string(node->id) << "\"" << ";" << "\n";
  // dump_property_automaton(node);
  // ostream << "}" << "\n";
  // node->write_reset();
}
void SymbolicTrajPrinter::dump_property_automaton(PropertyNode* node){
  // if (node->write == 0){
  //   node->write++;
  //   string name = "\"Property" + to_string(node->id) + "\"";
  //   ostream << name << " ";
  //   if (node->type != NOMAL){
  //     ostream << "[peripheries=2];" << "\n";
  //   }
  //   else
  //   {
  //     ostream << ";" << "\n";
  //   }
  //   for (Property_link_t::iterator it = node->link.begin();it != node->link.end();it++)
  //   {
  //     if (node->id != it->second->id){
  //       ostream << name << " -> " << "\"Property" << it->second->id << "\" ";
  //       ostream << "[label=\"" << it->first->get_string() << "\"];" << "\n";
  //       dump_property_automaton(it->second);
  //     }
  //     else
  //     {
  //       ostream << name << " -> " << name << " ";
  //       ostream << "[label=\"" << it->first->get_string() << "\"];" << "\n";
  //     }
  //   }
  // }
}

void SymbolicTrajPrinter::output_ltl_node(simulator::LTLNode* node)
{
  // if (node->property->type != ZERO){
  //   node->write_reset();
  //   ostream << "digraph g{" << "\n";
  //   ostream << "\"init\"[shape=\"point\"];" << "\n";
  //   ostream << "\"init\"" << " -> " << "\"" << node->id << "\"" << ";" << "\n";
  //   dump_ltl_node(node);
  //   ostream << "}" << "\n";
  //   node->write_reset();
  // }
  // else
  // {
  //   ltl_node_list_t::iterator it = node->link.begin();
  //   output_ltl_node(*it);
  // }
}
void SymbolicTrajPrinter::dump_ltl_node(simulator::LTLNode* node){
  // if (node->write == 0){
  //   node->write++;
  //   ostream << "\"" << node->id << "\"" << " ";
  //   if (node->property->type != NOMAL){
  //     if (node->red>0){
  //       ostream << "[peripheries=2 color=red];" << "\n";
  //     }
  //     else
  //     {
  //       ostream << "[peripheries=2];" << "\n";
  //     }
  //   }
  //   else
  //   {
  //     if (node->red>0){
  //       ostream << "[color=red];" << "\n";
  //     }
  //     else
  //     {
  //       ostream << ";" << "\n";
  //     }
  //   }
  //   for (ltl_node_list_t::iterator it = node->link.begin();it != node->link.end();it++)
  //   {
  //     if (node->id == (*it)->id)
  //     {
  //       if (node->red>0 && (*it)->red>0)
  //       {
  //         ostream << "\"" << node->id << "\"" << " -> " << "\"" << node->id << "\"" << "[color=red];" << "\n";
  //       }
  //       else
  //       {
  //         ostream << "\"" << node->id << "\"" << " -> " << "\"" << node->id << "\"" << ";" << "\n";
  //       }
  //     }
  //     else
  //     {
  //       if (node->red>0 && (*it)->red>0)
  //       {
  //         ostream << "\"" << node->id << "\"" << " -> " << "\"" << (*it)->id << "\"" << "[color=red];" << "\n";
  //       }
  //       else
  //       {
  //         ostream << "\"" << node->id << "\"" << " -> " << "\"" << (*it)->id << "\"" << ";" << "\n";
  //       }
  //       dump_ltl_node(*it);
  //     }
  //   }
  // }
}

} // namespace io
} // namespace hydla
