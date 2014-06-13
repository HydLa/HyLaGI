#include "InteractiveSimulator.h"
#include "Timer.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"
#include "Backend.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <assert.h>
#include <boost/make_shared.hpp>

#include "JsonReader.h"
#include "Logger.h"
#include <boost/spirit/include/classic_multi_pass.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>
#include <boost/spirit/include/classic_ast.hpp>
#include "HydLaAST.h"
#include "AffineApproximator.h"
#include "TimeModifier.h"
#include "SignalHandler.h"


// surpress warning "C4996: old 'strcpy'" on VC++2005
#define _CRT_SECURE_NO_DEPRECATE
#include <boost/algorithm/string.hpp>

using namespace boost;
using namespace std;
using namespace hydla::grammer_rule;
using namespace hydla::parser::error;
using namespace hydla::symbolic_expression;
using namespace hydla::parser;
using namespace hydla::interval;


namespace hydla {
namespace simulator {


void InteractiveSimulator::print_end(phase_result_sptr_t& p)
{
  cout << "this simulation has reached end" << endl;
  cout << "--- last phase ---" << endl;
  print_phase(p);
}


/**
 * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
 */
phase_result_const_sptr_t InteractiveSimulator::simulate()
{
  phase_simulator_->set_select_function(select_phase);
  printer_.set_output_variables(opts_->output_variables);
  simulation_todo_sptr_t todo(make_initial_todo());
  unsigned int todo_num = 1; // 連続して処理するTODOの残数
  int todo_id = 0;

  while(todo_num)
  {
    todo_id++;
    todo->id =todo_id;
    try
    {
      timer::Timer phase_timer;
      PhaseSimulator::result_list_t phases = phase_simulator_->calculate_phase_result(todo);

      phase_result_sptr_t phase;
      if(phases.empty())
      {
        simulation_todo_sptr_t tmp_todo = todo;
        do
        {
          if(todo->phase_type == PointPhase)
            cout << "---------PP "<<todo->id<< "---------" << endl;
          cout << "execution stuck" << endl;
          todo_num = input_and_process_command(todo);
        }while(todo_num > 0 && tmp_todo == todo);
        continue;
      }
      else
      {
        unsigned int select_num = select_case<phase_result_sptr_t>(phases);
        phase = phases[select_num];
        for(unsigned int i = 0; i < phases.size(); i++)
        {
          if(i == select_num) continue;
          phases[i]->cause_for_termination = NOT_SELECTED;
        }
      }


      PhaseSimulator::todo_list_t todos = phase_simulator_->make_next_todo(phase, todo);
      todo->profile["EntirePhase"] += phase_timer.get_elapsed_us();
      profile_vector_->push_back(todo);
      if(todos.empty())
      {
        simulation_todo_sptr_t tmp_todo = todo;
        do
        {
          print_end(phase);
          todo_num = input_and_process_command(todo);
        }while(todo_num > 0 && tmp_todo == todo);
        continue;
      }
      todo = todos[0];
      print_phase(todo);

      if(phase_simulator_->breaking)
      {
        cout << "break!" << endl;
        phase_simulator_->breaking = false;
        todo_num = input_and_process_command(todo);
      }
      else if(todo_num > 0 && --todo_num == 0)
      {
        todo_num = input_and_process_command(todo);
      }
      // TODO:現状だと，result_root_のところまで戻ると変なことになるのでそこまでは巻き戻せないようにしておく
      all_todo_.push_back(todo);
      if(signal_handler::interrupted) break;
    }
    catch(const runtime_error &se)
    {
      cout << se.what() << endl;
      todo->parent->cause_for_termination = SOME_ERROR;
      HYDLA_LOGGER_DEBUG(se.what());
      break;
    }

  }

  if(signal_handler::interrupted || todo->parent->cause_for_termination != SOME_ERROR){
    todo->parent->cause_for_termination = INTERRUPTED;
    io::JsonWriter().write_phase(todo->parent, "interrupted_phase");
  }
  return result_root_;
}



int InteractiveSimulator::input_and_process_command(simulation_todo_sptr_t& todo){
  io::JsonWriter writer;
  writer.write(*this, "interactive.hydat");
  while(true)
  {
    if(cin.good()==0)
    {
      cin.clear();//cin.seekg(0);
      cin.ignore( 1024,'\n');
    }
    string line;
    cout << "input a command ('h' is to show help)\n";
    cout << '>';
    cin.clear();
    getline(cin,line);
    cin.clear();
    if(line.empty()) return 1;
    switch(line[0]){
      case 'j':
        //jump
        int target_step;
        if(line.length() < 2 || (target_step = atoi(line.substr(1).c_str())) == 0)
        { 
          cout << "argument must be some number except 0" << endl;
          break;
        }
        if(target_step > 0)return target_step;
        else
        {
          if((int)all_todo_.size() < -target_step)
          {
            cout << "current history size is " << all_todo_.size() << endl;
            cout << "argument must be number greater than or equal to " << -(int)all_todo_.size() << endl;
            break;
          }
          else
          {
            todo = all_todo_[all_todo_.size() + target_step];
            for(int i=0;i < -target_step;i++)
            {
              all_todo_.pop_back();
            }
          }
        }

        todo->ms_to_visit = module_set_container_->get_full_ms_list();
        todo->maximal_mss.clear();
        todo->positive_asks.clear();
        todo->negative_asks.clear();
        todo->judged_prev_map.clear();
        cout << "jump" << endl;
        print_phase(todo);
        break;
      case 'q':
        cout << "exit" << endl;
        return 0;
      case 'h':
        show_help();
        break;
      case 'p':
        print(todo->parent);
        break;
      case 'a':
        {
          if( approx_variable(todo) ) print_phase(todo->parent);
          break;
        }
      case 'c':
        change_variable(todo);
        print_phase(todo->parent);
        break;
      case 't':
        change_time(todo);
        print_phase(todo->parent);
        break;
      case 'u':
        find_unsat_core(todo);
        break;
      case 'b':
        set_breakpoint(todo);
        break;
      case 'r':
        return -1;
      case 's':
        save_state(todo);
        break;
      case 'l':
        load_state(todo);
        break;
      default:
        cout << "invalid command: " << line << endl;
        break;
    }

  }
  return 0;
}


int InteractiveSimulator::show_help(){
  const char *help_list[] = {
    "Type <return> to simulate one step (equal to \"j 1\")",
    "Type [command] <return> to exec command",
    "List of commands:",
    "",
    "j [arg]        -- Jump forward or backward by given numer of phases",
    "                  positive number: forward (equal to normal simulate)",
    "                  negative number: backward",
    "q              -- Quit this simulation",
    "c              -- Change a value of a variable",
    "p              -- Display the information of the current phase",
    "a              -- Approx a value of a variable as interval",
    "u              -- Find unsat core constraints and print them",
    "breakpoints    -- Making program stop at certain points",
    //"debug          -- Simulate with debug-mode",
    //"edit           -- Edit constraint ",
    //"load           -- Load state ",
    "run            -- simulate program  until the breakpoint is reached",
    "s              -- Save state to file",
  };
  for(uint i = 0;i < sizeof(help_list) / sizeof(help_list[0]);i++){
    printf("%s\n",help_list[i]);
  }
  return 0;
}


void InteractiveSimulator::print(phase_result_sptr_t& phase)
{
  cout << "current phase\n";
  print_phase(phase);
}

int InteractiveSimulator::change_time(simulation_todo_sptr_t& todo){
  value_t& current_time = todo->parent->current_time;
  string time_str = excin<string>();
  current_time = time_str;
  todo->current_time = current_time;
  return 0;
}

int InteractiveSimulator::change_variable(simulation_todo_sptr_t& todo){

  cout << "(change variable mode)" << endl;
  variable_map_t& vm = todo->parent->variable_map;

  // 変数の選択
  variable_map_t::iterator v_it  = vm.begin();
  cout << "input variable name " << endl;
  cout << '>';
  string variable_str;
  while (true) {
    bool is_valid_variable_name = false;
    variable_str = excin<string>();
    for(v_it = vm.begin();v_it!=vm.end();v_it++){
      if( v_it->first.get_string() == variable_str){
        is_valid_variable_name = true;
        break;
      }
    }
    if(is_valid_variable_name)
      break;
    else
      cout << variable_str << " is not a member of variable map." << endl;
  }
  string value_str;
  cout << "input value" << endl;
  cout << '>';
  getline(cin, value_str);

  if(value_str.substr(0,1)!="(" && value_str.substr(0,1)!="["){
    value_t value = value_str;
    // TODO: stringでvalue作ると，バックエンド変えた時に対応できないので何とかする
    //     （これはSymbolicValueのコンストラクタ側の問題かもしれない）
    //       あとこれだと不正な式入力された場合も対応できない
    vm[v_it->first] = value;
  }else{
    parameter_map_t& pm = todo->parameter_map;

    bool upperflag = (value_str[0] == '[');
    bool lowerflag = (value_str[value_str.size()-1] == ']');

    string rangevalue = value_str.substr(1,value_str.size()-2);
    vector<string> v;
    boost::algorithm::split( v, rangevalue, boost::is_any_of(",") );

    value_t lowvalue(v[0]);
    value_t upvalue(v[1]);

    ValueRange range;
    range.set_upper_bound(upvalue,upperflag);
    range.set_lower_bound(lowvalue,lowerflag);
    parameter_t introduced_par = introduce_parameter(v_it->first, todo->parent, range);
    pm[introduced_par] = range;
    value_t pvalue(symbolic_expression::node_sptr(new symbolic_expression::Parameter(v_it->first.get_name(),
      v_it->first.get_differential_count(),
      todo->parent->id)));
    vm[v_it->first] = pvalue;
    
    todo->parent->parameter_map = todo->parameter_map;
  }
  return 0;
}


int InteractiveSimulator::approx_variable(simulation_todo_sptr_t& todo){
  if(todo->phase_type == PointPhase)
  {
    cout << "(approximate time)" << endl;
    affine_transformer_->approximate_time(todo->current_time, todo->parent->variable_map, todo->prev_map, todo->parent->parameter_map, (todo->discrete_causes.begin()->first)->get_guard());
    todo->parent->end_time = todo->current_time;
  }
  else
  {
    variable_map_t& vm = todo->parent->variable_map;
    cout << "(approximate variable)" << endl;
  
    // 変数の選択
    cout << "input variable name " << endl;
    cout << '>';
    string variable_str = excin<string>();

    // TODO: 変数自体が幅を持つ場合への対応
    // TODO: 時刻を近似したい場合への対応

    variable_t var;

    variable_map_t::iterator v_it  = vm.begin();
    for(;v_it!=vm.end();v_it++){
      if( v_it->first.get_string() == variable_str)
      {
        var = v_it->first;
        break;
      }
    }
    if(v_it == vm.end())
    {
      cout << "invalid variable name " << endl;
      return 0;
    }
    affine_transformer_->approximate(var, vm, todo->parent->parameter_map, (todo->discrete_causes.begin()->first)->get_guard());
    todo->prev_map = vm;
  }

  todo->parameter_map = todo->parent->parameter_map;

  return 1;
}


int InteractiveSimulator::select_phase(PhaseSimulator::result_list_t& results)
{
  return select_case<phase_result_sptr_t>(results);
}

/*
int InteractiveSimulator::select_options(){
   // programoption がシングルトンなので対応策を考える 
   ProgramOptions &po = ProgramOptions::instance();
  int argc = 0;
  cout << "argc : " << argc << endl;
  char input[100], *argv[20], *cp;
  const char *delim = " \t\n"; 
  argv[0] = "./hyrose";
  cout << "enter the hyrose option args" << endl;
  if (fgets(input, sizeof(input), stdin) != NULL) {
    cp = input;
    for (argc = 1; argc < 20; argc++) {
      if ((argv[argc] = strtok(cp,delim)) == NULL)
        break;
      cp = NULL;
    }
  }
  cp = NULL;
  cout << "argc : " << argc << endl;
  for (int i = 0; i < argc; i++) {
    printf("%d番目の引数 = %s\n", i, argv[i]);
  }
  po.parse(argc, argv);
  cp = NULL;
  string area_string(po.get<string>("debug"));
  string area_string;
  cin >> area_string;
  //cout << "area_string : " << area_string << endl;
  if(area_string!=""){                 // デバッグ出力
    Logger::instance().set_log_level(Logger::Debug);
    if(area_string.find('a') != string::npos){
     Logger::instance().set_log_level(Logger::Debug);
      Logger::parsing_area_ = true;
      Logger::calculate_closure_area_ = true;
      Logger::phase_area_ = true;
      Logger::vcs_area_ = true;
      Logger::extern_area_ = true;
      Logger::rest_area_ = true;
    }else if(area_string.find('n') != string::npos){
      Logger::instance().set_log_level(Logger::None);
      Logger::parsing_area_ = false;
      Logger::calculate_closure_area_ = false;
      Logger::phase_area_ = false;
      Logger::vcs_area_ = false;
      Logger::extern_area_ = false;
      Logger::rest_area_ = false;
      //cout << "none" << endl;
    }else{
      Logger::parsing_area_ = (area_string.find('p') != string::npos);
      Logger::calculate_closure_area_ = (area_string.find('c') != string::npos);
      Logger::phase_area_ = (area_string.find('m') != string::npos);
      Logger::vcs_area_ = (area_string.find('v') != string::npos);
      Logger::extern_area_ = (area_string.find('e') != string::npos);
      Logger::rest_area_ = (area_string.find('r') != string::npos);
    }
  }
  else {                              // 警告のみ出力
    Logger::instance().set_log_level(Logger::Warn);
  }
  cin.ignore( 1024,'\n');
  cin.clear();
 return 0;
}
*/



int InteractiveSimulator::save_state(simulation_todo_sptr_t& todo){
  cout << "input name of the file to save (default: last_phase)" <<endl;
  string file_name;
  getline(cin, file_name);
  if(file_name == "")file_name = "last_phase";
  io::JsonWriter writer;
  writer.write_phase(todo->parent, file_name);
  cout << "saved phase" << endl;
  return 1;
}


int InteractiveSimulator::load_state(simulation_todo_sptr_t& todo){
  cout << "input name of the file to load (default: last_phase)" <<endl;
  string file_name;
  getline(cin, file_name);
  if(file_name == "")file_name = "last_phase";
  io::JsonReader reader;
  phase_result_sptr_t loaded_phase = reader.read_phase(file_name);


  loaded_phase->step = 0;
  loaded_phase->parent = result_root_;
  result_root_->children.clear();
  result_root_->children.push_back(loaded_phase);
  todo.reset(new SimulationTodo(loaded_phase));
  todo->ms_to_visit = module_set_container_->get_full_ms_list();
  if(todo->phase_type == PointPhase)
  {
    TimeModifier modifier(*backend);
    for(auto entry : loaded_phase->variable_map)
    {
      todo->prev_map[entry.first] = modifier.substitute_time(todo->current_time, entry.second);
    }
  }
  else
  {
    todo->prev_map = loaded_phase->variable_map;
  }
  cout << "loaded phase" << endl;
  print_phase(loaded_phase);
  return 1;
}

int InteractiveSimulator::find_unsat_core(simulation_todo_sptr_t & todo){
/*  phase_simulator_->find_unsat_core(
    todo->parent->module_set_container->get_max_module_set(),
    todo,
    todo->parent->variable_map);
*/
  return 0;
}


int InteractiveSimulator::set_breakpoint(simulation_todo_sptr_t & todo){
  cout << "input break point" << endl;
  stringstream ss;
  string break_str;
  getline(cin, break_str);
  cin.clear();
  ss << break_str;
  
  HydLaAST ast;
  try
  {
    ast.parse(ss, HydLaAST::CONSTRAINT);
  }
  catch(SyntaxError e)
  {
    cout << "invalid condition" << endl;
    return 0;
  }

  NodeTreeGenerator genarator;
  symbolic_expression::node_sptr node_tree = genarator.generate(ast.get_tree_iterator());

  phase_simulator_->set_break_condition(node_tree);
  return 0;
}


}
}
