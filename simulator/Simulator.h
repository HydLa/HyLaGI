#ifndef _INCLUDED_HYDLA_SIMULATOR_H_
#define _INCLUDED_HYDLA_SIMULATOR_H_

#include <iostream>
#include <string>
#include <stack>
#include <cassert>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
//#include <boost/function.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/xpressive/xpressive.hpp>

#include "Logger.h"

#include "Node.h"
#include "ParseTree.h"
#include "ModuleSetContainer.h"
#include "ModuleSetContainerCreator.h"
#include "ModuleSetGraph.h"
#include "ModuleSetList.h"


#include "AskDisjunctionSplitter.h"
#include "AskDisjunctionFormatter.h"

#include "VariableMap.h"
#include "PhaseResult.h"
#include "InitNodeRemover.h"
#include "TreeInfixPrinter.h"
#include "PhaseSimulator.h"



namespace {
  using namespace hydla::ch;
  using namespace hydla::simulator;
  using namespace hydla::parse_tree;
  struct ModuleSetContainerInitializer {
    typedef boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree_sptr;
    typedef hydla::simulator::module_set_container_sptr         module_set_container_sptr;
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
          msc_original = mcc.create(pt_original);
        }

        {
          parse_tree_sptr pt_no_init(boost::make_shared<ParseTree>(*parse_tree));
          InitNodeRemover().apply(pt_no_init.get());
          AskDisjunctionFormatter().format(pt_no_init.get());
          AskDisjunctionSplitter().split(pt_no_init.get());
          msc_no_init = mcc.create(pt_no_init);

          // 最適化された形のパースツリーを得る
          member_parse_tree = pt_no_init;
        }
      }
  };
}

namespace hydla {
namespace simulator {

//TODO:PhaseResultTypeじゃなくて，ValueType入れるテンプレートクラスにできない？というか設計再検討．このクラスは何に対応している？
//実行アルゴリズムだろうか？
template<typename PhaseResultType>
class Simulator
{
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

  Simulator(Opts& opts):opts_(&opts)
  {}
  
  virtual ~Simulator()
  {}

  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  virtual void simulate()
  {
    assert(0);
  }
  
  /**
   * 使用するPhaseSimulatorを設定する．
   * この関数に渡すPhaseSimulatorのインスタンスはnewで作成し，呼び出し側でdeleteしないようにする
   */
  void set_phase_simulator(PhaseSimulator<PhaseResultType> *ps){
    phase_simulator_.reset(ps);
  }

  virtual void initialize(const parse_tree_sptr& parse_tree)
  {
    init_module_set_container(parse_tree);
    opts_->assertion = parse_tree->get_assertion_node();
    result_root_.reset(new phase_result_t());
    
    result_root_->module_set_container = msc_original_;
    
    //出力変数無指定な場合の出力制御（全部出力）
    if(opts_->output_variables.empty()){
      BOOST_FOREACH(const typename variable_set_t::value_type& i, variable_set_) {
        opts_->output_variables.insert(i.get_string());
      }
    }
    parse_tree_ = parse_tree;
    init_variable_map(parse_tree);
    phase_simulator_->initialize(parse_tree, variable_set_, parameter_set_, variable_map_);
  }
  
  
  void init_module_set_container(const parse_tree_sptr& parse_tree)
  {    
    if(opts_->nd_mode||opts_->interactive_mode) {
      //全解探索モードなど
      ModuleSetContainerInitializer::init<ch::ModuleSetGraph>(
          parse_tree, msc_original_, msc_no_init_, parse_tree_);
    }
    else {
      //通常実行モード
      ModuleSetContainerInitializer::init<ch::ModuleSetList>(
          parse_tree, msc_original_, msc_no_init_, parse_tree_);
    }
  }
    

  /**
   * 新たなPhaseResultの作成
   */
  phase_result_sptr create_new_phase_result() const
  {
    phase_result_sptr ph(new phase_result_t());
    return ph;
  }

  /**
   * 与えられたPhaseResultの情報をを引き継いだ，
   * 新たなPhaseResultの作成
   */
  phase_result_sptr create_new_phase_result(const phase_result_const_sptr& old) const
  {
    phase_result_sptr ph(new phase_result_t(*old));
    return ph;
  }


  void output_variable_labels(std::ostream &stream, const variable_map_t variable_map){
    // 変数のラベル
    // TODO: 未定義の値とかのせいでずれる可能性あり?
    stream << "# time\t";

    BOOST_FOREACH(const typename variable_map_t::value_type& i, variable_map) {
      if(opts_->output_variables.find(i.first->get_string()) != opts_->output_variables.end()){
        stream << i.first << "\t";
      }
    }
    stream << std::endl;
  }
  
  
  std::string get_state_output(const phase_result_t& result, const bool& numeric, const bool& is_in_progress){
    std::stringstream sstr;
    if(!numeric){
      if(result.phase==IntervalPhase){
        sstr << "---------IP---------" << std::endl;
        std::string end_time;
        if(is_in_progress){
          sstr << "time\t: " << result.current_time << "->" << result.end_time << "\n";
        }else{
          if(result.children.empty() || result.cause_of_termination == TIME_LIMIT){
            end_time = opts_->max_time;
          }else{
            end_time = result.children[0]->current_time.get_string();
          }
          sstr << "time\t: " << result.current_time << "->" << end_time << "\n";
        }
      }else{
        if(is_in_progress)
          sstr << "#-------" << result.step + 1 << "-------" << std::endl;
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
        time_value_t elapsed_time("0");
        time_value_t limit_time = result.current_time-result.parent->current_time;
        
        //TODO:できればSimulatorから直接ソルバは見たくないが，そうしないと数値に変換できないのでどうしましょう
        //solver_->simplify(limit_time);
        /*
        do{
          //solver_->apply_time_to_vm(result.variable_map, output_vm, elapsed_time);
          output_vm = result.variable_map;
          output_variable_map(sstr, output_vm, (elapsed_time+result.parent->current_time), true);
          elapsed_time += time_value_t(opts_->output_interval);
          //solver_->simplify(elapsed_time);
        }while(solver_->less_than(elapsed_time, limit_time));
        */
        //solver_->apply_time_to_vm(result.variable_map, output_vm, limit_time);
        output_vm = result.variable_map;
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
  
  
  
  void output_parameter_map(const parameter_map_t& pm)
  {
    typename parameter_map_t::const_iterator it  = pm.begin();
    typename parameter_map_t::const_iterator end = pm.end();
    if(it != end){
      std::cout << "\n#---------parameter condition---------\n";
    }
    for(; it!=end; ++it) {
      std::cout << *(it->first) << "\t: " << it->second << "\n";
    }
  }

  void output_variable_map(std::ostream &stream, const variable_map_t& vm, const time_value_t& time, const bool& numeric)
  {
    typename variable_map_t::const_iterator it  = vm.begin();
    typename variable_map_t::const_iterator end = vm.end();
    if(numeric){
    /*
      stream << std::endl;
      stream << solver_->get_real_val(time, opts_->output_precision, opts_->output_format) << "\t";
      for(; it!=end; ++it) {
        if(opts_->output_variables.find(it->first->get_string()) != opts_->output_variables.end())
          stream << solver_->get_real_val(it->second, opts_->output_precision, opts_->output_format) << "\t";
      }
      */
    }else{
      for(; it!=end; ++it) {
        stream << *(it->first) << "\t: " << it->second << "\n";
      }
    }
  }


  void const output_result_tree()
  {
    if(result_root_->children.size() == 0){
      std::cout << "No Result." << std::endl;
      return;
    }
    typename phase_result_sptrs_t::iterator it = result_root_->children.begin(), end = result_root_->children.end();
    int i=1, j=1;
    for(;it!=end;it++){
      std::vector<std::string> result;
      output_result_node(*it, result, i, j);
    }
  }

  void const output_result_node(const phase_result_sptr &node, std::vector<std::string> &result, int &case_num, int &phase_num){

    if(node->children.size() == 0){
    
      if(opts_->nd_mode){
        std::cout << "#---------Case " << case_num++ << "---------" << std::endl;
      }
      std::vector<std::string>::const_iterator r_it = result.begin(), r_end = result.end();
      for(;r_it!=r_end;r_it++){
        std::cout << *r_it;
      }
      
      if(node->cause_of_termination!=simulator::INCONSISTENCY)
          std::cout << get_state_output(*node, false,false);
      switch(node->cause_of_termination){
        case simulator::INCONSISTENCY:
          std::cout << "# execution stuck\n";
          output_parameter_map(node->parameter_map);
          break;

        case simulator::SOME_ERROR:
          output_parameter_map(node->parameter_map);
          std::cout << "# some error occurred\n" ;
          break;

        case simulator::ASSERTION:
          output_parameter_map(node->parameter_map);
          std::cout << "# assertion failed\n" ;
          break;
          
        case simulator::TIME_LIMIT:
          output_parameter_map(node->parameter_map);
          std::cout << "# time ended\n" ;
          break;
          
        case simulator::NOT_UNIQUE_IN_INTERVAL:
          output_parameter_map(node->parameter_map);
          std::cout << "# some values of variables are not unique in this phase\n" ;
          break;

        default:
        case simulator::NONE:
          output_parameter_map(node->parameter_map);
          std::cout << "# unknown termination occurred\n" ;
          break;
      }
      std::cout << std::endl;
    }else{
      if(node->phase==PointPhase){
        std::stringstream sstr;
        sstr << "#---------" << phase_num++ << "---------\n";
        result.push_back(sstr.str());
      }
      result.push_back(get_state_output(*node, false,false));
      typename phase_result_sptrs_t::const_iterator it = node->children.begin(), end = node->children.end();
      for(;it!=end;it++){
        output_result_node(*it, result, case_num, phase_num);
      }
      result.pop_back();
      if(node->phase==PointPhase){
        result.pop_back();
        phase_num--;
      }
    }
  }
  

  /**
   * シミュレーション時に使用される変数表のオリジナルの作成
   */
  virtual void init_variable_map(const parse_tree_sptr& parse_tree)
  {
    typedef hydla::parse_tree::ParseTree::variable_map_const_iterator vmci;

    vmci it  = parse_tree->variable_map_begin();
    vmci end = parse_tree->variable_map_end();
    for(; it != end; ++it)
    {
      for(int d=0; d<=it->second; ++d) {
        variable_t v;
        v.name             = it->first;
        v.derivative_count = d;
        variable_set_.push_front(v);
        variable_map_.set_variable(&(variable_set_.front()), value_t());
      }
    }

    HYDLA_LOGGER_REST(
      "#*** variable map ***\n",
      variable_map_);
  }

  //TODO:表示すると木が消える仕様になっているので，どうにかする
  void output_result_tree_mathematica(){
    if(result_root_->children.size() == 0){
      std::cout << "No Result." << std::endl;
      return;
    }
    std::cout << "Show[";
    while(1){
      std::string prev_node_time = "0";
      std::cout << "Table[";
      bool is_first = true;
      phase_result_sptr_t now_node = result_root_->children.back();
      while(1){
        if(now_node->phase==IntervalPhase){
          variable_map_t vm = now_node->variable_map;
          if(is_first){
            is_first = false;
            std::cout << "{";
          }else{
            std::cout << ",";
          }
          typename variable_map_t::const_iterator it = vm.begin();
          while(opts_->output_variables.find(it->first->get_string()) == opts_->output_variables.end()){
            it++;
            if(it == vm.end()) return;
          }
          std::cout << "Plot[";
          std::cout << it->second;
          std::cout << ", {t, ";
          std::cout << prev_node_time;
          std::cout << ", ";
          std::cout << now_node->end_time;
          std::cout << "} ";
          prev_node_time = now_node->end_time.get_string();
          if(now_node->cause_of_termination == simulator::ASSERTION)
            std::cout << ", PlotStyle -> Dashed";
          std::cout << "]"; //Plot
        }
        if(now_node->children.size() == 0){//葉に到達
          std::cout << "}, "; //式のリストここまで
          if(now_node->parameter_map.size() > 0){
            // 定数の条件
            std::cout << "{";
            typename parameter_map_t::const_iterator it = now_node->parameter_map.begin();
            while(it != now_node->parameter_map.end()){
              if(it->second.is_undefined()){
                it++;
              }else{
                std::cout << *it->first << ", " << it->second.get_lower_bound().value.get_string() << ", " << it->second.get_upper_bound().value.get_string() << ", step}";
                break;
              }
            }
            if(it == now_node->parameter_map.end()){
              std::cout << "{1}";
            }
          }else{
            std::cout << "{1}";
          }
          std::cout << "], "; //Table
          std::cout << std::endl;
          while(now_node->children.size() == 0){
            if(now_node->parent != NULL){
              now_node = now_node->parent;
            }else{//親がいないということは根と葉が同じ、つまり「空になった」ということなので終了．
              std::cout << "PlotRange -> {{0, " << opts_->max_time << "}, {lb, ub}}";
              std::cout << "]" << std::endl; //Show
              return;
            }
            now_node->children.pop_back();
          }
          break;
        }
        else{
          now_node = now_node->children.back();
        }
      }
    }
  };

  
protected:

  /**
   * シミュレーション対象となるパースツリー
   */
  parse_tree_sptr parse_tree_;
  
  
  /**
   * シミュレーション中で使用される変数表の原型
   */
  variable_map_t variable_map_;

  
  /*
   * シミュレーション中に使用される変数と記号定数の集合
   */
  variable_set_t variable_set_;
  parameter_set_t parameter_set_;
  int state_id_;
  
  /**
   * シミュレーションの実行状態をあらわす構造体
   */
  struct SimulationState {
    phase_result_sptr phase_result;
    /// フェーズ内で一時的に追加する制約．分岐処理などに使用
    constraints_t temporary_constraints;
    module_set_container_sptr module_set_container;
    /// 判定済みのモジュール集合を保持しておく．分岐処理時，同じ集合を複数回調べることが無いように
    std::set<module_set_sptr> visited_module_sets;
  };



  /**
   * 使用するPhaseSimulator
   */ 
  boost::shared_ptr<PhaseSimulator<PhaseResultType> > phase_simulator_;


  module_set_container_sptr msc_original_;
  module_set_container_sptr msc_no_init_;

  /**
   * 各状態を保存しておくためのスタック
   */
  std::stack<phase_result_sptr> state_stack_;
  
  parse_tree_sptr parse_tree;
  
  ///解軌道木の根．初期状態なので，子供以外の情報は入れない
  phase_result_sptr result_root_;
  
  Opts*     opts_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_H_
