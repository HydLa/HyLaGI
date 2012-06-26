#ifndef _INCLUDED_HYDLA_INTERACTIVE_SIMULATOR_H_
#define _INCLUDED_HYDLA_INTERACTIVE_SIMULATOR_H_

#include "Simulator.h"

namespace hydla {
namespace simulator {

class InteractiveSimulator:public Simulator{



    SymbolicSimulator::Phases SymbolicSimulator::point_phase(const module_set_sptr& ms, 
        phase_state_sptr& state, bool& consistent)
    {
      HYDLA_LOGGER_PHASE("#*** Begin SymbolicSimulator::point_phase***");
      //前準備
      Timer pp_timer;
      expanded_always_t expanded_always;
      expanded_always_id2sptr(state->expanded_always_id, expanded_always);
      solver_->change_mode(DiscreteMode, opts_->approx_precision);

      positive_asks_t positive_asks(state->positive_asks);
      negative_asks_t negative_asks;

      variable_map_t time_applied_map;
      //TODO:毎回時間適用してるけど，前のフェーズが決まってれば同じになるわけだから1フェーズにつき一回でいいはず．
      solver_->apply_time_to_vm(state->parent->variable_map, time_applied_map, state->current_time);
      HYDLA_LOGGER_PHASE("--- time_applied_variable_map ---\n", time_applied_map);
      solver_->reset(time_applied_map, state->parameter_map);

      Timer pp_cc_timer;

      //閉包計算
      CalculateClosureResult result = calculate_closure(state,ms,expanded_always,positive_asks,negative_asks);
      
      if(result.size() != 1){
        consistent = false;
        return result;
      }
      
      
      if(opts_->time_measurement){
      	pp_cc_timer.push_time("PP-CalculateClosure");
      }
      // Interval Phaseへ移行（次状態の生成）
      HYDLA_LOGGER_PHASE("%% SymbolicSimulator::create new phase states\n");  
      SymbolicVirtualConstraintSolver::create_result_t create_result = solver_->create_maps();

      assert(create_result.result_maps.size()>0);


      phase_state_sptr new_state_original(create_new_phase_state());
      new_state_original->step         = state->step;
      new_state_original->phase        = IntervalPhase;
      new_state_original->current_time = state->current_time;
      expanded_always_sptr2id(expanded_always, new_state_original->expanded_always_id);

      Phases phases;
      

      for(unsigned int create_it = 0; create_it < create_result.result_maps.size(); create_it++)
      {
        phase_state_sptr new_state(create_new_phase_state(new_state_original)), branch_state(create_new_phase_state(state));
        
        branch_state->variable_map = range_map_to_value_map(branch_state, create_result.result_maps[create_it], branch_state->parameter_map);
        new_state->parameter_map = branch_state->parameter_map;
        
        branch_state->parent->children.push_back(branch_state);
        new_state->parent = branch_state;

        if(is_safe_){
          phases.push_back(new_state);
        }else{
          branch_state->cause_of_termination = simulator::ASSERTION;
        }
        
        
        /*
           TellCollector   tell_collector(ms);
           tells_t         tell_list;
           tell_collector.collect_new_tells(&tell_list,
           &expanded_always, 
           &positive_asks);
           std::vector<std::string> v_print_pp = tell_collector.get_print_pp();
          
        std::vector<std::string> v_scan; = tell_collector.get_scan();

        // プリント出力
        if(v_print_pp.size()!=0)
        {
        print_pp(v_print_pp, new_state->variable_map, state->step, ms->get_name(), state->current_time, "1");
        //      print_pp(v_print_pp, new_state->variable_map, state->step, ms->get_name(), state->current_time, state->_case);
        std::cout << std::endl;
        }
        */
       
        //Scan入力
        
/*
        if(v_scan.size()!=0)
        {
        std::cout << "test scan" << std::endl;
        variable_map_t vm = branch_state->variable_map;
        variable_map_t::iterator vm_itr;
        variable_map_t::iterator vm_itr_copy = vm_itr;
        std::vector<std::string>::iterator v_it = v_scan.begin();
        for(;v_it != v_scan.end(); v_it++)
        {
        std::cout << *v_it << std::endl;
        std::string s;
        std::cin >> s;
        //value_t n(s);
        value_t n = s;
        int derivative_count = 0;
        std::string name = *v_it;
        std::string::iterator n_it;
        std::string sb("'");
        std::string sa("");
        std::string::size_type na, nb = 0;
        //'の削除
        while ((na = name.find(sb,nb)) != std::string::npos)
        {
        name.replace(na, sb.size(), sa);
        nb = na + sa.size();
        derivative_count++;
        }
        const int& dc = derivative_count;
        const std::string& _name = name;
        //variable_t* m(name, derivative_count);
        //DefaultVariable* m(name, derivative_count);
        hydla::simulator::DefaultVariable* m = get_variable(_name, dc);
        vm.set_variable(m, n);
        }
        new_state->variable_map = vm;
        std::cout << "end scan" << std::endl;
        }
      if(change_variable_flag&&opts_->interactive_mode){
        std::cout << "select variable" << std::endl;
        std::cout << get_state_output(*branch_state, false, true);
        variable_map_t vm = branch_state->variable_map;
        std::string name;// = "ht";
        std::cin >> name;
        std::cout << "input value" << std::endl;
        std::string s; //number
        std::cin >> s;
        value_t n = s;
        int derivative_count = 0;
        const int& dc = derivative_count;
        const std::string& _name = name;
        hydla::simulator::DefaultVariable* m = get_variable(_name, dc);
vm.set_variable(m,n); 

        branch_state->variable_map = vm;
        change_variable_flag = false;
        cin.ignore( 1024 , '\n');
      }
      */
      }

      if(opts_->time_measurement){
	      pp_timer.push_time("PointPhase");
      }
      HYDLA_LOGGER_PHASE("#*** End SymbolicSimulator::point_phase***\n");
      consistent = true;
      return phases;
    }
    
    
    
    SymbolicSimulator::Phases SymbolicSimulator::interval_phase(const module_set_sptr& ms, 
        phase_state_sptr& state, bool& consistent)
    {
      HYDLA_LOGGER_PHASE("#*** Begin SymbolicSimulator::interval_phase***");
      //前準備
      Timer ip_timer;
      expanded_always_t expanded_always;
      expanded_always_id2sptr(state->expanded_always_id, expanded_always);

      solver_->change_mode(ContinuousMode, opts_->approx_precision);
      negative_asks_t negative_asks;
      positive_asks_t positive_asks(state->positive_asks);
      solver_->reset(state->parent->variable_map, state->parameter_map);

      Timer ip_cc_timer;

      //閉包計算
      CalculateClosureResult result = calculate_closure(state,ms,expanded_always,positive_asks,negative_asks);

      if(result.size() != 1){
        consistent = false;
        return result;
      }

      if(opts_->time_measurement){
	      ip_cc_timer.push_time("IP-CalculateClosure");
      }
      
      /*
      TellCollector tell_collector(ms);
      tells_t         tell_list;
      tell_collector.collect_new_tells(&tell_list,
          &expanded_always, 
          &positive_asks);

      std::vector<std::string> v_print_ip = tell_collector.get_print_ip();
      int print_ip_flag = 0;
      if(v_print_ip.size() !=0)  print_ip_flag =1;
      */


      SymbolicVirtualConstraintSolver::create_result_t create_result = solver_->create_maps();
      SymbolicVirtualConstraintSolver::create_result_t::result_maps_t& results = create_result.result_maps;

      if(results.size() > 1){
        // 区分的に連続で無い解軌道を含む．中断．
        phase_state_sptr phase(create_new_phase_state(state));
        phase->cause_of_termination = simulator::NOT_UNIQUE_IN_INTERVAL;
        phase->parent->children.push_back(phase);
        consistent = true;
        return Phases();
      }

      phase_state_sptr new_state_original(create_new_phase_state());

      new_state_original->step         = state->step+1;
      new_state_original->phase        = PointPhase;
      
      // TODO: expanded_alwaysが大丈夫かどうか考える．変換が必要なのはなぜ？
      // expanded_always_sptr2id(expanded_always, new_state_original->expanded_always_id);
      state->variable_map = range_map_to_value_map(state, results[0], state->parameter_map);
      state->variable_map = shift_variable_map_time(state->variable_map, state->current_time);
      
      Phases phases;

      if(is_safe_){

        /*
        // MaxModuleの導出
        module_set_sptr max_module_set = (*msc_no_init_).get_max_module_set();
        HYDLA_LOGGER_DEBUG("#** interval_phase: ms: **\n",
         *ms,
         "\n#** interval_phase: max_module_set: ##\n",
         *max_module_set);


        // 採用していないモジュールのリスト導出
        hydla::ch::ModuleSet::module_list_t diff_module_list(max_module_set->size() - ms->size());

        std::set_difference(
        max_module_set->begin(),
        max_module_set->end(),
        ms->begin(),
        ms->end(),
        diff_module_list.begin());


        // それぞれのモジュールをsingletonなモジュール集合とする
        std::vector<module_set_sptr> diff_module_set_list;

        hydla::ch::ModuleSet::module_list_const_iterator diff_it = diff_module_list.begin();
        hydla::ch::ModuleSet::module_list_const_iterator diff_end = diff_module_list.end();
        for(; diff_it!=diff_end; ++diff_it){
        module_set_sptr diff_ms(new ModuleSet((*diff_it).first, (*diff_it).second));
        diff_module_set_list.push_back(diff_ms);
        }

        assert(diff_module_list.size() == diff_module_set_list.size());


        // diff_module_set_list内の各モジュール集合内にある条件なし制約をそれぞれ得る
        not_adopted_tells_list_t not_adopted_tells_list;

        std::vector<module_set_sptr>::const_iterator diff_ms_list_it = diff_module_set_list.begin();
        std::vector<module_set_sptr>::const_iterator diff_ms_list_end = diff_module_set_list.end();
        for(; diff_ms_list_it!=diff_ms_list_end; ++diff_ms_list_it){
        TellCollector not_adopted_tells_collector(*diff_ms_list_it);
        tells_t       not_adopted_tells;
        not_adopted_tells_collector.collect_all_tells(&not_adopted_tells,
        &expanded_always, 
        &positive_asks);
        not_adopted_tells_list.push_back(not_adopted_tells);
        }


        //現在採用されていない制約を離散変化条件として追加
        for(not_adopted_tells_list_t::const_iterator it = not_adopted_tells_list.begin(); it != not_adopted_tells_list.end(); it++){
        tells_t::const_iterator na_it = it -> begin();
        tells_t::const_iterator na_end = it -> end();
        for(; na_it != na_end; na_it++){
        disc_cause.push_back((*na_it)->get_child());
        }
        }
        */

        constraints_t disc_cause;
        //現在導出されているガード条件にNotをつけたものを離散変化条件として追加
        for(positive_asks_t::const_iterator it = positive_asks.begin(); it != positive_asks.end(); it++){
          disc_cause.push_back(node_sptr(new Not((*it)->get_guard() ) ) );
        }
        //現在導出されていないガード条件を離散変化条件として追加
        for(negative_asks_t::const_iterator it = negative_asks.begin(); it != negative_asks.end(); it++){
          disc_cause.push_back((*it)->get_guard());
        }

        //assertionの否定を追加
        if(opts_->assertion){
          disc_cause.push_back(node_sptr(new Not(opts_->assertion)));
        }

        SymbolicVirtualConstraintSolver::PPTimeResult time_result = solver_->calculate_next_PP_time(disc_cause,
            state->current_time,time_t(node_sptr(new hydla::parse_tree::Number(opts_->max_time))));

        for(unsigned int time_it=0; time_it<time_result.candidates.size(); time_it++){
          phase_state_sptr branch_state(create_new_phase_state(state));
          branch_state->parameter_map = time_result.candidates[time_it].parameter_map;
          branch_state->parent->children.push_back(branch_state);
          branch_state->end_time = time_result.candidates[time_it].time;
          phase_state_sptr new_state(create_new_phase_state(new_state_original));
          
          if(!time_result.candidates[time_it].is_max_time ) {
            new_state->current_time = time_result.candidates[time_it].time;
            solver_->simplify(new_state->current_time);
            new_state->parameter_map = branch_state->parameter_map;
            new_state->parent = branch_state;
            phases.push_back(new_state);
          }else{
            branch_state->cause_of_termination = simulator::TIME_LIMIT;
          }
        }
      }else{
        state->parent->children.push_back(state);
        state->cause_of_termination = simulator::ASSERTION;
      }

      /*
         if(print_ip_flag!=0)
         {
         variable_map_t print_variable = shift_variable_map_time(time_result.states[it].variable_map,state->current_time);
         print_ip(v_print_ip, print_variable, state->step, ms->get_name(), state->current_time, time_result.states[it].time, "1");
      //print_ip(v_print_ip, print_variable, state->step, ms->get_name(), state->current_time, time_result.states[it].time, state->_case);
      std::cout <<  std::endl;
      }
      }
      */
      if(opts_->time_measurement){
	      ip_timer.push_time("IntervalPhase");
      }
      HYDLA_LOGGER_PHASE("#*** End SymbolicSimulator::interval_phase***");
      consistent = true;
      return phases;
  }

  SymbolicSimulator::variable_map_t SymbolicSimulator::range_map_to_value_map(
      const phase_state_sptr& state,
      const hydla::vcs::SymbolicVirtualConstraintSolver::variable_range_map_t& rm,
      parameter_map_t &parameter_map){
    variable_map_t ret = *variable_map_;
    for(vcs::SymbolicVirtualConstraintSolver::variable_range_map_t::const_iterator r_it = rm.begin(); r_it != rm.end(); r_it++){
      variable_t* variable = get_variable(r_it->first->get_name(), r_it->first->get_derivative_count());
      if(r_it->second.is_unique()){
        ret.set_variable(variable, r_it->second.get_lower_bound().value);
      }else{
        parameter_t param(r_it->first, state);
        parameter_set_->push_front(param);
        parameter_map.set_variable(&(parameter_set_->front()), r_it->second);
        ret.set_variable(variable, value_t(node_sptr(new Parameter(variable->get_name(), variable->get_derivative_count(), state->id))));
        // TODO:記号定数導入後，各変数の数式に出現する変数を記号定数に置き換える
      }
    }
    return ret;
  }
  
  

/*

  //print_pp書式文字列に対応する値を挿入
  void SymbolicSimulator::print_pp(std::vector<std::string> v, variable_map_t v_map, int step, std::string ms_set_name, time_t time, std::string _case)
  {
    //variable_map_t v_map = state->variable_map;
    variable_map_t::iterator va_itr;
    std::map<std::string, std::string> s_map;
    std::ostringstream f;
    std::ostringstream s;
    //variable_mapをstringに変換しs_mapにいれる
    for (va_itr = v_map.begin(); va_itr != v_map.end(); va_itr++)
    {
      f << va_itr->first;
      s << va_itr->second;
      s_map.insert(pair<std::string, std::string>(f.str(), s.str()));
      f.str("");
      s.str("");

    }
    s_map.insert(pair<std::string, std::string>("MODULE", ms_set_name));
    s << step+1;
    s_map.insert(pair<std::string, std::string>("STEP", s.str()));
    s.str("");
    s << time;
    s_map.insert(pair<std::string, std::string>("TIME", s.str()));
    s.str("");
    s<< "PP";
    s_map.insert(pair<std::string, std::string>("PHASE", s.str()));
    s.str("");
    s << _case;
    s_map.insert(pair<std::string, std::string>("CASE", s.str()));

    //書式文字列の取り出し
    std::string output_str(*v.begin());
    std::vector<std::string>::iterator v_it = v.begin();
    ++v_it;
    //vの引数を変換して、代入
    while(v_it != v.end())
    { //++v_it;
      std::string sb("%d");
      std::string sa( s_map.find(*v_it)->second );
      std::string::size_type n, nb = 0;
      if ((n = output_str.find(sb,nb)) != std::string::npos)
      {
        output_str.replace(n, sb.size(), sa);
        nb = n + sa.size();
      }
      ++v_it;
    }
  }


  void SymbolicSimulator::print_ip(std::vector<std::string> v, variable_map_t v_map, int step, std::string ms_set_name, time_t start_time, time_t end_time, std::string _case)
  {
    variable_map_t::iterator va_itr;
    std::map<std::string, std::string> s_map;
    std::ostringstream f;
    std::ostringstream s;
    for (va_itr = v_map.begin(); va_itr != v_map.end(); va_itr++)
    {
      f << va_itr->first;
      s << va_itr->second;
      s_map.insert(pair<std::string, std::string>(f.str(), s.str()));
      f.str("");
      s.str("");

    }
    s_map.insert(pair<std::string, std::string>("MODULE", ms_set_name));
    s << step;
    s_map.insert(pair<std::string, std::string>("STEP", s.str()));
    s.str("");
    s << start_time << "->" << end_time;
    s_map.insert(pair<std::string, std::string>("TIME", s.str()));
    s_map.insert(pair<std::string, std::string>("PHASE", "IP"));
    s.str("");
    s << _case;
    s_map.insert(pair<std::string, std::string>("CASE", s.str()));

    //書式文字列の取り出し
    std::string output_str(*v.begin());
    std::vector<std::string>::iterator v_it = v.begin();
    ++v_it;
    //vの引数を変換して、代入
    while(v_it != v.end())
    { //++v_it;
      std::string sb("%d");
      std::string sa( s_map.find(*v_it)->second );
      std::string::size_type n, nb = 0;
      if ((n = output_str.find(sb,nb)) != std::string::npos)
      {
        output_str.replace(n, sb.size(), sa);
        nb = n + sa.size();
      }
      ++v_it;
    }
    std::cout << output_str <<std::endl;
  }
*/


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