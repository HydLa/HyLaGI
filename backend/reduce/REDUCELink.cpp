#include "REDUCELink.h"
#include <sstream>

using namespace std;

namespace hydla{
namespace backend{
namespace reduce{

REDUCELink::REDUCELink(){
  HYDLA_LOGGER_FUNC_BEGIN(BACKEND);
  typedef function_map_t::value_type f_value_t;
  function_map_.insert(f_value_t("ln", "Log"));
  function_map_.insert(f_value_t("resetConstraint", "resetConstraintStore"));
  function_map_.insert(f_value_t("resetConstraint", "resetConstraintStore"));
  function_map_.insert(f_value_t("checkConsistencyPoint", "myCheckConsistencyPoint"));
  function_map_.insert(f_value_t("checkConsistencyInterval", "myCheckConsistencyInterval"));
  function_map_.insert(f_value_t("checkConsistencyPoint", "myCheckConsistencyPoint"));
  function_map_.insert(f_value_t("createVariableMap", "myConvertCSToVM"));
  function_map_.insert(f_value_t("createVariableMapInterval", "convertCSToVMInterval"));
  function_map_.insert(f_value_t("simplify", "simplifyExpr"));

  HYDLA_LOGGER_FUNC_END(BACKEND);
}


void REDUCELink::init_opts(const simulator::Opts &opts)
{
  pre_send();
  HYDLA_LOGGER_LOCATION(BACKEND);
  std::stringstream debug_print_opt_str;
  debug_print_opt_str << "optUseDebugPrint__:=";
  debug_print_opt_str << (opts.debug_mode ? "t" : "nil");
  debug_print_opt_str << ";";
  put_string((debug_print_opt_str.str()).c_str());
  post_receive();
}


void REDUCELink::put_function(const char *s, int n)
{
  // n for arguments, 1 for function name
  arg_cnt_stack_.push(n + 1);
  put_symbol(s);
  post_put();
}

void REDUCELink::put_symbol(const char* s)
{
  put_string(s);
}
void REDUCELink::put_number(const char* value)
{
  put_string(value);
}
void REDUCELink::put_string(const char* s)
{
  send_string_to_reduce(s);
  post_put();
}
void REDUCELink::put_integer(int i)
{
  stringstream sstr;
  sstr << i;
  send_string_to_reduce(sstr.str().c_str());
}

void REDUCELink::post_put()
{
  if(arg_cnt_stack_.empty())
  {
    //TODO: throw exception
  }
  arg_cnt_stack_.top()--;
  if(arg_cnt_stack_.top() <= 0)
  {
    arg_cnt_stack_.pop();
  }
}

std::string REDUCELink::get_debug_print()
{
  return "Debug messages are not implemented";
}

std::string REDUCELink::get_input_print()
{
  return "Input messages are not implemented";
}

void REDUCELink::get_function(std::string &name, int &cnt)
{
}
std::string REDUCELink::get_symbol()
{
}
std::string REDUCELink::get_string()
{
}

int REDUCELink::get_integer()
{
}


int REDUCELink::get_arg_count()
{
}

REDUCELink::DataType REDUCELink::get_type()
{
}

REDUCELink::DataType REDUCELink::get_next()
{
}


void REDUCELink::pre_send()
{
  // reset stack
  arg_cnt_stack_ = std::stack<int>();
  arg_cnt_stack_.push(1);
}

void REDUCELink::pre_receive()
{
}

void REDUCELink::post_receive()
{
}


/*
 TODO: addInitEquation
 addInitVariablesとaddVariables無しで計算できる？
 REDUCE版でaddConstraintとaddguardは区別される？
 set_variable_setでdepend文を作成，送信
 NOT_FUNCが必要？
 initXやdf(),parameterのmapping
 VariableNameEncoder通す
*/
}
}
}
