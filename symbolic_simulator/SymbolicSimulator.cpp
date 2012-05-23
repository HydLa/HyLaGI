#include "SymbolicSimulator.h"

#include <iostream>
#include <fstream>
#include <stack>
#include <boost/xpressive/xpressive.hpp>
//#include <boost/thread.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include "Logger.h"

//仮追加
#include "RTreeVisitor.h"
#include "TellCollector.h"
#include "AskCollector.h"

#include "ModuleSetList.h"
#include "ModuleSetGraph.h"
#include "ModuleSetContainerCreator.h"
#include "InitNodeRemover.h"
#include "AskDisjunctionSplitter.h"
#include "AskDisjunctionFormatter.h"
#include "DiscreteAskRemover.h"
#include "AskTypeAnalyzer.h"
#include "../virtual_constraint_solver/mathematica/MathematicaVCS.h"
//#include "../virtual_constraint_solver/reduce/REDUCEVCS.h"
#include "ContinuityMapMaker.h"

#include "SimulateError.h"

using namespace hydla::vcs;
using namespace hydla::vcs::mathematica;

//using namespace hydla::vcs::reduce;

using namespace std;
using namespace boost;
using namespace boost::xpressive;

using namespace hydla::ch;
using namespace hydla::symbolic_simulator;
using namespace hydla::parse_tree;
using namespace hydla::logger;

using hydla::simulator::TellCollector;
using hydla::simulator::AskCollector;
using hydla::simulator::ContinuityMapMaker;
using hydla::simulator::IntervalPhase;
using hydla::simulator::PointPhase;

namespace hydla {
  namespace symbolic_simulator {

    SymbolicSimulator::SymbolicSimulator(const Opts& opts) :
      opts_(opts), is_safe_(true)
    {
    }

    SymbolicSimulator::~SymbolicSimulator()
    {
    }

    void SymbolicSimulator::do_initialize(const parse_tree_sptr& parse_tree)
    {
      init_module_set_container(parse_tree);

      opts_.assertion = parse_tree->get_assertion_node();
      result_root_.reset(new phase_state_t());
      state_id_ = 1;
      //初期状態を作ってスタックに入れる
      phase_state_sptr state(create_new_phase_state());
      state->phase        = simulator::PointPhase;
      state->step         = 0;
      state->current_time = value_t("0");
      state->module_set_container = msc_original_;
      state->parent = result_root_;
      push_phase_state(state);
      variable_derivative_map_ = parse_tree->get_variable_map();


      //出力変数無指定な場合の出力制御（全部出力）
      if(opts_.output_variables.empty()){
        BOOST_FOREACH(const variable_set_t::value_type& i, variable_set_) {
          opts_.output_variables.insert(i.get_string());
        }
      }


      // 使用するソルバの決定
      /*  if(opts_.solver == "r" || opts_.solver == "Reduce") {
          solver_.reset(new REDUCEVCS(opts_, variable_map_));
          }else{*/
      solver_.reset(new MathematicaVCS(opts_));
      //}
      solver_->set_variable_set(variable_set_);
      solver_->set_parameter_set(parameter_set_);
    }

    namespace {
      struct ModuleSetContainerInitializer {
        typedef boost::shared_ptr<ParseTree> parse_tree_sptr;
        template<typename MSCC>
          static void init(
              const parse_tree_sptr& parse_tree,
              module_set_container_sptr& msc_original, 
              module_set_container_sptr& msc_no_init,
              parse_tree_sptr& member_parse_tree)
          {
            ModuleSetContainerCreator<MSCC> mcc;
            {
              parse_tree_sptr pt_original(boost::make_shared<ParseTree>(*parse_tree));
              simulator::AskDisjunctionFormatter().format(pt_original.get());
              simulator::AskDisjunctionSplitter().split(pt_original.get());
              msc_original = mcc.create(pt_original);
            }

            {
              parse_tree_sptr pt_no_init(boost::make_shared<ParseTree>(*parse_tree));
              simulator::InitNodeRemover().apply(pt_no_init.get());
              simulator::AskDisjunctionFormatter().format(pt_no_init.get());
              simulator::AskDisjunctionSplitter().split(pt_no_init.get());
              msc_no_init = mcc.create(pt_no_init);

              // 最適化された形のパースツリーを得る
              member_parse_tree = pt_no_init;
            }
          }
      };

    }

    void SymbolicSimulator::init_module_set_container(const parse_tree_sptr& parse_tree)
    {    
      if(opts_.nd_mode||opts_.interactive_mode) {
        //全解探索モードなど
        ModuleSetContainerInitializer::init<ModuleSetGraph>(
            parse_tree, msc_original_, msc_no_init_, parse_tree_);
      }
      else {
        //通常実行モード
        ModuleSetContainerInitializer::init<ModuleSetList>(
            parse_tree, msc_original_, msc_no_init_, parse_tree_);
      }
    }

    void SymbolicSimulator::simulate()
    {
      while(!state_stack_.empty() && (is_safe_ || opts_.exclude_error)) {
        phase_state_sptr state(pop_phase_state());
        try{
          bool has_next = false;
          if( opts_.max_step >= 0 && state->step > opts_.max_step)
            continue;
          if(opts_.interactive_mode && state->phase == PointPhase)
          { 
            change_variable_flag = false;
            opts_.max_time = "100";
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
                /*for (int i=all_state.size();i<all_state.size();i++){
                  std::cout << get_state_output(*all_state[i], false);
                }*/
                std::cout << "unko" << std::endl;
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
            int key;
            //key = getchar();
            //if (std::cin.eof()) {
            //      std::cin.clear();
            //            std::cin.seekg(0, std::ios::end);
            //                }
            //rewind(stdin);
            while (getchar() != '\n'){ }
            key = getchar();
            //std::cout << key << std::endl;
            switch(key){
              case 'j':
                int target_step;
                if(all_state.size()!=0)
                {
                  //debug{{{
                  for(int i=0;i<all_state.size();i++)
                  { 
                    if(all_state[i]->phase==PointPhase)
                    {
                      std::cout <<  "[debug] step" << all_state[i]->step+1 << " pp t:" << all_state[i]-> current_time<< std::endl;
                    }else if(all_state[i]->phase==IntervalPhase)
                    {
                      std::cout <<  "[debug] step" << all_state[i]->step+1 << " ip t:" << all_state[i]-> current_time<< "->" <<std::endl;
                    }
                  }
                  //debug}}}                          
                  std::cout << "in mode jump test input step number" <<std::endl;
                  std::cin >> target_step;
                  int i;
                  for(i=0;i<all_state.size();i++)
                  {
                    //std::cout << all_state[i]->step << std::endl;
                    if(all_state[i]->step+1==target_step && all_state[i]->phase==PointPhase)
                    {
                      std::cout << "jump to step "<<all_state[i]->step+1 << " time:" << all_state[i]->current_time << std::endl;
                      std::cout << "[debug] state size"<< all_state.size() << "jump number" << i << std::endl;
                      state = all_state[i];
                      //for(int j=0;j<((all_state.size()/2)-target_step)*2;j++)
                      int j=0;
                      int delete_size = all_state.size()-i;
                      for(j=0;j<delete_size;j++)
                      {
                        all_state.pop_back();
                        std::cout << "[debug] delete " << j << std::endl;
                      }
                      break;
                    }
                  }
                }else
                {
                  std::cout << "0 step" << std::endl;
                  break;
                }
                // default:
              case 's':{
                std::cout << "save state in file" << std::endl;
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
                std::cout << "help" << std::endl;
                break;
              case 'w':
                std::cout << "change variable" << std::endl;
                change_variable_flag = 1;
                break;
            }

            if(key == 'q'||key == 's'){
              std::cout << "exit" << std::endl;
              continue;
            }
            
          }

          state->module_set_container->reset(state->visited_module_sets);
          while(state->module_set_container->go_next() && (is_safe_ || opts_.exclude_error)){
            is_safe_ = true;
            if(simulate_phase_state(state->module_set_container->get_module_set(), state)){
              state->module_set_container->mark_nodes();
              has_next = true;
              if(!opts_.nd_mode)break;
              if(opts_.interactive_mode)
              {
                std::cout << "in mode test push_back" << state->current_time <<std::endl;
                all_state.push_back(state);
              }
            }
            else{
              state->module_set_container->mark_current_node();
            }
            state->positive_asks.clear();
          }
          //無矛盾な解候補モジュール集合が存在しない場合
          /*if(!has_next){
            state->parent->cause_of_termination = PhaseState::INCONSISTENCY;
            }*/
        }catch(const std::runtime_error &se){
          std::cout << se.what() << std::endl;
          HYDLA_LOGGER_REST(se.what());
        }

        //無矛盾な解候補モジュール集合が存在しない場合
        if(state->children.empty()){
          state->parent->cause_of_termination = simulator::INCONSISTENCY;
        }
      }
      if(!opts_.interactive_mode) 
        output_result_tree();
    }

    void SymbolicSimulator::add_continuity(const continuity_map_t& continuity_map){
      HYDLA_LOGGER_CLOSURE("#*** Begin SymbolicSimulator::add_continuity ***\n");
      for(continuity_map_t::const_iterator it = continuity_map.begin(); it != continuity_map.end();it++){
        if(it->second>=0){
          for(int i=0; i<it->second;i++){
            solver_->set_continuity(it->first, i);
          }
        }else{
          node_sptr lhs(new Variable(it->first));
          for(int i=0; i<=-it->second;i++){
            solver_->set_continuity(it->first, i);
            lhs = node_sptr(new Differential(lhs));
          }
          node_sptr rhs(new Number("0"));
          node_sptr cons(new Equal(lhs, rhs));
          solver_->add_constraint(cons);
        }
      }
      HYDLA_LOGGER_CLOSURE("#*** End SymbolicSimulator::add_continuity ***\n");
    }


    void SymbolicSimulator::push_branch_states(phase_state_sptr &original, SymbolicVirtualConstraintSolver::check_consistency_result_t &result){
      for(int i=1; i<(int)result.true_parameter_maps.size();i++){
        phase_state_sptr branch_state(create_new_phase_state(original));
        branch_state->parameter_map = result.true_parameter_maps[i];
        push_phase_state(branch_state);
      }
      for(int i=0; i<(int)result.false_parameter_maps.size();i++){
        phase_state_sptr branch_state(create_new_phase_state(original));
        branch_state->parameter_map = result.false_parameter_maps[i];
        push_phase_state(branch_state);
      }
    }

    CalculateClosureResult SymbolicSimulator::calculate_closure(phase_state_sptr& state,
        const module_set_sptr& ms, expanded_always_t &expanded_always,
        positive_asks_t &positive_asks, negative_asks_t &negative_asks,
        const variable_map_t& variable_map){    
      HYDLA_LOGGER_CLOSURE("#*** Begin SymbolicSimulator::calculate_closure ***\n");

      //前準備
      TellCollector tell_collector(ms);
      AskCollector  ask_collector(ms, AskCollector::ENABLE_COLLECT_NON_TYPED_ASK | 
          AskCollector::ENABLE_COLLECT_DISCRETE_ASK |
          AskCollector::ENABLE_COLLECT_CONTINUOUS_ASK);
      tells_t         tell_list;
      constraints_t   constraint_list;
      boost::shared_ptr<hydla::parse_tree::Ask>  const *branched_ask;

      continuity_map_t continuity_map;
      ContinuityMapMaker maker;

      bool expanded;
      do{
        // tell制約を集める
        tell_collector.collect_new_tells(&tell_list,
            &expanded_always, 
            &positive_asks);

        HYDLA_LOGGER_CLOSURE("%% SymbolicSimulator::check_consistency in calculate_closure\n");
        //tellじゃなくて制約部分のみ送る
        constraint_list.clear();

        for(tells_t::iterator it = tell_list.begin(); it != tell_list.end(); it++){
          constraint_list.push_back((*it)->get_child());
          if(opts_.default_continuity > CONT_NONE){
            maker.visit_node((*it), state->phase == IntervalPhase, false);
          }
        }
        if(opts_.default_continuity > CONT_NONE){
          continuity_map = maker.get_continuity_map();
        }
        for(constraints_t::const_iterator it = state->temporary_constraints.begin(); it != state->temporary_constraints.end(); it++){
          constraint_list.push_back(*it);
        }

        // 制約を追加し，制約ストアが矛盾をおこしていないかどうか調べる

        solver_->add_constraint(constraint_list);

        add_continuity(continuity_map);


        {
          SymbolicVirtualConstraintSolver::check_consistency_result_t check_consistency_result = solver_->check_consistency();
          if(check_consistency_result.true_parameter_maps.empty()){
            // 必ず矛盾する場合
            return CC_FALSE;
          }else if (check_consistency_result.false_parameter_maps.empty()){
            // 必ず充足可能な場合
            // 何もしない
          }else{
            // 記号定数の条件によって充足可能性が変化する場合
            HYDLA_LOGGER_CLOSURE("%% consistency depends on conditions of parameters\n");
            // 自分以外の場合は別に作ってスタックに入れる
            push_branch_states(state, check_consistency_result);
            // 自分は導出可能な場合のうちの1つとする
            state->parameter_map = check_consistency_result.true_parameter_maps[0];
          }
        }

        // ask制約を集める
        ask_collector.collect_ask(&expanded_always, 
            &positive_asks, 
            &negative_asks);


        // ask制約のエンテール処理
        HYDLA_LOGGER_CLOSURE("%% SymbolicSimulator::check_entailment in calculate_closure\n");

        //デフォルト連続性の処理

        bool strong_continuity = (opts_.default_continuity >= CONT_STRONG ||
            (opts_.default_continuity == CONT_STRONG_IP && state->phase == IntervalPhase));
        if(strong_continuity){
          for(continuity_map_t::const_iterator it  = variable_derivative_map_.begin(); it!=variable_derivative_map_.end(); ++it) {
            continuity_map_t::iterator find_result = continuity_map.find(it->first);
            if( find_result == continuity_map.end()){
              continuity_map.insert(std::make_pair(it->first, -(it->second)));
            }
          }
        }
        if(opts_.default_continuity != CONT_GUARD){
          add_continuity(continuity_map);
        }

        {
          expanded = false;
          branched_ask=NULL;
          negative_asks_t::iterator it  = negative_asks.begin();
          negative_asks_t::iterator end = negative_asks.end();
          while(it!=end) {
            solver_->start_temporary();
            if(opts_.default_continuity == CONT_GUARD){
              maker.visit_node((*it)->get_child(), state->phase == IntervalPhase, true);
              add_continuity(maker.get_continuity_map());
            }
            solver_->add_guard((*it)->get_guard());

            SymbolicVirtualConstraintSolver::check_consistency_result_t check_consistency_result = solver_->check_consistency();
            if(!check_consistency_result.true_parameter_maps.empty()){
              HYDLA_LOGGER_CLOSURE("%% entailable");
              if(!check_consistency_result.false_parameter_maps.empty()){
                HYDLA_LOGGER_CLOSURE("%% entailablity depends on conditions of parameters\n");
                // 自分以外の場合は別に作ってスタックに入れる
                push_branch_states(state, check_consistency_result);
                // 自分は導出可能な場合のうちの1つとする
                state->parameter_map = check_consistency_result.true_parameter_maps[0];
              }
              solver_->end_temporary();
              solver_->start_temporary();
              add_continuity(maker.get_continuity_map());
              solver_->add_guard(node_sptr(new Not((*it)->get_guard())));
              check_consistency_result = solver_->check_consistency();
              if(!check_consistency_result.true_parameter_maps.empty()){
                HYDLA_LOGGER_CLOSURE("%% entailment branches");
                // ガード条件による分岐が発生する可能性あり
                if(!check_consistency_result.false_parameter_maps.empty()){
                  HYDLA_LOGGER_CLOSURE("%% inevitable entailment depends on conditions of parameters");
                  // 自分以外の場合は別に作ってスタックに入れる
                  push_branch_states(state, check_consistency_result);
                  // 自分は導出可能な場合のうちの1つとする
                  state->parameter_map = check_consistency_result.true_parameter_maps[0];
                }else{
                  branched_ask = &(*it);
                  it++;
                  solver_->end_temporary();
                  continue;
                }
              }
              HYDLA_LOGGER_CLOSURE("--- entailed ask ---\n", *((*it)->get_guard()));
              positive_asks.insert(*it);
              //eraseと後置インクリメントは同時にやらないとイテレータが壊れるので，注意
              negative_asks.erase(it++);
              expanded = true;
            }else{
              it++;
            }

            if(opts_.default_continuity == CONT_GUARD){
              maker.set_continuity_map(continuity_map);
            }
            solver_->end_temporary();
          }
        }
      }while(expanded);

      if(opts_.default_continuity == CONT_GUARD){
        add_continuity(continuity_map);
      }

      if(branched_ask!=NULL){
        HYDLA_LOGGER_CLOSURE("%% branched_ask:", **branched_ask);
        if(opts_.nd_mode){
          // 分岐先を生成（導出されない方）
          phase_state_sptr new_state(create_new_phase_state(state));
          new_state->temporary_constraints.push_back(node_sptr(new Not((*branched_ask)->get_guard())));
          push_phase_state(new_state);
        }
        // 分岐先を生成（導出される方）
        phase_state_sptr new_state(create_new_phase_state(state));
        new_state->temporary_constraints.push_back((*branched_ask)->get_guard());
        push_phase_state(new_state);
        return CC_BRANCH;
      }

      if(opts_.assertion){
        HYDLA_LOGGER_CLOSURE("%% SymbolicSimulator::check_assertion");
        solver_->start_temporary();
        solver_->add_constraint(node_sptr(new Not(opts_.assertion)));
        SymbolicVirtualConstraintSolver::check_consistency_result_t result = solver_->check_consistency();
        solver_->end_temporary();
        if(!result.true_parameter_maps.empty()){
          if(!result.false_parameter_maps.empty()){
            // TODO:assertion失敗なので，本来ならやり直す必要はない．trueがassertion failedで，falseはこのまま続行すべき
            // ガード条件による分岐が発生する可能性あり
            HYDLA_LOGGER_CLOSURE("%% failure of assertion depends on conditions of parameters");
            // 自分以外の場合は別に作ってスタックに入れる
            push_branch_states(state, result);
            // 自分は導出可能な場合のうちの1つとする
            state->parameter_map = result.true_parameter_maps[0];
          }
          std::cout << "Assertion Failed!" << std::endl;
          is_safe_ = false;
          return CC_TRUE;
        }
      }
      HYDLA_LOGGER_CLOSURE("#*** End SymbolicSimulator::calculate_closure ***\n");
      return CC_TRUE;
    }

    bool SymbolicSimulator::point_phase(const module_set_sptr& ms, 
        phase_state_sptr& state)
    {
      HYDLA_LOGGER_PHASE("#*** Begin SymbolicSimulator::point_phase***");
      //前準備
      expanded_always_t expanded_always;
      expanded_always_id2sptr(state->expanded_always_id, expanded_always);
      solver_->change_mode(DiscreteMode, opts_.approx_precision);

      positive_asks_t positive_asks(state->positive_asks);
      negative_asks_t negative_asks;

      variable_map_t time_applied_map;
      solver_->apply_time_to_vm(state->parent->variable_map, time_applied_map, state->current_time);
      HYDLA_LOGGER_PHASE("--- time_applied_variable_map ---\n", time_applied_map);
      solver_->reset(time_applied_map, state->parameter_map);

      //閉包計算
      switch(calculate_closure(state,ms,expanded_always,positive_asks,negative_asks, time_applied_map)){
        case CC_TRUE:
          break;
        case CC_FALSE:
          return false;    
        case CC_BRANCH:
          return true;
      }

      // Interval Phaseへ移行（次状態の生成）
      HYDLA_LOGGER_PHASE("%% SymbolicSimulator::create new phase states\n");  
      SymbolicVirtualConstraintSolver::create_result_t create_result = solver_->create_maps();

      assert(create_result.result_maps.size()>0);


      phase_state_sptr new_state_original(create_new_phase_state());
      new_state_original->step         = state->step;
      new_state_original->phase        = IntervalPhase;
      new_state_original->current_time = state->current_time;
      new_state_original->module_set_container = msc_no_init_;
      expanded_always_sptr2id(expanded_always, new_state_original->expanded_always_id);


      for(unsigned int create_it = 0; create_it < create_result.result_maps.size()&&(opts_.nd_mode||create_it==0); create_it++)
      {
        phase_state_sptr new_state(create_new_phase_state(new_state_original)), branch_state(create_new_phase_state(state));
        
        branch_state->variable_map = range_map_to_value_map(branch_state, create_result.result_maps[create_it], branch_state->parameter_map);
        new_state->parameter_map = branch_state->parameter_map;
        branch_state->parent->children.push_back(branch_state);
        new_state->parent = branch_state;

        if(is_safe_){
          //状態をスタックに押し込む
          all_state.push_back(branch_state);
          push_phase_state(new_state);
        }else{
          branch_state->cause_of_termination = simulator::ASSERTION;
        }

        if(opts_.dump_in_progress)
          std::cout << get_state_output(*branch_state, false,true);

        /*
           TellCollector   tell_collector(ms);
           tells_t         tell_list;
           tell_collector.collect_new_tells(&tell_list,
           &expanded_always, 
           &positive_asks);
           std::vector<std::string> v_print_pp = tell_collector.get_print_pp();
          */
        std::vector<std::string> v_scan;// = tell_collector.get_scan();
/*
        // プリント出力
        if(v_print_pp.size()!=0)
        {
        print_pp(v_print_pp, new_state->variable_map, state->step, ms->get_name(), state->current_time, "1");
        //      print_pp(v_print_pp, new_state->variable_map, state->step, ms->get_name(), state->current_time, state->_case);
        std::cout << std::endl;
        }
        */
       
        //Scan入力
        
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

      if(change_variable_flag&&opts_.interactive_mode){
        std::cout << "select variable" << std::endl;
        std::cout << get_state_output(*branch_state, false, true);
        variable_map_t vm = branch_state->variable_map;
        std::string s; //number
        std::cin >> s;
        value_t n = s;
        int derivative_count = 0;
        std::string name = "ht";
        const int& dc = derivative_count;
        const std::string& _name = name;
        hydla::simulator::DefaultVariable* m = get_variable(_name, dc);
vm.set_variable(m,n); 

        branch_state->variable_map = vm;
        change_variable_flag = false;
      } 
      }




      HYDLA_LOGGER_PHASE("#*** End SymbolicSimulator::point_phase***\n");
      return true;
    }

    bool SymbolicSimulator::interval_phase(const module_set_sptr& ms, 
        phase_state_sptr& state)
    {
      HYDLA_LOGGER_PHASE("#*** Begin SymbolicSimulator::interval_phase***");
      //前準備
      expanded_always_t expanded_always;
      expanded_always_id2sptr(state->expanded_always_id, expanded_always);

      solver_->change_mode(ContinuousMode, opts_.approx_precision);
      negative_asks_t negative_asks;
      positive_asks_t positive_asks(state->positive_asks);
      solver_->reset(state->parent->variable_map, state->parameter_map);


      //閉包計算
      switch(calculate_closure(state, ms, expanded_always, positive_asks ,negative_asks, state->parent->variable_map)){
        case CC_TRUE:
          break;
        case CC_FALSE:
          return false;    
        case CC_BRANCH:
          return true;
      }

      TellCollector tell_collector(ms);
      tells_t         tell_list;
      tell_collector.collect_new_tells(&tell_list,
          &expanded_always, 
          &positive_asks);

      std::vector<std::string> v_print_ip = tell_collector.get_print_ip();
      int print_ip_flag = 0;
      if(v_print_ip.size() !=0)  print_ip_flag =1;


      SymbolicVirtualConstraintSolver::create_result_t create_result = solver_->create_maps();
      SymbolicVirtualConstraintSolver::create_result_t::result_maps_t& results = create_result.result_maps;

      // TODO:変数表が複数ある場合に，findPPTimeをどう扱うべきか．とりあえず変数表は１つしか出てこないものとする．
      // 区分的連続の前提を置くなら，これが正しい？
      assert(results.size()==1);

      phase_state_sptr new_state_original(create_new_phase_state());

      new_state_original->step         = state->step+1;
      new_state_original->phase        = PointPhase;
      new_state_original->module_set_container = msc_no_init_;
      expanded_always_sptr2id(expanded_always, new_state_original->expanded_always_id);
      state->variable_map = range_map_to_value_map(state, results[0], state->parameter_map);
      state->variable_map = shift_variable_map_time(state->variable_map, state->current_time);


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
        if(opts_.assertion){
          disc_cause.push_back(node_sptr(new Not(opts_.assertion)));
        }

        SymbolicVirtualConstraintSolver::PPTimeResult time_result = solver_->calculate_next_PP_time(disc_cause,
            state->current_time,time_t(node_sptr(new hydla::parse_tree::Number(opts_.max_time))));

        for(unsigned int time_it=0; time_it<time_result.candidates.size()&&(opts_.nd_mode||time_it==0); time_it++){
          phase_state_sptr branch_state(create_new_phase_state(state));
          branch_state->parameter_map = time_result.candidates[time_it].parameter_map;
          branch_state->parent->children.push_back(branch_state);
          if(!time_result.candidates[time_it].is_max_time ) {
            phase_state_sptr new_state(create_new_phase_state(new_state_original));
            new_state->current_time = time_result.candidates[time_it].time;
            branch_state->end_time = time_result.candidates[time_it].time;
            solver_->simplify(new_state->current_time);
            new_state->parameter_map = branch_state->parameter_map;

            //状態をスタックに押し込む
            new_state->parent = branch_state;
            all_state.push_back(branch_state);
            push_phase_state(new_state);
          }else{
            branch_state->cause_of_termination = simulator::TIME_LIMIT;
          }
          if(opts_.dump_in_progress){
            std::cout << get_state_output(*branch_state, false,true);
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

      HYDLA_LOGGER_PHASE("#*** End SymbolicSimulator::interval_phase***");
      return true;
  }

  SymbolicSimulator::variable_map_t SymbolicSimulator::range_map_to_value_map(
      const phase_state_sptr& state,
      const hydla::vcs::SymbolicVirtualConstraintSolver::variable_range_map_t& rm,
      parameter_map_t &parameter_map){
    variable_map_t ret = variable_map_;
    for(vcs::SymbolicVirtualConstraintSolver::variable_range_map_t::const_iterator r_it = rm.begin(); r_it != rm.end(); r_it++){
      variable_t* variable = get_variable(r_it->first->get_name(), r_it->first->get_derivative_count());
      if(r_it->second.is_unique()){
        ret.set_variable(variable, r_it->second.get_lower_bound().value);
      }else{
        parameter_t param(r_it->first, state);
        parameter_set_.push_front(param);
        parameter_map.set_variable(&(parameter_set_.front()), r_it->second);
        ret.set_variable(variable, value_t(node_sptr(new Parameter(variable->get_name(), variable->get_derivative_count(), state->id))));
        // TODO:記号定数導入後，各変数の数式に出現する変数を記号定数に置き換える
      }
    }
    return ret;
  }


  SymbolicSimulator::variable_map_t SymbolicSimulator::shift_variable_map_time(const variable_map_t& vm, const time_t &time){
    variable_map_t shifted_vm;
    variable_map_t::const_iterator it  = vm.begin();
    variable_map_t::const_iterator end = vm.end();
    for(; it!=end; ++it) {
      if(it->second.is_undefined())
        shifted_vm.set_variable(it->first, it->second);
      else
        shifted_vm.set_variable(it->first, solver_->shift_expr_time(it->second, time));
    }
    return shifted_vm;
  }


  void SymbolicSimulator::output_parameter_map(const parameter_map_t& pm)
  {
    parameter_map_t::const_iterator it  = pm.begin();
    parameter_map_t::const_iterator end = pm.end();
    if(it != end){
      std::cout << "\n#---------parameter condition---------\n";
    }
    for(; it!=end; ++it) {
      std::cout << *(it->first) << "\t: " << it->second << "\n";
    }
  }

  void SymbolicSimulator::output_variable_map(std::ostream &stream, const variable_map_t& vm, const time_t& time, const bool& numeric)
  {
    variable_map_t::const_iterator it  = vm.begin();
    variable_map_t::const_iterator end = vm.end();
    if(numeric){
      stream << std::endl;
      stream << solver_->get_real_val(time, opts_.output_precision, opts_.output_format) << "\t";
      for(; it!=end; ++it) {
        if(opts_.output_variables.find(it->first->get_string()) != opts_.output_variables.end())
          stream << solver_->get_real_val(it->second, opts_.output_precision, opts_.output_format) << "\t";
      }
    }else{
      for(; it!=end; ++it) {
        stream << *(it->first) << "\t: " << it->second << "\n";
      }
    }
  }


  void const SymbolicSimulator::output_result_tree()
  {
    if(result_root_->children.size() == 0){
      std::cout << "No Result." << std::endl;
      return;
    }
    phase_state_sptrs_t::iterator it = result_root_->children.begin(), end = result_root_->children.end();
    int i=1, j=1;
    for(;it!=end;it++){
      std::vector<std::string> result;
      output_result_node(*it, result, i, j);
    }
  }


  void SymbolicSimulator::output_result_node(const phase_state_sptr_t &node, std::vector<std::string> &result, int &case_num, int &phase_num){

    if(node->phase==PointPhase){
      std::stringstream sstr;
      sstr << "#---------" << phase_num++ << "---------\n";
      result.push_back(sstr.str());
    }
    result.push_back(get_state_output(*node, false,false));
    if(node->children.size() == 0){
      if(opts_.nd_mode){
        std::cout << "#---------Case " << case_num++ << "---------" << std::endl;
      }
      std::vector<std::string>::const_iterator r_it = result.begin(), r_end = result.end();
      for(;r_it!=r_end;r_it++){
        std::cout << *r_it;
      }

      output_parameter_map(node->parameter_map);
      std::cout << std::endl;
      std::cout << "#";
      switch(node->cause_of_termination){
        case simulator::INCONSISTENCY:
          std::cout << "execution stuck\n";
          break;
        case simulator::TIME_LIMIT:
          std::cout << "time ended\n" ;
          break;

        case simulator::SOME_ERROR:
          std::cout << "some error occured\n" ;
          break;

        case simulator::ASSERTION:
          std::cout << "assertion failed\n" ;
          break;

        default:
        case simulator::NONE:
          std::cout << "unknown termination occured\n" ;
          break;
      }
      std::cout << std::endl;
    }else{
      phase_state_sptrs_t::const_iterator it = node->children.begin(), end = node->children.end();
      for(;it!=end;it++){
        output_result_node(*it, result, case_num, phase_num);
      }
    }

    result.pop_back();

    if(node->phase==PointPhase){
      result.pop_back();
      phase_num--;
    }
  }

  std::string SymbolicSimulator::get_state_output(const phase_state_t& result, const bool& numeric, const bool& is_in_progress){
    std::stringstream sstr;
    if(!numeric){
      if(result.phase==IntervalPhase){
        sstr << "---------IP---------" << std::endl;
        std::string end_time;
        if(is_in_progress){
          sstr << "time\t: " << result.current_time << "->" << result.end_time << "\n";
        }else{
          if(result.children.empty()){
            end_time = opts_.max_time;
          }else{
            end_time = result.children[0]->current_time.get_string();
          }
          sstr << "time\t: " << result.current_time << "->" << end_time << "\n";
        }
      }else{
        if(is_in_progress)
          sstr << "#-------" << result.step +1 << "-------" << std::endl;
        sstr << "---------PP---------" << std::endl;
        sstr << "time\t: " << result.current_time << "\n";
      }
      output_variable_map(sstr, result.variable_map, result.current_time, false);
      sstr << "\n" ;
    }else{
      if(result.phase==IntervalPhase){
        sstr << "#---------IP---------" << std::endl;
        output_variable_labels(sstr, result.variable_map);
        variable_map_t output_vm;
        time_t elapsed_time("0");
        time_t limit_time = result.current_time-result.parent->current_time;
        solver_->simplify(limit_time);
        do{
          solver_->apply_time_to_vm(result.variable_map, output_vm, elapsed_time);
          output_variable_map(sstr, output_vm, (elapsed_time+result.parent->current_time), true);
          elapsed_time += time_t(opts_.output_interval);
          solver_->simplify(elapsed_time);
        }while(solver_->less_than(elapsed_time, limit_time));
        solver_->apply_time_to_vm(result.variable_map, output_vm, limit_time);
        output_variable_map(sstr, output_vm, result.current_time, true);
        sstr << std::endl;
      }else{
        sstr << "#---------PP---------" << std::endl;
        output_variable_labels(sstr, result.variable_map);
        output_variable_map(sstr, result.variable_map, result.current_time, true);
      }
      sstr << std::endl;
    }

    return sstr.str();
  }


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



  void SymbolicSimulator::output_variable_labels(std::ostream &stream, const variable_map_t variable_map){
    // 変数のラベル
    // TODO: 未定義の値とかのせいでずれる可能性あり?
    stream << "# time\t";


    BOOST_FOREACH(const variable_map_t::value_type& i, variable_map) {
      if(opts_.output_variables.find(i.first->get_string()) != opts_.output_variables.end()){
        stream << i.first << "\t";
      }
    }
    stream << std::endl;
  }

} //namespace symbolic_simulator
} //namespace hydla
