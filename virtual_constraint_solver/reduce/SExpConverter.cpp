#include "SExpConverter.h"


using namespace hydla::parse_tree; // NegativeとNumber用

namespace hydla {
namespace vcs {
namespace reduce {

SExpConverter::string_map_t SExpConverter::string_map_;


SExpConverter::SExpConverter() 
{}

SExpConverter::~SExpConverter(){}

void SExpConverter::initialize(){
  //ノードと文字列の対応関係を作っておく．
  string_map_.insert(std::make_pair("plus", function_and_node(for_binary_node, NODE_PLUS)));
  string_map_.insert(std::make_pair("difference", function_and_node(for_binary_node, NODE_SUBTRACT)));
  string_map_.insert(std::make_pair("times", function_and_node(for_binary_node, NODE_TIMES)));
  string_map_.insert(std::make_pair("quotient", function_and_node(for_binary_node, NODE_DIVIDE)));
  string_map_.insert(std::make_pair("expt", function_and_node(for_binary_node, NODE_POWER)));
  string_map_.insert(std::make_pair("df", function_and_node(for_derivative, NODE_DIFFERENTIAL)));
  string_map_.insert(std::make_pair("prev", function_and_node(for_unary_node, NODE_PREVIOUS)));
  string_map_.insert(std::make_pair("sqrt", function_and_node(for_unary_node, NODE_SQRT)));
  string_map_.insert(std::make_pair("minus", function_and_node(for_unary_node, NODE_NEGATIVE)));
  string_map_.insert(std::make_pair("sin", function_and_node(for_unary_node, NODE_SIN)));
  string_map_.insert(std::make_pair("cos", function_and_node(for_unary_node, NODE_COS)));
  string_map_.insert(std::make_pair("tan", function_and_node(for_unary_node, NODE_TAN)));
  string_map_.insert(std::make_pair("asin", function_and_node(for_unary_node, NODE_ASIN)));
  string_map_.insert(std::make_pair("acos", function_and_node(for_unary_node, NODE_ACOS)));
  string_map_.insert(std::make_pair("atan", function_and_node(for_unary_node, NODE_ATAN)));
}

SExpConverter::value_t SExpConverter::convert_s_exp_to_symbolic_value(SExpParser &sp, const_tree_iter_t iter){

  HYDLA_LOGGER_REST("#*** convert s-expression to value ***\n",
                    iter);

  value_t value;
  value.set(convert_s_exp_to_symbolic_tree(sp, iter));
  HYDLA_LOGGER_REST("#*** convert result value ***\n",
                    value.get_string());

  return value;
}

SExpConverter::node_sptr SExpConverter::make_equal(const variable_t &variable, const node_sptr& node, const bool& prev){
    HYDLA_LOGGER_REST("*** Begin:SExpConverter::make_equal ***\n");
    node_sptr new_node(new Variable(variable.get_name()));
    for(int i=0;i<variable.get_derivative_count();i++){
      new_node = node_sptr(new Differential(new_node));
    }
    if(prev){
      new_node = node_sptr(new Previous(new_node));
    }
    HYDLA_LOGGER_REST("*** End:SExpConverter::make_equal ***\n");
    return node_sptr(new Equal(new_node, node));
  }

SExpConverter::node_sptr SExpConverter::convert_s_exp_to_symbolic_tree(SExpParser &sp, const_tree_iter_t iter){

  switch(iter->value.id().to_long()) {
    case SExpGrammar::RI_Number: {
      std::stringstream number_ss;
      std::string number_str = std::string(iter->value.begin(),iter->value.end());
      int number_value;
      number_ss << number_str;
      number_ss >> number_value;
      if(number_value < 0) {
        std::string positive_number_str = std::string(iter->value.begin()+1,iter->value.end());
        return node_sptr(new Negative(node_sptr(new Number(positive_number_str))));
      }
      else {
        assert(number_value >= 0);
        return node_sptr(new Number(number_str));
      }
      break;
    }
    // headerとidentifierとで分けたい
    default:
      std::string value_str = std::string(iter->value.begin(), iter->value.end());
      // 変数名の先頭にスペースが入ることがあるので除去する
      // TODO:S式パーサを修正してスペース入らないようにする
      if(value_str.at(0) == ' ') value_str.erase(0,1);

      string_map_t::const_iterator strmap_it = string_map_.find(value_str);
      if(strmap_it == string_map_.end()){
        if(value_str=="t"){//時刻
          return node_sptr(new hydla::parse_tree::SymbolicT());
        }
        // TODO:パラメタや変数名やそれ以外のfactorへの対応
        assert(0);
      }

      return (*(strmap_it->second.function))(sp, iter, strmap_it->second.node);
  }
}

SExpConverter::node_sptr SExpConverter::for_derivative(
  SExpParser &sp,
  const_tree_iter_t iter,
  const SExpConverter::nodeType &nt){
  //まず微分回数
  int derivative_count = sp.get_derivative_count(iter);
  //次に中身
  node_sptr tmp_node = convert_s_exp_to_symbolic_tree(sp, iter->children.begin());
  for(int i=0;i<derivative_count-1;i++){
    tmp_node.reset(new hydla::parse_tree::Differential(tmp_node));
  }

  return node_sptr(new hydla::parse_tree::Differential(tmp_node));
}

SExpConverter::node_sptr SExpConverter::for_unary_node(
  SExpParser &sp,
  const_tree_iter_t iter,
  const SExpConverter::nodeType &nt){
  //中身
  node_sptr tmp_node = convert_s_exp_to_symbolic_tree(sp, iter->children.begin());
  switch(nt){
    default:
      assert(0);
      return node_sptr(new hydla::parse_tree::Previous(tmp_node));

    case NODE_PREVIOUS:
      return node_sptr(new hydla::parse_tree::Previous(tmp_node));

    case NODE_SQRT:
      return node_sptr(new hydla::parse_tree::Power
                       (tmp_node,
                        node_sptr(new hydla::parse_tree::Divide(
                                    node_sptr(new hydla::parse_tree::Number("1")),
                                    node_sptr(new hydla::parse_tree::Number("2")) ) ) ) );

    case NODE_NEGATIVE:
      return node_sptr(new hydla::parse_tree::Negative(tmp_node));

    case NODE_SIN:
      return node_sptr(new hydla::parse_tree::Sin(tmp_node));

    case NODE_COS:
      return node_sptr(new hydla::parse_tree::Cos(tmp_node));

    case NODE_TAN:
      return node_sptr(new hydla::parse_tree::Tan(tmp_node));

    case NODE_ASIN:
      return node_sptr(new hydla::parse_tree::Asin(tmp_node));

    case NODE_ACOS:
      return node_sptr(new hydla::parse_tree::Acos(tmp_node));

    case NODE_ATAN:
      return node_sptr(new hydla::parse_tree::Atan(tmp_node));
  }
}

SExpConverter::node_sptr SExpConverter::for_binary_node(
  SExpParser &sp,
  const_tree_iter_t iter,
  const SExpConverter::nodeType &nt){
  //BinaryNodeを作るための関数だけど，plusとtimesはリストで複数引数取れるみたいだから特別にループ

  //左
  node_sptr lhs = convert_s_exp_to_symbolic_tree(sp, iter->children.begin());
  size_t args_count = 0;
  while(1){
    args_count++;
    //右
    node_sptr rhs = convert_s_exp_to_symbolic_tree(sp, iter->children.begin()+args_count);
    switch(nt){
      case NODE_PLUS:
        if(args_count == iter->children.size()-1){//ここで終了
          return node_sptr(new hydla::parse_tree::Plus(lhs, rhs));
        }
        else{
          lhs = node_sptr(new hydla::parse_tree::Plus(lhs, rhs));
        }
        break;

      case NODE_TIMES:
        if(args_count == iter->children.size()-1)//ここで終了
          return node_sptr(new hydla::parse_tree::Times(lhs, rhs));
        else
          lhs = node_sptr(new hydla::parse_tree::Times(lhs, rhs));
        break;

      case NODE_SUBTRACT:
        return node_sptr(new hydla::parse_tree::Subtract(lhs, rhs));
      case NODE_DIVIDE:
        return node_sptr(new hydla::parse_tree::Divide(lhs, rhs));
      case NODE_POWER:
        return node_sptr(new hydla::parse_tree::Power(lhs, rhs));

      default:
        assert(0);
        return node_sptr(new hydla::parse_tree::Power(lhs, rhs));
    }
  }
}

std::string SExpConverter::convert_node_to_reduce_string(const node_sptr &node){
  string_for_reduce_.clear();
  differential_count_=0;
  in_prev_=0;
  accept(node);
  return string_for_reduce_;
}

std::string SExpConverter::convert_symbolic_value_to_reduce_string(const value_t &val){
  return convert_node_to_reduce_string(val.get_node());
}

#define START_P "("
#define END_P   ")"

#define DEFINE_VISIT_BINARY(NODE_NAME, FUNC_NAME)              \
  void SExpConverter::visit(boost::shared_ptr<NODE_NAME> node) \
  {                                                            \
  string_for_reduce_.append(#FUNC_NAME START_P);               \
  accept(node->get_lhs());                                     \
  string_for_reduce_.append(", ");                             \
  accept(node->get_rhs());                                     \
  string_for_reduce_.append(END_P);                            \
}

#define DEFINE_VISIT_UNARY(NODE_NAME, FUNC_NAME)               \
  void SExpConverter::visit(boost::shared_ptr<NODE_NAME> node) \
  {                                                            \
  string_for_reduce_.append(#FUNC_NAME START_P);               \
  accept(node->get_child());                                   \
  string_for_reduce_.append(END_P);                            \
}

#define DEFINE_VISIT_FACTOR(NODE_NAME, FUNC_NAME)              \
  void SExpConverter::visit(boost::shared_ptr<NODE_NAME> node) \
  {                                                            \
  string_for_reduce_.append(#FUNC_NAME);                       \
}


// 比較演算子

DEFINE_VISIT_BINARY(Equal, equal)
DEFINE_VISIT_BINARY(UnEqual, neq)
DEFINE_VISIT_BINARY(Less, lessp)
DEFINE_VISIT_BINARY(LessEqual, leq)
DEFINE_VISIT_BINARY(Greater, greaterp)
DEFINE_VISIT_BINARY(GreaterEqual, geq)



// 論理演算子
DEFINE_VISIT_BINARY(LogicalAnd, and)
DEFINE_VISIT_BINARY(LogicalOr, or)


// 算術二項演算子
DEFINE_VISIT_BINARY(Plus, plus)
DEFINE_VISIT_BINARY(Subtract, difference)
DEFINE_VISIT_BINARY(Times, times)
DEFINE_VISIT_BINARY(Divide, quotient)
DEFINE_VISIT_BINARY(Power, expt)


// 算術単項演算子

DEFINE_VISIT_UNARY(Negative, -) // サーバーモードではminus使用不可
void SExpConverter::visit(boost::shared_ptr<Positive> node)
{
  accept(node->get_child());
}

// 微分
void SExpConverter::visit(boost::shared_ptr<Differential> node)
{
  differential_count_++;
  accept(node->get_child());
  differential_count_--;
}

// 左極限
void SExpConverter::visit(boost::shared_ptr<Previous> node)
{
  in_prev_++;
  accept(node->get_child());
  in_prev_--;
}

// 否定
DEFINE_VISIT_UNARY(Not, not)

// 三角関数
DEFINE_VISIT_UNARY(Sin, sin)
DEFINE_VISIT_UNARY(Cos, cos)
DEFINE_VISIT_UNARY(Tan, tan)
// 逆三角関数
DEFINE_VISIT_UNARY(Asin, asin)
DEFINE_VISIT_UNARY(Acos, acos)
DEFINE_VISIT_UNARY(Atan, atan)
// 円周率
DEFINE_VISIT_FACTOR(Pi, pi)
// 対数
DEFINE_VISIT_BINARY(Log, logb) // 使えない？
DEFINE_VISIT_UNARY(Ln, log)
// 自然対数の底
DEFINE_VISIT_FACTOR(E, e)

// 任意の文字列
void SExpConverter::visit(boost::shared_ptr<ArbitraryBinary> node){
  string_for_reduce_.append(node->get_string() + "(");
  accept(node->get_lhs());
  string_for_reduce_.append(", ");
  accept(node->get_rhs());
  string_for_reduce_.append(")");
}
void SExpConverter::visit(boost::shared_ptr<ArbitraryUnary> node){
  string_for_reduce_.append(node->get_string() + "(");
  accept(node->get_child());
  string_for_reduce_.append(")");
}
void SExpConverter::visit(boost::shared_ptr<ArbitraryFactor> node){
  string_for_reduce_.append(node->get_string());
}

// 変数
void SExpConverter::visit(boost::shared_ptr<Variable> node)
{
  std::ostringstream tmp;
  if(in_prev_){
    tmp << "prev(";
  }

  if(differential_count_){
    tmp << "df(" << node->get_name() << ", t, " << differential_count_ << ")";
  }else{
    tmp << node->get_name();
  }

  if(in_prev_){
    tmp << ")";
  }

  var_info_t new_var =
    boost::make_tuple(node->get_name(),
                      differential_count_,
                      in_prev_ && !ignore_prev_);
  vars_.insert(new_var);

  string_for_reduce_.append(tmp.str());
}

// 数字
void SExpConverter::visit(boost::shared_ptr<Number> node)
{
  string_for_reduce_.append(node->get_number());
}


// 記号定数
void SExpConverter::visit(boost::shared_ptr<Parameter> node)
{
  // TODO
  //string_for_reduce_.append(PacketSender::par_prefix + node->get_name());
}

// t
DEFINE_VISIT_FACTOR(SymbolicT, t)

} // namespace reduce
} // namespace vcs
} // namespace hydla
