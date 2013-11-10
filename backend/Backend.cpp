#include "Backend.h"
#include <stdarg.h>
#include "LinkError.h"
#include "InterfaceError.h"
#include <sstream>
#include "TreeInfixPrinter.h"

using namespace hydla::parse_tree;
using namespace std;

namespace hydla{
namespace backend{

const std::string Backend::prev_prefix = "prev";
const std::string Backend::par_prefix = "p";
const std::string Backend::var_prefix = "usrVar";


Backend::Backend(Link* link):
link_(link)
{
}

Backend::~Backend()
{
}


Backend::variable_t *Backend::get_variable
(const std::string &name, int derivative_count)const
{
  variable_t variable(name, derivative_count);
  variable_set_t::iterator it = std::find(variable_set_->begin(), variable_set_->end(), variable);
  if(it == variable_set_->end()) return NULL;
  return &(*it);
}

Backend::parameter_t* Backend::get_parameter(const std::string &name, int derivative_count, int id) const
{
  for(parameter_set_t::iterator it = parameter_set_->begin(); it != parameter_set_->end();it++){
    parameter_t& param = *it;
    if(param.get_variable()->get_name() == name && param.get_variable()->get_derivative_count() == derivative_count && param.get_phase()->id == id){
      return &param;
    }
  }
  assert(0);
  return NULL;
}

void Backend::invalid_fmt(const char* fmt, int idx)
{
  std::stringstream sstr;
  sstr << "invalid format \"" << fmt << "\" at " << idx;
  throw InterfaceError(sstr.str().c_str());
}

int Backend::read_args_fmt(const char* args_fmt, const int& idx, void *arg)
{
  int i = idx;
  switch(args_fmt[i])
  {
  case 'i':
  {
    int* num = (int*)arg;
    link_->put_integer(*num);
  }
  break;

  case 's':
  {
    const char* sym = (const char *)arg;
    link_->put_symbol(sym);
  }
  break;

  case 'l':
  {
    value_t * val = (value_t *)arg;
    variable_form_t form;
    if(!get_form(args_fmt[++i], form))
    {
      invalid_fmt(args_fmt, i);
    }
    else
    {
      send_value(*val, form);
    }
    break;
  }

  case 'e':
  {
    node_sptr* node = (node_sptr *)arg;
    variable_form_t form;
    if(!get_form(args_fmt[++i], form))
    {
      invalid_fmt(args_fmt, i);
    }
    else
    {
      send_node(*node, form);
    }
  }
  break;
  case 'd':
    switch(args_fmt[++i])
    {
      
    case 'j':
    {
      variable_form_t form;
      if(!get_form(args_fmt[++i], form))
      {
        invalid_fmt(args_fmt, i);
      }
      else
      {
        disjunction_t *cs = (disjunction_t *)arg;
        link_->put_function("Or", cs->size());
        for(disjunction_t::iterator it = cs->begin(); 
            it != cs->end();
            it++)
        {
          send_node(*it, form);
        }
      }
    }
    break;
    default:
      invalid_fmt(args_fmt, i);
      break;
    }
    break;

  case 'c':
    switch(args_fmt[++i])
    {
    case 'j':
    {
      variable_form_t form;
      if(!get_form(args_fmt[++i], form))
      {
        invalid_fmt(args_fmt, i);
      }
      else
      {
        conjunction_t *cs = (conjunction_t *)arg;
        link_->put_function("And", cs->size());
        for(conjunction_t::iterator it = cs->begin(); 
            it != cs->end();
            it++)
        {
          send_node(*it, form);
        }
      }
    }
    break;
    default:
      invalid_fmt(args_fmt, i);
      break;
    }
    break;
  case 'm':
    switch(args_fmt[++i])
    {
    case 'v':
    {
      variable_map_t* vm = (variable_map_t*)arg;
      bool send_derivative = true;
      char form_char;
      if(args_fmt[++i] == '0')
      {
        send_derivative = false;
        form_char = args_fmt[++i];
      }
      else
      {
        form_char = args_fmt[i];
      }
      variable_form_t form;
      if(!get_form(form_char, form))
      {
        invalid_fmt(args_fmt, i);
        break;
      }
      send_variable_map(*vm, form, send_derivative);
      break;
    }
    case 'p':
    {
      parameter_map_t* pm = (parameter_map_t*)arg;
      send_parameter_map(*pm);
      break;
    }
    default:
      invalid_fmt(args_fmt, i);
      break;
    }
    break;
  case 'p':     
  {
    parameter_t* par = (parameter_t *)arg;
    send_parameter(par->get_name(), par->get_derivative_count(), par->get_phase_id());
  }
  break;

  case 'v':
  {
    variable_t* var = (variable_t*)arg;
    variable_form_t vf;
    if(!get_form(args_fmt[++i], vf))
    {
      invalid_fmt(args_fmt, i);
      break;
    }
    send_variable(var->get_name(), var->get_derivative_count(), vf);
  }
  break;


  default:      
    invalid_fmt(args_fmt, i);
    break;
  }
  return i - idx;
}

int Backend::read_ret_fmt(const char *ret_fmt, const int& idx, void* ret)
{
  int i = idx;
  switch(ret_fmt[i])
  {
  case 'i':
  {
    int* num = (int *)ret;
    *num = link_->get_integer();
  }
  break;

  case 'e':
  {
    node_sptr* node = (node_sptr*)ret;
    variable_form_t form;
    if(!get_form(ret_fmt[++i], form))
    {
      invalid_fmt(ret_fmt, i);
    }
    else
    {
      *node = receive_node();
    }
  }
  break;

  case 'l':
  {
    value_t* val = (value_t *)ret;
    *val = receive_value();
    break;
  }
                   
  case 'm':
    switch(ret_fmt[++i])
    {
    case 'v':
    {
      variable_map_t* vm = (variable_map_t*)ret;
      variable_form_t form;
      if(!get_form(ret_fmt[++i], form))
      {
        invalid_fmt(ret_fmt, i);
        break;
      }
      receive_map(*vm);
      break;
    }
    case 'p':
    {
      parameter_map_t* pm = (parameter_map_t*)ret;
      receive_parameter_map(*pm);
      break;
    }
    default:
      invalid_fmt(ret_fmt, i);
      break;
    }
    break;
  case 'c':
    switch(ret_fmt[++i])
    {
    case 'c':
    {
      check_consistency_result_t* cc = (check_consistency_result_t *)ret;
      *cc = receive_cc();
      break;
    }
    case 'v':
    {
      create_vm_t* cv = (create_vm_t*)ret;
      *cv = receive_cv();
      break;
    }
    case 'p':
    {
      pp_time_result_t* cp = (pp_time_result_t*)ret;
      *cp = receive_cp();
      break;
    }
    default:
      invalid_fmt(ret_fmt, i);     
      break;
    }
    break;

  default:      
    invalid_fmt(ret_fmt, i);
    break;
  }
  return i - idx;
}

int Backend::call(const char* name, const int& arg_cnt, const char* args_fmt, const char* ret_fmt, ...)
{
  HYDLA_LOGGER_FUNC_BEGIN(BACKEND);
  HYDLA_LOGGER_BACKEND("%%name: ",  name, 
                   ", arg_cnt: ", arg_cnt,
                   ", args_fmt: ", args_fmt,
                   ", ret_fmt: ", ret_fmt);
  link_->pre_send();
  std::string converted_name;
  if(link_->convert(name, true, converted_name))
  {
    link_->put_function(converted_name, arg_cnt);
  }
  else
  {
    link_->put_function(name, arg_cnt);
  }
  va_list args;
  va_start(args, ret_fmt);
  for(int i = 0; args_fmt[i] != '\0'; i++)
  {
    void* arg = va_arg(args, void *);
    i += read_args_fmt(args_fmt, i, arg);
  }
  link_->pre_receive();
  for(int i = 0; ret_fmt[i] != '\0'; i++)
  {
    void* ret = va_arg(args, void *);
    i += read_ret_fmt(ret_fmt, i, ret);
  }
  link_->post_receive();

  // TODO: 例外投げた場合もva_endを呼び出すように
  va_end(args);
  HYDLA_LOGGER_FUNC_END(BACKEND);
  return 0;
}

bool Backend::get_form(const char &form_c, variable_form_t &form)
{
  switch(form_c)
  {
  case 'p':
    form = VF_PREV;
    return true;
  case 'n':
    form = VF_NONE;
    return true;
  case 'z':
    form = VF_ZERO;
    return true;
  case 't':
    form = VF_TIME;
    return true;
  default:
    return false;
  }
}

int Backend::send_node(const node_sptr& node, const variable_form_t &form)
{
  HYDLA_LOGGER_FUNC_BEGIN(BACKEND);
  HYDLA_LOGGER_BACKEND("%%node: ", TreeInfixPrinter().get_infix_string(node));
  differential_count_ = 0;
  in_prev_ = false;
  variable_arg_ = form;
  accept(node);
  HYDLA_LOGGER_FUNC_END(BACKEND);
  return 0;
}

int Backend::send_variable_map(const variable_map_t& vm, const variable_form_t& vf, const bool &send_derivative)
{
  int size_to_sent = 0;
  for(variable_map_t::const_iterator it = vm.begin(); it != vm.end(); it++)
  {
    const variable_t *const var = it->first;
    const value_range_t &range = it->second;
    if(!send_derivative && var->get_derivative_count() > 0 )continue;
    if(range.unique())
    {
      size_to_sent++;
    }
    else
    {
      size_to_sent += range.get_lower_cnt();
      size_to_sent += range.get_upper_cnt();
    }
  }
  link_->put_function("And", size_to_sent);
  for(variable_map_t::const_iterator it = vm.begin(); it != vm.end(); it++)
  {
    const variable_t *const var = it->first;
    const value_range_t &range = it->second;
    if(!send_derivative && var->get_derivative_count() > 0 )continue;
    if(range.unique())
    {
      link_->put_function("Equal", 2);
      send_variable(*var, vf);
      send_value(range.get_unique(), vf);
    }
    else
    {
      for(uint i = 0; i < range.get_lower_cnt(); i++)
      {
        const value_range_t::bound_t &bnd = range.get_lower_bound(i);
        if(bnd.include_bound)
        {
          link_->put_function("GreaterEqual", 2);
        }
        else
        {
          link_->put_function("Greater", 2);
        }
        send_variable(*var, vf);
        send_value(bnd.value, vf);
      }

      for(uint i = 0; i < range.get_upper_cnt(); i++)
      {
        const value_range_t::bound_t &bnd = range.get_upper_bound(i);
        if(bnd.include_bound)
        {
          link_->put_function("LessEqual", 2);
        }
        else
        {
          link_->put_function("Less", 2);
        }
        send_variable(*var, vf);
        send_value(bnd.value, vf);
      }
      
    }
  }
  return 0;
}

int Backend::send_parameter_map(const parameter_map_t& parameter_map)
{
  parameter_map_t::const_iterator it = parameter_map.begin();
  int size=0;
  for(; it!=parameter_map.end(); ++it)
  {
    const value_range_t &range = it->second;
    if(range.unique()){
      size++;
    }else{
      size += range.get_lower_cnt() + range.get_upper_cnt();
    }
  }
  
  link_->put_function("And", size);
  it = parameter_map.begin();
  for(; it!=parameter_map.end(); ++it)
  {
    if(it->second.unique()){
      const value_t &value = it->second.get_lower_bound().value;
      parameter_t& param = *it->first;
      link_->put_function("Equal", 2);
      send_parameter(param.get_name(), param.get_derivative_count(), param.get_phase_id());
      send_value(value, VF_PREV);
    }else{
      for(uint i=0; i < it->second.get_lower_cnt();i++)
      {
        const value_range_t::bound_t &bnd = it->second.get_lower_bound(i);
        const value_t &value = bnd.value;
        parameter_t& param = *it->first;
        if(!bnd.include_bound)
        {
          link_->put_function("Greater", 2);
        }
        else
        {
          link_->put_function("GreaterEqual", 2);
        }
        send_parameter(param.get_name(), param.get_derivative_count(), param.get_phase_id());
        send_value(value, VF_PREV);
      }
      for(uint i=0; i < it->second.get_upper_cnt();i++)
      {
        const value_range_t::bound_t &bnd = it->second.get_upper_bound(i);
        const value_t &value = bnd.value;
        parameter_t& param = *it->first;
        if(!bnd.include_bound)
        {
          link_->put_function("Less", 2);
        }
        else
        {
          link_->put_function("LessEqual", 2);
        }
        send_parameter(param.get_name(), param.get_derivative_count(), param.get_phase_id());
        send_value(value, VF_PREV);
      }
    }
  }
  return 0;
}

void Backend::visit(boost::shared_ptr<Ask> node)                   
{
  throw InterfaceError("ask node cannot be sent to backend");
}

void Backend::visit(boost::shared_ptr<Tell> node)
{
  throw InterfaceError("Tell node cannot be sent to backend");
}

#define DEFINE_VISIT_BINARY(NODE_NAME, FUNC_NAME)                       \
void Backend::visit(boost::shared_ptr<NODE_NAME> node)        \
{                                                                       \
  HYDLA_LOGGER_REST("put:" #NODE_NAME);                                 \
  link_->put_function(#FUNC_NAME, 2);                                  \
  accept(node->get_lhs());                                              \
  accept(node->get_rhs());                                              \
}

#define DEFINE_VISIT_UNARY(NODE_NAME, FUNC_NAME)                        \
void Backend::visit(boost::shared_ptr<NODE_NAME> node)        \
{                                                                       \
  HYDLA_LOGGER_REST("put:" #NODE_NAME);                                 \
  link_->put_function(#FUNC_NAME, 1);                                  \
  accept(node->get_child());                                            \
}

#define DEFINE_VISIT_FACTOR(NODE_NAME, FUNC_NAME)                       \
void Backend::visit(boost::shared_ptr<NODE_NAME> node)        \
{                                                                       \
  HYDLA_LOGGER_REST("put:" #NODE_NAME);                                 \
  link_->put_symbol(#FUNC_NAME);                                       \
}

DEFINE_VISIT_BINARY(Equal, Equal)
DEFINE_VISIT_BINARY(UnEqual, Unequal)
DEFINE_VISIT_BINARY(Less, Less)
DEFINE_VISIT_BINARY(LessEqual, LessEqual)
DEFINE_VISIT_BINARY(Greater, Greater)
DEFINE_VISIT_BINARY(GreaterEqual, GreaterEqual)

/// 論理演算子
DEFINE_VISIT_BINARY(LogicalAnd, And)
DEFINE_VISIT_BINARY(LogicalOr, Or)
  
/// 算術二項演算子
DEFINE_VISIT_BINARY(Plus, Plus)
DEFINE_VISIT_BINARY(Subtract, Subtract)
DEFINE_VISIT_BINARY(Times, Times)
DEFINE_VISIT_BINARY(Divide, Divide)
DEFINE_VISIT_BINARY(Power, Power)

  
/// 算術単項演算子

DEFINE_VISIT_UNARY(Negative, Minus)
void Backend::visit(boost::shared_ptr<Positive> node)              
{
  accept(node->get_child());
}

/// 微分
void Backend::visit(boost::shared_ptr<Differential> node)          
{
  differential_count_++;
  accept(node->get_child());
  differential_count_--;
}

/// 左極限
void Backend::visit(boost::shared_ptr<Previous> node)              
{
  in_prev_ = true;
  accept(node->get_child());
  in_prev_ = false;
}


/// 否定
DEFINE_VISIT_UNARY(Not, Not)


/// 関数
void Backend::visit(boost::shared_ptr<Function> node)              
{
  string name;
  int arg_cnt = node->get_arguments_size();
  if(!link_->convert(node->get_string(), true, name))
  {
    throw InterfaceError(node->get_string() + " is not suppported in " + link_->backend_name());
  }
  link_->put_function(name.c_str(), arg_cnt);
  for(int i=0; i < arg_cnt;i++){
    accept(node->get_argument(i));
  }
}

void Backend::visit(boost::shared_ptr<UnsupportedFunction> node)              
{
  link_->put_function(node->get_string().c_str(), node->get_arguments_size());
  for(int i=0; i<node->get_arguments_size();i++){
    accept(node->get_argument(i));
  }
}

/// 円周率
DEFINE_VISIT_FACTOR(Infinity, Infinity)
/// 円周率
DEFINE_VISIT_FACTOR(Pi, Pi)
/// 自然対数の底
DEFINE_VISIT_FACTOR(E, E)

// 変数
void Backend::visit(boost::shared_ptr<hydla::parse_tree::Variable> node)              
{
  // 変数の送信
  variable_form_t va;
  if(variable_arg_== VF_NONE && in_prev_)
  {
    va = VF_PREV;
  }
  else{
    va = variable_arg_;
  }

  send_variable(node->get_name(), differential_count_, va);
}

// 数字
void Backend::visit(boost::shared_ptr<Number> node)                
{
  // link_->put_integer(atoi(node->get_number().c_str())); //数値がでかいとオーバーフローする
  // link_->put_symbol(node->get_number().c_str()); // put_symbolだと送れない
  link_->put_number(node->get_number().c_str());
}


// 記号定数
void Backend::visit(boost::shared_ptr<Parameter> node)
{
  send_parameter(node->get_name(), node->get_derivative_count(), node->get_phase_id());
}

// t
void Backend::visit(boost::shared_ptr<SymbolicT> node)                
{    
  link_->put_symbol("t");
}


int Backend::send_value(const value_t &val, const variable_form_t& var){
  variable_arg_ = var;
  val->accept(*this);
  return 0;
}


void Backend::visit(hydla::simulator::symbolic::SymbolicValue& value){
  send_node(value.get_node(), variable_arg_);
}
 
int Backend::send_parameter(const std::string& name, const int &diff_count, const int& id)
{
  link_->put_function(par_prefix.c_str(), 3);
  link_->put_symbol(name.c_str());
  link_->put_integer(diff_count);
  link_->put_integer(id);
  return 0;
}



int Backend::send_variable(const variable_t &var, const variable_form_t &variable_arg)
{
  return send_variable(var.get_name(), var.get_derivative_count(), variable_arg);
}


int Backend::send_variable(const std::string& name, const int &diff_count, const variable_form_t &variable_arg)
{
  if(variable_arg == VF_PREV){
    link_->put_function(prev_prefix.c_str(), 2);
    link_->put_symbol(name);
    link_->put_integer(diff_count);
  }else{
    if(variable_arg == VF_NONE)
    {
      link_->put_function("derivative", 2);
      link_->put_integer(diff_count);
      link_->put_symbol(var_prefix + name);
    }
    else
    {
      link_->put_function("derivative", 3);
      link_->put_integer(diff_count);
      link_->put_symbol(var_prefix + name);
      if(variable_arg == VF_TIME)
      {
        link_->put_symbol("t");
      }
      else
      {
        link_->put_integer(0);
      }
    }
  }
  return 0;
}


// コマンド文
void Backend::visit(boost::shared_ptr<hydla::parse_tree::PrintPP> node){link_->put_symbol("True");}
void Backend::visit(boost::shared_ptr<hydla::parse_tree::PrintIP> node){link_->put_symbol("True");}
void Backend::visit(boost::shared_ptr<hydla::parse_tree::Scan> node){link_->put_symbol("True");}


void Backend::set_range(const value_t &val, value_range_t &range, const int& relop){
  switch(relop){
    case 0://Equal
    range.set_unique(val);
    break;
    case 1://Less
    range.set_upper_bound(val, false);
    break;
    case 2://Greater
    range.set_lower_bound(val, false);
    break;
    case 3://LessEqual
    range.set_upper_bound(val, true);
    break;
    case 4://GreaterEqual
    range.set_lower_bound(val, true);
    break;
  }
}

Backend::create_vm_t Backend::receive_cv()
{
  create_vm_t ret;
  std::string name;
  int cnt;
//  link_->check();
  link_->get_function(name, cnt);
  for(int i = 0; i < cnt; i++)
  {
    variable_map_t map; receive_map(map);
    ret.push_back(map);
  }
  return ret;
}

Backend::pp_time_result_t Backend::receive_cp()
{

  HYDLA_LOGGER_FUNC_BEGIN(BACKEND);
  std::string name;
  int next_time_size; 
  link_->get_function(name, next_time_size);
  pp_time_result_t result;
  for(int time_it = 0; time_it < next_time_size; time_it++){
    pp_time_result_t::candidate_t candidate;
    int dummy_buf;
    link_->get_function(name, dummy_buf);
    // 時刻を受け取る
    candidate.time = receive_value();
    // 条件を受け取る
    receive_parameter_map(candidate.parameter_map);
    HYDLA_LOGGER_LOCATION(BACKEND);
    HYDLA_LOGGER_LOCATION(BACKEND);
    // 終了時刻かどうかを受け取る
    int is_max_time = link_->get_integer();
    HYDLA_LOGGER_LOCATION(BACKEND);
    candidate.is_max_time = (bool)(is_max_time != 0);
    result.candidates.push_back(candidate);
  }

  HYDLA_LOGGER_FUNC_END(BACKEND);
  return result;
}

Backend::check_consistency_result_t Backend::receive_cc()
{
  HYDLA_LOGGER_FUNC_BEGIN(BACKEND);
  check_consistency_result_t ret;
  std::string outer_name;
  int outer_cnt;
  link_->get_function(outer_name, outer_cnt);
  for(int i = 0; i < outer_cnt; i++)
  {
    std::string inner_name;
    int inner_cnt;
    vector<parameter_map_t> maps;
    Link::DataType dt = link_->get_type();
    if(dt == Link::DT_SYM)
    {
      HYDLA_LOGGER_LOCATION(BACKEND);
      string sym = link_->get_symbol();
      if(sym == "True")
      {
        maps.push_back(parameter_map_t());
      }
    }
    else if(dt == Link::DT_FUNC)
    {
      HYDLA_LOGGER_LOCATION(BACKEND);
      link_->get_function(inner_name, inner_cnt);
      for(int j = 0; j < inner_cnt; j++)
      {
        parameter_map_t map;
        receive_parameter_map(map);
        maps.push_back(map);
      }
    }
    else
    {
      HYDLA_LOGGER_LOCATION(BACKEND);
      assert(0);
    }
    ret.push_back(maps);
  }
  HYDLA_LOGGER_FUNC_END(BACKEND);
  return ret;
}

Backend::node_sptr Backend::receive_function()
{
  HYDLA_LOGGER_FUNC_BEGIN(BACKEND);
// TODO: UnsupportedFunctionを含む関数は，バックエンドを切り替えられないので各Valueごとにそのことを示すフラグを持たせた方が良いかも
  int arg_count;
  node_sptr ret;
  std::string symbol;
  link_->get_function(symbol, arg_count);
  if(symbol == "Sqrt"){//1引数関数
    ret = node_sptr(new hydla::parse_tree::Power(receive_node(), node_sptr(new hydla::parse_tree::Number("1/2"))));
  }
  else if(symbol == "parameter"){
    std::string name;
    name = link_->get_symbol();
    std::string d_str;
    d_str = link_->get_string();
    int derivative_count = boost::lexical_cast<int, std::string>(d_str);
    std::string id_str;
    id_str = link_->get_string();
    int id = boost::lexical_cast<int, std::string>(id_str);
    ret = node_sptr(new hydla::parse_tree::Parameter(name, derivative_count, id));
  }
  else if(symbol == "prev"){
    std::string name;
    name = link_->get_symbol();
    std::string d_str = link_->get_string();
    int derivative_count = boost::lexical_cast<int, std::string>(d_str);
    hydla::parse_tree::node_sptr tmp_var = node_sptr(new hydla::parse_tree::Variable(name));
    for(int i = 0; i < derivative_count; i++) tmp_var = node_sptr(new hydla::parse_tree::Differential(tmp_var));
    ret = node_sptr(new hydla::parse_tree::Previous(tmp_var));
  }
  else if(symbol == "minus"){
    ret = node_sptr(new hydla::parse_tree::Negative(receive_node()));
  }
  else if(symbol == "Plus" 
          || symbol == "Subtract"
          || symbol == "Times"
          || symbol == "Divide"
          || symbol == "Power"
          || symbol == "Rational"
          || symbol == "And"
          || symbol == "Or"
          || symbol == "Equal"
          || symbol == "Unequal"
          || symbol == "Less"
          || symbol == "LessEqual"
          || symbol == "Greater"
          || symbol == "GreaterEqual")        
  { // 加減乗除など，二項演算子で書かれる関数
    node_sptr lhs, rhs;
    ret = receive_node();
    for(int arg_it=1;arg_it<arg_count;arg_it++){
      lhs = ret;
      rhs = receive_node();
      if(symbol == "Plus")
        ret = node_sptr(new hydla::parse_tree::Plus(lhs, rhs));
      else if(symbol == "Subtract")
        ret = node_sptr(new hydla::parse_tree::Subtract(lhs, rhs));
      else if(symbol == "Times")
        ret = node_sptr(new hydla::parse_tree::Times(lhs, rhs));
      else if(symbol == "Divide")
        ret = node_sptr(new hydla::parse_tree::Divide(lhs, rhs));
      else if(symbol == "Power")
        ret = node_sptr(new hydla::parse_tree::Power(lhs, rhs));
      else if(symbol == "Rational")
        ret = node_sptr(new hydla::parse_tree::Divide(lhs, rhs));
      else if(symbol == "And")
        ret = node_sptr(new hydla::parse_tree::LogicalAnd(lhs, rhs));
      else if(symbol == "Or")
        ret = node_sptr(new hydla::parse_tree::LogicalOr(lhs, rhs));
      else if(symbol == "Equal")
        ret = node_sptr(new hydla::parse_tree::Equal(lhs, rhs));
      else if(symbol == "Unequal")
        ret = node_sptr(new hydla::parse_tree::UnEqual(lhs, rhs));
      else if(symbol == "Less")
        ret = node_sptr(new hydla::parse_tree::Less(lhs, rhs));
      else if(symbol == "LessEqual")
        ret = node_sptr(new hydla::parse_tree::LessEqual(lhs, rhs));
      else if(symbol == "Greater")
        ret = node_sptr(new hydla::parse_tree::Greater(lhs, rhs));
      else if(symbol == "GreaterEqual")
        ret = node_sptr(new hydla::parse_tree::GreaterEqual(lhs, rhs));
    }
  }
  else if(symbol == "derivative")
  {
    std::string name = link_->get_symbol();
    assert(name == "Derivative");
    std::string d_str = link_->get_string();
    int variable_derivative_count = boost::lexical_cast<int, std::string>(d_str.c_str());
    std::string variable_name = link_->get_symbol();
    if(variable_name.length() < var_prefix.length())invalid_ret();
    assert(variable_name.substr(0, var_prefix.length()) == var_prefix);
    variable_name = variable_name.substr(var_prefix.length());
    ret = node_sptr(new hydla::parse_tree::Variable(variable_name));
    for(int i = 0; i < variable_derivative_count; i++)
    {
      ret = node_sptr(new hydla::parse_tree::Differential(ret));
    } 
  }
  else{
    // その他の関数
    boost::shared_ptr<hydla::parse_tree::ArbitraryNode> f;
    std::string name;
    if(link_->convert(symbol, false, name))
    {
      // 対応している関数
      f.reset(new hydla::parse_tree::Function(name));

    }
    else{
      // 謎の関数
      f.reset(new hydla::parse_tree::UnsupportedFunction(symbol));
    }

    for(int arg_it=0; arg_it < arg_count; arg_it++){
      f->add_argument(receive_node());
    }
    ret = f;
  }
  
  HYDLA_LOGGER_FUNC_END(BACKEND);
  return ret;
}

Backend::value_t Backend::receive_value()
{
  value_t val(new hydla::simulator::symbolic::SymbolicValue(receive_node()));
  HYDLA_LOGGER_LOCATION(BACKEND);
  HYDLA_LOGGER_BACKEND("%% val: ", *val);
  return val;
}

Backend::node_sptr Backend::receive_node(){
  HYDLA_LOGGER_FUNC_BEGIN(BACKEND);
  node_sptr ret;
  Link::DataType type = link_->get_type();
  switch(type){
  case Link::DT_STR: // 文字列
    {
      HYDLA_LOGGER_LOCATION(REST);
      std::string str = link_->get_string();
      ret = node_sptr(new hydla::parse_tree::Number(str));
      break;
    }
  case Link::DT_SYM: // シンボル（記号）
    {
      HYDLA_LOGGER_LOCATION(REST);
      std::string symbol = link_->get_symbol();
      if(symbol=="t")
        ret = node_sptr(new hydla::parse_tree::SymbolicT());
      else if(symbol=="Pi")
        ret = node_sptr(new hydla::parse_tree::Pi());
      else if(symbol=="E")
        ret = node_sptr(new hydla::parse_tree::E());
      else if(symbol=="inf")
        ret = node_sptr(new hydla::parse_tree::Infinity());
      else if(symbol.length() > var_prefix.length() && symbol.substr(0, var_prefix.length()) == var_prefix)
        ret = node_sptr(new hydla::parse_tree::Variable(symbol.substr(var_prefix.length())));
      break;
    }
  case Link::DT_INT: // オーバーフローする可能性があるなら文字列使う
    {
      HYDLA_LOGGER_LOCATION(REST);
      std::stringstream sstr;
      int num = link_->get_integer();
      sstr << num;
      ret = node_sptr(new hydla::parse_tree::Number(sstr.str() ) );
      break;
    }
  case Link::DT_FUNC: // 合成関数
      ret = receive_function();
      break;

    default:
      break;
  }
  if(ret == NULL){
    invalid_ret();
  }
  HYDLA_LOGGER_FUNC_END(BACKEND);
  return ret;
}


void Backend::invalid_ret()
{
  throw InterfaceError("invalid return value. \ninput:\n" + link_->get_input_print() + "\n\ntrace:\n" + link_->get_debug_print());
}

int Backend::receive_map(variable_map_t& map)
{
  value_t symbolic_value;
  std::string f_name;
  int and_size, size;
  link_->get_function(f_name, and_size);
  for(int i = 0; i < and_size; i++)
  {
    //{{変数名，微分回数}, 関係演算子コード，数式}で来るはず
    link_->get_function(f_name, size);  //List
    link_->get_function(f_name, size); //List
    std::string variable_name = link_->get_symbol();
    int d_cnt = link_->get_integer();
    // 関係演算子のコード
    int rel = link_->get_integer();
        
    symbolic_value = value_t(new hydla::simulator::symbolic::SymbolicValue(receive_node()));
    HYDLA_LOGGER_BACKEND("%% received: ", *symbolic_value);

    // TODO: ↓の一行消す
    if(variable_name == "t")continue;
    variable_t* variable_ptr = get_variable(variable_name.substr(var_prefix.length()), d_cnt);
    if(!variable_ptr){
      continue;
    }
    value_range_t tmp_range = map[variable_ptr];
    set_range(symbolic_value, tmp_range, rel);
    if(symbolic_value->undefined()){
      throw InterfaceError("invalid value");
    }
    map[variable_ptr] = tmp_range;  
  }
  return 0;
}

int Backend::receive_parameter_map(parameter_map_t& map)
{
  HYDLA_LOGGER_FUNC_BEGIN(BACKEND);
  string func_name;
  int condition_size; link_->get_function(func_name, condition_size);
  for(int cond_it = 0; cond_it < condition_size; cond_it++){
    link_->get_next(); link_->get_next(); // 先頭要素のparameterを読み飛ばす
    std::string name = link_->get_symbol();
    int derivative_count = link_->get_integer();
    int id = link_->get_integer();
    parameter_t* tmp_param = get_parameter(name, derivative_count, id);
    if(tmp_param == NULL){
      throw InterfaceError("some unknown form of result at receive_parameter_map");
    }
    value_range_t tmp_range = map[tmp_param];
    int relop_code = link_->get_integer();
    value_t tmp_value = value_t(new hydla::simulator::symbolic::SymbolicValue(receive_node()));
    set_range(tmp_value, tmp_range, relop_code);
    map[tmp_param] = tmp_range;
  }
  HYDLA_LOGGER_FUNC_END(BACKEND);
  return 0;
}


}
}
