#include "SExpConverter.h"


using namespace hydla::parser;
using namespace hydla::parse_tree; // Negative‚ÆNumber—p

namespace hydla {
namespace vcs {
namespace reduce {


SExpConverter::value_t SExpConverter::convert_s_exp_to_symbolic_value(const_tree_iter_t iter){

  HYDLA_LOGGER_REST("#*** convert s-expression to value ***\n",
                    iter);

  node_sptr node;
  // TODO:®”ˆÈŠO‚Ìê‡‚àl‚¦‚é
  switch(iter->value.id().to_long()) {
    case SExpGrammar::RI_Number: {
      std::stringstream number_ss;
      std::string number_str = std::string(iter->value.begin(),iter->value.end());
      int number_value;
      number_ss << number_str;
      number_ss >> number_value;
      if(number_value < 0) {
        std::string positive_number_str = std::string(iter->value.begin()+1,iter->value.end());
        node = node_sptr(new Negative(node_sptr(new Number(positive_number_str))));
      }
      else {
        assert(number_value >= 0);
        node = node_sptr(new Number(number_str));
      }
      break;
    }
    default:
      ;
  }


  value_t value;
  value.set(node);
  HYDLA_LOGGER_REST("#*** convert result value ***\n",
                    value.get_string());

  return value;

}





} // namespace reduce
} // namespace vcs
} // namespace hydla
