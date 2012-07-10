#ifndef _INCLUDED_HYDLA_INTERACTIVE_SIMULATOR_H_
#define _INCLUDED_HYDLA_INTERACTIVE_SIMULATOR_H_

#include "Simulator.h"
#include "Logger.h"
using namespace std;
using namespace hydla::logger;

namespace hydla {
  namespace simulator {


    template<typename PhaseStateType>
      class InteractiveSimulator:public Simulator<PhaseStateType>{
        public:
          typedef PhaseStateType                                   phase_state_t;
          typedef typename boost::shared_ptr<phase_state_t>        phase_state_sptr;
          typedef typename boost::shared_ptr<const phase_state_t>  phase_state_const_sptr;
          typedef PhaseSimulator<PhaseStateType>                   phase_simulator_t;
          typedef typename phase_state_t::phase_state_sptr_t      phase_state_sptr_t;
          typedef typename std::vector<phase_state_sptr_t >                  phase_state_sptrs_t;

          typedef typename phase_state_t::variable_map_t variable_map_t;
          typedef typename phase_state_t::variable_t     variable_t;
          typedef typename phase_state_t::parameter_t     parameter_t;
          typedef typename phase_state_t::value_t        value_t;
          typedef typename phase_state_t::parameter_map_t     parameter_map_t;

          typedef std::list<variable_t>                            variable_set_t;
          typedef std::list<parameter_t>                           parameter_set_t;
          typedef value_t                                          time_value_t;


          InteractiveSimulator(Opts &opts):Simulator<phase_state_t>(opts){
          }

          virtual ~InteractiveSimulator(){}
          /**
           * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
           */
          virtual void simulate()
          {
            while(!state_stack_.empty()) {
              phase_state_sptr state(pop_phase_state());
              bool consistent;
              int exit = 0;
              try{
                if( Simulator<phase_state_t>::opts_->max_step >= 0 && state->step > Simulator<phase_state_t>::opts_->max_step)
                  continue;
                if(state->phase == PointPhase)
                {
                  exit = interactivesimulate(state);
                  //std::cout << get_state_output(*state,0,1) << std::endl;
                  //std::cout << get_state_output(*phases[0]->parent,0,1) << std::endl;

                }
                state->module_set_container->reset(state->visited_module_sets);
                phase_state_sptrs_t phases = Simulator<phase_state_t>::phase_simulator_->simulate_phase_state(state, consistent);
                std::cout << get_state_output(*phases[0]->parent,0,1) << std::endl;


                if(!phases.empty()){
                  if(Simulator<phase_state_t>::opts_->nd_mode){
                    for(typename phase_state_sptrs_t::iterator it = phases.begin();it != phases.end();it++){
                      if(consistent){
                        (*it)->module_set_container = Simulator<phase_state_t>::msc_no_init_;
                      }
                      else{
                        (*it)->module_set_container = (*it)->parent->module_set_container;
                      }
                      push_phase_state(*it);
                    }
                  }else if(!phases.empty()){
                    if(consistent){
                      phases[0]->module_set_container = Simulator<phase_state_t>::msc_no_init_;
                    }else{
                      phases[0]->module_set_container = phases[0]->parent->module_set_container;
                    }
                    push_phase_state(phases[0]);
                  }
                }
                if(exit)
                {
                  std::cout << get_state_output(*phases[0],0,1) << std::endl;
                  //std::cout << get_state_output(state_stack_[0],0,1) << std::endl;
                  break;
                }
              }catch(const std::runtime_error &se){
                std::cout << se.what() << std::endl;
                HYDLA_LOGGER_REST(se.what());
              }
            }
            /*if(Simulator<phase_state_t>::opts_->output_format == fmtMathematica){
              Simulator<phase_state_t>::output_result_tree_mathematica();
              }
              else{
              Simulator<phase_state_t>::output_result_tree();
              }
              */
          }

          virtual void initialize(const parse_tree_sptr& parse_tree){
            Simulator<phase_state_t>::initialize(parse_tree);
            //Simulator::initialize(parse_tree);
            Simulator<phase_state_t>::state_id_ = 1;
            //初期状態を作ってスタックに入れる
            phase_state_sptr state(Simulator<phase_state_t>::create_new_phase_state());
            state->phase        = simulator::PointPhase;
            state->step         = 0;
            state->current_time = value_t("0");
            state->module_set_container = Simulator<phase_state_t>::msc_original_;
            state->parent = Simulator<phase_state_t>::result_root_;
            push_phase_state(state);
          }


          /**
           * 状態キューに新たな状態を追加する
           */
          virtual void push_phase_state(const phase_state_sptr& state)
          {
            state->id = Simulator<phase_state_t>::state_id_++;
            HYDLA_LOGGER_PHASE("%% InteractiveSimulator::push_phase_state\n");
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
          phase_state_sptr pop_phase_state()
          {
            phase_state_sptr state(state_stack_.top());
            state_stack_.pop();
            return state;
          }

          int interactivesimulate(phase_state_sptr& state){
            all_state_.push_back(state); 
            //std::cout << all_state_.size() << std::endl;
            /* for(int i=0; i < all_state_.size() ; i++){
               std::cout << get_state_output(*all_state_[i]->parent,0,1) << std::endl;
               }*/
            //  change_variable_flag = false;
            Simulator<phase_state_t>::opts_->max_time = "100";
            /*
               if(0){
               if(1){
               char *name = "save_data.dat";
               FILE *fp;
               phase_state_sptr tmp;
               if( ( fp = fopen( name, "rb" ) ) == NULL ) {
               printf("ファイルオープンエラー\n");
               }else{
               while(fread( &*tmp, sizeof(368), 1, fp )){
               std::cout << tmp->phase << std::endl;
               all_state_.push_back(tmp);
               }
               }  
               fclose( fp );
               for (int i=all_state_.size();i<all_state_.size();i++){
            //std::cout << get_state_output(*all_state_[i], false);
            }
            std::cout << "" << std::endl;
            all_state_.clear();
            }
            }
            if(all_state_.size() > 1){
            for (int i=all_state_.size()-2;i<all_state_.size();i++){
            //std::cout << get_state_output(*all_state_[i], false,true);
            }
            }else{
            for (int i=0;i<all_state_.size();i++){
            //std::cout << get_state_output(*all_state_[i], false,true);
            }
            }
            */
            std::cout << "[debug] in mode test" <<std::endl;
            std::string line;
            getline(cin,line);

            switch(line[0]){
              case 'j':
                std::cout << "j" << std::endl;
                /*
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
                cout << "in mode jump test input step number" <<endl;
                cin >> target_step;
                int i;
                for(i=0;i<all_state_.size();i++)
                {
                if(all_state_[i]->step+1==target_step && all_state_[i]->phase==PointPhase)
                {
                cout << "jump to step "<<all_state_[i]->step+1 << " time:" << all_state_[i]->current_time << endl;
                cout << "[debug] state size"<< all_state_.size() << "jump number" << i << endl;
                state = all_state_[i];
                int j=0;
                int delete_size = all_state_.size()-i;
                for(j=0;j<delete_size;j++)
                {
                all_state_.pop_back();
                cout << "[debug] delete " << j << endl;
                }
                }
                }
                cin.ignore( 1024, '\n');
                break;
                }else
                {
                cout << "0 step" << endl;
                break;
                }
                // default:
                */
                return 0;
              case 's':{
              //           save_state();
                         /*
                            cout << "save state in file" << endl;
                            char *name = "save_data.dat"; // save_data.dat(セーブデータファイル)
                            FILE *fp;
                            phase_state_sptr tmp;
                            if( ( fp = fopen( name, "wb" ) ) == NULL ) {
                            printf("ファイルオープンエラー\n");
                            }else{
                            for(int i=0;i<all_state_.size();i++){
                            tmp = all_state_[i];
                            fwrite( &*tmp, sizeof(*tmp), 1, fp ) ;
                            }
                            }
                            fclose( fp );
                            */
                         return 0;
                       }
              case 'h':
              //         show_helplist();
                       //              cout << "help" << endl;
                                     return 0;
              case 'w':
                                     //change_variables();
                       //            cout << "change variable" << endl;
                       //change_variable_flag = 1;
                       return 0;
              case 'd':
                       select_options();
                                return 0;
            }


            if(line[0] == 'q'||line[0] == 's'){
              cout << "exit" << endl;
              return 1;
            }

            return 0;

          }

          int select_options(){
            ProgramOptions &po = ProgramOptions::instance();
            int argc = 0;
            char input[100], *argv[20], *cp;
            const char *delim = " \t\n"; 
            argv[0] = "./hyrose";
            std::cout << "enter the hyrose option args (only debug options)" << std::endl;

            if (fgets(input, sizeof(input), stdin) != NULL) {
              cp = input;
              for (argc = 1; argc < 20; argc++) {
                if ((argv[argc] = strtok(cp,delim)) == NULL)
                  break;
                cp = NULL;
              }
            }
            cp = NULL;
            po.parse(argc, argv);
            cp = NULL;
            std::string area_string(po.get<std::string>("debug"));
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
            return 0; 
          }
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
            phase_state_sptr phase_state;
            /// フェーズ内で一時的に追加する制約．分岐処理などに使用
            constraints_t temporary_constraints;
            module_set_container_sptr module_set_container;
            /// 判定済みのモジュール集合を保持しておく．分岐処理時，同じ集合を複数回調べることが無いように
            std::set<module_set_sptr> visited_module_sets;
          };


          /**
           * 各状態を保存しておくためのスタック
           */
          std::stack<phase_state_sptr> state_stack_;


          /**
           * シミュレーション対象となるパースツリー
           */
          parse_tree_sptr parse_tree_;

          bool is_safe_;
          std::vector<phase_state_sptr> all_state_;
      };

  } // simulator
} // hydla

#endif // _INCLUDED_HYDLA_INTERACTIVE_SIMULATOR_H_
