#include "ConsistencyCheckerInterval.h"

#define BP_USERVAR_PREFIX "userVar"

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace bp_simulator {

  ConsistencyCheckerInterval::ConsistencyCheckerInterval(MathLink& ml) :
  packet_sender_(ml, false),
  ml_(ml)
  {}

  ConsistencyCheckerInterval::~ConsistencyCheckerInterval(){}

  bool ConsistencyCheckerInterval::is_consistent(tells_t& collected_tells,
                                                 ConstraintStoreInterval& constraint_store)
  {
    // �X�g�A�̕ϐ������R�s�[
    //this->vars_ = constraint_store.get_store_vars();
    // tell��ϕ��Crp_constraint�W���𐶐�

    // ������
    // tells�𑗐M�\�Ȍ`�֕ϊ�
    this->ml_.put_function("ToString", 1);
    tells_t::iterator it = collected_tells.begin();
    for(; it!=collected_tells.end(); it++) {
      this->packet_sender_.put_node(*it);
    }
    this->ml_.MLEndPacket();
    ml_.skip_pkt_until(RETURNPKT);
    int r = this->ml_.MLGetNext();
    std::string str(ml_.get_string());
    std::cout << r << " : " << str << "\n\n";

    // tells�Ɏg���Ă���ϐ����X�g�H
    this->packet_sender_.put_vars();
    this->ml_.MLEndPacket();

    // �󂯎���������X�g��rp_constraint�W���ɕϊ�
    // tell������R�s�[���Ă���
    // �X�g�A�̐����ǉ�
    // ���ƃ\���o���쐬���C�����ă`�F�b�N
    // �\���o��������o�͂�����consistent�Ctell������X�g�A�ɒǉ�
    return true;
  }

}
}
