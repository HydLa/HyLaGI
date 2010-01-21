#ifndef _INCLUDED_HYDLA_VCS_REALPAVER_RP_CONSTRAINT_STORE_INTERVAL_H_
#define _INCLUDED_HYDLA_VCS_REALPAVER_RP_CONSTRAINT_STORE_INTERVAL_H_

#include "RPVCSType.h"

#include <set>
#include <iostream>
#include "rp_constraint.h"

#include "Node.h"

namespace hydla {
namespace vcs {
namespace realpaver {

//typedef boost::bimaps::bimap<std::string, int> var_name_map_t;

/**
 * 制約ストア
 */
class ConstraintStoreInterval
{
public:
  ConstraintStoreInterval();

  ConstraintStoreInterval(const ConstraintStoreInterval& src);

  ~ConstraintStoreInterval();

  ConstraintStoreInterval& operator=(const ConstraintStoreInterval& src);

  void build(const virtual_constraint_solver_t::variable_map_t& variable_map);

  // 消す予定
  void build_variable_map(virtual_constraint_solver_t::variable_map_t& variable_map) const;

  //std::set<rp_constraint> get_store_exprs_copy() const;

  //void add_constraint(rp_constraint c, const var_name_map_t& vars);

  //void add_constraint(std::set<rp_constraint>::iterator start, std::set<rp_constraint>::iterator end, const var_name_map_t& vars);

  //const var_name_map_t& get_store_vars() const
  //{
  //  return this->vars_;
  //}

  //std::ostream& dump_cs(std::ostream& s) const;

  friend std::ostream& operator<<(std::ostream& s, const ConstraintStoreInterval& cs)
  {
    return s;
  }

  // 有効なtell制約のリスト(TODO: privateにしてアクセサを作る？)
  hydla::simulator::tells_t nodes_;

private:
  rp_vector_variable to_rp_vector() const;

  std::set<rp_constraint> exprs_;
  var_name_map_t vars_;
};

} // namespace realpaver
} // namespace vcs
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_REALPAVER_RP_CONSTRAINT_STORE_INTERVAL_H_