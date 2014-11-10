#pragma once

#include "Node.h"
#include <vector>

namespace hydla{
namespace simulator{

typedef symbolic_expression::node_sptr constraint_t;
typedef std::set<constraint_t> constraints_t;


/**
 * 制約ストアに対応するクラス。
 * virtual デストラクタを持たないクラスを継承しているのでアップキャストしてはいけない。
 */
class ConstraintStore : public constraints_t
{
public:
  ConstraintStore();
  ConstraintStore(constraint_t t);

  void add_constraint(const constraint_t &constraint);
  void add_constraint_store(const ConstraintStore &store);

  bool consistent() const;
  // return if this constraint store is always true
  bool valid() const;
  void set_consistency(bool);
private:
  bool is_consistent;
};

std::ostream &operator<<(std::ostream &ost, const ConstraintStore &store);

}
}

