#include "MathematicaVCSPoint.h"

#include <cassert>

#include <boost/algorithm/string/predicate.hpp>

#include "mathlink_helper.h"
#include "Logger.h"

using namespace hydla::vcs;

namespace hydla {
namespace vcs {
namespace mathematica {

MathematicaVCSPoint::MathematicaVCSPoint(MathLink* ml) :
  ml_(ml)
{
}

MathematicaVCSPoint::~MathematicaVCSPoint()
{}

bool MathematicaVCSPoint::reset()
{
  constraint_store_ = constraint_store_t();
  return true;
}

bool MathematicaVCSPoint::reset(const variable_map_t& variable_map)
{
  HYDLA_LOGGER_DEBUG("#*** Reset Constraint Store ***");

  if(variable_map.size() == 0)
  {
    HYDLA_LOGGER_DEBUG("no Variables");
    return true;
  }
  HYDLA_LOGGER_DEBUG("------Variable map------\n", variable_map);

  std::set<MathValue> and_cons_set;

  variable_map_t::variable_list_t::const_iterator it = 
    variable_map.begin();
  variable_map_t::variable_list_t::const_iterator end = 
    variable_map.end();
  for(; it!=end; ++it)
  {
    const MathVariable& variable = (*it).first;
    const MathValue&    value = (*it).second;    

    if(value.str != "") { //����`���ǂ���  TODO: ����`����֐�����H
      std::ostringstream val_str;

      // MathVariable���Ɋւ��镶������쐬
      val_str << "Equal[";
      if(variable.derivative_count > 0)
      {
        val_str << "Derivative["
                << variable.derivative_count
                << "][prev["
                << PacketSender::var_prefix
                << variable.name
                << "]]";
      }
      else
      {
        val_str << "prev["
                << PacketSender::var_prefix
                << variable.name
                << "]";
      }

      val_str << ","
              << value.str
              << "]"; // Equal�̕�����

      MathValue new_math_value;
      new_math_value.str = val_str.str();
      and_cons_set.insert(new_math_value);

      // ����X�g�A���̕ϐ��ꗗ���쐬
      constraint_store_.second.insert(
        boost::make_tuple(variable.name,
                          variable.derivative_count,
                          true));
    }
  }

  constraint_store_.first.insert(and_cons_set);

  HYDLA_LOGGER_DEBUG(*this);

  return true;
}

bool MathematicaVCSPoint::create_variable_map(variable_map_t& variable_map)
{
  HYDLA_LOGGER_DEBUG(
    "#*** MathematicaVCSPoint::create_variable_map ***\n",
    "--- variable_map ---\n",
    variable_map,
    "--- constraint_store ---\n",
    *this);    

  std::set<std::set<MathValue> >::const_iterator or_cons_it = 
    constraint_store_.first.begin();
  std::set<std::set<MathValue> >::const_iterator or_cons_end = 
    constraint_store_.first.end();

  for(; or_cons_it!=or_cons_end; ++or_cons_it) {
    std::set<MathValue>::const_iterator and_cons_it = (*or_cons_it).begin();
    for(; (and_cons_it) != (*or_cons_it).end(); and_cons_it++)
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
        variable_name =  variable_str.substr(variable_loc, bracket_loc-variable_loc);
      }
      else
      {
        variable_name =  variable_str; // "usrVar"�̒�����      
        variable_derivative_count = 0;
      }

      // prev�ϐ��łȂ������珈��
      if(!boost::starts_with(variable_name, "prev")) {
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
        symbolic_variable.name = 
          variable_name.substr(PacketSender::var_prefix.size());
        symbolic_variable.derivative_count = 
          variable_derivative_count;
        symbolic_value.str = value_str;

        variable_map.set_variable(symbolic_variable, symbolic_value);
      } 
    }
  }

  return true;
}

VCSResult MathematicaVCSPoint::add_constraint(const tells_t& collected_tells)
{
  HYDLA_LOGGER_DEBUG(
    "#*** Begin MathematicaVCSPoint::add_constraint ***");

  PacketSender ps(*ml_);


/////////////////// ���M����

  // isConsistent[expr, vars]��n������
  ml_->put_function("isConsistent", 2);

  ml_->put_function("Join", 2);
  int tells_size = collected_tells.size();
  ml_->put_function("List", tells_size);

  // tell����̏W������expr�𓾂�Mathematica�ɓn��
  tells_t::const_iterator tells_it  = collected_tells.begin();
  tells_t::const_iterator tells_end = collected_tells.end();
  for(; tells_it!=tells_end; ++tells_it) {
    HYDLA_LOGGER_DEBUG("put node: ", *(*tells_it)->get_child());
    ps.put_node((*tells_it)->get_child(), PacketSender::VA_None);
  }

  // ����X�g�A�����expr�𓾂�Mathematica�ɓn��
  send_cs();

  // vars��n��
  ml_->put_function("Join", 2);
  ps.put_vars(PacketSender::VA_None);
  // ����X�g�A���ɏo������ϐ����n��
  send_cs_vars();

  
  
//   ml_->skip_pkt_until(TEXTPKT);
//   std::cout << ml_->get_string() << std::endl;


/////////////////// ��M����
  HYDLA_LOGGER_DEBUG( "--- receive ---");

  HYDLA_LOGGER_DEBUG(
    "-- math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));  

  ml_->skip_pkt_until(RETURNPKT);

  // ���ʂ��󂯎��O�ɐ���X�g�A��������
//  reset();

  ml_->MLGetNext();
  ml_->MLGetNext();
  ml_->MLGetNext();

  int ret = ml_->get_integer();
  HYDLA_LOGGER_DEBUG("ret: ", ret);
  switch(ret) 
  {
    case 0: {
      // �������ꍇ�͉����u������Łv�Ԃ��Ă���̂ł���𐧖�X�g�A�ɓ����
      // List[List[List["Equal[x, 1]"], List["Equal[x, -1]"]], List[x]]��
      // List[List[List["Equal[x, 1]", "Equal[y, 1]"], List["Equal[x, -1]", "Equal[y, -1]"]], List[x, y, z]]��
      // List[List[List["Equal[x,1]", "Equal[Derivative[1][x],1]", "Equal[prev[x],1]", "Equal[prev[Derivative[2][x]],1]"]],
      // List[x, Derivative[1][x], prev[x], prev[Derivative[2][x]]]]  ��List[List["True"], List[]]�Ȃ�
      HYDLA_LOGGER_DEBUG( "---build constraint store---");
      ml_->MLGetNext();

      // List�֐��̗v�f���iOr�Ō��΂ꂽ���̌��j�𓾂�
      int or_size = ml_->get_arg_count();
      HYDLA_LOGGER_DEBUG( "or_size: ", or_size);
      ml_->MLGetNext(); // List�Ƃ����֐���

      for(int i=0; i<or_size; i++)
      {
        ml_->MLGetNext(); // List�֐��iAnd�Ō��΂ꂽ����\���Ă���j

        // List�֐��̗v�f���iAnd�Ō��΂ꂽ���̌��j�𓾂�
        int and_size = ml_->get_arg_count();
        HYDLA_LOGGER_DEBUG( "and_size: ", and_size);
        ml_->MLGetNext(); // List�Ƃ����֐���
        ml_->MLGetNext(); // List�̒��̐擪�v�f

        std::set<MathValue> value_set;    
        for(int j=0; j<and_size; j++)
        {
          std::string str = ml_->get_string();
          MathValue math_value;
          math_value.str = str;
          value_set.insert(math_value);
        }
        constraint_store_.first.insert(value_set);
      }
      
      constraint_store_.second.insert(ps.vars_begin(), ps.vars_end());
      break;
    }
    
    case 1:
      HYDLA_LOGGER_DEBUG("inconsistent");
      ml_->MLNewPacket();
      return VCSR_FALSE;
    
    case 2:
      HYDLA_LOGGER_DEBUG("cannot solve");
      ml_->MLNewPacket();
      return VCSR_SOLVER_ERROR;

    default:
      assert(0);
  }

  HYDLA_LOGGER_DEBUG(
    *this,
    "\n#*** End MathematicaVCSPoint::add_constraint ***");
  
  return VCSR_TRUE;
}
  
VCSResult MathematicaVCSPoint::check_entailment(const ask_node_sptr& negative_ask)
{
  HYDLA_LOGGER_DEBUG(
    "#*** MathematicaVCSPoint::check_entailment ***\n", 
    "ask: ", *negative_ask);

  PacketSender ps(*ml_);
  

  // checkEntailment[guard, store, vars]��n������
  ml_->put_function("checkEntailment", 3);

  // ask����̃K�[�h�̎��𓾂�Mathematica�ɓn��
  ps.put_node(negative_ask->get_guard(), PacketSender::VA_None);

  // ����X�g�A���玮�𓾂�Mathematica�ɓn��
  send_cs();

  // vars��n��
  ml_->put_function("Join", 2);
  ps.put_vars(PacketSender::VA_None);
  // ����X�g�A���ɏo������ϐ����n��
  send_cs_vars();

  HYDLA_LOGGER_DEBUG(
    "-- math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));  

  ml_->skip_pkt_until(RETURNPKT);
  
  // Mathematica����1�iTrue��\���j���Ԃ��true���A0�iFalse��\���j���Ԃ��false��Ԃ�
  VCSResult ret;
  switch(ml_->get_integer())
  {
    case 0:
      ret = VCSR_FALSE;
      HYDLA_LOGGER_DEBUG("not entailed");
      break;

    case 1:
      ret = VCSR_TRUE;
      HYDLA_LOGGER_DEBUG("entailed");
      break;

    default:
      assert(0);
  }
  return ret;
}

bool MathematicaVCSPoint::integrate(
  integrate_result_t& integrate_result,
  const positive_asks_t& positive_asks,
  const negative_asks_t& negative_asks,
  const time_t& current_time,
  const time_t& max_time)
{
  // Point�ł�integrate�֐�����
  assert(0);
  return false;
}

void MathematicaVCSPoint::send_cs() const
{
  HYDLA_LOGGER_DEBUG("---- Send Constraint Store -----");

  int or_cons_size = constraint_store_.first.size();
  if(or_cons_size <= 0)
  {
    HYDLA_LOGGER_DEBUG("no Constraints");
    ml_->put_function("List", 0);
    return;
  }

  ml_->put_function("List", 1);
  ml_->put_function("Or", or_cons_size);
  HYDLA_LOGGER_DEBUG("or cons size: ", or_cons_size);

  std::set<std::set<MathValue> >::const_iterator or_cons_it = 
    constraint_store_.first.begin();
  std::set<std::set<MathValue> >::const_iterator or_cons_end = 
    constraint_store_.first.end();
  for(; or_cons_it!=or_cons_end; ++or_cons_it)
  {
    int and_cons_size = (*or_cons_it).size();
    ml_->put_function("And", and_cons_size);
    HYDLA_LOGGER_DEBUG("and cons size: ", and_cons_size);

    std::set<MathValue>::const_iterator and_cons_it = 
      (*or_cons_it).begin();
    std::set<MathValue>::const_iterator and_cons_end = 
      (*or_cons_it).end();
    for(; and_cons_it!=and_cons_end; ++and_cons_it)
    {
      ml_->put_function("ToExpression", 1);
      std::string str = (*and_cons_it).str;
      ml_->put_string(str);
      HYDLA_LOGGER_DEBUG("put cons: ", str);
    }
  }
}

void MathematicaVCSPoint::send_cs_vars() const
{
  int vars_size = constraint_store_.second.size();

  HYDLA_LOGGER_DEBUG(
    "---- Send Constraint Store Vars -----\n",
    "vars_size: ", vars_size);

  PacketSender ps(*ml_);
  
  ml_->put_function("List", vars_size);

  constraint_store_vars_t::const_iterator it = 
    constraint_store_.second.begin();
  constraint_store_vars_t::const_iterator end = 
    constraint_store_.second.end();
  for(; it!=end; ++it) {
    ps.put_var(*it, PacketSender::VA_None);
  }
}

std::ostream& MathematicaVCSPoint::dump(std::ostream& s) const
{
  s << "#*** Dump MathematicaVCSPoint ***\n"
    << "--- constraint store ---\n";

  // 
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

  // ����X�g�A���ɑ��݂���ϐ��̃_���v
  s << "-- vars --\n";
  constraint_store_vars_t::const_iterator vars_it = 
    constraint_store_.second.begin();
  while((vars_it) != constraint_store_.second.end())
  {
    s << *(vars_it) << "\n";
    vars_it++;
  }

  return s;
}

std::ostream& operator<<(std::ostream& s, const MathematicaVCSPoint& m)
{
  return m.dump(s);
}


} // namespace mathematica
} // namespace simulator
} // namespace hydla 

