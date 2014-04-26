#include "AffineTransformer.h"
#include <boost/lexical_cast.hpp>
#include "TreeInfixPrinter.h"
#include <exception>
#include "Backend.h"
#include "Logger.h"

using namespace std;
using namespace hydla::symbolic_expression;
using namespace boost;
using namespace hydla::simulator;

#define HYDLA_LOGGER_NODE_VAR \
  HYDLA_LOGGER_DEBUG("node: ", node->get_node_type_name(), ", current_val: ", current_val_)

namespace hydla {
namespace interval {


class ApproximateException:public std::runtime_error{
public:
  ApproximateException(const std::string& msg):
    std::runtime_error("error occurred in approximation: " + msg){}
};

AffineTransformer* AffineTransformer::affine_translator_ = NULL;

AffineTransformer* AffineTransformer::get_instance()
{
  if(affine_translator_ == NULL)affine_translator_ = new AffineTransformer();
  return affine_translator_;
}

AffineTransformer::AffineTransformer(): epsilon_index(0)
{}

AffineTransformer::~AffineTransformer()
{}

void AffineTransformer::set_simulator(Simulator* simulator)
{
  simulator_ = simulator;
}

affine_t AffineTransformer::pow(affine_t affine, int exp)
{
  bool is_negative = exp < 0;
  exp = is_negative?-exp:exp;
  affine_t power_val(1);
  while(exp != 0)
  {
    if(exp & 1)
    {
      power_val = power_val * affine;
    }
    affine *= affine;
    exp /= 2;
  }
  return power_val;
}

AffineOrInteger AffineTransformer::pow(AffineOrInteger x, AffineOrInteger y)
{
  AffineOrInteger ret;
  if(x.is_integer && y.is_integer)
  {
    ret.is_integer = true;
    ret.integer = ::pow(x.integer, y.integer);
  }
  else
  {
    ret.is_integer = false;
    affine_t x_affine, y_affine;
    if(x.is_integer)x_affine = x.integer;
    else x_affine = x.affine_value;
    if(y.is_integer)y_affine = y.integer;
    else y_affine = y.affine_value;
    kv::interval<double> itv = to_interval(x_affine);
    double l = itv.lower(), u = itv.upper();
    if(u >= 0 && l >= 0)
    {
      HYDLA_LOGGER_DEBUG(x_affine);
      HYDLA_LOGGER_DEBUG(y_affine);
      ret.affine_value = exp(y_affine * log(x_affine));
    }
    else
    {
      if(!y.is_integer)
      {
          HYDLA_LOGGER_DEBUG("l: ", l, ", u: ", u);
          throw ApproximateException("noninteger power function for interval including zero");
      }
      if(u >= 0)
      {
        // include zero
        ret.affine_value = pow(x_affine, y.integer);
        HYDLA_LOGGER_DEBUG_VAR(ret.affine_value);
      }
      else
      {
        // pure negative interval
        if(y.integer % 2)
        {
          ret.affine_value = -exp(y_affine * log(-x_affine));
        }
        else
        {
          ret.affine_value = exp(y_affine * log(-x_affine));
        }
      }
    }
  }
  return ret;
}

void AffineTransformer::reduce_dummy_variables(ub::vector<affine_t> &formulas, int limit)
{
  std::map<int, int> index_map = kv::epsilon_reduce(formulas, limit);
  if(!index_map.empty())
  {
    parameter_idx_map_.clear();
    for(auto pair : index_map)
    {
      HYDLA_LOGGER_DEBUG_VAR(pair.first);
      HYDLA_LOGGER_DEBUG_VAR(pair.second);
      parameter_idx_map_t::right_iterator r_it = parameter_idx_map_.right.find(pair.first);
      if(r_it == parameter_idx_map_.right.end())continue;
      if(pair.second == -1)
      {
        parameter_idx_map_.right.erase(r_it);
      }
      else
      {
        parameter_idx_map_.right.replace_key(r_it, pair.second);
      }
    }
  }
}

value_t AffineTransformer::transform(symbolic_expression::node_sptr& node, parameter_map_t &parameter_map)
{
  accept(node);
  if(current_val_.is_integer)return value_t(current_val_.integer);

  affine_t affine_value = current_val_.affine_value;
  // 試験的にダミー変数の削減をしてみる
  ub::vector<affine_t> formulas(1);
  formulas(0) = affine_value;
  reduce_dummy_variables(formulas, 2);
  affine_value = formulas(0);  
  HYDLA_LOGGER_DEBUG_VAR(affine_t::maxnum());
  HYDLA_LOGGER_DEBUG_VAR(affine_value);
  // set rounding mode
  kv::hwround::roundup();


  value_t ret(affine_value.a(0));
  simulator_->backend->call("transformToRational", 1, "vln", "vl", &ret, &ret);
  double sum = 0;
  int available_index;
  for(int i = 1; i < affine_value.a.size(); i++)
  {
    if(affine_value.a(i) == 0)continue;
    if(parameter_idx_map_.right.find(i)
       == parameter_idx_map_.right.end())
    {
      // 新規に追加されるダミー変数は1つにまとめる（この時点では他の変数と関係を持っていないため）
      // TODO: kvライブラリ内のダミー変数も削除する？結果に誤りは生まれないはずだが、インデックスが無駄に増える。
      sum += affine_value.a(i);
      available_index = i;
    }
    else
    {
      parameter_idx_map_t::right_iterator r_it = parameter_idx_map_.right.find(i);
      // convert floating point into rational
      value_t val = value_t(affine_value.a(i));
      simulator_->backend->call("transformToRational", 1, "vln", "vl", &val, &val);
      ret = ret + val * value_t(r_it->second);
    }
  }
  if(sum != 0)
  {
    range_t range;
    range.set_lower_bound(value_t("-1"), true);
    range.set_upper_bound(value_t("1"), true);
    // parameters whose differential counts are -1 are regarded as dummy variables
    parameter_t param = simulator_->introduce_parameter("affine", -1, ++epsilon_index, range);
    parameter_idx_map_.insert(parameter_idx_t(param, available_index));
    parameter_map[param] = range_t(value_t(-1), value_t(1));
    value_t val = value_t(sum);
    simulator_->backend->call("transformToRational", 1, "vln", "vl", &val, &val);
    ret = ret + val * value_t(param);
  }

  // reset rounding mode
  kv::hwround::roundnear();


  kv::interval<double> itv = to_interval(affine_value);
  HYDLA_LOGGER_DEBUG_VAR(itv);

  return ret;
}

void AffineTransformer::visit(boost::shared_ptr<hydla::symbolic_expression::Plus> node)
{
  accept(node->get_lhs());
  AffineOrInteger lhs = current_val_;
  accept(node->get_rhs());
  AffineOrInteger rhs = current_val_;
  current_val_ = lhs + rhs;
  HYDLA_LOGGER_NODE_VAR;
  return;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::symbolic_expression::Subtract> node)
{
  accept(node->get_lhs());
  AffineOrInteger lhs = current_val_;
  accept(node->get_rhs());
  AffineOrInteger rhs = current_val_;
  current_val_ = lhs - rhs;
  HYDLA_LOGGER_NODE_VAR;
  return;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::symbolic_expression::Times> node)
{
  accept(node->get_lhs());
  AffineOrInteger lhs = current_val_;
  accept(node->get_rhs());
  AffineOrInteger rhs = current_val_;
  current_val_ = lhs * rhs;
  HYDLA_LOGGER_NODE_VAR;
  return;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::symbolic_expression::Divide> node)
{
  accept(node->get_lhs());
  AffineOrInteger lhs = current_val_;
  accept(node->get_rhs());
  AffineOrInteger rhs = current_val_;
  current_val_ = lhs / rhs;
  HYDLA_LOGGER_NODE_VAR;
  return;
}


AffineOrInteger sqrt(const AffineOrInteger &a)
{
  affine_t affine_value;
  if(a.is_integer)
  {
    affine_value = a.integer;
  }
  else
  {
    affine_value = a.affine_value;
  }
  affine_value = sqrt(affine_value);
  AffineOrInteger result_ai;
  result_ai.is_integer = false;
  result_ai.affine_value = affine_value;
  return result_ai;
}

void AffineTransformer::visit(boost::shared_ptr<hydla::symbolic_expression::Power> node)
{
  accept(node->get_lhs());
  AffineOrInteger lhs = current_val_;  
  // TODO: 文字列以外で判定する
  if(get_infix_string(node->get_rhs())=="1/2")
  {
    current_val_ = sqrt(lhs);
  }
  else
  {
    accept(node->get_rhs());
    AffineOrInteger rhs = current_val_;
    current_val_ = pow(lhs, rhs);
  }
  HYDLA_LOGGER_NODE_VAR;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::symbolic_expression::Negative> node)
{
  accept(node->get_child());
  current_val_ = -current_val_;
  HYDLA_LOGGER_NODE_VAR;
  return;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::symbolic_expression::Positive> node)
{
  // do nothing
  return;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::symbolic_expression::Pi> node)
{
  current_val_.affine_value = kv::constants<double>::pi();
  current_val_.is_integer = false;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::symbolic_expression::E> node)
{
  current_val_.affine_value = kv::constants<double>::e();
  current_val_.is_integer = false;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::symbolic_expression::Number> node)
{
  std::string number_str = node->get_number();
  HYDLA_LOGGER_DEBUG(number_str);

  // try translation to int
  try{
    int integer = lexical_cast<int>(number_str);
    current_val_.is_integer = true;
    current_val_.integer = integer;
    return;
  }catch(const bad_lexical_cast &e){
    HYDLA_LOGGER_DEBUG("name: ",
                 typeid(e).name(), "\n what: ", e.what());
  }

  //try approximation as double with upper rounding
  kv::interval<double> itv = kv::interval<double>(number_str);
  current_val_.affine_value = affine_t(itv);
  current_val_.is_integer = false;
  HYDLA_LOGGER_DEBUG(current_val_);
}


void AffineTransformer::visit(boost::shared_ptr<hydla::symbolic_expression::Float> node)
{
  current_val_.affine_value = affine_t(node->get_number());
  current_val_.is_integer = false;
  HYDLA_LOGGER_NODE_VAR;
}

void AffineTransformer::visit(boost::shared_ptr<hydla::symbolic_expression::Function> node)
{
  string name = node->get_string();
  HYDLA_LOGGER_DEBUG(name);
  if(name == "ln")
  {
    if(node->get_arguments_size() != 1)invalid_node(*node);
    accept(node->get_argument(0) );
    if(current_val_.is_integer)current_val_.affine_value = log(current_val_.integer);
    else current_val_.affine_value = log(current_val_.affine_value);
  }
  else
  {
    invalid_node(*node);
  }
  HYDLA_LOGGER_NODE_VAR;
}

void AffineTransformer::visit(boost::shared_ptr<hydla::symbolic_expression::Parameter> node)
{
  current_val_.is_integer = false;
  current_val_.affine_value = affine_t();
  parameter_t param(node->get_name(),
                    node->get_differential_count(),
                    node->get_phase_id());
  parameter_idx_map_t::left_iterator it = parameter_idx_map_.left.find(param);
  int idx;
  if(it == parameter_idx_map_.left.end())
  {
    idx = affine_t::maxnum();
    parameter_idx_map_.insert(
      parameter_idx_t(param, affine_t::maxnum()));
  }
  else
  {
    idx = it->second;
  }
  current_val_.affine_value.a.resize(idx + 1);
  current_val_.affine_value.a(idx) = 1;
  current_val_.affine_value.er = 0;
  HYDLA_LOGGER_NODE_VAR;
  return;
}


void AffineTransformer::invalid_node(symbolic_expression::Node& node)
{
  throw ApproximateException("invalid node" + node.get_string());
}


#define DEFINE_INVALID_NODE(NODE_NAME)                           \
void AffineTransformer::visit(boost::shared_ptr<NODE_NAME> node) \
{                                                                \
  HYDLA_LOGGER_DEBUG("");                                        \
  invalid_node(*node);                                           \
}

DEFINE_INVALID_NODE(ConstraintDefinition)
DEFINE_INVALID_NODE(ProgramDefinition)
DEFINE_INVALID_NODE(ConstraintCaller)
DEFINE_INVALID_NODE(ProgramCaller)
DEFINE_INVALID_NODE(Constraint)
DEFINE_INVALID_NODE(Ask)
DEFINE_INVALID_NODE(Tell)

DEFINE_INVALID_NODE(Equal)
DEFINE_INVALID_NODE(UnEqual)

DEFINE_INVALID_NODE(Less)
DEFINE_INVALID_NODE(LessEqual)

DEFINE_INVALID_NODE(Greater)
DEFINE_INVALID_NODE(GreaterEqual)

DEFINE_INVALID_NODE(LogicalAnd)
DEFINE_INVALID_NODE(LogicalOr)

DEFINE_INVALID_NODE(Weaker)
DEFINE_INVALID_NODE(Parallel)

DEFINE_INVALID_NODE(Always)
DEFINE_INVALID_NODE(Differential)

DEFINE_INVALID_NODE(Previous)

DEFINE_INVALID_NODE(Print)
DEFINE_INVALID_NODE(PrintPP)
DEFINE_INVALID_NODE(PrintIP)
DEFINE_INVALID_NODE(Scan)
DEFINE_INVALID_NODE(Exit)
DEFINE_INVALID_NODE(Abort)
DEFINE_INVALID_NODE(SVtimer)

DEFINE_INVALID_NODE(Not)

DEFINE_INVALID_NODE(UnsupportedFunction)

DEFINE_INVALID_NODE(symbolic_expression::Variable)
DEFINE_INVALID_NODE(SymbolicT)
DEFINE_INVALID_NODE(Infinity)
DEFINE_INVALID_NODE(True)
DEFINE_INVALID_NODE(False)


}
}
