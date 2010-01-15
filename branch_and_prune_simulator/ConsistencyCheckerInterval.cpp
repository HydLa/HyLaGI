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
    // ストアの変数情報をコピー
    //this->vars_ = constraint_store.get_store_vars();
    // tellを積分，rp_constraint集合を生成

    // 実験中
    // tellsを送信可能な形へ変換
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

    // tellsに使われている変数リスト？
    this->packet_sender_.put_vars();
    this->ml_.MLEndPacket();

    // 受け取った式リストをrp_constraint集合に変換
    // tell制約をコピーしておく
    // ストアの制約を追加
    // 問題とソルバを作成し，解いてチェック
    // ソルバから解が出力されればconsistent，tell制約をストアに追加
    return true;
  }

}
}
