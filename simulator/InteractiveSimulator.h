#ifndef _INCLUDED_HYDLA_INTERACTIVE_SIMULATOR_H_
#define _INCLUDED_HYDLA_INTERACTIVE_SIMULATOR_H_

#include "Simulator.h"

namespace hydla {
namespace simulator {

class InteractiveSimulator:public Simulator{

    /*
    void SymbolicSimulator::simulate()
    {
      while(!state_stack_.empty() && (is_safe_ || opts_->exclude_error)) {
        phase_state_sptr state(pop_phase_state());
        try{
          bool has_next = false;
          if( opts_->max_step >= 0 && state->step > opts_->max_step)
            continue;
          if(opts_->interactive_mode && state->phase == PointPhase)
          { 
            change_variable_flag = false;
            opts_->max_time = "100";
            //if(all_state.size()==0){
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
                      all_state.push_back(tmp);
                    }
                  }  
                fclose( fp );
                for (int i=all_state.size();i<all_state.size();i++){
                  std::cout << get_state_output(*all_state[i], false);
                }
                std::cout << "" << std::endl;
                all_state.clear();
              }
            }
            //std::cout <<  "size : " << all_state.size() <<std::endl;
            if(all_state.size() > 1){
              for (int i=all_state.size()-2;i<all_state.size();i++){
                std::cout << get_state_output(*all_state[i], false,true);
              }
            }else{
              for (int i=0;i<all_state.size();i++){
                std::cout << get_state_output(*all_state[i], false,true);
              }
            }
            std::cout << "[debug] in mode test" <<std::endl;
            //int key;
            std::string line;
            //int ignore_flag=0;
            //while (getchar() != '\n'){ }
            //key = getchar();
            //cin.ignore(1024, '\n');
            getline(cin,line);
            //switch(key){
            switch(line[0]){
              case 'j':
                int target_step;
                if(all_state.size()!=0)
                {
                  //debug{{{
                  for(int i=0;i<all_state.size();i++)
                  { 
                    if(all_state[i]->phase==PointPhase)
                    {
                      cout <<  "[debug] step" << all_state[i]->step+1 << " pp t:" << all_state[i]-> current_time<< endl;
                    }else if(all_state[i]->phase==IntervalPhase)
                    {
                      cout <<  "[debug] step" << all_state[i]->step+1 << " ip t:" << all_state[i]-> current_time<< "->" <<endl;
                    }
                  }
                  //debug}}}                          
                  cout << "in mode jump test input step number" <<endl;
                  cin >> target_step;
                  int i;
                  for(i=0;i<all_state.size();i++)
                  {
                    //std::cout << all_state[i]->step << std::endl;
                    if(all_state[i]->step+1==target_step && all_state[i]->phase==PointPhase)
                    {
                      cout << "jump to step "<<all_state[i]->step+1 << " time:" << all_state[i]->current_time << endl;
                      cout << "[debug] state size"<< all_state.size() << "jump number" << i << endl;
                      state = all_state[i];
                      //for(int j=0;j<((all_state.size()/2)-target_step)*2;j++)
                      int j=0;
                      int delete_size = all_state.size()-i;
                      for(j=0;j<delete_size;j++)
                      {
                        all_state.pop_back();
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
              case 's':{
                cout << "save state in file" << endl;
                char *name = "save_data.dat"; // save_data.dat(セーブデータファイル)
                FILE *fp;
                phase_state_sptr tmp;
                if( ( fp = fopen( name, "wb" ) ) == NULL ) {
                  printf("ファイルオープンエラー\n");
                }else{
                  for(int i=0;i<all_state.size();i++){
                    tmp = all_state[i];
                    fwrite( &*tmp, sizeof(*tmp), 1, fp ) ;
                  }
                }
                fclose( fp );
                
                  break;
                       }
              case 'h':
                cout << "help" << endl;
                break;
              case 'w':
                cout << "change variable" << endl;
                change_variable_flag = 1;
                break;
            }

            //if(key == 'q'||key == 's'){
            if(line[0] == 'q'||line[0] == 's'){
              cout << "exit" << endl;
              continue;
            }
            
          }

          state->module_set_container->reset(state->visited_module_sets);
          while(state->module_set_container->go_next() && (is_safe_ || opts_->exclude_error)){
            is_safe_ = true;
            if(simulate_phase_state(state->module_set_container->get_module_set(), state)){
              state->module_set_container->mark_nodes();
              has_next = true;
              if(!opts_->nd_mode)break;
              if(opts_->interactive_mode)
              {
                cout << "in mode test push_back" << state->current_time <<endl;
                all_state.push_back(state);
              }
            }
            else{
              state->module_set_container->mark_current_node();
            }
            state->positive_asks.clear();
          }
        }catch(const std::runtime_error &se){
          std::cout << se.what() << std::endl;
          HYDLA_LOGGER_REST(se.what());
        }

        //無矛盾な解候補モジュール集合が存在しない場合
        if(state->children.empty()){
          state->parent->cause_of_termination = simulator::INCONSISTENCY;
        }
      }
      if(!opts_->interactive_mode){
        if(opts_->output_format == fmtMathematica){
          output_result_tree_mathematica();
        }
        else{
          output_result_tree();
        }
      }

    }
    */

};

} // simulator
} // hydla

#endif