#ifndef _INCLUDED_HYDLA_INTERACTIVE_SIMULATOR_H_
#define _INCLUDED_HYDLA_INTERACTIVE_SIMULATOR_H_

#include "Simulator.h"
#include <fstream>
#include "SymbolicSimulator.h"
#include "Logger.h"
#include "version.h"
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "SymbolicTrajPrinter.h"
//#include <ncurses.h>

using namespace std;
using namespace hydla::logger;

namespace hydla {
namespace simulator {

class InteractiveSimulator:public Simulator{
public:
  typedef PhaseResult                                       phase_result_t;
  typedef boost::shared_ptr<phase_result_t>        phase_result_sptr;
  typedef boost::shared_ptr<const phase_result_t>  phase_result_const_sptr;
  typedef PhaseSimulator                                    phase_simulator_t;
  typedef phase_result_t::phase_result_sptr_t      phase_result_sptr_t;
  typedef std::vector<phase_result_sptr_t >                  phase_result_sptrs_t;
  typedef std::vector<simulation_phase_sptr_t>             simulation_phases_t;

  typedef phase_result_t::variable_map_t variable_map_t;
  typedef phase_result_t::variable_t     variable_t;
  typedef phase_result_t::parameter_t     parameter_t;
  typedef phase_result_t::value_t        value_t;
  typedef phase_result_t::parameter_map_t     parameter_map_t;

  typedef std::list<variable_t>                            variable_set_t;
  typedef std::list<parameter_t>                           parameter_set_t;
  typedef simulator::ValueRange                            value_range_t;
  typedef hydla::vcs::SymbolicVirtualConstraintSolver solver_t;
  typedef PhaseSimulator::TodoAndResult                   todo_and_result_t;


  InteractiveSimulator(Opts &opts):Simulator(opts){
  }

  virtual ~InteractiveSimulator(){}
  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  virtual phase_result_const_sptr_t simulate(){
    hydla::output::SymbolicTrajPrinter Printer(opts_->output_variables);
    while(!state_stack_.empty()) {
      simulation_phase_sptr_t state(pop_simulation_phase());
      phase_result_sptr_t& pr = state->phase_result;
      bool consistent;
      int exit = 0;
      // TODO:comment out for phase_simulator
      try{
        if( opts_->max_phase >= 0 && pr->step > opts_->max_phase)
          continue;
        all_state_.push_back(state); 
        state->module_set_container->reset(state->ms_to_visit);
        PhaseSimulator::todo_and_results_t phases = phase_simulator_->simulate_phase(state, consistent);

        if(pr->phase == PointPhase){
          exit = interactive_simulate(phases[0]);
        }

        if(!phases.empty()){
          Printer.output_one_phase(phases[0].todo->phase_result->parent);
          // std::cout << "size" << phases.size() << std::endl;
          int selectcase = 0;
          if(phases.size()>1){
            cout << "branch off into "<< phases.size() << "cases" << endl;
            cout << "-------------------------------------" << endl;
            for(unsigned int i=0;i<phases.size();i++){
              cout << "-----Case " << i << "--------------------------" << endl;
              Printer.output_one_phase(phases[i].todo->phase_result->parent);
            }
            cout << "-------------------------------------" << endl;
            cout << "-------------------------------------" << endl;
            message_ = "input select case number";
            //selectcase = excin<int>(message_);
          }
          if(phases[selectcase].todo->phase_result->parent != result_root_){
            phases[selectcase].todo->module_set_container = msc_no_init_;
          }else{
            // TODO これだと，最初のPPで分岐が起きた時のモジュール集合がおかしくなるはず
            phases[selectcase].todo->module_set_container = msc_original_;
          }

          push_simulation_phase(phases[selectcase].todo);
        }
        if(exit){
          variable_map_t vm = pr->variable_map;
          variable_map_t::const_iterator v_it  = vm.begin();
          for(v_it = vm.begin(); v_it!=vm.end(); ++v_it) {
            cout << *(v_it->first) << "\t: " << *(v_it->second) << endl;
          }
          break;
        }
      }catch(const std::runtime_error &se){
        cout << se.what() << endl;
        HYDLA_LOGGER_REST(se.what());
      }
    }
    return result_root_;
  }


  virtual void initialize(const parse_tree_sptr& parse_tree){
    Simulator::initialize(parse_tree);
    state_id_ = 1;
    //初期状態を作ってスタックに入れる
    simulation_phase_sptr_t state(new simulation_phase_t());
    phase_result_sptr_t& pr = state->phase_result;
    pr.reset(new phase_result_t());
    pr->cause_of_termination = NONE;
    pr->phase        = simulator::PointPhase;
    pr->step         = 0;
    pr->current_time = value_t(new hydla::symbolic_simulator::SymbolicValue("0"));
    state->module_set_container = msc_original_;
    pr->parent = result_root_;
    push_simulation_phase(state);
  }

  int change_parameter(){
    return 0;
  }
  
  /*
   * interactiveにsimulate
   * 入力を受付けそれぞれの処理に飛ばす
   * jを別メソッドに
   */

  int interactive_simulate(todo_and_result_t& phase){
    //  change_variable_flag = false;
    opts_->max_time = "100";
    string line;
    cout << '>';
    getline(cin,line);

    switch(line[0]){
      case 'j':
        //jump
        cout << "j" << endl;
        int target_step;
        if(all_state_.size()!=0)
        {
          //debug{{{
          for(unsigned int i=0;i<all_state_.size();i++)
          { 
            phase_result_sptr_t &pr = all_state_[i]->phase_result;
            if(pr->phase==PointPhase)
            {
              cout <<  "[debug] step" << pr->step+1 << " pp t:" << pr-> current_time<< endl;
            }else if(pr->phase==IntervalPhase)
            {
              cout <<  "[debug] step" << pr->step+1 << " ip t:" << pr-> current_time<< "->" <<endl;
            }
          }
          //debug}}}
          message_="input step number to jump";
          target_step = excin<int>(message_);
          for(unsigned int i=0;i<all_state_.size();i++)
          {
            phase_result_sptr_t &pr = all_state_[i]->phase_result;
            if(pr->step+1==target_step && pr->phase==PointPhase)
            {
              cout << "jump to step "<<pr->step+1 << " time:" << pr->current_time << endl;
              cout << "[debug] state size"<< all_state_.size() << "jump number" << i << endl;
              phase.todo->phase_result = pr;
              int j=0;
              int delete_size = all_state_.size()-i;
              for(j=0;j<delete_size;j++)
              {
                all_state_.pop_back();
                cout << "[debug] delete " << j << endl;
              }
            }
          }
          break;
        }else
        {
          cout << "0 step" << endl;
          break;
        }
        // default:
        return 0;
      case 's':
        save_state(phase);
        return 0;
      case 'l':
        load_state(phase);
        return 0;
      case 'h':
        show_help();
        return 0;
      case 'w':
        change_variables(phase.todo->phase_result);
        //change_variable_flag = 1;
        return 0;
      case 'd':
        select_options();
        return 0;
      case 'b':
        set_breakpoint();
      case 'p':
        print();
    }
    if(line[0] == 'q'||line[0] == 's'){
      cout << "exit" << endl;
      return 1;
    }
    return 0;
  }

  /*
   * hyroseのオプションを自由に変更する
   * 現在debugオプションのみ
   */
  int select_options(){
    /*
     * programoption がシングルトンなので対応策を考える 
     ProgramOptions &po = ProgramOptions::instance();
    int argc = 0;
    std::cout << "argc : " << argc << std::endl;
    char input[100], *argv[20], *cp;
    const char *delim = " \t\n"; 
    argv[0] = "./hyrose";
    std::cout << "enter the hyrose option args" << std::endl;
    if (fgets(input, sizeof(input), stdin) != NULL) {
      cp = input;
      for (argc = 1; argc < 20; argc++) {
        if ((argv[argc] = strtok(cp,delim)) == NULL)
          break;
        cp = NULL;
      }
    }
    cp = NULL;
    std::cout << "argc : " << argc << std::endl;
    for (int i = 0; i < argc; i++) {
      printf("%d番目の引数 = %s\n", i, argv[i]);
    }
    po.parse(argc, argv);
    cp = NULL;
    std::string area_string(po.get<std::string>("debug"));
    */
    std::string area_string;
    std::cin >> area_string;
    //std::cout << "area_string : " << area_string << std::endl;
    if(area_string!=""){                 // デバッグ出力
      Logger::instance().set_log_level(Logger::Debug);
      if(area_string.find('a') != std::string::npos){
        Logger::instance().set_log_level(Logger::Debug);
        Logger::parsing_area_ = true;
        Logger::calculate_closure_area_ = true;
        Logger::phase_area_ = true;
        Logger::vcs_area_ = true;
        Logger::extern_area_ = true;
        Logger::rest_area_ = true;
      }else if(area_string.find('n') != std::string::npos){
        Logger::instance().set_log_level(Logger::None);
        Logger::parsing_area_ = false;
        Logger::calculate_closure_area_ = false;
        Logger::phase_area_ = false;
        Logger::vcs_area_ = false;
        Logger::extern_area_ = false;
        Logger::rest_area_ = false;
        //std::cout << "none" << std::endl;
      }else{
        Logger::parsing_area_ = (area_string.find('p') != std::string::npos);
        Logger::calculate_closure_area_ = (area_string.find('c') != std::string::npos);
        Logger::phase_area_ = (area_string.find('m') != std::string::npos);
        Logger::vcs_area_ = (area_string.find('v') != std::string::npos);
        Logger::extern_area_ = (area_string.find('e') != std::string::npos);
        Logger::rest_area_ = (area_string.find('r') != std::string::npos);
      }
    }
    else {                              // 警告のみ出力
      Logger::instance().set_log_level(Logger::Warn);
    }
    cin.ignore( 1024,'\n');
    cin.clear();
     return 0;
  }

  /*
   * interactiveモードにおけるヘルプを表示する
   * 各コマンドの細かいhelp必要
   */
  int show_help(){
    const char *help_list[] = {
      "Type <return> to exec one step.",
      "Type [command] <return> to exec command",
      "List of commands:",
      "",
      "breakpoints    -- Making program stop at certain points",
      "debug          -- Simulate with debug-mode",
      "edit           -- Edit constraint ",
      "help [command] -- Show help (for command)",
      "jump           -- Jump to certain step",
      "load           -- Load state ",
      "quit           -- ",
      "run            -- simulate program  until the breakpoint is reached",
      "save           -- Save state to file",
      "write          -- Rewrite selected variable",
      "print          -- Display a variety of information"
    };
    for(int i = 0;i<15;i++){
      printf("%s\n",help_list[i]);
      }
    return 0;
  }
  /*
   * 変数の変更
   * 区間値を入れる
   */
  int change_variables(phase_result_sptr& phase){
    cout << "(change variable mode)" << endl;
    variable_map_t vm = phase->parent->variable_map;
    //phase_result_t result = *phase->parent;
    //vm = result.variable_map;
    variable_map_t::const_iterator v_it  = vm.begin();
    //v_itを選ぶ
    string changevariable;
    ostringstream name;
    cout << "input variable name " << endl;
    changevariable = excin<string>();
    //cout << changevariable << " : "<< changevariable.substr(0,1) <<endl;
    for(;v_it!=vm.end();v_it++){
      name.str("");
      name << *(v_it->first);
    //cout << name.str() << " : "<< changevariable <<endl;
      if( name.str() == changevariable)
        break;
    }
    //cout << *(v_it->first) << endl;
    string strvalue;
    cout << "input values to change " << endl;
    cout << '>';
    getline(cin,strvalue);
    //後で関数化する
    //cout << strvalue << " : "<< strvalue.substr(0,1) <<endl;
    if(strvalue.substr(0,1)!="(" & strvalue.substr(0,1)!="["){
    
      value_t testvalue(new hydla::symbolic_simulator::SymbolicValue(strvalue));
      vm[v_it->first] = testvalue;
      cout << "(result of change)" << endl;
      for(v_it = vm.begin(); v_it!=vm.end(); ++v_it) {
        cout << *(v_it->first) << "\t: " << *(v_it->second) << endl;
      }
      cout << "" << endl;
      phase->parent->variable_map = vm;
      
    }else{
      cout << "parameter change" << endl;
      parameter_map_t pm = phase->parent->parameter_map;
      parameter_map_t::const_iterator it  = pm.begin();
      
      bool upperflag = (strvalue.substr(0,0)=="[");
      bool lowerflag = (strvalue.substr(0,strvalue.size())=="]");
      string rangevalue = strvalue.substr(1,strvalue.size()-2);
      vector<string> v;
      // boost::algorithm::split( v, rangevalue, boost::is_any_of(",") );
      cout << v[0] << ":" <<v[1]<<endl;
      
      value_t lowvalue(new hydla::symbolic_simulator::SymbolicValue(v[0]));
      value_t upvalue(new hydla::symbolic_simulator::SymbolicValue(v[1]));
      
      value_range_t testrange;
      testrange.set_upper_bound(upvalue,upperflag);
      testrange.set_lower_bound(lowvalue,lowerflag);
      parameter_t param(v_it->first, phase->parent);
      phase_simulator_->set_parameter_set(param);
      pm[&param] = testrange;
      value_t pvalue(new hydla::symbolic_simulator::SymbolicValue(
        node_sptr(new Parameter(v_it->first->get_name(),
        v_it->first->get_derivative_count(),
        phase->parent->id))));
      vm[v_it->first] = pvalue;
      
      cout << "parameter change id :" << phase->parent->id << endl;
      phase->parent->variable_map = vm;
      phase->parent->parameter_map = pm;

      cout << "-------parameter test--------" << endl;
      for(it = pm.begin(); it!=pm.end(); ++it) {
        cout << *(it->first) << "\t: " << it->second << endl;
      }
      cout << "-------variable map  --------"  << endl;
      for(v_it = vm.begin(); v_it!=vm.end(); ++v_it) {
        cout << *(v_it->first) << "\t: " << *(v_it->second) << endl;
      }
      cout << "-------parameter test--------"  << endl;
    }
    return 0;
  }
  /*
   * breakpoint をセットする
   * 現状step実行のみなので、xが0になるまで実行とかできるように
   */
  int set_breakpoint(){
    return 0;
  }
  /*
   * printし直す
   */
  int print(){
    return 0;
  }
  /*
   * save state
   */
  int save_state(todo_and_result_t& simulation_phase){
    //FILE *fp;
    //fp = fopen("save.dat", "w");
    //phase_root_まで
    ofstream ofs( "test.txt", ios::out|ios::binary|ios::trunc );
    phase_result_sptr_t temp;
    temp = simulation_phase.todo->phase_result->parent;
    for(int i = temp->id;i>0; i--){
      Phase                     phase;
      int                       id;
      hydla::simulator::PhaseResult::time_t                    current_time, end_time;
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

  int load_state(todo_and_result_t& simulation_phase){
    //FILE *fp;
    //fp = fopen("save.dat", "rb+");
    ifstream ifs( "test.txt" ,ios::in | ios::binary);
    //fseek(fp, 0L, SEEK_SET);
    simulation_phase_sptr_t temp_phase(phase_simulator_->create_new_simulation_phase());
    phase_result_sptr_t temp = temp_phase->phase_result;

    
    for(int i = 0;i<1;i++){
    /*
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
      */
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

      /*
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
      std::cout <<"load " <<  *(v_it->first) << "\t: " << *(v_it->second) << std::endl;
      }
      temp->variable_map         = variable_map;
      //variable_map_t::const_iterator v_it  = variable_map.begin();
      //for(v_it = variable_map.begin(); v_it!=variable_map.end(); ++v_it) {
      //  std::cout << *(v_it->first) << "\t: " << *(v_it->second) << std::endl;
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
      */
    }

    //simulation_phase.phase_result->parent = temp;


    return 1;
  }
  /*
   * template mwthod for input 
   */
  template<typename T> T excin(string message=""){
    T text;
    while(1){
      if(!message.empty())
        cout << message << endl;
      cout << '>' ;
      std::cin >> text;
      if(!std::cin.fail())
        break;
      std::cin.clear();
      std::cin.ignore( 1024, '\n' );
    }
    std::cin.clear();
    std::cin.ignore( 1024, '\n' );
    return text;
  }
  /*
   * message string for input
   */
  string message_;

  /**
   * 各PhaseResultに振っていくID
   */
  int phase_id_;

  /**
   * シミュレーション中で使用される変数表の原型
   */
  variable_map_t variable_map_;


  /*
   * シミュレーション中に使用される変数と記号定数の集合
   */
  variable_set_t variable_set_;
  parameter_set_t parameter_set_;

  /**
   * シミュレーション対象となるパースツリー
   */
  parse_tree_sptr parse_tree_;

  std::vector<simulation_phase_sptr_t> all_state_;
  boost::shared_ptr<solver_t> solver_;
};

} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_INTERACTIVE_SIMULATOR_H_

