#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_POINT_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_POINT_H_

#include <ostream>

#include "mathlink_helper.h"
#include "MathVCSType.h"
#include "PacketSender.h"
#include "MathematicaExpressionConverter.h"

namespace hydla {
namespace vcs {
namespace mathematica {

class MathematicaVCSPoint : 
    public virtual_constraint_solver_t
{
public:
  typedef std::set<PacketSender::var_info_t> constraint_store_vars_t;
  
  typedef std::pair<std::set<std::set<MathValue> >, 
                    constraint_store_vars_t> constraint_store_t;

  MathematicaVCSPoint(MathLink* ml);

  virtual ~MathematicaVCSPoint();


  /**
   * 現在の制約ストアから変数表を作成する
   */
  virtual bool create_maps(create_result_t & create_result);
  
  /**
   * 制約ストアが無矛盾かを判定する．
   * 引数で制約を渡された場合は一時的に制約ストアに追加する．
   */
  virtual VCSResult check_consistency();
  virtual VCSResult check_consistency(const constraints_t& constraints);
  

  /**
   * 与えられたmapを元に，各変数の連続性を設定する．
   */
  virtual void set_continuity(const continuity_map_t& continuity_map);

private:
  void receive_constraint_store(constraint_store_t& store);
  virtual void add_left_continuity_constraint(const continuity_map_t& continuity_map, PacketSender &ps);
  
  /**
   * 制約を出現する変数や左連続制約とともに送信する
   */
  void send_constraint(const constraints_t& constraints);
  
  /**
   * check_consistency の受信部分
   */
  VCSResult check_consistency_receive();

  
  mutable MathLink* ml_;
  continuity_map_t continuity_map_;
};


} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_POINT_H_
