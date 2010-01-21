#include "RPConstraintStoreInterval.h"
#include "Logger.h"
#include "realpaver.h"
#include "rp_problem_ext.h"
#include "rp_constraint_ext.h"
#include "RPConstraintSolver.h"

namespace hydla {
namespace vcs {
namespace realpaver {

ConstraintStoreInterval::ConstraintStoreInterval()
{}

ConstraintStoreInterval::ConstraintStoreInterval(const ConstraintStoreInterval& src)
{}

ConstraintStoreInterval::~ConstraintStoreInterval()
{}

ConstraintStoreInterval& ConstraintStoreInterval::operator=(const ConstraintStoreInterval& src)
{
  return *this;
}

void ConstraintStoreInterval::build(const virtual_constraint_solver_t::variable_map_t& variable_map)
{
  typedef var_name_map_t::value_type vars_type_t;
  virtual_constraint_solver_t::variable_map_t::const_iterator it;
  for(it=variable_map.begin(); it!=variable_map.end(); it++) {
    // •Ï”–¼‚ğì‚é
    std::string name(it->first.name);
    for(int i=it->first.derivative_count; i>0; i--) name += BP_DERIV_STR;
    std::string ini_name(name);
    ini_name += BP_INITIAL_STR;
    // •\‚É“o˜^
    unsigned int size = this->vars_.size();
    var_property vp(it->first.derivative_count, false),
      vp_p(it->first.derivative_count, true);
    this->vars_.insert(vars_type_t(name, size, vp)); // “o˜^Ï‚İ‚Ì•Ï”‚Í•ÏX‚³‚ê‚È‚¢
    this->vars_.insert(vars_type_t(ini_name, size+1, vp_p));
    // rp_interval‚©‚çrp_constraint‚ğì‚é
    rp_interval i;
    it->second.get(rp_binf(i), rp_bsup(i));
    // (-oo, +oo) ==> ì‚ç‚È‚¢C“o˜^‚µ‚È‚¢
    if(rp_binf(i)==-RP_INFINITY && rp_bsup(i)==RP_INFINITY) continue;
    if(rp_interval_point(i)) {
      // rp_interval_point ==> ®1‚Â“o˜^(var = val)
      rp_erep l, r;
      rp_ctr_num cnum;
      rp_constraint c;
      rp_erep_create_var(&l, this->vars_.left.at(ini_name));
      rp_erep_create_cst(&r, "", i);
      rp_ctr_num_create(&cnum, &l, RP_RELATION_EQUAL, &r);
      rp_constraint_create_num(&c, cnum);
      this->exprs_.insert(c);
    } else {
      // else ==> ®2‚Â“o˜^(inf <= var, var <= sup)
      rp_interval i_tmp;
      rp_erep l, r;
      rp_ctr_num cnum;
      rp_constraint c;
      rp_erep_create_var(&l, this->vars_.left.at(ini_name));
      rp_interval_set_point(i_tmp, rp_binf(i));
      rp_erep_create_cst(&r, "", i_tmp);
      rp_ctr_num_create(&cnum, &l, RP_RELATION_SUPEQUAL, &r);
      rp_constraint_create_num(&c, cnum);
      this->exprs_.insert(c);
      rp_erep_create_var(&l, this->vars_.left.at(ini_name));
      rp_interval_set_point(i_tmp, rp_bsup(i));
      rp_erep_create_cst(&r, "", i_tmp);
      rp_ctr_num_create(&cnum, &l, RP_RELATION_INFEQUAL, &r);
      rp_constraint_create_num(&c, cnum);
      this->exprs_.insert(c);
    }
  }
}

// Á‚·—\’è
void ConstraintStoreInterval::build_variable_map(virtual_constraint_solver_t::variable_map_t& variable_map) const
{}

//std::set<rp_constraint> ConstraintStoreInterval::get_store_exprs_copy() const
//{}

//void ConstraintStoreInterval::add_constraint(rp_constraint c, const var_name_map_t& vars)
//{}

//void ConstraintStoreInterval::add_constraint(std::set<rp_constraint>::iterator start, std::set<rp_constraint>::iterator end, const var_name_map_t& vars)
//{}

std::ostream& ConstraintStoreInterval::dump_cs(std::ostream& s) const
{
  rp_vector_variable vec = ConstraintSolver::create_rp_vector(this->vars_);
  std::set<rp_constraint>::const_iterator ctr_it = this->exprs_.begin();
  while(ctr_it != this->exprs_.end()){
    rp::dump_constraint(s, *ctr_it, vec); // digits, mode);
    s << "\n";
    ctr_it++;
  }
  s << "\n";
  rp_vector_destroy(&vec);
  return s;
}

} // namespace realpaver
} // namespace vcs
} // namespace hydla 
