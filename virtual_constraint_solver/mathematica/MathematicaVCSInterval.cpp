#include "MathematicaVCSInterval.h"

#include <cassert>

#include "mathlink_helper.h"
#include "Logger.h"
#include "PacketSender.h"

using namespace hydla::vcs;

namespace hydla {
namespace vcs {
namespace mathematica {

MathematicaVCSInterval::MathematicaVCSInterval(MathLink* ml) :
  ml_(ml)
{
}

MathematicaVCSInterval::~MathematicaVCSInterval()
{}

bool MathematicaVCSInterval::reset()
{
  constraint_store_ = constraint_store_t();
  return true;
}

bool MathematicaVCSInterval::reset(const variable_map_t& variable_map)
{
  HYDLA_LOGGER_DEBUG("#*** Reset Constraint Store ***");
  if(variable_map.size() == 0)
  {
    HYDLA_LOGGER_DEBUG("no Variables");
    return true;
  }
  HYDLA_LOGGER_DEBUG("------Variable map------", variable_map);

  MathValue symbolic_value;
  std::string value;
  MathVariable symbolic_variable;
  std::string variable_name;

  variable_map_t::variable_list_t::const_iterator it = variable_map.begin();

  std::set<MathValue> value_set;
  while(it != variable_map.end())
  {
    symbolic_value = (*it).second;    
    value = symbolic_value.str;
    if(value != "") break;
    it++;
  }

  while(it != variable_map.end())
  {
    symbolic_variable = (*it).first;
    symbolic_value = (*it).second;    
    variable_name = symbolic_variable.name;
    value = symbolic_value.str;

    std::string str = "";

    // MathVariable���Ɋւ��镶������쐬
    str += "Equal[";
    if(symbolic_variable.derivative_count > 0)
    {
      std::ostringstream derivative_count;
      derivative_count << symbolic_variable.derivative_count;
      str += "Derivative[";
      str += derivative_count.str();
      str += "][usrVar";
      str += variable_name;
      str += "][0]";
    }
    else
    {
      str += "usrVar";
      str += variable_name;
      str += "[0]";
    }

    str += ",";

    // MathValue���Ɋւ��镶������쐬
    str += value;
    str += "]"; // Equal�̕�����

    MathValue new_symbolic_value;
    new_symbolic_value.str = str;
    value_set.insert(new_symbolic_value);


    // ����X�g�A���̕ϐ��ꗗ���쐬
    symbolic_variable.name = "usrVar" + variable_name;
    constraint_store_.second.insert(symbolic_variable);

    it++;
    while(it != variable_map.end())
    {
      symbolic_value = (*it).second;
      value = symbolic_value.str;
      if(value != "") break;
      it++;
    }
  }
  constraint_store_.first.insert(value_set);

  HYDLA_LOGGER_DEBUG(*this);

  return true;
}

bool MathematicaVCSInterval::create_variable_map(variable_map_t& variable_map)
{
  //TODO: cout�ɏo�͂���̂���߂�

  std::set<std::set<MathValue> >::const_iterator or_cons_it = 
    constraint_store_.first.begin();
  // Or�łȂ���������̂����A�ŏ���1�������̗p���邱�Ƃɂ���
  std::set<MathValue>::const_iterator and_cons_it = (*or_cons_it).begin();
  while((and_cons_it) != (*or_cons_it).end())
  {
    std::string cons_str = (*and_cons_it).str;
    // cons_str��"Equal[usrVarx,2]"��"Equal[Derivative[1][usrVary],3]"�Ȃ�

    unsigned int loc = cons_str.find("Equal[", 0);
    loc += 6; // ������"Equal["�̒�����
    unsigned int comma_loc = cons_str.find(",", loc);
    if(comma_loc == std::string::npos)
    {
      std::cout << "can't find comma." << std::endl;
      return false;
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
        return false;
      }
      std::string variable_derivative_count_str = variable_str.substr(variable_loc, bracket_loc-variable_loc);
      variable_derivative_count = std::atoi(variable_derivative_count_str.c_str());
      variable_loc = bracket_loc + 2; // "]["�̒�����
      bracket_loc = variable_str.find("]", variable_loc);
      if(bracket_loc == std::string::npos)
      {
        std::cout << "can't find bracket." << std::endl;
        return false;
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
      return false;
    }
    std::string value_str = cons_str.substr(comma_loc + 1, end_loc - (comma_loc + 1));

    MathVariable symbolic_variable;
    MathValue symbolic_value;
    symbolic_variable.name = variable_name;
    symbolic_variable.derivative_count = variable_derivative_count;
    symbolic_value.str = value_str;

    variable_map.set_variable(symbolic_variable, symbolic_value); 
    and_cons_it++;
  }

  // [t]�����������͗v��Ȃ������H

  return true;
}

VCSResult MathematicaVCSInterval::add_constraint(const tells_t& collected_tells)
{
  // isConsistentInterval[tells, store, tellsVars, storeVars]��n������
  ml_->put_function("isConsistentInterval", 4);

  // tell����̏W������tells�𓾂�Mathematica�ɓn��
  int tells_size = collected_tells.size();
  ml_->put_function("List", tells_size);
  tells_t::const_iterator tells_it = collected_tells.begin();
  PacketSender psi(*ml_, PacketSender::NP_INTERVAL_PHASE);

  while((tells_it) != collected_tells.end())
  {
    psi.visit((*tells_it));
    tells_it++;
  }

  // ����X�g�Astore��Mathematica�ɓn��
  send_cs();

  // TODO: �����l����̑�������ԈႦ�Ă���
  
  // tellsvars��n��
  psi.put_vars();

  // storevars��n��
  send_cs_vars();

  // ���ʂ��󂯎��O�ɐ���X�g�A��������
  reset();

  ml_->skip_pkt_until(RETURNPKT);

  // �����Ȃ������ꍇ��0���Ԃ�i����Ԃɖ���������A�܂���over-constraint�Ƃ������Ɓj
  if(ml_->MLGetType() == MLTKINT)
  {
    HYDLA_LOGGER_DEBUG("over-constraint");
    ml_->MLNewPacket();
    return VCSR_FALSE;
  }

  // List[���l, ����ꗗ, �ϐ��ꗗ]���Ԃ�
  // ���l�����͖��Ȃ���������1�Aunder-constraint���N���Ă����2���Ԃ�

  ml_->MLGetNext(); // List�Ƃ����֐���
  ml_->MLGetNext(); // List�̒��̐擪�i���l�j

  int n = ml_->get_integer();

  HYDLA_LOGGER_DEBUG(
    n==2 ? "over-constraint" : "", 
    "---build constraint store---");

  ml_->MLGetNext(); // List�֐�

  // List�֐��̗v�f���i����̌��j�𓾂�
  int cons_size = ml_->get_arg_count();
  ml_->MLGetNext(); // List�Ƃ����֐���
  ml_->MLGetNext(); // List�̒��̐擪�v�f

  std::set<MathValue> value_set;    
  for(int i=0; i<cons_size; i++)
  {
    std::string str = ml_->get_string();
    MathValue symbolic_value;
    symbolic_value.str = str;
    value_set.insert(symbolic_value);
  }
  constraint_store_.first.insert(value_set);


  // �o������ϐ��̈ꗗ��������ŕԂ��Ă���̂ł���𐧖�X�g�A�ɓ����
  ml_->MLGetNext(); // List�֐�

  // List�֐��̗v�f���i�ϐ��ꗗ�Ɋ܂܂��ϐ��̌��j�𓾂�
  int vars_size = ml_->get_arg_count();
  ml_->MLGetNext(); // List�Ƃ����֐���

  for(int k=0; k<vars_size; k++)
  {
    MathVariable symbolic_variable;
    std::string sym;
    ml_->MLGetNext(); // Derivative[number][�ϐ���][]�܂���x[]�Ȃǂ̊֐�
    switch(ml_->MLGetNext()) // Derivative[number][�ϐ���]�܂���x�Ƃ����֐���
    {
      case MLTKFUNC:
        ml_->MLGetNext(); // Derivative[number]�Ƃ����֐���
        ml_->MLGetNext(); // Derivative�Ƃ����֐���
        ml_->MLGetNext(); // number
        symbolic_variable.derivative_count = ml_->get_integer();
        ml_->MLGetNext(); // �ϐ���
        symbolic_variable.name = ml_->get_symbol();
        ml_->MLGetNext(); // t
        break;
      case MLTKSYM:
        sym = ml_->get_symbol();
        symbolic_variable.derivative_count = 0;
        ml_->MLGetNext(); // t
        symbolic_variable.name = sym;
        break;
      default:
        ;
    }
    constraint_store_.second.insert(symbolic_variable);
  }
  ml_->MLNewPacket(); // �G���[���p�B�G���[�̌����s��

  HYDLA_LOGGER_DEBUG(*this);

  return n >= 1 ? VCSR_TRUE : VCSR_FALSE;
}
  
VCSResult MathematicaVCSInterval::check_entailment(const ask_node_sptr& negative_ask)
{
  // checkEntailment[guard, store, vars]��n������
  ml_->put_function("checkEntailment", 3);


  // ask����̃K�[�h�̎��𓾂�Mathematica�ɓn��
  PacketSender psi(*ml_, PacketSender::NP_INTERVAL_PHASE);
  psi.put_node(negative_ask);

  // ����X�g�A���玮store�𓾂�Mathematica�ɓn��
  send_cs();

  // vars��n��
  ml_->put_function("Join", 2);
  psi.put_vars();
  // ����X�g�A���ɏo������ϐ����n��
  send_cs_vars();

  ml_->skip_pkt_until(RETURNPKT);
  
  int num = ml_->get_integer();
  HYDLA_LOGGER_DEBUG("EntailmentCheckerInterval#num:", num);

  // Mathematica����1�iTrue��\���j���Ԃ��true���A0�iFalse��\���j���Ԃ��false��Ԃ�
  return num==1 ? VCSR_TRUE : VCSR_FALSE;
}

bool MathematicaVCSInterval::integrate(
  integrate_result_t& integrate_result,
  const positive_asks_t& positive_asks,
  const negative_asks_t& negative_asks,
  const time_t& current_time,
  const time_t& max_time)
{
  //TODO: �����l����̑�����ɖ�肠��

  HYDLA_LOGGER_DEBUG("#*** MathematicaVCSInterval::integrate ***");

//   HYDLA_LOGGER_DEBUG(
//     "#*** Integrator ***\n",
//     "--- positive asks ---\n",
//     positive_asks,
//     "--- negative asks ---\n",
//     negative_asks,
//     "--- current time ---\n",
//     current_time,
//     "--- max time ---\n",
//     max_time);

////////////////// ���M����
  PacketSender psi(*ml_, PacketSender::NP_INTERVAL_PHASE);
  
  // integrateCalc[store, posAsk, negAsk, vars, maxTime]��n������
  ml_->put_function("integrateCalc", 5);

  // ����X�g�A���玮store�𓾂�Mathematica�ɓn��
  send_cs();

  // posAsk��n���i{�K�[�h�̎��Aask��ID}�����ꂼ��j
  int pos_asks_size = positive_asks.size();
  ml_->put_function("List", pos_asks_size);
  positive_asks_t::const_iterator pos_asks_it = positive_asks.begin();
  while((pos_asks_it) != positive_asks.end())
  {
    HYDLA_LOGGER_DEBUG("send pos ask : ", **pos_asks_it);

    ml_->put_function("List", 2);    
    psi.put_node((*pos_asks_it)->get_guard());
    // ID�𑗂�
    int pos_id = (*pos_asks_it)->get_id();
    ml_->MLPutInteger(pos_id);

    pos_asks_it++;
  }

  // negAsk��n���i{�K�[�h�̎��Aask��ID}�����ꂼ��j
  int neg_asks_size = negative_asks.size();
  ml_->put_function("List", neg_asks_size);
  negative_asks_t::const_iterator neg_asks_it = negative_asks.begin();
  while((neg_asks_it) != negative_asks.end())
  {
    HYDLA_LOGGER_DEBUG("send neg ask : ", **neg_asks_it);

    ml_->put_function("List", 2);
    psi.put_node((*neg_asks_it)->get_guard());
    // ID�𑗂�
    int neg_id = (*neg_asks_it)->get_id();
    ml_->MLPutInteger(neg_id);

    neg_asks_it++;
  }

  // vars��n��
  ml_->put_function("DeleteDuplicates", 1); // �d��������B�v�C���B
  ml_->put_function("Join", 2);
  psi.put_vars();
  // ����X�g�A���ɏo������ϐ����n��
  send_cs_vars();

  // maxTime��n��
  ml_->put_function("ToExpression", 1);
  time_t send_time(max_time);
  send_time -= current_time;
  send_time.send_time(*ml_);

  ml_->skip_pkt_until(RETURNPKT);

////////////////// ��M����

  HYDLA_LOGGER_DEBUG("---integrate calc result---");
  integrate_result.states.resize(1);
  virtual_constraint_solver_t::IntegrateResult::NextPhaseState& state = 
    integrate_result.states.back();

  ml_->MLGetNext(); // List�֐�
  int list_size = ml_->get_arg_count();
  HYDLA_LOGGER_DEBUG("list_size : ", list_size);
  
  // List[����ꗗ, �ω�����ask�Ƃ���ID�̑g�̈ꗗ]���Ԃ�
  ml_->MLGetNext(); // List�Ƃ����֐���
  ml_->MLGetNext(); // List�̒��̐擪�v�f

  // next_point_phase_time�𓾂�
  MathTime next_phase_time;
  state.time.receive_time(*ml_);
  state.time += current_time;
  HYDLA_LOGGER_DEBUG("next_phase_time : ", state.time);  
  ml_->MLGetNext(); // List�Ƃ����֐���
  
  // �ϐ��\�̍쐬
  int variable_list_size = ml_->get_arg_count();
  HYDLA_LOGGER_DEBUG("variable_list_size : ", variable_list_size);  
  ml_->MLGetNext(); ml_->MLGetNext();
  for(int i=0; i<variable_list_size; i++)
  {
    ml_->MLGetNext(); 
    ml_->MLGetNext();

    MathVariable variable;
    MathValue    value;

    HYDLA_LOGGER_DEBUG("--- add variable ---");

    // �ϐ���
    variable.name = ml_->get_symbol().substr(6);
    HYDLA_LOGGER_DEBUG("name  : ", variable.name);
    ml_->MLGetNext();

    // ������
    variable.derivative_count = ml_->get_integer();
    HYDLA_LOGGER_DEBUG("derivative : ", variable.derivative_count);
    ml_->MLGetNext();

    // �l
    value.str = ml_->get_string();
    // Function[List, .....] ���̂���
    // TODO: ����ȏ����{���͂��Ȃ�Ȃ��͂�
    value.str = value.str.substr(15, value.str.size() - 16);
    HYDLA_LOGGER_DEBUG("value : ", value.str);
    ml_->MLGetNext();

    state.variable_map.set_variable(variable, value); 
  }

  // ask�Ƃ���ID�̑g�ꗗ�𓾂�
  int changed_asks_size = ml_->get_arg_count();
  HYDLA_LOGGER_DEBUG("changed_asks_size : ", changed_asks_size);
  state.is_max_time = changed_asks_size == 0; // �ω�����ask���Ȃ��ꍇ��max_time�ɒB�����ꍇ�ł���
  HYDLA_LOGGER_DEBUG("is_max_time : ", state.is_max_time);

  if(changed_asks_size>0) {
  ml_->MLGetNext(); // List�֐�
  ml_->MLGetNext(); // List�Ƃ����֐���
  }
  for(int j=0; j<changed_asks_size; j++)
  {
    HYDLA_LOGGER_DEBUG("--- add changed ask ---");

    ml_->MLGetNext(); // List�֐�
    ml_->MLGetNext(); // List�Ƃ����֐���
    std::string changed_ask_type_str = ml_->get_symbol(); // pos2neg�܂���neg2pos
    HYDLA_LOGGER_DEBUG("changed_ask_type_str : ", changed_ask_type_str);

    hydla::simulator::AskState changed_ask_type;
    if(changed_ask_type_str == "pos2neg"){
      changed_ask_type = hydla::simulator::Positive2Negative;
    }
    else if(changed_ask_type_str == "neg2pos") {
      changed_ask_type = hydla::simulator::Negative2Positive;
    }
    else {
      assert(0);
    }

    int changed_ask_id = ml_->get_integer();
    HYDLA_LOGGER_DEBUG("changed_ask_id : ", changed_ask_id);

    integrate_result.changed_asks.push_back(
      std::make_pair(changed_ask_type, changed_ask_id));
  }

//   HYDLA_LOGGER_DEBUG(
//     "--- integrate result ---\n", 
//     integrate_result);

  return true;
}

void MathematicaVCSInterval::send_cs() const
{
  HYDLA_LOGGER_DEBUG("---- Send Constraint Store -----");

  int or_cons_size = constraint_store_.first.size();
  if(or_cons_size <= 0)
  {
    HYDLA_LOGGER_DEBUG("no Constraints");
    ml_->put_function("List", 0);
    return;
  }

  std::set<std::set<MathValue> >::const_iterator or_cons_it;
  std::set<MathValue>::const_iterator and_cons_it;
//     or_cons_it = constraint_store_.first.begin();
//     while((or_cons_it) != constraint_store_.first.end())
//     {
//       and_cons_it = (*or_cons_it).begin();
//       while((and_cons_it) != (*or_cons_it).end())
//       {
//         std::cout << (*and_cons_it).str << " ";
//         and_cons_it++;
//       }
//       std::cout << std::endl;
//       or_cons_it++;
//     }

//     if(debug_mode_) {
//       std::cout << "----------------------------" << std::endl;
//     }

  ml_->put_function("List", 1);
  ml_->put_function("Or", or_cons_size);
  or_cons_it = constraint_store_.first.begin();
  while((or_cons_it) != constraint_store_.first.end())
  {
    int and_cons_size = (*or_cons_it).size();
    ml_->put_function("And", and_cons_size);
    and_cons_it = (*or_cons_it).begin();
    while((and_cons_it) != (*or_cons_it).end())
    {
      ml_->put_function("ToExpression", 1);
      std::string str = (*and_cons_it).str;
      ml_->put_string(str);
      and_cons_it++;
    }
    or_cons_it++;
  }
}

void MathematicaVCSInterval::send_cs_vars() const
{
  HYDLA_LOGGER_DEBUG("---- Send Constraint Store Vars -----");

  int vars_size = constraint_store_.second.size();
  std::set<MathVariable>::const_iterator vars_it = 
    constraint_store_.second.begin();

  ml_->put_function("List", vars_size);
  while((vars_it) != constraint_store_.second.end())
  {
    if(int value = (*vars_it).derivative_count > 0)
    {
      ml_->MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
      ml_->MLPutArgCount(1);      // this 1 is for the 'f'
      ml_->MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
      ml_->MLPutArgCount(1);      // this 1 is for the '*number*'
      ml_->put_symbol("Derivative");
      ml_->MLPutInteger(value);
      ml_->put_symbol((*vars_it).name);

      HYDLA_LOGGER_DEBUG("Derivative[", value, "][", (*vars_it).name, "]");
    }
    else
    {
      ml_->put_symbol((*vars_it).name);
        
      HYDLA_LOGGER_DEBUG((*vars_it).name);
    }
    vars_it++;
  }
}

std::ostream& MathematicaVCSInterval::dump(std::ostream& s) const
{
  s << "#*** Dump MathematicaVCSInterval ***\n"
    << "--- constraint store ---\n";

  std::set<MathVariable>::const_iterator vars_it = 
    constraint_store_.second.begin();
  std::set<std::set<MathValue> >::const_iterator or_cons_it = 
    constraint_store_.first.begin();
  while((or_cons_it) != constraint_store_.first.end())
  {
    std::set<MathValue>::const_iterator and_cons_it = 
      (*or_cons_it).begin();
    while((and_cons_it) != (*or_cons_it).end())
    {
      s << (*and_cons_it).str << " ";
      and_cons_it++;
    }
    s << "\n";
    or_cons_it++;
  }

  while((vars_it) != constraint_store_.second.end())
  {
    s << *(vars_it) << " ";
    vars_it++;
  }

  return s;
}

std::ostream& operator<<(std::ostream& s, const MathematicaVCSInterval& m)
{
  return m.dump(s);
}


} // namespace mathematica
} // namespace simulator
} // namespace hydla 

