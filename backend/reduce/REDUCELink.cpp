#include "REDUCELink.h"
#include "reduce_source.h"
#include "Logger.h"
#include <sstream>
#include "sexp/SExpGrammar.h"

using namespace std;
using namespace hydla::parser;

namespace hydla{
namespace backend{
namespace reduce{


const std::string REDUCELink::par_prefix = "parameter";
const std::string REDUCELink::var_prefix = "usrvar";


REDUCELink::REDUCELink():sexp_ast_(""){
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
  function_map_.insert(f_value_t("derivative", "df"));
  HYDLA_LOGGER_FUNC_END(BACKEND);
}


void REDUCELink::init_opts(const simulator::Opts &opts)
{
  HYDLA_LOGGER_LOCATION(BACKEND);
  //pre_send();
  std::stringstream init_str;
  init_str << "optUseDebugPrint__:=";
  init_str << (opts.debug_mode ? "t" : "nil");
  init_str << ";\n" ;
  send_buffer_ += init_str.str();
  flush(false);
  
  send_buffer_ += reduce_source();
  flush(false);
  skip_until_redeval();
}


void REDUCELink::put_function(const char *s, int n)
{
  // n for arguments, 1 for function name
  arg_cnt_stack_.push(n + 1);
  send_buffer_ += "(";
  put_symbol(s);
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
  send_buffer_ += s;
  post_put();
}

void REDUCELink::put_integer(int i)
{
  stringstream sstr;
  sstr << i;
  send_buffer_ += sstr.str();
  post_put();
}

void REDUCELink::post_put()
{
  if(arg_cnt_stack_.empty())
  {
    //TODO: throw exception
  }
  send_buffer_ += " ";

  while(!arg_cnt_stack_.empty() && --(arg_cnt_stack_.top()) <= 0)
  {
    arg_cnt_stack_.pop();
    send_buffer_ += ")";
  }
}



void REDUCELink::put_parameter(const std::string& name, int diff_count, int id)
{
  std::ostringstream oss;
  oss << par_prefix << "_" << name << "_" << diff_count << "_" << id;
  send_buffer_ += oss.str();
  post_put();
}


void REDUCELink::put_variable(const std::string &name, int diff_count, const variable_form_t &variable_arg)
{
  std::ostringstream var_str;
  std::string prefixed_name = var_prefix + name;

  if(variable_arg == VF_ZERO){
    var_str << "init";
    var_str << prefixed_name;
    if(diff_count > 0){
      var_str << "_"
              << diff_count;
    }
    var_str << "lhs";
  }else{
    if (diff_count > 0 && variable_arg == VF_PREV){
      var_str << "(prev (df "
              << prefixed_name
              << " t "    
              << diff_count
              << "))";
    }
    else if (diff_count > 0){
      var_str << "(df "
              << prefixed_name
              << " t "    
              << diff_count
              << ")";
    }
    else if (variable_arg == VF_PREV) {
      var_str << "(prev "
              << prefixed_name
              << ")";
    }
    else {
      var_str << prefixed_name;
    }
  }

  send_buffer_ += var_str.str();
  post_put();
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
  SExpAST::const_tree_iter_t it = get_next_iter();
  name = std::string(it->value.begin(), it->value.end());
  cnt = it->children.size();
  tree_stack_.push(std::make_pair(it, -1));
  HYDLA_LOGGER(BACKEND, "name: ", name, ", cnt: ", cnt);
}

std::string REDUCELink::get_symbol()
{
  return get_string();
}

std::string REDUCELink::get_string()
{
  SExpAST::const_tree_iter_t it = get_next_iter();
  std::string ret = std::string(it->value.begin(), it->value.end());
  HYDLA_LOGGER(BACKEND, "ret: ", ret);
  return ret;
}

int REDUCELink::get_integer()
{
  int relop_code;
  std::stringstream relop_code_ss;
  SExpAST::const_tree_iter_t it = get_next_iter();
  std::string str(it->value.begin(), it->value.end());
  relop_code_ss << str;
  relop_code_ss >> relop_code;
  return relop_code;
}


REDUCELink::DataType REDUCELink::get_type()
{
  switch(tree_stack_.top().first->value.id().to_long()) {
    case SExpGrammar::RI_Number: 
    {
      // TODO: DT_INTを返すべきかどうか検討する
      return DT_STR;
    }

    case SExpGrammar::RI_List:
    {
      return DT_FUNC;
    }
    
    case SExpGrammar::RI_Identifier:
    case SExpGrammar::RI_Header:
    {
      return DT_SYM;
    }

    default:
      // TODO: throw exception
      return DT_NONE;
  }
}

REDUCELink::DataType REDUCELink::get_next()
{
  get_next_iter();  
  return get_type();
}


void REDUCELink::pre_send()
{
  // reset stack
  arg_cnt_stack_ = std::stack<int>();
  send_buffer_ += "symbolic redeval '";
}

REDUCELink::tree_iter_t REDUCELink::get_next_iter()
{
  if(first_get_)
  {
    sexp_ast_ = get_as_s_exp_parse_tree();
    tree_stack_ = tree_stack_t();
    first_get_ = false;
    return sexp_ast_.root();
  }
  else
  {
    while(!tree_stack_.empty() && ++(tree_stack_.top().second) >= (int)(tree_stack_.top().first->children.size()))
    {
      tree_stack_.pop();
    }
    if(tree_stack_.empty())
    {
      //TODO: throw exception
    }
    return (tree_stack_.top().first->children.begin()
            + tree_stack_.top().second);
  }
}


void REDUCELink::pre_receive()
{
  send_buffer_ += ";\n";
  flush();
  skip_until_redeval();
  first_get_ = true;
}

void REDUCELink::post_receive()
{
}

void REDUCELink::flush(bool debug)
{
  if(debug)
  {
    HYDLA_LOGGER(BACKEND, "send_buffer_: ", send_buffer_);
  }
  else
  {
    HYDLA_LOGGER(BACKEND, "send_buffer_: (omitted)");
  }
  send_string_to_reduce(send_buffer_.c_str());
  send_buffer_ = "";
}


/*
 TODO:
 addInitVariablesとaddVariables無しで計算できる？
 set_variable_setでdepend文を作成，送信
 NOT_FUNCが必要？
 VariableNameEncoder通す
*/
}
}
}
