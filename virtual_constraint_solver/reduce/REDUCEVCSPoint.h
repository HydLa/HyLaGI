#ifndef _INCLUDED_HYDLA_VCS_REDUCE_VCS_POINT_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_VCS_POINT_H_

#include <ostream>

#include "REDUCELink.h"
#include "REDUCEVCSType.h"
#include "REDUCEStringSender.h"

#include "../../parser/SExpParser.h"

using namespace hydla::parser;

namespace hydla {
namespace vcs {
namespace reduce {

class REDUCEVCSPoint :
public virtual_constraint_solver_t
{
public:
  typedef SExpParser::const_tree_iter_t const_tree_iter_t;

  REDUCEVCSPoint(REDUCELink* cl);

  virtual ~REDUCEVCSPoint();

  /**
   * 現在の制約ストアから変数表を作成する
   */
  virtual bool create_maps(create_result_t & create_result);
  
  /**
   * 制約ストアが無矛盾かを判定する．
   * 引数で制約を渡された場合は一時的に制約ストアに追加する．
   */
  virtual VCSResult check_consistency();

  /**
   * 現在の制約ストアから与えたaskが導出可能かどうか
   */
  virtual VCSResult check_entailment(const node_sptr &node);

  /**
   * 
   */
  virtual void set_continuity(const continuity_map_t& continuity_map);

private:
  /**
   * 左連続性に関する制約を加える
   */
  void add_left_continuity_constraint(const continuity_map_t& continuity_map, REDUCEStringSender &rss);

  /**
   * 制約を出現する変数や左連続制約とともに送信する
   */
  void send_constraint(const constraints_t& constraints);

  /**
   * check_consistency の受信部分
   */
  VCSResult check_consistency_receive();



  mutable REDUCELink* cl_;
  continuity_map_t continuity_map_;
  SExpParser sp_;
};

} // namespace reduce
} // namespace simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REDUCE_VCS_POINT_H_
