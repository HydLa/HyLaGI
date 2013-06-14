#include "MathematicaExpressionConverter.h"
#include "PacketSender.h"
#include "../SolveError.h"

#include <stdlib.h>
#include <cassert>
#include <iostream>
#include <sstream>

namespace hydla {
namespace vcs {
namespace mathematica {

using namespace hydla::parse_tree;


void MathematicaExpressionConverter::initialize(){
}

void MathematicaExpressionConverter::set_range(const value_t &val, value_range_t &range, const int& relop){
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

MathematicaExpressionConverter::value_t MathematicaExpressionConverter::receive_and_make_symbolic_value(MathLink &ml){
  value_t ret(new hydla::simulator::symbolic::SymbolicValue(make_tree(ml)));
  HYDLA_LOGGER_LOCATION(VCS);
  HYDLA_LOGGER_VCS("value: ", ret);
  return ret;
}

MathematicaExpressionConverter::node_sptr MathematicaExpressionConverter::make_tree(MathLink &ml){
  node_sptr ret;
  switch(ml.get_type()){ // ���s�I�u�W�F�N�g�̌^�𓾂�
    case MLTKSTR: // ������
    {
      HYDLA_LOGGER_REST("%% MLTKSTR(make_tree)");
      std::string str = ml.get_string();
      ret = node_sptr(new hydla::parse_tree::Number(str));
      break;
    }
    case MLTKSYM: // �V���{���i�L���j
    {
    HYDLA_LOGGER_REST("%% MLTKSYM(make_tree)");
      std::string symbol = ml.get_symbol();
      if(symbol=="t")
        ret = node_sptr(new hydla::parse_tree::SymbolicT());
      else if(symbol=="Pi")
        ret = node_sptr(new hydla::parse_tree::Pi());
      else if(symbol=="E")
        ret = node_sptr(new hydla::parse_tree::E());
      else if(symbol=="inf")
        ret = node_sptr(new hydla::parse_tree::Infinity());
      else if(symbol.length() > var_prefix.length() && symbol.substr(0, var_prefix.length()) == var_prefix)
        // �ϐ��������ꍇ�́C�ォ�瓱������邱�Ƃ��l���ĉ��ɋL���萔�Ƃ��Ă��� id��-1�Ȃ̂�ParameterReplacer�Q��
        ret = node_sptr(new hydla::parse_tree::Parameter(symbol.substr(var_prefix.length()), 0, -1));
      break;
    }
    case MLTKINT: // �����͕�����`���ł̂ݎ󂯎����̂Ƃ���iint�^���ƌ��E�����邽�߁j
    {
      HYDLA_LOGGER_REST("%% MLTKINT(make_tree)");
      assert(0);
      break;
    }
    case MLTKFUNC: // �����֐�
    HYDLA_LOGGER_REST("%% MLTKFUNC(make_tree)");
    {
      int arg_count = ml.get_arg_count();
      int next_type = ml.get_type();
      if(next_type == MLTKSYM){
        std::string symbol = ml.get_symbol();
        HYDLA_LOGGER_REST("%% symbol_name:", symbol);
        if(symbol == "Sqrt"){//1�����֐�
          ret = node_sptr(new hydla::parse_tree::Power(make_tree(ml), node_sptr(new hydla::parse_tree::Number("1/2"))));
        }
        else if(symbol == "parameter"){
          std::string name = ml.get_symbol();
          int derivative_count = boost::lexical_cast<int, std::string>(ml.get_string());
          int id = boost::lexical_cast<int, std::string>(ml.get_string());
          ret = node_sptr(new hydla::parse_tree::Parameter(name, derivative_count, id));
        }
        else if(symbol == "minus"){
          ret = node_sptr(new hydla::parse_tree::Negative(make_tree(ml)));
        }
        else if(symbol == "Plus" 
           || symbol == "Subtract"
           || symbol == "Times"
           || symbol == "Divide"
           || symbol == "Power"
           || symbol == "Rational")        
        { // �����揜�ȂǁC�񍀉��Z�q�ŏ������֐�
          node_sptr lhs, rhs;
          ret = make_tree(ml);
          for(int arg_it=1;arg_it<arg_count;arg_it++){
            lhs = ret;
            rhs = make_tree(ml);
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
          }
        }
        else{
          // ���̑��̊֐�
          boost::shared_ptr<hydla::parse_tree::ArbitraryNode> f;
          PacketSender::function_map_t::right_const_iterator it = 
            PacketSender::function_map_.right.find(PacketSender::function_t(symbol, arg_count));
          if(it != PacketSender::function_map_.right.end() && it->second.second == arg_count){
            // �Ή����Ă���֐�
            f.reset(new hydla::parse_tree::Function(it->second.first));
          }
          else{
            // ��̊֐�
            f.reset(new hydla::parse_tree::UnsupportedFunction(symbol));
          }
          for(int arg_it=0;arg_it<arg_count;arg_it++){
            f->add_argument(make_tree(ml));
          }
          ret = f;
        }
      }else{
        // Derivative�̂͂��D
        assert(next_type == MLTKFUNC);
        HYDLA_LOGGER_REST("%% derivative");
        ml.get_next();
        assert(ml.get_symbol() == "Derivative");
        ml.get_next();
        int variable_derivative_count = ml.get_integer();
        ml.get_next();
        std::string variable_name = ml.get_symbol();
        if(variable_name.length() < var_prefix.length()){
          throw SolveError("invalid symbol name");
        }
        assert(variable_name.substr(0, var_prefix.length()) == var_prefix);
        variable_name = variable_name.substr(var_prefix.length());
        // �ϐ��������ꍇ�́C�ォ�瓱������邱�Ƃ��l���ĉ��ɋL���萔�Ƃ��Ă��� id��-1�Ȃ̂�ParameterReplacer�Q��
        ret = node_sptr(new hydla::parse_tree::Parameter(variable_name.substr(var_prefix.length()), variable_derivative_count, -1));
      }
      break;
    }

    default:
      HYDLA_LOGGER_REST("%% UNKNOWN(make_tree)");
      assert(0);
      break;
  }
  if(ret == NULL){
    throw SolveError("unknown element\ninput:\n" + ml.get_input_print() + "\n\ntrace:\n" + ml.get_debug_print());
  }
  return ret;
}


} // namespace mathematica
} // namespace vcs
} // namespace hydla 
