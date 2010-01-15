#include "Integrator.h"

#include "PacketChecker.h"
#include <iostream>

#include "Logger.h"

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {


Integrator::Integrator(MathLink& ml, bool debug_mode) :
  ml_(ml),
  debug_mode_(debug_mode)
{}

Integrator::~Integrator()
{}

/**
 * collected_tells����Anegative_asks����ask����̃K�[�h��������������邩�ǂ������ׂ�
 * @param negative_ask     �܂��W�J����Ă��Ȃ�ask����1��
 * @param collected_tells  tell����̃��X�g�i�W�J���ꂽask����́u=>�v�̉E�ӂ͂����ɒǉ������j
 * @return �`�F�b�N�̌��ʁA����ask���񂪓W�J���ꂽ���ǂ���
 */

IntegrateResult Integrator::integrate(
  ConstraintStoreInterval& constraint_store,
  positive_asks_t& positive_asks,
  negative_asks_t& negative_asks,
  const SymbolicTime& current_time,
  std::string max_time)
{
  HYDLA_LOGGER_DEBUG(
    "#*** Integrator ***\n",
    "--- positive asks ---\n",
    positive_asks,
    "--- negative asks ---\n",
    negative_asks,
    "--- current time ---\n",
    current_time,
    "--- max time ---\n",
    max_time);

  
// integrateCalc[store, posAsk, negAsk, vars, maxTime]��n������
  ml_.put_function("integrateCalc", 5);

  PacketSenderInterval psi(ml_, true);

  // ����X�g�A���玮store�𓾂�Mathematica�ɓn��
  psi.put_cs(constraint_store);

  // posAsk��n���i{�K�[�h�̎��Aask��ID}�����ꂼ��j
  int pos_asks_size = positive_asks.size();
  ml_.put_function("List", pos_asks_size);
  positive_asks_t::iterator pos_asks_it = positive_asks.begin();
  while((pos_asks_it) != positive_asks.end())
  {
    HYDLA_LOGGER_DEBUG("send pos ask : ", **pos_asks_it);

    ml_.put_function("List", 2);    
    psi.put_node((*pos_asks_it)->get_guard());
    // ID�𑗂�
    int pos_id = (*pos_asks_it)->get_id();
    ml_.MLPutInteger(pos_id);

    pos_asks_it++;
  }

  // negAsk��n���i{�K�[�h�̎��Aask��ID}�����ꂼ��j
  int neg_asks_size = negative_asks.size();
  ml_.put_function("List", neg_asks_size);
  negative_asks_t::iterator neg_asks_it = negative_asks.begin();
  while((neg_asks_it) != negative_asks.end())
  {
    HYDLA_LOGGER_DEBUG("send neg ask : ", **neg_asks_it);

    ml_.put_function("List", 2);
    psi.put_node((*neg_asks_it)->get_guard());
    // ID�𑗂�
    int neg_id = (*neg_asks_it)->get_id();
    ml_.MLPutInteger(neg_id);

    neg_asks_it++;
  }

  // vars��n��
  ml_.put_function("DeleteDuplicates", 1); // �d��������B�v�C���B
  ml_.put_function("Join", 2);
  psi.put_vars();
  // ����X�g�A���ɏo������ϐ����n��
  psi.put_cs_vars(constraint_store);

  // maxTime��n��
  ml_.put_function("ToExpression", 1);
  ml_.put_string(max_time);


  // �Ԃ��Ă���p�P�b�g�����
//  PacketChecker pc(ml_);
//  pc.check();

  ml_.skip_pkt_until(RETURNPKT);

  HYDLA_LOGGER_DEBUG("---integrate calc result---");

  ml_.MLGetNext(); // List�֐�
  int list_size = ml_.get_arg_count();
  HYDLA_LOGGER_DEBUG("list_size : ", list_size);
  
  // List[����ꗗ, �ω�����ask�Ƃ���ID�̑g�̈ꗗ]���Ԃ�
  ml_.MLGetNext(); // List�Ƃ����֐���

  // next_point_phase_time�𓾂�
  ml_.MLGetNext(); // List�̒��̐擪�v�f
  SymbolicTime next_point_phase_time;
  next_point_phase_time.str = ml_.get_string();
  HYDLA_LOGGER_DEBUG("next_point_phase_time : ", next_point_phase_time.str);  

  // ����ꗗ�𓾂�
  ml_.MLGetNext(); // List�Ƃ����֐���
  // List�֐��̗v�f���i����̌��j�𓾂�
  int variable_list_size = ml_.get_arg_count();
  HYDLA_LOGGER_DEBUG("variable_list_size : ", variable_list_size);  
  ml_.MLGetNext(); // List���̐擪�v�f

  variable_map_t variable_map;
  for(int i=0; i<variable_list_size; i++)
  {
    ml_.MLGetNext(); ml_.MLGetNext();
    
    std::string name = ml_.get_symbol();
    int derivative = ml_.get_integer();
    std::string value = ml_.get_string();

    HYDLA_LOGGER_DEBUG("name  : ", name);

//    std::string variable_value = ml_.get_string();

//     HYDLA_LOGGER_DEBUG(
//       "name  : ", variable_name,
//       "value : ", variable_value);

//    tmp_cons.insert(str);
  }
  ml_.MLGetNext(); // List�֐�
/*

  // ask�Ƃ���ID�̑g�ꗗ�𓾂�
  int changed_asks_size;
  if(! ml_.MLGetArgCount(&changed_asks_size)){
    std::cout << "#funcCase:MLGetArgCount:unable to get the number of arguments from ml" << std::endl;
    throw MathLinkError("MLGetArgCount", ml_.MLError());
  }
  bool is_max_time = changed_asks_size > 0; // �ω�����ask���Ȃ��ꍇ��max_time�ɒB�����ꍇ�ł���
  ml_.MLGetNext(); // List�Ƃ����֐���
  ask_list_t changed_asks;
  for(int j=0; j<changed_asks_size; j++)
  {
    ml_.MLGetNext(); // List�֐�
    ml_.MLGetNext(); // List�Ƃ����֐���
    std::string changed_ask_type_str = ml_.get_symbol(); // pos2neg�܂���neg2pos
    AskState changed_ask_type;
    if(changed_ask_type_str == "pos2neg"){
      changed_ask_type = Positive2Negative;
    }
    else{
      changed_ask_type = Negative2Positive;
    }
    int changed_ask_id;
    if(! ml_.MLGetInteger(&changed_ask_id)){
      HYDLA_LOGGER_ERROR("MLGetInteger:unable to read the int from ml");
      throw MathLinkError("MLGetInteger", ml_.MLError());
    }
    changed_asks.push_back(std::make_pair(changed_ask_type, changed_ask_id));
  }


  // tmp_cons����ϐ��\������
  std::set<std::string>::iterator tmp_cons_it = tmp_cons.begin();
  variable_map_t variable_map;
  while((tmp_cons_it) != tmp_cons.end())
  {
    std::string cons_str = (*tmp_cons_it);
    // cons_str��"Equal[usrVarx,2]"��"Equal[Derivative[1][usrVary],3]"�Ȃ�

    unsigned int loc = cons_str.find("Equal[", 0);
    loc += 6; // ������"Equal["�̒�����
    unsigned int comma_loc = cons_str.find(",", loc);
    if(comma_loc == std::string::npos)
    {
      std::cout << "can't find comma." << std::endl;
//      return;
    }
    std::string variable_str = cons_str.substr(loc, comma_loc-loc);
    // variable_str��"usrVarx"��"Derivative[1][usrVarx]"�Ȃ�

    // name��derivative_count�ւ̕���
    std::string variable_name;
    int variable_derivative_count;
    unsigned int variable_loc = variable_str.find("Derivative[", 0);
    if(variable_loc != std::string::npos)
    {
      variable_loc += 11; // "Derivative["�̒�����
      unsigned int bracket_loc = variable_str.find("][", variable_loc);
      if(bracket_loc == std::string::npos)
      {
        std::cout << "can't find bracket." << std::endl;
//        return;
      }
      std::string variable_derivative_count_str = variable_str.substr(variable_loc, bracket_loc-variable_loc);
      variable_derivative_count = std::atoi(variable_derivative_count_str.c_str());
      variable_loc = bracket_loc + 2; // "]["�̒�����
      bracket_loc = variable_str.find("]", variable_loc);
      if(bracket_loc == std::string::npos)
      {
        std::cout << "can't find bracket." << std::endl;
//        return;
      }
      variable_loc += 6; // "usrVar"�̒�����
      variable_name =  variable_str.substr(variable_loc, bracket_loc-variable_loc);
    }
    else
    {
      variable_name =  variable_str.substr(6); // "usrVar"�̒�����
      variable_derivative_count = 0;
    }

    // �l�̎擾
    int str_size = cons_str.size();
    unsigned int end_loc = cons_str.rfind("]", str_size-1);

    if(end_loc == std::string::npos)
    {
      std::cout << "can't find bracket." << std::endl;
//      return;
    }
    std::string value_str = cons_str.substr(comma_loc + 1, end_loc - (comma_loc + 1));

    SymbolicVariable symbolic_variable;
    SymbolicValue symbolic_value;
    symbolic_variable.name = variable_name;
    symbolic_variable.derivative_count = variable_derivative_count;
    symbolic_value.str = value_str;

    variable_map.set_variable(symbolic_variable, symbolic_value); 
    tmp_cons_it++;
  }

  NextPointPhaseState next_point_phase_state;
  next_point_phase_state.next_point_phase_time = next_point_phase_time;
  next_point_phase_state.variable_map = variable_map;
  next_point_phase_state.is_max_time = is_max_time;
  std::vector<NextPointPhaseState> next_point_phase_states_vector;
  next_point_phase_states_vector.push_back(next_point_phase_state);
*/
  IntegrateResult integrate_result;
  //integrate_result.states = next_point_phase_states_vector;
  //integrate_result.ask_list = changed_asks;

  return integrate_result;
}


} //namespace symbolic_simulator
} // namespace hydla
