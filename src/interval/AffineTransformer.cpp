#include "AffineTransformer.h"
#include <boost/lexical_cast.hpp>
#include "TreeInfixPrinter.h"
#include <exception>
#include "Backend.h"
#include "Logger.h"

using namespace std;
using namespace hydla::parse_tree;
using namespace boost;
using namespace hydla::simulator;

#define HYDLA_LOGGER_NODE_VAR \
  HYDLA_LOGGER_DEBUG("node: ", node->get_node_type_name(), ", current_val: ", current_val_)

namespace hydla {
namespace interval {

AffineTransformer* AffineTransformer::affine_translator_ = NULL;

AffineTransformer* AffineTransformer::get_instance()
{
  if(affine_translator_ == NULL)affine_translator_ = new AffineTransformer();
  return affine_translator_;
}

AffineTransformer::AffineTransformer()
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
      ret.affine_value = exp(y_affine * log(x_affine));
    }
    else
    {
      if(!y.is_integer)
      {
          // TODO: throw appropriate exception
          HYDLA_LOGGER_DEBUG("l: ", l, ", u: ", u);
          throw std::exception();
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


value_t AffineTransformer::transform(node_sptr& node, parameter_map_t &parameter_map)
{
  accept(node);
  if(current_val_.is_integer)return value_t(current_val_.integer);
  affine_t affine_value = current_val_.affine_value;
  kv::interval<double> itv = to_interval(affine_value);
  HYDLA_LOGGER_DEBUG_VAR(itv);
  value_t ret(affine_value.a(0));
  simulator_->backend->call("transformToRational", 1, "vln", "vl", &ret, &ret);
  double sum = 0;
  int available_index;
  // set rounding mode
  kv::hwround::roundup();
  for(int i = 1; i < affine_value.a.size(); i++)
  {
    if(affine_value.a(i) == 0)continue;
    if(parameter_idx_map_.right.find(i)
       == parameter_idx_map_.right.end())
    {
      //新規に追加されるダミー変数は1つにまとめる（現状だと他の変数と関係を持っていないため）
      // TODO: kvライブラリ内のダミー変数も削除する
      sum += affine_value.a(i);
      available_index = i;
    }
    else
    {
      parameter_idx_map_t::right_iterator r_it = parameter_idx_map_.right.find(i);
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
    // a parameter whose differential count is -1 is regarded as a dummy variable
    parameter_t param = simulator_->introduce_parameter("affine", -1, available_index, range);
    parameter_idx_map_.insert(parameter_idx_t(param, available_index));
    parameter_map[param] = range_t(value_t(-1), value_t(1));
    value_t val = value_t(affine_value.a(available_index));
    simulator_->backend->call("transformToRational", 1, "vln", "vl", &val, &val);
    ret = ret + val * value_t(param);
  }

  // reset rounding mode
  kv::hwround::roundnear();

  return ret;
}

void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Plus> node)
{
  accept(node->get_lhs());
  AffineOrInteger lhs = current_val_;
  accept(node->get_rhs());
  AffineOrInteger rhs = current_val_;
  current_val_ = lhs + rhs;
  HYDLA_LOGGER_NODE_VAR;
  return;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Subtract> node)
{
  accept(node->get_lhs());
  AffineOrInteger lhs = current_val_;
  accept(node->get_rhs());
  AffineOrInteger rhs = current_val_;
  current_val_ = lhs - rhs;
  HYDLA_LOGGER_NODE_VAR;
  return;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Times> node)
{
  accept(node->get_lhs());
  AffineOrInteger lhs = current_val_;
  accept(node->get_rhs());
  AffineOrInteger rhs = current_val_;
  current_val_ = lhs * rhs;
  HYDLA_LOGGER_NODE_VAR;
  return;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Divide> node)
{
  accept(node->get_lhs());
  AffineOrInteger lhs = current_val_;
  accept(node->get_rhs());
  AffineOrInteger rhs = current_val_;
  current_val_ = lhs / rhs;
  HYDLA_LOGGER_NODE_VAR;
  return;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Power> node)
{
  accept(node->get_lhs());
  AffineOrInteger lhs = current_val_;  
  accept(node->get_rhs());
  AffineOrInteger rhs = current_val_;
  current_val_ = pow(lhs, rhs);
  HYDLA_LOGGER_NODE_VAR;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Negative> node)
{
  accept(node->get_child());
  current_val_ = -current_val_;
  return;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Positive> node)
{
  // do nothing
  return;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Pi> node)
{
  current_val_.affine_value = kv::constants<double>::pi();
  current_val_.is_integer = false;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::E> node)
{
  current_val_.affine_value = kv::constants<double>::e();
  current_val_.is_integer = false;
}


void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Number> node)
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

}


void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Float> node)
{
  current_val_.affine_value = affine_t(node->get_number());
  current_val_.is_integer = false;
  HYDLA_LOGGER_NODE_VAR;
}

void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Function> node)
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

void AffineTransformer::visit(boost::shared_ptr<hydla::parse_tree::Parameter> node)
{
  current_val_.is_integer = false;
  current_val_.affine_value = affine_t();
  parameter_t param(node->get_name(),
                    node->get_derivative_count(),
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
  HYDLA_LOGGER_NODE_VAR;
  return;
}


void AffineTransformer::invalid_node(parse_tree::Node& node)
{
  // TODO: throw exception specified for interval
  throw std::exception();
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

DEFINE_INVALID_NODE(Variable)
DEFINE_INVALID_NODE(SymbolicT)
DEFINE_INVALID_NODE(Infinity)
DEFINE_INVALID_NODE(True)
DEFINE_INVALID_NODE(False)


}
}
