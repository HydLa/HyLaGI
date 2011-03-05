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

using namespace hydla::vcs;
using namespace hydla::vcs::mathematica;

using namespace std;
using namespace boost;
using namespace boost::xpressive;

using namespace hydla::ch;
using namespace hydla::simulator;
using namespace hydla::parse_tree;
using namespace hydla::logger;

namespace hydla {
namespace symbolic_simulator {

//struct rawchar_formatter
//{
//  string operator()(smatch const &what) const
//  {
//    char c[2] = {0, 0};
//    c[0] = (char)strtol(what.str(1).c_str(), NULL, 8);
//    return c;
//  }
//};
//sregex rawchar_reg = sregex::compile("\\\\(\\d\\d\\d)");
//std::ostream_iterator< char > out_iter( std::cout );
//rawchar_formatter rfmt;
//regex_replace( out_iter, str.begin(), str.end(), rawchar_reg, rfmt);	

SymbolicSimulator::SymbolicSimulator(const Opts& opts) :
  opts_(opts)
{
}

SymbolicSimulator::~SymbolicSimulator()
{
}

void SymbolicSimulator::do_initialize(const parse_tree_sptr& parse_tree)
{
  init_module_set_container(parse_tree);

  //初期状態を作ってスタックに入れる
  phase_state_sptr state(create_new_phase_state());
  state->phase        = PointPhase;
  state->current_time = time_t();
  state->variable_map = variable_map_;
  state->module_set_container = msc_original_;
  push_phase_state(state);

  //reduce出力用分岐
  if(opts_.solver == "r" || opts_.solver == "Reduce") {
    reduce_simulate();
//    exit(0);
  }



  // 変数のラベル
  // TODO: 未定義の値とかのせいでずれる可能性あり?
  if(opts_.output_format == fmtNumeric){
    std::cout << "# time\t";
    BOOST_FOREACH(variable_map_t::value_type& i, variable_map_) {
     std::cout << i.first << "\t";
    }
    std::cout << std::endl;
  }
  solver_.reset(new MathematicaVCS(opts_));   //使用するソルバを決定
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
      AskDisjunctionFormatter().format(pt_original.get());
      AskDisjunctionSplitter().split(pt_original.get());
      //AskTypeAnalyzer().analyze(pt_original.get());
      msc_original = mcc.create(pt_original);
    }

    {
      parse_tree_sptr pt_no_init(boost::make_shared<ParseTree>(*parse_tree));
      InitNodeRemover().apply(pt_no_init.get());
      AskDisjunctionFormatter().format(pt_no_init.get());
      AskDisjunctionSplitter().split(pt_no_init.get());
      //AskTypeAnalyzer().analyze(pt_no_init.get());
      msc_no_init = mcc.create(pt_no_init);

      // 最適化された形のパースツリーを得る
      member_parse_tree = pt_no_init;
    }

    //{
    //  parse_tree_sptr pt_no_init_discreteask(boost::make_shared<ParseTree>(*parse_tree));
    //  InitNodeRemover().apply(pt_no_init_discreteask.get());
    //  DiscreteAskRemover().apply(pt_no_init_discreteask.get());
    //  AskDisjunctionFormatter().format(pt_no_init_discreteask.get());
    //  AskDisjunctionSplitter().split(pt_no_init_discreteask.get());
    //  msc_no_init_discreteask_ = mcc.create(pt_no_init_discreteask);
    //}
  }
};

}

void SymbolicSimulator::init_module_set_container(const parse_tree_sptr& parse_tree)
{  
  HYDLA_LOGGER_DEBUG("#*** create module set list ***\n",
                     "nd_mode=", opts_.nd_mode);

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

  while(!state_stack_.empty()) {
    phase_state_sptr state(pop_phase_state());
    branch_=0;
    state->module_set_container->dispatch(
    boost::bind(&SymbolicSimulator::simulate_phase_state, 
               this, _1, state));
    if(opts_.nd_mode&&opts_.output_style==styleList){
     if(!branch_){
       if(!output_vector_.empty()){
         std::cout << std::endl;
         std::cout << "# time\t";
         BOOST_FOREACH(variable_map_t::value_type& i, variable_map_) {
           std::cout << i.first << "\t";
         }
         std::cout << std::endl;
         std::vector<string>::iterator it = output_vector_.begin();
         std::vector<string>::iterator end = output_vector_.end();
         for(;it!=end;it++){
            std::cout << *it;
            std::cout << std::endl;
         }
       }
       std::cout << output_buffer_.str();

       while(!branch_stack_.empty()&&--branch_stack_.top()<=0){
         branch_stack_.pop();
         if(!output_vector_.empty()){
           output_vector_.pop_back();
         }
       }
       output_buffer_.str("");
     }else if(branch_>1){
       branch_stack_.push(branch_);
       output_vector_.push_back(output_buffer_.str());
       output_buffer_.str("");
     }
    }
  }
}


CalculateClosureResult SymbolicSimulator::calculate_closure(const phase_state_const_sptr& state,
                                          const module_set_sptr& ms, expanded_always_t &expanded_always,
                                          positive_asks_t &positive_asks, negative_asks_t &negative_asks){

  //前準備
  TellCollector tell_collector(ms);
  AskCollector  ask_collector(ms, AskCollector::ENABLE_COLLECT_NON_TYPED_ASK | 
                              AskCollector::ENABLE_COLLECT_DISCRETE_ASK |
                              AskCollector::ENABLE_COLLECT_CONTINUOUS_ASK);
  tells_t         tell_list;
  boost::shared_ptr<hydla::parse_tree::Ask>  const *branched_ask;                           //UNKNOWN返されたAsk入れる


  bool expanded;
  do{
    branched_ask=NULL;
    // tell制約を集める
    tell_collector.collect_new_tells(&tell_list,
                                     &expanded_always, 
                                     &positive_asks);
    if(Logger::constflag==3){
      HYDLA_LOGGER_AREA("#** calculate_closure: expanded always after collect_new_tells: **\n",
                       expanded_always); 
    }

    HYDLA_LOGGER_DEBUG("#** calculate_closure: expanded always after collect_new_tells: **\n",
                       expanded_always);  

    // 制約を追加し，制約ストアが矛盾をおこしていないかどうか
    switch(solver_->add_constraint(tell_list,state->appended_asks)) 
    {
      case VCSR_TRUE:
        // do nothing
        break;
      case VCSR_FALSE:
        return CC_FALSE;
        break;
      case VCSR_UNKNOWN:
        assert(0);
        break;
      case VCSR_SOLVER_ERROR:
        // TODO: 例外とかなげたり、BPシミュレータに移行したり
        assert(0);
        break;
    }

    // ask制約を集める
    ask_collector.collect_ask(&expanded_always, 
                              &positive_asks, 
                              &negative_asks);

    if(Logger::constflag==3){
      HYDLA_LOGGER_AREA("#** calculate_closure: expanded always after collect_ask: **\n",
                       expanded_always);
    }

    HYDLA_LOGGER_DEBUG("#** calculate_closure: expanded always after collect_ask: **\n",
                       expanded_always);  

    // ask制約のエンテール処理
    expanded = false;
    negative_asks_t::iterator it  = negative_asks.begin();
    negative_asks_t::iterator end = negative_asks.end();
    while(it!=end) {
      switch(solver_->check_entailment(*it))
      {
        case VCSR_TRUE:
          expanded = true;
          positive_asks.insert(*it);
          negative_asks.erase(it++);
          break;
        case VCSR_FALSE:
          it++;
          break;
        case VCSR_UNKNOWN:
          if(!expanded&&branched_ask==NULL){
            branched_ask=&(*it);
          }
          it++;
          break;
        case VCSR_SOLVER_ERROR:
          // TODO: 例外とかなげたり、BPシミュレータに移行したり
          assert(0);
          break;
      }
    }
  }while(expanded);
  
  if(branched_ask!=NULL){
    // 分岐先を生成
    HYDLA_LOGGER_DEBUG("#*** create new phase state (branch) ***");
    phase_state_sptr new_state(create_new_phase_state(state)),new_state_not(create_new_phase_state(state));
    appended_ask_t tmpAppend;
    tmpAppend.ask = *branched_ask;
    tmpAppend.entailed = true;
    new_state->appended_asks.push_back(tmpAppend);
    tmpAppend.entailed = false;
    new_state_not->appended_asks.push_back(tmpAppend);
    //状態をスタックに押し込む
    push_phase_state(new_state_not);
    if(opts_.nd_mode){
      push_phase_state(new_state);
    }
    return CC_BRANCH;
  }
  return CC_TRUE;
}


bool SymbolicSimulator::point_phase(const module_set_sptr& ms, 
                                const phase_state_const_sptr& state)
{
  //旧 reduce出力用分岐
  if(opts_.solver == "r" || opts_.solver == "Reduce") {
    reduce_output(ms,state);
    return true;
  }

  //前準備
  if(state->changed_asks.size() != 0) {
    HYDLA_LOGGER_DEBUG("#** point_phase: changed_ask_id: ",
                       state->changed_asks.at(0).second,
                       " **");
  }
  expanded_always_t expanded_always;
  expanded_always_id2sptr(state->expanded_always_id, expanded_always);
  if(Logger::constflag==5){
  HYDLA_LOGGER_AREA("#** point_phase: expanded always from IP: **\n",
                     expanded_always);
  }
  HYDLA_LOGGER_DEBUG("#** point_phase: expanded always from IP: **\n",
                     expanded_always);  
  solver_->change_mode(DiscreteMode, opts_.approx_precision);
  solver_->reset(state->variable_map, state->parameter_map);

  positive_asks_t positive_asks;
  negative_asks_t negative_asks;


  //閉包計算
  switch(calculate_closure(state,ms,expanded_always,positive_asks,negative_asks)){
    case CC_TRUE:
    break;
    case CC_FALSE:
    return false;    
    case CC_BRANCH:
    return true;
  }


  // Interval Phaseへ移行（次状態の生成）
  HYDLA_LOGGER_DEBUG("#*** create new phase state ***");
  phase_state_sptr new_state(create_new_phase_state());
  new_state->phase        = IntervalPhase;
  new_state->current_time = state->current_time;
  expanded_always_sptr2id(expanded_always, new_state->expanded_always_id);
  HYDLA_LOGGER_DEBUG("--- expanded always ID ---\n",
                     new_state->expanded_always_id);
  new_state->module_set_container = msc_no_init_;
  new_state->parameter_map = state->parameter_map;

  {
    // 暫定的なフレーム公理の処理
    // 未定義の値や変数表に存在しない場合は以前の値をコピー
    solver_->create_variable_map(new_state->variable_map, new_state->parameter_map);
    variable_map_t::const_iterator it  = state->variable_map.begin();
    variable_map_t::const_iterator end = state->variable_map.end();
    for(; it!=end; ++it) {
      if(new_state->variable_map.get_variable(it->first).is_undefined())
      {
        new_state->variable_map.set_variable(it->first, it->second);
      }
    }
  }

  //出力
  output_point(new_state->current_time, new_state->variable_map, new_state->parameter_map);

  //状態をスタックに押し込む
  push_phase_state(new_state);
  
  if(Logger::varflag==3){
  HYDLA_LOGGER_AREA("%%%%%%%%%%%%% point phase result  %%%%%%%%%%%%%\n",
                     "time:", new_state->current_time, "\n",
                     new_state->variable_map);
  HYDLA_LOGGER_AREA("#*** end point phase ***");
  }

  HYDLA_LOGGER_SUMMARY("%%%%%%%%%%%%% point phase result  %%%%%%%%%%%%%\n",
                     "time:", new_state->current_time, "\n",
                     new_state->variable_map);
  HYDLA_LOGGER_SUMMARY("#*** end point phase ***");

  return true;
}

bool SymbolicSimulator::interval_phase(const module_set_sptr& ms, 
                                   const phase_state_const_sptr& state)
{

  //前準備
  expanded_always_t expanded_always;
  expanded_always_id2sptr(state->expanded_always_id, expanded_always);
  if(Logger::constflag==5){
	  HYDLA_LOGGER_DEBUG("#** interval_phase: expanded always from PP: **\n",
                     expanded_always);
  }
  HYDLA_LOGGER_DEBUG("#** interval_phase: expanded always from PP: **\n",
                     expanded_always);

  solver_->change_mode(ContinuousMode, opts_.approx_precision);
  solver_->reset(state->variable_map, state->parameter_map);
  positive_asks_t positive_asks;
  negative_asks_t negative_asks;

  //閉包計算
  switch(calculate_closure(state,ms,expanded_always,positive_asks,negative_asks)){
    case CC_TRUE:
    break;
    case CC_FALSE:
    return false;    
    case CC_BRANCH:
    return true;
  }
  if(Logger::constflag==4){
  // MaxModuleの導出
  module_set_sptr max_module_set = (*msc_no_init_).get_max_module_set();
  HYDLA_LOGGER_AREA("#** interval_phase: ms: **\n",
                     *ms,
                     "\n#** interval_phase: max_module_set: ##\n",
                     *max_module_set);
  }
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

  
  // askの導出状態が変化するまで積分をおこなう
  virtual_constraint_solver_t::IntegrateResult integrate_result;
  solver_->integrate(
    integrate_result,
    positive_asks,
    negative_asks,
    state->current_time,
    time_t(opts_.max_time),
    not_adopted_tells_list);

  //出力
  output_interval(state->current_time,
                  integrate_result.states[0].time-state->current_time,
                  integrate_result.states[0].variable_map);



  //to next pointphase
  assert(integrate_result.states.size() == 1);

  if(!integrate_result.states[0].is_max_time) {
    phase_state_sptr new_state(create_new_phase_state());
    new_state->phase        = PointPhase;
    expanded_always_sptr2id(expanded_always, new_state->expanded_always_id);
    HYDLA_LOGGER_DEBUG("--- expanded always ID ---\n",
                       new_state->expanded_always_id);
    new_state->module_set_container = msc_no_init_;
    new_state->current_time = integrate_result.states[0].time;
    new_state->parameter_map = state->parameter_map;

    if(Logger::varflag==5){
     //次のフェーズにおける変数の値を導出する
     HYDLA_LOGGER_AREA("--- calc next phase variable map ---");  
	  }

    //次のフェーズにおける変数の値を導出する
    HYDLA_LOGGER_DEBUG("--- calc next phase variable map ---");  
    solver_->apply_time_to_vm(integrate_result.states[0].variable_map, new_state->variable_map, integrate_result.states[0].time-state->current_time);

    push_phase_state(new_state);
  }
  if(Logger::varflag==4){
  HYDLA_LOGGER_AREA("%%%%%%%%%%%%% interval phase result  %%%%%%%%%%%%%\n",
                     "time:", solver_->get_real_val(integrate_result.states[0].time, 5), "\n",
                     integrate_result.states[0].variable_map);
  }

  HYDLA_LOGGER_SUMMARY("%%%%%%%%%%%%% interval phase result  %%%%%%%%%%%%%\n",
                     "time:", solver_->get_real_val(integrate_result.states[0].time, 5), "\n",
                     integrate_result.states[0].variable_map);

  return true;
}

variable_map_t SymbolicSimulator::shift_variable_map_time(const variable_map_t& vm, const time_t &time){
    variable_map_t shifted_vm;
    variable_map_t::const_iterator it  = vm.begin();
    variable_map_t::const_iterator end = vm.end();
    for(; it!=end; ++it) {
      shifted_vm.set_variable(it->first, solver_->shift_expr_time(it->second, time));
    }
    return shifted_vm;
}

void SymbolicSimulator::output_interval(const time_t& current_time, const time_t& limit_time,
                                        const variable_map_t& variable_map){


  time_t tmp_time = limit_time+current_time;
  solver_->simplify(tmp_time);
  if(opts_.output_format != fmtNumeric||!current_time.is_unique()){
    std::cout << "(in IP)" << std::endl;
    variable_map_t shifted_vm = shift_variable_map_time(variable_map, current_time);
    output(tmp_time, shifted_vm);
  }else{
    variable_map_t output_vm;
    time_t elapsed_time;

    do{
      solver_->apply_time_to_vm(variable_map, output_vm, elapsed_time);
      output((elapsed_time+current_time),output_vm);
      elapsed_time += time_t(opts_.output_interval);
      solver_->simplify(elapsed_time);
    }while(solver_->less_than(elapsed_time, limit_time));
    elapsed_time = limit_time;
    solver_->apply_time_to_vm(variable_map, output_vm, elapsed_time);
    output(tmp_time,output_vm);

    std::cout << std::endl << std::endl;
  }
}


void SymbolicSimulator::output_point(const time_t& time, const variable_map_t& variable_map,const parameter_map_t& parameter_map){
  if(opts_.output_format != fmtNumeric||!time.is_unique()){
    std::cout << std::endl <<"(in PP)" << std::endl;
  }
  output(time, variable_map, parameter_map);
}

/*
* reduce用のファイル出力関数
* 出力: シミュレーション終了時刻MaxT
*       depend文, 変数表var_list
        解候補モジュール集合のリストmsc
*/
bool SymbolicSimulator::reduce_simulate()
{
/*
* input: opts_, variable_map_, msc_original_, 
*/

//ファイルストリームを開く
  std::ofstream ofs( "in.red" );

//vcsにTreeVisitorを仮設置 引数はダミー
//  RTreeVisitor rtv(1);

/*
* シミュレーション終了時刻の出力
*/
  std::cout << "MaxT:= " << opts_.max_time << ";" << std::endl;
  ofs       << "MaxT:= " << opts_.max_time << ";" << std::endl;

/*
* depend文の出力
* TODO: 場合により同じ文が複数回出るのを直す
*/
  variable_map_t::const_iterator v_it = variable_map_.begin();
  variable_map_t::const_iterator v_end = variable_map_.end();
  std::string name;
  while(v_it!=v_end) {
    if(name!=v_it->first.get_name()){
      name = v_it->first.get_name();
      std::cout << "depend " << name << ",t;" << std::endl;
      ofs       << "depend " << name << ",t;" << std::endl;
    }
    v_it++;
  }


/*
* 変数表の出力
*/
  std::cout << "%variable map" << std::endl;
  std::cout << "%{var_string, var_name, var_derivative_count}" << std::endl;
  ofs       << "%{var_string, var_name, var_derivative_count}" << std::endl;
  std::cout << "var_list:={";
  ofs       << "var_list:={";

  v_it = variable_map_.begin();
//v_end = variable_map_.end();
  while(v_it!=v_end) {
    std::cout << "{\"" << v_it->first << "\", ";
    ofs       << "{\"" << v_it->first << "\", ";
    std::cout << v_it->first.get_name() << ", ";
    ofs       << v_it->first.get_name() << ", ";
    std::cout << v_it->first.get_derivative_count() << "}";
    ofs       << v_it->first.get_derivative_count() << "}";
//    std::cout << "v_it->second: " << v_it->second << std::endl;
    v_it++;
    if(v_it!=v_end) {
    std::cout << ", ";
    ofs       << ", ";
    }
  }
  std::cout << "};" << std::endl;
  ofs       << "};" << std::endl;

/*
* 制約の出力
* dispatchでmscからmsを一つ一つ取得
* TODO: ofsの代わりをコールバック関数に渡す
*/
  std::cout << "msc:={";

  phase_state_sptr state;
  
  msc_original_->dispatch(
  boost::bind(&SymbolicSimulator::reduce_simulate_phase_state, 
             this, _1));

  std::cout << "\b\b \n};" << std::endl;

// これを記述しとくと ./hydla | reduce とパイプ処理出来るかも
//  std::cout << "in \"bball.data\";" << std::endl;

  std::cout << ";end;" << std::endl;
  ofs       << ";end;" << std::endl;

  return true;
} 

/*
* dispatch用コールバック関数
* 引数をbind使わなくても良いよう直す
*/

bool SymbolicSimulator::reduce_simulate_phase_state(const module_set_sptr& ms)
{
  RTreeVisitor rtv(1);
  hydla::ch::ModuleSet::module_list_t::const_iterator it = ms->begin();
  hydla::ch::ModuleSet::module_list_t::const_iterator end = ms->end();

/*
* ms_name:={"BOUNCE$0", "FALL$0", "INIT$0"};

  std::cout << "ms_name:={";
  while(it!=end){
    std::cout << "\"" << it->first << "\"";
    it++;
    if(it!=end) {
      std::cout << ", ";
    }
  }

  std::cout << "};" << std::endl;
//  ofs       << "};" << std::endl;
*/

//  std::cout << ms->get_name() << std::endl;
  std::cout << "\n{";
  ms->dispatch(&rtv);
  std::cout << rtv.get_expr() << "}, ";
  return false;
}

/*
* Reduce用のファイル出力関数 中間発表までに作った物
* svn Rev: 1062
*/
bool SymbolicSimulator::reduce_output(const module_set_sptr& ms, 
                                const phase_state_const_sptr& state)
{
  TellCollector tell_collector(ms);

  AskCollector  ask_collector(ms, AskCollector::ENABLE_COLLECT_NON_TYPED_ASK | 
                              AskCollector::ENABLE_COLLECT_DISCRETE_ASK |
                              AskCollector::ENABLE_COLLECT_CONTINUOUS_ASK);

  tells_t         tell_list;
  positive_asks_t   positive_asks;
  negative_asks_t   negative_asks;

  if(state->changed_asks.size() != 0) {
    HYDLA_LOGGER_DEBUG("#** point_phase: changed_ask_id: ",
                       state->changed_asks.at(0).second,
                       " **");
  }

  expanded_always_t expanded_always;

//ファイル出力
  std::ofstream ofs( "in.red" );
  int count = 0;

//vcsにTreeVisitorを仮設置 引数はダミー
  RTreeVisitor rtv("reduce_output");

/*
* depend文の出力
* TODO: 場合により同じ文が複数回出るのを直す
*/
  variable_map_t::const_iterator v_it = variable_map_.begin();
  variable_map_t::const_iterator v_end = variable_map_.end();
  std::string name;
  while(v_it!=v_end) {
    if(name!=v_it->first.get_name()){
      name = v_it->first.get_name();
      std::cout << "depend " << name << ",t;" << std::endl;
      ofs       << "depend " << name << ",t;" << std::endl;
    }
    v_it++;
  }

/*
* 変数表の出力
*/
  std::cout << "%{var_string, var_name, var_derivative_count}" << std::endl;
  ofs       << "%{var_string, var_name, var_derivative_count}" << std::endl;
  std::cout << "var_list:={";
  ofs       << "var_list:={";

  v_it = variable_map_.begin();
//v_end = variable_map_.end();
  while(v_it!=v_end) {
    std::cout << "{\"" << v_it->first << "\", ";
    ofs       << "{\"" << v_it->first << "\", ";
    std::cout << v_it->first.get_name() << ", ";
    ofs       << v_it->first.get_name() << ", ";
    std::cout << v_it->first.get_derivative_count() << "}";
    ofs       << v_it->first.get_derivative_count() << "}";
    v_it++;
    if(v_it!=v_end) {
    std::cout << ", ";
    ofs       << ", ";
    }
  }
  std::cout << "};" << std::endl;
  ofs       << "};" << std::endl;



// 制約
  // tell制約を集める
  tell_collector.collect_new_tells(&tell_list,
                                   &expanded_always, 
                                   &positive_asks);
  tells_t::const_iterator tells_it  = tell_list.begin();
  tells_t::const_iterator tells_end = tell_list.end();

  std::cout << "%tell_list" << std::endl;
  ofs       << "%tell_list" << std::endl;
  while(tells_it!=tells_end) {
    std::cout << "tell" << count << ":= " << rtv.get_expr(*tells_it) << ";" << std::endl;
    ofs       << "tell" << count << ":= " << rtv.get_expr(*tells_it) << ";" << std::endl;
    count++;
    tells_it++;
  }
  count = 0;

  
  // ask制約を集める
  std::cout << "%negative_asks_t" << std::endl;
  ofs       << "%negative_asks_t" << std::endl;
  ask_collector.collect_ask(&expanded_always, 
                            &positive_asks, 
                            &negative_asks);
  // ask制約のエンテール処理
//    expanded = false;
  negative_asks_t::iterator it  = negative_asks.begin();
  negative_asks_t::iterator end = negative_asks.end();
  
  while(it!=end) {
    std::cout << "map" << count << ":= "<< rtv.get_ask_rhs(*it) << ";" << std::endl;
    ofs       << "map" << count << ":= "<< rtv.get_ask_rhs(*it) << ";" << std::endl;
    std::cout << "guard" << count << ":= "<< rtv.get_guard(*it) << ";" << std::endl;
    ofs       << "guard" << count << ":= "<< rtv.get_guard(*it) << ";" << std::endl;
  
    count++;
    it++;
  }
  count = 0;


// シミュレーション終了時刻
  std::cout << "MaxT:= " << opts_.max_time << ";" << std::endl;
  ofs       << "MaxT:= " << opts_.max_time << ";" << std::endl;
  std::cout << ";end;" << std::endl;
  ofs       << ";end;" << std::endl;
  

  return true;
} 

void SymbolicSimulator::output(const time_t& time, 
                           const variable_map_t& vm)
{
  
  /*
  if(opts_.output_style==styleList){
    output_buffer_ << solver_->get_real_val(time, opts_.output_precision) << "\t";

    variable_map_t::const_iterator it  = vm.begin();
    variable_map_t::const_iterator end = vm.end();
    for(; it!=end; ++it) {
       output_buffer_ << solver_->get_real_val(it->second, opts_.output_precision) << "\t";
    }
    output_buffer_ << std::endl;
  }else{*/
  std::cout << std::endl;
  if(opts_.output_format == fmtNumeric && time.is_unique()){
    std::cout << solver_->get_real_val(time, opts_.output_precision) << "\t";
    variable_map_t::const_iterator it  = vm.begin();
    variable_map_t::const_iterator end = vm.end();
    for(; it!=end; ++it) {
      std::cout << solver_->get_real_val(it->second, opts_.output_precision) << "\t";
    }
  }else{
    std::cout << "time\t: " << time << "\n";
    variable_map_t::const_iterator it  = vm.begin();
    variable_map_t::const_iterator end = vm.end();
    for(; it!=end; ++it) {
      std::cout << it->first << "\t: " << it->second << "\n" ;
    }
  }
}

std::string SymbolicSimulator::range_to_string(const value_range_t& val){
  std::string tmp;
  if(val.is_undefined()||val.is_unique())
    return val.get_first_value().get_string();
  value_range_t::or_const_iterator or_it = val.or_begin(), or_end  = val.or_end();
  while(1){
    value_range_t::and_const_iterator and_it = or_it->begin(), and_end  = or_it->end();
    while(1){
      tmp.append(and_it->get_symbol());
      tmp.append(and_it->get_value().get_string());
      if(++and_it==and_end)break;
      tmp.append("&");
    }
    if(++or_it==or_end)break;
    tmp.append("|");
  }
  return tmp;
}

void SymbolicSimulator::output(const time_t& time, 
                           const variable_map_t& vm,const parameter_map_t& pm)
{
  output(time,vm);
  parameter_map_t::const_iterator it  = pm.begin();
  parameter_map_t::const_iterator end = pm.end();
  for(; it!=end; ++it) {
    std::cout << "p" << it->first << "\t: " << range_to_string(it->second) << "\n";
  }
  std::cout << std::endl;
}

} //namespace symbolic_simulator
} //namespace hydla
