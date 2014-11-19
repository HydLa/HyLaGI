#include "REDUCELink.h"
#include "reduce_source.h"
#include "Logger.h"
#include "LinkError.h"

using namespace std;
using namespace hydla::parser;

namespace hydla{
namespace backend{
namespace reduce{


const std::string REDUCELink::par_prefix = "parameter_";
const std::string REDUCELink::var_prefix = "usrvar_";


REDUCELink::REDUCELink():sexp_ast_(""){
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
  function_map_.insert(f_value_t("Power", "expt"));
  function_map_.insert(f_value_t("Subtract", "difference"));
  function_map_.insert(f_value_t("Divide", "quotient"));

  function_map_.insert(f_value_t("Unequal", "neq"));
  function_map_.insert(f_value_t("Less", "lessp"));
  function_map_.insert(f_value_t("Greater", "greaterp"));
  function_map_.insert(f_value_t("LessEqual", "leq"));
  function_map_.insert(f_value_t("GreaterEqual", "heq"));
}


void REDUCELink::init_opts(const Opts &opts)
{
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

void REDUCELink::put_pre_list_caller(const std::string &name, int diff_count, const variable_form_t &variable_arg)
{
  // TODO : implement
}

void REDUCELink::put_post_list_caller(int diff_count, const variable_form_t &variable_arg)
{
  // TODO : implement
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
  SExpAST::const_tree_iter_t it = get_current_iter();
  HYDLA_LOGGER_DEBUG_VAR(it);
  name = std::string(it->value.begin(), it->value.end());
  cnt = it->children.size();
  HYDLA_LOGGER_DEBUG("name: ", name, ", cnt: ", cnt);
  go_next_iter();
}

std::string REDUCELink::get_symbol()
{
  return get_string();
}

std::string REDUCELink::get_string()
{
  SExpAST::const_tree_iter_t it = get_current_iter();
  std::string ret = std::string(it->value.begin(), it->value.end());
  HYDLA_LOGGER_DEBUG( "ret: ", ret);
  go_next_iter();
  return ret;
}

int REDUCELink::get_integer()
{
  int ret;
  std::stringstream ss;
  SExpAST::const_tree_iter_t it = get_current_iter();
  std::string str(it->value.begin(), it->value.end());
  ss << str;
  ss >> ret;
  HYDLA_LOGGER_DEBUG("ret: ", ret); 
  go_next_iter();
  return ret;
}


REDUCELink::DataType REDUCELink::get_type()
{
  tree_iter_t it = get_current_iter();
  switch(it->value.id().to_long()) {
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
    if(it->children.size() > 0)
    {
      return DT_FUNC;
    }
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
  go_next_iter();
  return get_type();
}


void REDUCELink::pre_send()
{
  // reset stack
  arg_cnt_stack_ = std::stack<int>();
  send_buffer_ += "symbolic redeval '";
}

REDUCELink::tree_iter_t REDUCELink::get_current_iter()
{
  if(first_get_)
  {
    sexp_ast_ = get_as_s_exp_parse_tree();
    tree_stack_ = tree_stack_t();
    tree_iter_t root = sexp_ast_.root();
    tree_stack_.push(std::make_pair(root, -1));
    first_get_ = false;
    HYDLA_LOGGER_DEBUG_VAR(sexp_ast_);
    return root;
  }
  else
  {
    if(tree_stack_.empty())
    {
      throw LinkError("reduce", "tree stack_ is empty", 0);
      //TODO: throw exception
    }
    int idx = tree_stack_.top().second;
    if(idx == -1)
    {
      return tree_stack_.top().first;
    }
    else
    {
      return (tree_stack_.top().first->children.begin() + idx);
    }
  }
}

void REDUCELink::go_next_iter()
{
  while(!tree_stack_.empty() && ++(tree_stack_.top().second) >= (int)(tree_stack_.top().first->children.size()))
  {
    tree_stack_.pop();
  }
  if(!tree_stack_.empty())
  {
    tree_iter_t it = get_current_iter();
    if(it->children.size() > 0)
    {
      tree_stack_.push(std::make_pair(it, -1));
    }
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
    HYDLA_LOGGER_DEBUG_VAR(send_buffer_);
  }
  else
  {
    HYDLA_LOGGER_DEBUG("send_buffer_: (omitted)");
  }
  send_string_to_reduce(send_buffer_.c_str());
  send_buffer_ = "";
}


/*
 TODO:
 VariableNameEncoder通す
*/
}
}
}
