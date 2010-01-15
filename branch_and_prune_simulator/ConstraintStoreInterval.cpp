#include "ConstraintStoreInterval.h"
#include "rp_constraint_ext.h"

#include <cassert>

namespace hydla {
namespace bp_simulator {

  ConstraintStoreInterval::ConstraintStoreInterval(){}
  
  ConstraintStoreInterval::~ConstraintStoreInterval()
  {
    std::set<rp_constraint>::iterator it = this->exprs_.begin();
    while(it != this->exprs_.end()) {
      rp_constraint c = *it;
      rp_constraint_destroy(&c);
      this->exprs_.erase(it++);
    }
  }

  /**
   * variable_mapÇ©ÇÁÉXÉgÉAÇç\ízÇ∑ÇÈ
   * initial_var <- var ÇÃïœä∑ÇÕÇ±Ç±Ç≈Ç®Ç±Ç»Ç§
   * IPÇ≈ÇÕprevÇÕë∂ç›ÇπÇ∏ÅCinital_varÇ∆Ç»ÇÈ
   * ñ ì|Ç»ÇÃÇ≈var_propertyÇÃis_prevÇégÇ¡ÇƒinitialÇ≈Ç†ÇÈÇ©Çä«óùÇ∑ÇÈ
   * @param variable_map ïœêîï\
   * TODO: ÇøÇ·ÇÒÇ∆èëÇ≠
   */
  void ConstraintStoreInterval::build(const variable_map_t& variable_map)
  {
    typedef var_name_map_t::value_type vars_type_t;
    variable_map_t::const_iterator it;
    for(it=variable_map.begin(); it!=variable_map.end(); it++) {
      // ïœêîñºÇçÏÇÈ
      std::string name(it->first.name);
      for(int i=it->first.derivative_count; i>0; i--) name += BP_DERIV_STR;
      std::string initial_name(name);
      initial_name += BP_INITIAL_STR;
      // ï\Ç…ìoò^
      unsigned int size = this->vars_.size();
      var_property vp(it->first.derivative_count, false),
        vp_0(it->first.derivative_count, true);
      this->vars_.insert(vars_type_t(name, size, vp)); // ìoò^çœÇ›ÇÃïœêîÇÕïœçXÇ≥ÇÍÇ»Ç¢
      this->vars_.insert(vars_type_t(initial_name, size+1, vp_0));
      // rp_intervalÇ©ÇÁrp_constraintÇçÏÇÈ
      rp_interval i;
      it->second.get(i);
      // (-oo, +oo) ==> çÏÇÁÇ»Ç¢ÅCìoò^ÇµÇ»Ç¢
      if(rp_binf(i)==-RP_INFINITY && rp_bsup(i)==RP_INFINITY) continue;
      if(rp_interval_point(i)) {
        // rp_interval_point ==> éÆ1Ç¬ìoò^
        this->add_vm_constraint(initial_name, i, RP_RELATION_EQUAL);
      } else {
        // else ==> éÆ2Ç¬ìoò^
        rp_interval i_tmp;
        rp_interval_set_point(i_tmp, rp_binf(i));
        this->add_vm_constraint(initial_name, i_tmp, RP_RELATION_SUPEQUAL);
        rp_interval_set_point(i_tmp, rp_bsup(i));
        this->add_vm_constraint(initial_name, i_tmp, RP_RELATION_INFEQUAL);
      }
    }
  }

  /**
   * variable_mapÇ©ÇÁrp_constraintÇê∂ê¨ÅCí«â¡Ç∑ÇÈÉwÉãÉp
   * @param var ïœêîñº
   * @param val íl(ì_ãÊä‘Ç≈Ç†ÇÈÇ±Ç∆)
   * @param op ââéZéq({=,>=,<=})
   */
  void ConstraintStoreInterval::add_vm_constraint(std::string var, rp_interval val, const int op)
  {
    rp_erep l, r;
    rp_ctr_num cnum;
    rp_constraint c;
    rp_erep_create_var(&l, this->vars_.left.at(var));
    assert(rp_interval_point(val));
    rp_erep_create_cst(&r, "", val);
    rp_ctr_num_create(&cnum, &l, op, &r);
    rp_constraint_create_num(&c, cnum);
    this->exprs_.insert(c);
  }

  void ConstraintStoreInterval::add_constraint(rp_constraint c, const var_name_map_t& vars)
  {
  }

  void ConstraintStoreInterval::add_constraint(std::set<rp_constraint>::iterator start, std::set<rp_constraint>::iterator end, const var_name_map_t& vars)
  {
  }

  std::ostream& ConstraintStoreInterval::dump_cs(std::ostream& s) const
  {
    rp_vector_variable vec = this->to_rp_vector();
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

  rp_vector_variable ConstraintStoreInterval::to_rp_vector() const
  {
    rp_vector_variable vec;
    rp_vector_variable_create(&vec);
    var_name_map_t::right_const_iterator it;
    for(it=this->vars_.right.begin(); it!=this->vars_.right.end(); it++){
      rp_variable v;
      rp_variable_create(&v, ((it->second).c_str()));
      rp_variable_set_decision(v);
      rp_interval interval;
      rp_interval_set(interval,(-1)*RP_INFINITY,RP_INFINITY);
      rp_union_insert(rp_variable_domain(v), interval);
      rp_vector_insert(vec, v);
    }
    return vec;
  }

} // namespace bp_simulator
} // namespace hydla
