#include "MathematicaVCSPoint.h"

#include <cassert>

#include "mathlink_helper.h"
#include "Logger.h"
#include "PacketSender.h"

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
  return cons_store_.reset();
}

bool MathematicaVCSPoint::reset(const variable_map_t& vm)
{
  return cons_store_.reset(vm);
}

bool MathematicaVCSPoint::create_variable_map(variable_map_t& vm)
{
  return cons_store_.create_variable_map(vm);
}

Trivalent MathematicaVCSPoint::add_constraint(const tells_t& collected_tells)
{
  HYDLA_LOGGER_DEBUG("#*** add constraint ***");

  // isConsistent[expr, vars]��n������
  ml_->put_function("isConsistent", 2);

  ml_->put_function("Join", 2);
  // tell����̏W������expr�𓾂�Mathematica�ɓn��
  int tells_size = collected_tells.size();
  ml_->put_function("List", tells_size);
  tells_t::const_iterator tells_it = collected_tells.begin();
  PacketSender ps(*ml_);
  while((tells_it) != collected_tells.end())
  {
    ps.visit((*tells_it));
    tells_it++;
  }

  // ����X�g�A�����expr�𓾂�Mathematica�ɓn��
  cons_store_.send_cs(ml_);

  // vars��n��
  ml_->put_function("Join", 2);
  ps.put_vars();
  // ����X�g�A���ɏo������ϐ����n��
  cons_store_.send_cs_vars(ml_);

  // ���ʂ��󂯎��O�ɐ���X�g�A��������
  cons_store_.reset();

/*
//ml_->skip_pkt_until(RETURNPKT);
// �Ԃ��Ă���p�P�b�g�����
PacketChecker pc(ml_);
pc.check();
*/

  ml_->skip_pkt_until(RETURNPKT);
  // �����Ȃ������ꍇ��0���Ԃ�i����Ԃɖ���������Ƃ������Ɓj
  if(ml_->MLGetType() == MLTKINT)
  {
    HYDLA_LOGGER_DEBUG("Consistency Check : false");
    ml_->MLNewPacket();
    return Tri_FALSE;
  }

  // �������ꍇ�͉����u������Łv�Ԃ��Ă���̂ł���𐧖�X�g�A�ɓ����
  // List[List[List["Equal[x, 1]"], List["Equal[x, -1]"]], List[x]]��
  // List[List[List["Equal[x, 1]", "Equal[y, 1]"], List["Equal[x, -1]", "Equal[y, -1]"]], List[x, y, z]]��
  // List[List[List["Equal[x,1]", "Equal[Derivative[1][x],1]", "Equal[prev[x],1]", "Equal[prev[Derivative[2][x]],1]"]],
  // List[x, Derivative[1][x], prev[x], prev[Derivative[2][x]]]]  ��List[List["True"], List[]]�Ȃ�
  HYDLA_LOGGER_DEBUG( "---build constraint store---");

  ml_->MLGetNext(); // List�Ƃ����֐���
  ml_->MLGetNext(); // List�֐��iOr�Ō��΂ꂽ����\���Ă���j

  // List�֐��̗v�f���iOr�Ō��΂ꂽ���̌��j�𓾂�
  int or_size = ml_->get_arg_count();
  ml_->MLGetNext(); // List�Ƃ����֐���

  for(int i=0; i<or_size; i++)
  {
    ml_->MLGetNext(); // List�֐��iAnd�Ō��΂ꂽ����\���Ă���j

    // List�֐��̗v�f���iAnd�Ō��΂ꂽ���̌��j�𓾂�
    int and_size = ml_->get_arg_count();
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
    cons_store_.store.first.insert(value_set);
  }


  // �o������ϐ��̈ꗗ��������ŕԂ��Ă���̂ł���𐧖�X�g�A�ɓ����
  ml_->MLGetNext(); // List�֐�

  // List�֐��̗v�f���i�ϐ��ꗗ�Ɋ܂܂��ϐ��̌��j�𓾂�
  int vars_size = ml_->get_arg_count();
  ml_->MLGetNext(); // List�Ƃ����֐���

  for(int k=0; k<vars_size; k++)
  {
    MathVariable symbolic_variable;
    switch(ml_->MLGetNext())
    {
      case MLTKFUNC: // Derivative[number][]
        ml_->MLGetNext(); // Derivative[number]�Ƃ����֐���
        ml_->MLGetNext(); // Derivative�Ƃ����֐���
        ml_->MLGetNext(); // number
        symbolic_variable.derivative_count =
          ml_->get_integer();
        ml_->MLGetNext(); // �ϐ�
        symbolic_variable.name = ml_->get_symbol();
        break;
      case MLTKSYM: // �V���{���i�L���jx�Ƃ�y�Ƃ�
        symbolic_variable.derivative_count = 0;
        symbolic_variable.name = ml_->get_symbol();
        break;
      default:
        ;
    }

    cons_store_.store.second.insert(symbolic_variable);
  }


  //if(debug_mode_) {
  //  std::set<std::set<MathValue> >::iterator or_cons_it;
  //  std::set<MathValue>::iterator and_cons_it;
  //  or_cons_it = constraint_store.first.begin();
  //  while((or_cons_it) != constraint_store.first.end())
  //  {
  //    and_cons_it = (*or_cons_it).begin();
  //    while((and_cons_it) != (*or_cons_it).end())
  //    {
  //      std::cout << (*and_cons_it).str << " ";
  //      and_cons_it++;
  //    }
  //    std::cout << std::endl;
  //    or_cons_it++;
  //  }
  //  std::cout << "----------------------------" << std::endl;
  //  std::cout << "ConsistencyChecker: true" << std::endl;
  //}


  return Tri_TRUE;
}
  
Trivalent MathematicaVCSPoint::check_entailment(const ask_node_sptr& negative_ask)
{
  // checkEntailment[guard, store, vars]��n������
  ml_->put_function("checkEntailment", 3);


  // ask����̃K�[�h�̎��𓾂�Mathematica�ɓn��
  PacketSender ps(*ml_, PacketSender::NP_POINT_PHASE);
  ps.put_node(negative_ask);

  // ����X�g�A���玮�𓾂�Mathematica�ɓn��
  cons_store_.send_cs(ml_);

  // vars��n��
  ml_->put_function("Join", 2);
  ps.put_vars();
  // ����X�g�A���ɏo������ϐ����n��
  cons_store_.send_cs_vars(ml_);

  ml_->skip_pkt_until(RETURNPKT);
  
  int num  = ml_->get_integer();
  HYDLA_LOGGER_DEBUG("EntailmentChecker#num:", num);

  // Mathematica����1�iTrue��\���j���Ԃ��true���A0�iFalse��\���j���Ԃ��false��Ԃ�
  return num==1 ? Tri_TRUE : Tri_FALSE;
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


} // namespace mathematica
} // namespace simulator
} // namespace hydla 

