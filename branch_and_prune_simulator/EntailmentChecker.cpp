#include "EntailmentChecker.h"

#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace bp_simulator {


EntailmentChecker::EntailmentChecker() :
  debug_mode_(false)
{}

EntailmentChecker::EntailmentChecker(bool debug_mode) :
  debug_mode_(debug_mode)
{}

EntailmentChecker::~EntailmentChecker()
{}

// Ask制約
void EntailmentChecker::visit(boost::shared_ptr<Ask> node)
{
  // ask条件から
  // guard_list(andつながり)
  // not_guard_list(orつながり)
  // の両方を作る
  this->accept(node->get_guard());
}

// Tell制約 必要ない？
void EntailmentChecker::visit(boost::shared_ptr<Tell> node)
{
}

// 比較演算子
// ここで一気に二つの式を作る
void EntailmentChecker::visit(boost::shared_ptr<Equal> node)
{
}

void EntailmentChecker::visit(boost::shared_ptr<UnEqual> node)               
{
}

void EntailmentChecker::visit(boost::shared_ptr<Less> node)                  
{
}

void EntailmentChecker::visit(boost::shared_ptr<LessEqual> node)             
{
}

void EntailmentChecker::visit(boost::shared_ptr<Greater> node)               
{
}

void EntailmentChecker::visit(boost::shared_ptr<GreaterEqual> node)          
{
}

// 論理演算子
void EntailmentChecker::visit(boost::shared_ptr<LogicalAnd> node)            
{
}


/**
 * collected_tellsからnegative_askのガード条件がentailされるどうか調べる
 * TRUEならcollected_tellsにask制約の後件を追加する
 * 
 * @param negative_ask まだ展開されていないask制約
 * @param collected_tells tell制約のリスト（展開されたask制約の「=>」の右辺がここに追加される）
 * @param constraint_store 制約ストア
 * 
 * @return entailされるかどうか {TRUE, UNKNOWN, FALSE}
 */
Trivalent EntailmentChecker::check_entailment(
  const boost::shared_ptr<Ask>& negative_ask,
  tells_t& collected_tells,
  ConstraintStore& constraint_store)
{
  // constraint_store + collected_tells = 現制約 S
  // ask条件からgとngを作る
  // solve(S & g) == empty -> FALSE
  // solve(S&ng0)==empty /\ solve(S&ng1)==empty /\ ... -> TRUE
  // else -> UNKNOWN
  return FALSE;
}

} //namespace bp_simulator
} // namespace hydla
