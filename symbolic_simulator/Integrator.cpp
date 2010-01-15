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
  IntegrateResult integrate_result;
  integrate_result.states.resize(1);
  NextPointPhaseState& state = integrate_result.states.back();

  ml_.MLGetNext(); // List�֐�
  int list_size = ml_.get_arg_count();
  HYDLA_LOGGER_DEBUG("list_size : ", list_size);
  
  // List[����ꗗ, �ω�����ask�Ƃ���ID�̑g�̈ꗗ]���Ԃ�
  ml_.MLGetNext(); // List�Ƃ����֐���
  ml_.MLGetNext(); // List�̒��̐擪�v�f

  // next_point_phase_time�𓾂�
  SymbolicTime next_point_phase_time;
  state.next_point_phase_time.str = ml_.get_string();
  HYDLA_LOGGER_DEBUG("next_point_phase_time : ", state.next_point_phase_time.str);  
  ml_.MLGetNext(); // List�Ƃ����֐���
  
  // �ϐ��\�̍쐬
  int variable_list_size = ml_.get_arg_count();
  HYDLA_LOGGER_DEBUG("variable_list_size : ", variable_list_size);  
  ml_.MLGetNext(); ml_.MLGetNext();
  for(int i=0; i<variable_list_size; i++)
  {
    ml_.MLGetNext(); ml_.MLGetNext();

    SymbolicVariable variable;
    SymbolicValue    value;

    HYDLA_LOGGER_DEBUG("--- add variable ---");

    // �ϐ���
    variable.name = ml_.get_symbol();
    HYDLA_LOGGER_DEBUG("name  : ", variable.name);
    ml_.MLGetNext();

    // ������
    variable.derivative_count = ml_.get_integer();
    HYDLA_LOGGER_DEBUG("derivative : ", variable.derivative_count);
    ml_.MLGetNext();

    // �l
    value.str = ml_.get_string();
    HYDLA_LOGGER_DEBUG("value : ", value.str);
    ml_.MLGetNext();

    state.variable_map.set_variable(variable, value); 
  }

  // ask�Ƃ���ID�̑g�ꗗ�𓾂�
  int changed_asks_size = ml_.get_arg_count();
  HYDLA_LOGGER_DEBUG("changed_asks_size : ", changed_asks_size);
  state.is_max_time = changed_asks_size > 0; // �ω�����ask���Ȃ��ꍇ��max_time�ɒB�����ꍇ�ł���
  HYDLA_LOGGER_DEBUG("is_max_time : ", state.is_max_time);
  ml_.MLGetNext(); // List�֐�
  ml_.MLGetNext(); // List�Ƃ����֐���
  for(int j=0; j<changed_asks_size; j++)
  {
    HYDLA_LOGGER_DEBUG("--- add changed ask ---");

    ml_.MLGetNext(); // List�֐�
    ml_.MLGetNext(); // List�Ƃ����֐���
    std::string changed_ask_type_str = ml_.get_symbol(); // pos2neg�܂���neg2pos
    HYDLA_LOGGER_DEBUG("changed_ask_type_str : ", changed_ask_type_str);

    AskState changed_ask_type;
    if(changed_ask_type_str == "pos2neg"){
      changed_ask_type = Positive2Negative;
    }
    else if(changed_ask_type_str == "neg2pos") {
      changed_ask_type = Negative2Positive;
    }
    else {
      assert(0);
    }

    int changed_ask_id = ml_.get_integer();
    HYDLA_LOGGER_DEBUG("changed_ask_id : ", changed_ask_id);

    integrate_result.ask_list.push_back(
      std::make_pair(changed_ask_type, changed_ask_id));
  }

  HYDLA_LOGGER_DEBUG(
    "--- integrate result ---\n", 
    integrate_result);


  return integrate_result;
}


} //namespace symbolic_simulator
} // namespace hydla
