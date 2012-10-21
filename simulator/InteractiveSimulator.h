#ifndef _INCLUDED_HYDLA_INTERACTIVE_SIMULATOR_H_
#define _INCLUDED_HYDLA_INTERACTIVE_SIMULATOR_H_

#include "Simulator.h"
#include "SymbolicSimulator.h"
#include "Logger.h"
#include "version.h"
#include <sstream>
#include <boost/algorithm/string.hpp>
//#include <ncurses.h>

#ifndef _MSC_VER
#include <curses.h>
#endif
using namespace std;
using namespace hydla::logger;

namespace hydla {
namespace simulator {


  template<typename PhaseResultType>
    class InteractiveSimulator:public Simulator<PhaseResultType>{
      public:
        typedef PhaseResultType                                   phase_result_t;
        typedef typename boost::shared_ptr<phase_result_t>        phase_result_sptr;
        typedef typename boost::shared_ptr<const phase_result_t>  phase_result_const_sptr;
        typedef PhaseSimulator<PhaseResultType>                   phase_simulator_t;
        typedef typename phase_result_t::phase_result_sptr_t      phase_result_sptr_t;
        typedef typename std::vector<phase_result_sptr_t >                  phase_result_sptrs_t;

        typedef typename phase_result_t::variable_map_t variable_map_t;
        typedef typename phase_result_t::variable_t     variable_t;
        typedef typename phase_result_t::parameter_t     parameter_t;
        typedef typename phase_result_t::value_t        value_t;
        typedef typename phase_result_t::parameter_map_t     parameter_map_t;

        typedef std::list<variable_t>                            variable_set_t;
        typedef std::list<parameter_t>                           parameter_set_t;
        typedef value_t                                          time_value_t;
        typedef simulator::ValueRange<value_t>         value_range_t;
        typedef hydla::vcs::SymbolicVirtualConstraintSolver solver_t;


        InteractiveSimulator(Opts &opts):Simulator<phase_result_t>(opts){
        }

        virtual ~InteractiveSimulator(){}
        /**
         * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
         */
        virtual void simulate(){
          while(!state_stack_.empty()) {
            phase_result_sptr state(pop_phase_result());
            bool consistent;
            int exit = 0;
            try{
              if( Simulator<phase_result_t>::opts_->max_step >= 0 && state->step > Simulator<phase_result_t>::opts_->max_step)
                continue;
              all_state_.push_back(state); 
              state->module_set_container->reset(state->visited_module_sets);
              phase_result_sptrs_t phases = Simulator<phase_result_t>::phase_simulator_->simulate_phase(state, consistent);

              if(state->phase == PointPhase){
                exit = interactivesimulate(state,phases[0]);
              }

              if(!phases.empty())
                std::cout << get_state_output(*phases[0]->parent,0,1) << std::endl;
              //std::cout << "size" << phases.size() << std::endl;
                int selectcase = 0;
                //* 変数変更ができるまでいったんコメントアウト できたら順番を考える
                if(phases.size()>1){
                  std::cout << "branch off into "<< phases.size() << "cases" << std::endl;
                  std::cout << "-------------------------------------" << std::endl;
                  for(int i=0;i<phases.size();i++){
                    std::cout << "-----Case " << i << "--------------------------" << std::endl;
                    std::cout << get_state_output(*phases[i]->parent,0,1) << std::endl;
                  }
                  std::cout << "-------------------------------------" << std::endl;
                  std::cout << "-------------------------------------" << std::endl;
                  message_ = "input select case number";
                  selectcase = excin<int>(message_);
                }
                //*/
                if(!phases.empty()){
                  if(consistent){
                    phases[selectcase]->module_set_container = Simulator<phase_result_t>::msc_no_init_;
                  }else{
                    phases[selectcase]->module_set_container = phases[selectcase]->parent->module_set_container;
                  }
                  push_phase_result(phases[selectcase]);
                }

                if(exit){
                  break;
                }
              }catch(const std::runtime_error &se){
                std::cout << se.what() << std::endl;
                HYDLA_LOGGER_REST(se.what());
              }
            }
      Simulator<phase_result_t>::output_result_tree();
          }

          /*
          variable_map_t shift_variable_map_time(const variable_map_t& vm, const time_t &time){
            variable_map_t shifted_vm;
            typename variable_map_t::const_iterator it  = vm.begin();
            typename variable_map_t::const_iterator end = vm.end();
            for(; it!=end; ++it) {
              if(it->second.is_undefined())
                shifted_vm.set_variable(it->first, it->second);
              else
                shifted_vm.set_variable(it->first, solver_->shift_expr_time(it->second, time));
            }
            return shifted_vm;
          }
          */

          virtual void initialize(const parse_tree_sptr& parse_tree){
            Simulator<phase_result_t>::initialize(parse_tree);
            //Simulator::initialize(parse_tree);
            Simulator<phase_result_t>::state_id_ = 1;
            //初期状態を作ってスタックに入れる
            phase_result_sptr state(Simulator<phase_result_t>::create_new_phase_result());
            state->phase        = simulator::PointPhase;
            state->step         = 0;
            state->current_time = value_t("0");
            state->module_set_container = Simulator<phase_result_t>::msc_original_;
            state->parent = Simulator<phase_result_t>::result_root_;
            push_phase_result(state);
          }
          /**
           * 状態キューに新たな状態を追加する
           */
          virtual void push_phase_result(const phase_result_sptr& state)
          {
            state->id = Simulator<phase_result_t>::state_id_++;
            HYDLA_LOGGER_PHASE("%% InteractiveSimulator::push_phase_result\n");
            HYDLA_LOGGER_PHASE("%% state Phase: ", state->phase);
            HYDLA_LOGGER_PHASE("%% state id: ", state->id);
            HYDLA_LOGGER_PHASE("%% state time: ", state->current_time);
            HYDLA_LOGGER_PHASE("--- parent state variable map ---\n", state->parent->variable_map);
            HYDLA_LOGGER_PHASE("--- state parameter map ---\n", state->parameter_map);
            state_stack_.push(state);
          }

          /**
           * 状態キューから状態をひとつ取り出す
           */
          phase_result_sptr pop_phase_result()
          {
            phase_result_sptr state(state_stack_.top());
            state_stack_.pop();
            return state;
          }

          int change_parameter(){
            return 0;
          }
          /*
           * interactiveにsimulate
           * 入力を受付けそれぞれの処理に飛ばす
           * jを別メソッドに
           */

          int interactivesimulate(phase_result_sptr& state, phase_result_sptr& phase){
            //  change_variable_flag = false;
            Simulator<phase_result_t>::opts_->max_time = "100";
            string line;
            cout << ">";
            getline(cin,line);

            switch(line[0]){
              case 'j':
                //jump
                std::cout << "j" << std::endl;
                int target_step;
                if(all_state_.size()!=0)
                {
                  //debug{{{
                  for(int i=0;i<all_state_.size();i++)
                  { 
                    if(all_state_[i]->phase==PointPhase)
                    {
                      cout <<  "[debug] step" << all_state_[i]->step+1 << " pp t:" << all_state_[i]-> current_time<< endl;
                    }else if(all_state_[i]->phase==IntervalPhase)
                    {
                      cout <<  "[debug] step" << all_state_[i]->step+1 << " ip t:" << all_state_[i]-> current_time<< "->" <<endl;
                    }
                  }
                  //debug}}}
                  message_="input step number to jump";
                  target_step = excin<int>(message_);
                  int i;
                  for(i=0;i<all_state_.size();i++)
                  {
                    if(all_state_[i]->step+1==target_step && all_state_[i]->phase==PointPhase)
                    {
                      cout << "jump to step "<<all_state_[i]->step+1 << " time:" << all_state_[i]->current_time << endl;
                      cout << "[debug] state size"<< all_state_.size() << "jump number" << i << endl;
                      phase = all_state_[i];
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
                         //           save_state();
                         return 0;
              case 'h':
                       show_help();
                       return 0;
              case 'w':
                       change_variables(phase);
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
            std::cout << "area_string : " << area_string << std::endl;
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
                std::cout << "none" << std::endl;
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
            int ch;
            const char *list[] = {
              "breakpoints -- Making program stop at certain points",
              "debug -- Simulate with debug-mode",
              "edit -- Edit constraint ",
              "help [command] -- Show help (for command)",
              "jump --  Jump to certain step", 
              "load -- Load state ",
              "quit --  ",
              "run -- simulate program  until the breakpoint is reached",
              "save --  Save state to file",
              "write -- Rewrite selected variable",
              "print -- Display a variety of information"
            };
#ifndef _MSC_VER
            initscr(); cbreak(); noecho();
            while((ch = getch()) != 'q'){
              mvaddstr(0, 0, "Push [q] to quit.");
              mvaddstr(1, 0,"Type <return> to exec one step.");
              mvaddstr(2, 0,"Type [command] <return> to exec command");
              mvaddstr(4, 0,"List of commands:");
              for(int i = 0;i<10;i++){
                mvaddstr(i+5, 0, list[i]);
              }
            }
            endwin();
#endif
            return 0;
          }
          /*
           * 変数の変更
           * 区間値を入れる
           */
          int change_variables(phase_result_sptr& phase){ 
            std::cout << "change variable " << std::endl;
            variable_map_t vm = phase->parent->variable_map;
            //phase_result_t result = *phase->parent;
            //vm = result.variable_map;
            typename variable_map_t::const_iterator v_it  = vm.begin();
            //v_itを選ぶ
            string changevariable;
            ostringstream name;
            changevariable = excin<string>();
            cout << changevariable << " : "<< changevariable.substr(0,1) <<endl;
            for(;v_it!=vm.end();v_it++){
              name.str("");
              name << *(v_it->first);
            cout << name.str() << " : "<< changevariable <<endl;
              if( name.str() == changevariable)
                break;
            }
            cout << *(v_it->first) << endl;
            string strvalue;
            cout << ">";
            getline(cin,strvalue);
            //後で関数化する
            cout << strvalue << " : "<< strvalue.substr(0,1) <<endl;
            if(strvalue.substr(0,1)!="(" & strvalue.substr(0,1)!="["){
              value_t testvalue(strvalue);
              vm.set_variable(v_it->first,testvalue);
              std::cout << "--------" << std::endl;
              for(v_it = vm.begin(); v_it!=vm.end(); ++v_it) {
                std::cout << *(v_it->first) << "\t: " << v_it->second << std::endl;
              }
              std::cout << "--------"  << std::endl;
              phase->parent->variable_map = vm;
            }else{
              std::cout << "parameter change" << std::endl;
              parameter_map_t pm = phase->parent->parameter_map;
              typename parameter_map_t::const_iterator it  = pm.begin();
              bool upperflag = (strvalue.substr(0,0)=="[");
              bool lowerflag = (strvalue.substr(0,strvalue.size())=="]");
              string rangevalue = strvalue.substr(1,strvalue.size()-2);
              vector<string> v;
              boost::algorithm::split( v, rangevalue, boost::is_any_of(",") );
              std::cout << v[0] << ":" <<v[1]<<endl;
              value_t lowvalue(v[0]);
              value_t upvalue(v[1]);
              value_range_t testrange;
              testrange.set_upper_bound(upvalue,upperflag);
              testrange.set_lower_bound(lowvalue,lowerflag);
              parameter_t param(v_it->first, phase->parent);
              Simulator<phase_result_t>::phase_simulator_->set_parameter_set(param);
              pm.set_variable(&param, testrange);
              value_t pvalue = node_sptr(new Parameter(v_it->first->get_name(), v_it->first->get_derivative_count(),phase->parent->id));
              vm.set_variable(v_it->first, pvalue);
              cout << "parameter change id :" << phase->parent->id << endl;
              phase->parent->variable_map = vm;
              phase->parent->parameter_map = pm;

              std::cout << "-------parameter test--------" << std::endl;
              for(it = pm.begin(); it!=pm.end(); ++it) {
                std::cout << *(it->first) << "\t: " << it->second << std::endl;
              }
              std::cout << "-------variable map  --------"  << std::endl;
              for(v_it = vm.begin(); v_it!=vm.end(); ++v_it) {
                std::cout << *(v_it->first) << "\t: " << v_it->second << std::endl;
              }
              std::cout << "-------parameter test--------"  << std::endl;
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
           * template mwthod for input 
           */
          template<typename T> T excin(string message=""){
            T text;
            while(1){
              if(!message.empty())
                cout << message << endl;
              cout << ">" ;
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
          //int state_id_;


          struct SimulationState {
            phase_result_sptr phase_result;
            /// フェーズ内で一時的に追加する制約．分岐処理などに使用
            constraints_t temporary_constraints;
            module_set_container_sptr module_set_container;
            /// 判定済みのモジュール集合を保持しておく．分岐処理時，同じ集合を複数回調べることが無いように
            std::set<module_set_sptr> visited_module_sets;
          };


          /**
           * 各状態を保存しておくためのスタック
           */
          std::stack<phase_result_sptr> state_stack_;


          /**
           * シミュレーション対象となるパースツリー
           */
          parse_tree_sptr parse_tree_;

          bool is_safe_;
          std::vector<phase_result_sptr> all_state_;
          boost::shared_ptr<solver_t> solver_;
          boost::shared_ptr<PhaseSimulator<PhaseResultType> > phase_simulator_;
    };

  } // simulator
} // hydla

#endif // _INCLUDED_HYDLA_INTERACTIVE_SIMULATOR_H_

