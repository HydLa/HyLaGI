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
 * ����X�g�A
 */
class ConstraintStoreInterval
{
public:
  typedef std::set<rp_constraint> ctr_set_t;

  ConstraintStoreInterval();

  ConstraintStoreInterval(const ConstraintStoreInterval& src);

  ~ConstraintStoreInterval();

  ConstraintStoreInterval& operator=(const ConstraintStoreInterval& src);

  void build(const virtual_constraint_solver_t::variable_map_t& variable_map);

  // �����\��
  void build_variable_map(virtual_constraint_solver_t::variable_map_t& variable_map) const;

  std::set<rp_constraint> get_store_exprs_copy() const;

  ctr_set_t get_store_non_init_constraint_copy() const;

  void clear_non_init_constraint();

  void set_non_init_constraint(const ctr_set_t ctrs);

  //void add_constraint(rp_constraint c, const var_name_map_t& vars);

  //void add_constraint(std::set<rp_constraint>::iterator start, std::set<rp_constraint>::iterator end, const var_name_map_t& vars);

  const var_name_map_t& get_store_vars() const
  {
    return this->vars_;
  }

  std::ostream& dump_cs(std::ostream& s) const;

  friend std::ostream& operator<<(std::ostream& s, const ConstraintStoreInterval& cs)
  {
    return cs.dump_cs(s);
  }

  // �L����tell����̃��X�g(TODO: private�ɂ��ăA�N�Z�T�����H)
  hydla::simulator::tells_t nodes_;

private:
  ctr_set_t exprs_; // �����l�Ɋւ��鐧�񂪓���
  ctr_set_t non_init_exprs_; // IP�ŐV���ɖ������ׂ����񂪓���
  var_name_map_t vars_; // �ϐ��\
};

} // namespace realpaver
} // namespace vcs
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_REALPAVER_RP_CONSTRAINT_STORE_INTERVAL_H_