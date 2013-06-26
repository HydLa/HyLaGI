#include "InteractiveSimulator.h"
#include "Dumpers.h"
// surpress warning "C4996: old 'strcpy'" on VC++2005
#define _CRT_SECURE_NO_DEPRECATE
#include <boost/algorithm/string.hpp>

namespace hydla {
namespace simulator {

using namespace std;
hydla::output::SymbolicTrajPrinter InteractiveSimulator::printer_;


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

  while(todo_num)
  {
    try
    {
      PhaseSimulator::result_list_t phases = phase_simulator_->calculate_phase_result(todo);
      
      phase_result_sptr_t phase;
      if(phases.empty())
      {
        simulation_todo_sptr_t tmp_todo = todo;
        do
        {
          print_end(todo->parent);
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
          phases[i]->cause_of_termination = NOT_SELECTED;
        }
      }
      
      PhaseSimulator::todo_list_t todos = phase_simulator_->make_next_todo(phase, todo);

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
      
      if(--todo_num == 0)
      {
        todo_num = input_and_process_command(todo);
      }
      // TODO:現状だと，result_root_のところまで戻ると変なことになるのでそこまでは巻き戻せないようにしておく
      all_todo_.push_back(todo);
    }
    catch(const runtime_error &se)
    {
      cout << se.what() << endl;
      HYDLA_LOGGER_REST(se.what());
      break;
    }
  }
  return result_root_;
}


int InteractiveSimulator::input_and_process_command(simulation_todo_sptr_t& todo){

  while(true)
  {
    string line;
    cout << "input a command ('h' is to show help)\n";
    cout << '>';
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
        todo->reset_from_start_of_phase();
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
      case 'c':
        change_variable(todo);
        print_phase(todo->parent);
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
    "j [arg]        -- jump forward or backward by given numer of phases",
    "                  positive number: forward (equal to normal simulate)",
    "                  negative number: backward",
    "q              -- quit this simulation",
    "c              -- change a value of a variable",
    "p              -- display the information of the current phase"
    //"breakpoints    -- Making program stop at certain points",
    //"debug          -- Simulate with debug-mode",
    //"edit           -- Edit constraint ",
    //"load           -- Load state ",
    //"run            -- simulate program  until the breakpoint is reached",
    //"save           -- Save state to file",
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

int InteractiveSimulator::change_variable(simulation_todo_sptr_t& todo){

  cout << "(change variable mode)" << endl;
  variable_map_t& vm = todo->parent->variable_map;
  
  // 変数の選択
  variable_map_t::iterator v_it  = vm.begin();
  cout << "input variable name " << endl;
  cout << '>';
  string variable_str = excin<string>();
  for(;v_it!=vm.end();v_it++){
    if( v_it->first->get_string() == variable_str) break;
  }
  
  string value_str;
  cout << "input value" << endl;
  cout << '>';
  getline(cin, value_str);
  
  if(value_str.substr(0,1)!="(" && value_str.substr(0,1)!="["){
    value_t value(new hydla::simulator::symbolic::SymbolicValue(value_str));
    // TODO: stringでvalue作ると，バックエンド変えた時に対応できないので何とかする
    // 　　　（これはSymbolicValueのコンストラクタ側の問題かもしれない）
    //       あとこれだと不正な式入力された場合も対応できない
    vm[v_it->first] = value;
  }else{
    parameter_map_t& pm = todo->parameter_map;
    parameter_map_t::const_iterator it  = pm.begin();

    bool upperflag = (value_str[0] == '[');
    bool lowerflag = (value_str[value_str.size()-1] == ']');
 
    string rangevalue = value_str.substr(1,value_str.size()-2);
    vector<string> v;
    boost::algorithm::split( v, rangevalue, boost::is_any_of(",") );
    
    value_t lowvalue(new hydla::simulator::symbolic::SymbolicValue(v[0]));
    value_t upvalue(new hydla::simulator::symbolic::SymbolicValue(v[1]));
    
    ValueRange range;
    range.set_upper_bound(upvalue,upperflag);
    range.set_lower_bound(lowvalue,lowerflag);
    parameter_t* introduced_par = introduce_parameter(v_it->first, todo->parent, range);
    pm[introduced_par] = range;
    value_t pvalue(new hydla::simulator::symbolic::SymbolicValue(
      hydla::parse_tree::node_sptr(new hydla::parse_tree::Parameter(v_it->first->get_name(),
      v_it->first->get_derivative_count(),
      todo->parent->id))));
    vm[v_it->first] = pvalue;
    
    todo->parent->parameter_map = todo->parameter_map;
  }
  return 0;
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


 /*
int save_state(simulation_todo_sptr_t& simulation_phase){
  //FILE *fp;
  //fp = fopen("save.dat", "w");
  //phase_root_まで
  ofstream ofs( "test.txt", ios::out|ios::binary|ios::trunc );
  phase_result_sptr_t temp;
  temp = simulation_phase.todo->phase_result->parent;
  for(int i = temp->id;i>0; i--){
    Phase                     phase;
    int                       id;
    hydla::simulator::time_t                    current_time, end_time;
    variable_map_t            variable_map;
    parameter_map_t           parameter_map;
    expanded_always_t         expanded_always;
    positive_asks_t           positive_asks;
    int                       step;
    //hydla::ch::ModuleSet module_set;
    CauseOfTermination         cause_of_termination;
    //phase_result_sptrs_t       children;
    //phase_result_sptr_t        parent;

    phase           = temp->phase;
    id              = temp->id;
    current_time    = temp->current_time;
    end_time        = temp->end_time;
    variable_map    = temp->variable_map;
    int vm_size     = variable_map.size();
    variable_map_t::const_iterator v_it  = variable_map.begin();
    parameter_map   = temp->parameter_map;
    int pm_size     = parameter_map.size();
    parameter_map_t::const_iterator p_it  = parameter_map.begin();
    expanded_always = temp->expanded_always;
    positive_asks   = temp->positive_asks;
    step            = temp->step;
    //module_set           = *(temp->module_set);
    cause_of_termination = temp->cause_of_termination;
    //children             = pr->children;
    //parent               = pr->parent;

      //ofs << phase << endl;
      ofs.write((char*) &phase, sizeof(Phase));
      cout << "phase " << phase << endl;
      //ofs << id << endl;
      ofs.write((char*) &id, sizeof(int));
      cout << "id " << id << endl;
    //fwrite(&current_time    , sizeof(time_t)               , 1 , fp);
    //fwrite(&end_time        , sizeof(time_t)               , 1 , fp);
      //ofs << vm_size << endl;
      ofs.write((char*) &vm_size, sizeof(int));
      cout << "vm_size " << vm_size << endl;
      string first;
      string second;
      int first_size,second_size;
      for(v_it = variable_map.begin(); v_it!=variable_map.end(); ++v_it) {
        first = (*(v_it->first)).get_string();
        second = (*(v_it->second)).get_string();
        first_size = first.size();
        second_size = second.size();
        ofs.write((char*) &first_size, sizeof(int));
        ofs.write((char*)(first.c_str()), first_size);
        cout << "first " << first << " size "<< endl;
        ofs.write((char*) &second_size, sizeof(int));
        ofs.write((char*)(second.c_str()), second_size);
        cout << "second " << second << "size " << endl;
        //ofs << first << endl;
        //ofs << second << endl;
      }
    //ofs << pm_size << endl;
    for(p_it = parameter_map.begin(); p_it!=parameter_map.end(); ++p_it) {
      first = (*(p_it->first)).get_name();
      second = (p_it->second).get_string();
        first_size = first.size();
        second_size = second.size();
        ofs.write((char*) &first_size, sizeof(int));
        ofs.write((char*)(first.c_str()), first_size);
        cout << "p first " << first << " size "<< endl;
        ofs.write((char*) &second_size, sizeof(int));
        ofs.write((char*)(second.c_str()), second_size);
        cout << "p second " << second << "size " << endl;
      //ofs << *(p_it->first) << endl;
      //ofs << p_it->second << endl;
      //cout << "save parameter "<< *(p_it->first) << "\t: " << p_it->second << endl;
    }
      //ofs << expanded_always << endl;
      //ofs << positive_asks << endl;
      //ofs << changed_asks << endl;
      //ofs << step << endl;
     //fwrite(&module_set    , sizeof(hydla::ch::ModuleSet) , 1 , fp);
      //ofs << cause_of_termination << endl;
    temp = temp->parent;
  }

  //fclose(fp);

  return 1;
}
*/

/*
int load_state(simulation_todo_sptr_t& simulation_phase){
  //FILE *fp;
  //fp = fopen("save.dat", "rb+");
  ifstream ifs( "test.txt" ,ios::in | ios::binary);
  //fseek(fp, 0L, SEEK_SET);
  simulation_todo_sptr_t temp_phase(phase_simulator_->create_new_simulation_phase());
  phase_result_sptr_t temp = temp_phase->phase_result;
  
  for(int i = 0;i<1;i++){
    Phase                     phase;
    int                       id;
    hydla::simulator::PhaseResult::time_t                    current_time, end_time;
    variable_map_t            variable_map;
    int vm_size;
    int pm_size;
    parameter_map_t           parameter_map;
    expanded_always_t         expanded_always;
    positive_asks_t           positive_asks;
    int                       step;
    //hydla::ch::ModuleSet module_set;
    CauseOfTermination         cause_of_termination;

    cout << "file read start" << endl;

    //ifs >> phase;
    ifs.read((char *) &phase, sizeof(Phase));
    cout << "read phase "<< phase  << endl;
    //ifs >> id;
    ifs.read((char *) &id, sizeof(int));
    cout << "read id "<< id << endl;
    //fread(&current_time         , sizeof(time_t)               , 1 , fp);
    //fread(&end_time             , sizeof(time_t)               , 1 , fp);
    //ifs >> vm_size;
    ifs.read((char *) &vm_size, sizeof(int));
    cout << "read vm_size "<< vm_size << endl;
    string v_first,v_second;
    int v_first_size,v_second_size;
    for(int i=0;i<vm_size;i++){
      //ifs >> v_first;
      ifs.read((char *) &v_first_size, sizeof(int));
      char* buf = new char[v_first_size+1];
      ifs.read(buf, v_first_size);
      buf[v_first_size]=0;
      cout << 1 << endl;
      v_first = buf;
      cout << 1 << endl;
      cout << "first " << v_first << " size "<< endl;
      cout << 1 << endl;
      //ifs >> v_second;
      ifs.read((char *) &v_second_size, sizeof(int));
      char* buf2 = new char[v_second_size+1];
      ifs.read(buf, v_second_size);
      buf2[v_second_size]=0; 
      v_second = buf;
      cout << "second " << v_second << " size "<< endl;
      delete []buf;
      delete []buf2;
    }
    //ifs >> pm_size;;
    //for(int i=0;i<pm_size;i++){
    //ifs >> v_first;
    //ifs >> v_second;
    //cout << "read parameter "<< v_first << "\t: " << v_second << endl;
    //}
    //ifs >> expanded_always;
    //ifs >> positive_asks;
    //ifs >> changed_asks;
    //ifs >> step;
    //fread(&module_set         , sizeof(hydla::ch::ModuleSet) , 1 , fp);
    //ifs >> cause_of_termination;

    cout << "read end "<< endl;

    //temp->phase                = phase;
    cout << "phase " << temp->phase << endl;
    temp->id                   = id;
    cout << "id " << temp->id << endl;
    //temp->current_time         = current_time;
    //cout << "current_time " << temp->current_time << endl;
    //temp->end_time             = end_time;
    //cout << "end_time " << temp->end_time << endl;
    variable_map_t::const_iterator v_it  = variable_map.begin();
    for(v_it = variable_map.begin(); v_it!=variable_map.end(); ++v_it) {
    cout <<"load " <<  *(v_it->first) << "\t: " << *(v_it->second) << endl;
    }
    temp->variable_map         = variable_map;
    //variable_map_t::const_iterator v_it  = variable_map.begin();
    //for(v_it = variable_map.begin(); v_it!=variable_map.end(); ++v_it) {
    //  cout << *(v_it->first) << "\t: " << *(v_it->second) << endl;
    //}
    temp->parameter_map        = parameter_map;
    cout << 3333333<< endl;
    //temp->expanded_always      = expanded_always;
    //temp->positive_asks        = positive_asks;
    //temp->changed_asks         = changed_asks;
    //temp->step                 = step;
    // *(temp->module_set)        = module_set;
    //temp->cause_of_termination = cause_of_termination;
    cout << 444444<< endl;
  }

  //simulation_phase.phase_result->parent = temp;


    
  return 1;
}
*/

}
}
