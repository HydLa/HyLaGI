#include "MathematicaVCSPoint.h"

#include <cassert>

#include <boost/algorithm/string/predicate.hpp>

#include "mathlink_helper.h"
#include "PacketErrorHandler.h"
#include "Logger.h"
#include "PacketChecker.h"
#include "MathematicaExpressionConverter.h"

using namespace hydla::vcs;
using namespace hydla::logger;

namespace hydla {
namespace vcs {
namespace mathematica {

MathematicaVCSPoint::MathematicaVCSPoint(MathLink* ml) :
  ml_(ml)
{
}

MathematicaVCSPoint::~MathematicaVCSPoint()
{}

bool MathematicaVCSPoint::reset()
{
  // TODO: チョイ考える
  assert(0);
  return true;
}

bool MathematicaVCSPoint::reset(const variable_map_t& variable_map)
{
  //旧版
  assert(0);
  return false;
}

bool MathematicaVCSPoint::reset(const variable_map_t& variable_map, const parameter_map_t& parameter_map){
  if(parameter_map.size() == 0)
  {
    HYDLA_LOGGER_VCS_SUMMARY("no Parameters");
  }else{
    HYDLA_LOGGER_VCS_SUMMARY("------Parameter map------\n", parameter_map);

    std::set<MathValue> and_cons_set;
    par_names_.clear();

    parameter_map_t::variable_list_t::const_iterator it = 
      parameter_map.begin();
    parameter_map_t::variable_list_t::const_iterator end = 
      parameter_map.end();
    for(; it!=end; ++it)
    {
      const value_range_t&    value = it->second;
      if(!value.is_undefined()) {
        value_range_t::or_vector::const_iterator or_it = value.or_begin(), or_end = value.or_end();
        for(;or_it != or_end; or_it++){
          value_range_t::and_vector::const_iterator and_it = or_it->begin(), and_end = or_it->end();
          for(; and_it != and_end; and_it++){
            std::ostringstream val_str;

            val_str << MathematicaExpressionConverter::get_relation_math_string(and_it->relation) << "[" ;
            val_str << PacketSender::par_prefix
                    << it->first.get_name();
            val_str << ","
                    << and_it->value
                    << "]"; // 閉じ括弧
                    
            MathValue new_math_value;
            new_math_value.set(val_str.str());
            and_cons_set.insert(new_math_value);
            par_names_.insert(it->first.get_name());
          }
        }
      }
    }
    parameter_store_.first.insert(and_cons_set);
  }
  
  if(variable_map.size() == 0)
  {
    HYDLA_LOGGER_VCS_SUMMARY("no Variables");
    return true;
  }
  HYDLA_LOGGER_VCS_SUMMARY("------Variable map------\n", variable_map);
  std::set<MathValue> and_cons_set;
  MathematicaExpressionConverter mec;
  reset_sub(variable_map, and_cons_set, mec, false);
  constraint_store_.first.insert(and_cons_set);
  mec.create_max_diff_map(max_diff_map_);
  HYDLA_LOGGER_VCS(*this);

  return true;
}

void MathematicaVCSPoint::reset_sub(const variable_map_t& variable_map, std::set<MathValue>& and_cons_set,
    MathematicaExpressionConverter& mec, const bool& is_current){
    
  variable_map_t::variable_list_t::const_iterator it = 
    variable_map.begin();
  variable_map_t::variable_list_t::const_iterator end = 
    variable_map.end();
  for(; it!=end; ++it)
  {
    const MathVariable& variable = (*it).first;
    const value_t&    value = it->second;
    if(!value.is_undefined()) {
      std::ostringstream val_str;
      val_str << "Equal[";

      if(variable.derivative_count > 0)
      {
        val_str << "Derivative[" << variable.derivative_count << "][";
        if(!is_current){
          val_str << "prev[" << PacketSender::var_prefix << variable.name << "]";
        }else{
          val_str << PacketSender::var_prefix << variable.name;
        }
        val_str << "]";
      }
      else
      {
        if(!is_current){
          val_str << "prev[" << PacketSender::var_prefix << variable.name << "]";
        }else{
          val_str << PacketSender::var_prefix << variable.name;
        }
      }
      val_str << ","
              << mec.convert_symbolic_value_to_math_string(value)
              << "]"; // Equalの閉じ括弧

      MathValue new_math_value;
      new_math_value.set(val_str.str());
      and_cons_set.insert(new_math_value);

      // 制約ストア内の変数一覧を作成
      constraint_store_.second.insert(
        boost::make_tuple(variable.name,
                          variable.derivative_count,
                          true));
    }
  }
}


bool MathematicaVCSPoint::create_maps(create_result_t& create_result)
{
  HYDLA_LOGGER_VCS(
    "#*** MathematicaVCSPoint::create_variable_map ***\n", *this);
  // 制約ストアが空（true）の場合は変数表も空で良い
  if(cs_is_true()) return true;

/////////////////// 送信処理

// convertCSToVM[exprs]を渡したい
  ml_->put_function("convertCSToVM", 1);
  send_cs();

/////////////////// 受信処理                        

  HYDLA_LOGGER_EXTERN(
    "-- math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
  HYDLA_LOGGER_EXTERN((ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
  ml_->skip_pkt_until(RETURNPKT);

  ml_->MLGetNext();

  // List関数の要素数（式の個数）を得る
  int or_size = ml_->get_arg_count();
  HYDLA_LOGGER_VCS("or_size: ", or_size);
  ml_->MLGetNext();
  for(int or_it = 0; or_it < or_size; or_it++){
    create_result_t::maps_t maps;
    std::set<std::string> added_parameters;  //「今回追加された記号定数」の一覧
    variable_t symbolic_variable;
    value_t symbolic_value;
    parameter_t tmp_param;
    ml_->MLGetNext();
    int and_size = ml_->get_arg_count();
    HYDLA_LOGGER_VCS("and_size: ", and_size);
    ml_->MLGetNext(); // Listという関数名
    for(int i = 0; i < and_size; i++)
    {
      value_range_t tmp_range;
      ml_->MLGetNext();
      ml_->MLGetNext();
      
      // 変数名（名前、微分回数、prev）
      ml_->MLGetNext();
      ml_->MLGetNext();
      ml_->MLGetNext(); // ?
      std::string variable_name = ml_->get_string();
      int variable_derivative_count = ml_->get_integer();
      int prev = ml_->get_integer();

      // 関係演算子のコード
      int relop_code = ml_->get_integer();
      // 値
      std::string value_str = ml_->get_string();

      // prev変数は処理しない
      if(prev==1) continue;
      
      symbolic_variable.name = variable_name;
      symbolic_variable.derivative_count = variable_derivative_count;

      if(prev==-1){//既存の記号定数の場合
        tmp_param.name = variable_name;
        tmp_range = maps.parameter_map.get_variable(tmp_param);
        value_t tmp_value = MathematicaExpressionConverter::convert_math_string_to_symbolic_value(value_str);
        tmp_range.add(value_range_t::Element(tmp_value,MathematicaExpressionConverter::get_relation_from_code(relop_code)));
        maps.parameter_map.set_variable(tmp_param, tmp_range);
        continue;
      }
      // 関係演算子コードを元に、変数表の対応する部分に代入する
      // TODO: Orの扱い
      else if(!relop_code){
        //等号
        symbolic_value = MathematicaExpressionConverter::convert_math_string_to_symbolic_value(value_str);
        symbolic_value.set_unique(true);
      }else{
        //不等号．この変数の値の範囲を表現するための記号定数を作成
        tmp_param.name = variable_name;
        value_t tmp_value = MathematicaExpressionConverter::convert_math_string_to_symbolic_value(value_str);

        for(int i=0;i<variable_derivative_count;i++){
          //とりあえず微分回数分dをつける
          tmp_param.name.append("d");
        }
        while(1){
          if(added_parameters.find(tmp_param.name)!=added_parameters.end())break; //今回追加された記号定数に含まれるなら，同じ場所に入れる
          value_range_t &value = maps.parameter_map.get_variable(tmp_param);
          if(value.is_undefined()){
            added_parameters.insert(tmp_param.name);
            break;
          }
          //とりあえず重複回数分iをつける
          tmp_param.name.append("i");
        }
        tmp_range = maps.parameter_map.get_variable(tmp_param);
        MathematicaExpressionConverter::set_parameter_on_value(symbolic_value, tmp_param.name);
        symbolic_value.set_unique(false);
        tmp_range.add(value_range_t::Element(tmp_value,MathematicaExpressionConverter::get_relation_from_code(relop_code)));
        maps.parameter_map.set_variable(tmp_param, tmp_range);
      }
      maps.variable_map.set_variable(symbolic_variable, symbolic_value);
    }
    create_result.result_maps.push_back(maps);
  }
  
  
  HYDLA_LOGGER_VCS("#*** END MathematicaVCSPoint::create_variable_map ***\n");
  return true;
}


void MathematicaVCSPoint::add_left_continuity_constraint(
  PacketSender& ps, max_diff_map_t& max_diff_map)
{
  HYDLA_LOGGER_VCS("---- Begin MathematicaVCSPoint::add_left_continuity_constraint ----");
  // 送信する制約の個数を求める
  int left_cont_vars_count = 0;

  max_diff_map_t::const_iterator md_it = max_diff_map.begin();
  max_diff_map_t::const_iterator md_end = max_diff_map.end();
  for(; md_it!=md_end; ++md_it) {
    left_cont_vars_count += md_it->second;
  }
  
  HYDLA_LOGGER_VCS("left_cont_vars_count: ", left_cont_vars_count);
  
  ml_->put_function("List", left_cont_vars_count);
  // 実際に送信する
  md_it = max_diff_map.begin();
  md_end = max_diff_map.end();
  for(; md_it!=md_end; ++md_it) {
    for(int i=0; i<md_it->second; ++i){
      ml_->put_function("Equal", 2);
      
      // Prev変数側
      ps.put_var(
        boost::make_tuple(md_it->first, i, true),
        PacketSender::VA_None);
      
      // Now変数側
      ps.put_var(
        boost::make_tuple(md_it->first, i, false),
        PacketSender::VA_None);
    }
  }
}

void MathematicaVCSPoint::add_constraint(const constraints_t& constraints)
{
  HYDLA_LOGGER_VCS(
    "#*** Begin MathematicaVCSPoint::add_constraint ***");

  MathematicaExpressionConverter mec;
  constraints_t::const_iterator it = constraints.begin();
  constraints_t::const_iterator end = constraints.end();
  std::set<MathValue> and_cons_set = *(constraint_store_.first.begin());
  constraint_store_.first.erase(constraint_store_.first.begin());
  for(; it!=end; ++it)
  {
    MathValue new_math_value;
    new_math_value.set(mec.convert_symbolic_value_to_math_string(*it));
    and_cons_set.insert(new_math_value);
  }
  constraint_store_.first.insert(and_cons_set);
  constraint_store_.second.insert(mec.vars_begin(), mec.vars_end());
    
  mec.create_max_diff_map(max_diff_map_);
  HYDLA_LOGGER_VCS(*this, "\n#*** End MathematicaVCSPoint::add_constraint ***");

  return;
}



VCSResult MathematicaVCSPoint::check_consistency(const constraints_t& constraints)
{
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCSPoint::check_consistency(tmp) ***");
  tmp_constraints_ = constraints;
  
  VCSResult result = check_consistency_sub();
  switch(result){
    default: assert(0); break;
    case VCSR_TRUE:
      HYDLA_LOGGER_VCS_SUMMARY("consistent");
      ml_->MLNewPacket();
      break;
    case VCSR_FALSE:
      HYDLA_LOGGER_VCS_SUMMARY("inconsistent");//矛盾
    case VCSR_SOLVER_ERROR:
      break;
  }
  tmp_constraints_.clear();
  HYDLA_LOGGER_VCS("#*** End MathematicaVCSPoint::check_consistency(tmp) ***");
  return result;
}

VCSResult MathematicaVCSPoint::check_consistency()
{
  HYDLA_LOGGER_VCS(
    "#*** Begin MathematicaVCSPoint::check_consistency() ***");
  
  VCSResult result = check_consistency_sub();
  
  switch(result){
    default: assert(0); break;
    case VCSR_TRUE:
    {
      HYDLA_LOGGER_VCS_SUMMARY("consistent");
      //無矛盾性判定
      constraint_store_.first.clear();
      receive_constraint_store(constraint_store_);
    }
    break;
    case VCSR_FALSE:
      HYDLA_LOGGER_VCS_SUMMARY("inconsistent");//矛盾
    case VCSR_SOLVER_ERROR:
      break;
  }
  HYDLA_LOGGER_VCS(
    *this,
    "\n#*** End MathematicaVCSPoint::check_consistency() ***");
  return result;
}

void MathematicaVCSPoint::receive_constraint_store(constraint_store_t& constraint_store){
    //「文字列で」返って来たものを制約ストアに入れる
    // List[List[List["Equal[x, 1]"], List["Equal[x, -1]"]], List[x]]や
    // List[List[List["Equal[x, 1]", "Equal[y, 1]"], List["Equal[x, -1]", "Equal[y, -1]"]], List[x, y, z]]や
    // List[List[List["Equal[x,1]", "Equal[Derivative[1][x],1]", "Equal[prev[x],1]", "Equal[prev[Derivative[2][x]],1]"]],
    // List[x, Derivative[1][x], prev[x], prev[Derivative[2][x]]]]  やList[List["True"], List[]]など
    HYDLA_LOGGER_VCS( "--- MathematicaVCSPoint::receive constraint store---");
    ml_->MLGetNext();
    // List関数の要素数（Orで結ばれた解の個数）を得る
    int or_size = ml_->get_arg_count();
    HYDLA_LOGGER_VCS( "or_size: ", or_size);
    ml_->MLGetNext(); // Listという関数名

    for(int i=0; i<or_size; i++)
    {
      ml_->MLGetNext(); // List関数（Andで結ばれた解を表している）

      // List関数の要素数（Andで結ばれた解の個数）を得る
      int and_size = ml_->get_arg_count();
      HYDLA_LOGGER_VCS( "and_size: ", and_size);
      ml_->MLGetNext(); // Listという関数名
      if(and_size > 0) ml_->MLGetNext(); // Listの中の先頭要素

      std::set<MathValue> value_set;    
      for(int j=0; j<and_size; j++)
      {
        std::string str = ml_->get_string();
        MathValue math_value;
        math_value.set(str);
        value_set.insert(math_value);
      }
      constraint_store.first.insert(value_set);
    }
}

VCSResult MathematicaVCSPoint::check_consistency_sub()
{
  PacketSender ps(*ml_);

/////////////////// 送信処理

  // checkConsistency[ expr, vars]を渡したい
  ml_->put_function("checkConsistency", 2);

  // exprは4つの部分から成る
  ml_->put_function("Join", 4);
  int tmp_size = tmp_constraints_.size();
  ml_->put_function("List", tmp_size);
  HYDLA_LOGGER_VCS(
    "tmp_size:", tmp_size);

  // 一時的な制約ストアを送信する
  constraints_t::const_iterator tmp_it  = tmp_constraints_.begin();
  constraints_t::const_iterator tmp_end = tmp_constraints_.end();
  for(; tmp_it!= tmp_end; ++tmp_it) {
    HYDLA_LOGGER_VCS("put node: ", (**tmp_it));
    ps.put_node(*tmp_it, PacketSender::VA_None);
  }

  // 制約ストアからもexprを得てMathematicaに渡す
  send_cs();
  // 定数値に関する制約も送る
  send_ps();

  // 左連続性に関する制約を渡す
  // 現在採用している制約に出現する変数の最大微分回数よりも小さい微分回数のものについてprev(x)=x追加
  max_diff_map_t tmp_max_diff_map = max_diff_map_;
  ps.create_max_diff_map(tmp_max_diff_map);
  add_left_continuity_constraint(ps, tmp_max_diff_map);

  // varsを渡す
  ml_->put_function("Join", 2);
  ps.put_vars(PacketSender::VA_None);
  // 制約ストア内に出現する変数も渡す
  send_cs_vars();

/////////////////// 受信処理
  HYDLA_LOGGER_VCS( "--- receive ---");
 
  //PacketChecker pc(*ml_);
  //pc.check();

  HYDLA_LOGGER_EXTERN(
    "-- math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
  HYDLA_LOGGER_EXTERN((ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));


  ml_->skip_pkt_until(RETURNPKT);

  ml_->MLGetNext();
  ml_->MLGetNext();
  ml_->MLGetNext();
  
  VCSResult result;
  int ret_code = ml_->get_integer();
  if(PacketErrorHandler::handle(ml_, ret_code)) {
    result = VCSR_SOLVER_ERROR;
  }
  else if(ret_code==1) {
    // 充足
    result = VCSR_TRUE;
  }
  else {
    assert(ret_code==2);
    result = VCSR_FALSE;
  }
  return result;
}

VCSResult MathematicaVCSPoint::integrate(integrate_result_t& integrate_result,
  const constraints_t& constraints, const time_t& current_time, const time_t& max_time)
{
  assert(0); // Pointではintegrate関数無効
  return VCSR_FALSE;
}

void MathematicaVCSPoint::send_cs() const
{
  HYDLA_LOGGER_VCS("---- Send Constraint Store -----");
  
  if(constraint_store_.first.empty())
  {
    HYDLA_LOGGER_VCS("no Constraints");
    ml_->put_function("List", 0);
    return;
  }
  send_store(constraint_store_);
}

void MathematicaVCSPoint::send_ps() const
{
  HYDLA_LOGGER_VCS("---- Send Parameter Store -----");

  if(parameter_store_.first.empty())
  {
    HYDLA_LOGGER_VCS("no Parameters");
    ml_->put_function("List", 0);
    return;
  }
  send_store(parameter_store_);
}


void MathematicaVCSPoint::send_store(const constraint_store_t& store) const{
  
  //ストアは空で無いとする
  assert(!store.first.empty());
  int or_cons_size = store.first.size();
  ml_->put_function("List", 1);
  ml_->put_function("Or", or_cons_size);
  HYDLA_LOGGER_VCS("or cons size: ", or_cons_size);

  std::set<std::set<MathValue> >::const_iterator or_cons_it = store.first.begin();
  std::set<std::set<MathValue> >::const_iterator or_cons_end = store.first.end();
  for(; or_cons_it!=or_cons_end; ++or_cons_it)
  {
    int and_cons_size = (*or_cons_it).size();
    ml_->put_function("And", and_cons_size);
    HYDLA_LOGGER_VCS("and cons size: ", and_cons_size);

    std::set<MathValue>::const_iterator and_cons_it = 
      (*or_cons_it).begin();
    std::set<MathValue>::const_iterator and_cons_end = 
      (*or_cons_it).end();
    for(; and_cons_it!=and_cons_end; ++and_cons_it)
    {
      ml_->put_function("ToExpression", 1);
      std::string str = (*and_cons_it).get_string();
      ml_->put_string(str);
      HYDLA_LOGGER_VCS("put cons: ", str);
    }
  }
}


void MathematicaVCSPoint::send_pars() const{
  ml_->put_function("List", par_names_.size());
  for(std::set<std::string>::const_iterator it=par_names_.begin();it!=par_names_.end();it++){
    ml_->put_symbol(PacketSender::par_prefix + *it);
  }
}

void MathematicaVCSPoint::send_cs_vars() const
{
  int vars_size = constraint_store_.second.size();
  HYDLA_LOGGER_VCS(
    "---- Send Constraint Store Vars -----\n",
    "vars_size: ", vars_size);

  PacketSender ps(*ml_);
  
  ml_->put_function("List", vars_size);

  constraint_store_vars_t::const_iterator it = 
    constraint_store_.second.begin();
  constraint_store_vars_t::const_iterator end = 
    constraint_store_.second.end();
  for(; it!=end; ++it) {
    ps.put_var(*it, PacketSender::VA_None);
  }
}

std::ostream& MathematicaVCSPoint::dump(std::ostream& s) const
{
  s << "#*** Dump MathematicaVCSPoint ***\n"
    << "--- constraint store ---\n";

  // 
  std::set<std::set<MathValue> >::const_iterator or_cons_it = 
    constraint_store_.first.begin();
  while((or_cons_it) != constraint_store_.first.end())
  {
    std::set<MathValue>::const_iterator and_cons_it = 
      (*or_cons_it).begin();
    while((and_cons_it) != (*or_cons_it).end())
    {
      s << (*and_cons_it).get_string() << " ";
      and_cons_it++;
    }
    s << "\n";
    or_cons_it++;
  }

  // 制約ストア内に存在する変数のダンプ
  s << "-- vars --\n";
  constraint_store_vars_t::const_iterator vars_it = 
    constraint_store_.second.begin();
  while((vars_it) != constraint_store_.second.end())
  {
    s << *(vars_it) << "\n";
    vars_it++;
  }

  return s;
}

std::ostream& operator<<(std::ostream& s, const MathematicaVCSPoint& m)
{
  return m.dump(s);
}


} // namespace mathematica
} // namespace simulator
} // namespace hydla 

