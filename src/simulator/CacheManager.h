#pragma once

#include <vector>

#include <boost/shared_ptr.hpp>

#include "Variable.h"
#include "ValueRange.h"
#include "ModuleSet.h"
#include "ModuleSetContainer.h"
#include "ConstraintStore.h"
#include "Parameter.h"

using namespace::std;

namespace hydla {
namespace simulator {

struct Cache;

typedef boost::shared_ptr<symbolic_expression::Ask>                ask_t;
typedef std::set<ask_t>                                            asks_t;
typedef hierarchy::ModuleSet                                       module_set_t;

typedef std::map<variable_t, range_t, VariableComparator>          variable_map_t;
typedef std::set<variable_t, VariableComparator>                   variable_set_t;
typedef std::map<parameter_t, range_t, ParameterComparator>        parameter_map_t;

typedef std::map<module_set_t::module_t, bool>                     module_diff_t;
typedef hierarchy::ModuleSet                                       module_set_t;
typedef std::set<module_set_t>                                     module_set_set_t;

typedef std::tuple<asks_t, module_set_t, ConstraintStore>          cache_key_t;
typedef std::map<cache_key_t, vector<Cache*> >                     cache_map_t;
typedef std::pair<ConstraintStore, ConstraintStore>                 cache_result_t;

typedef boost::shared_ptr<backend::Backend>                        backend_sptr_t;

/*
 * 共通: A+, A_
 *       parent_unadopted_mod
 * IP  :
 *       parent_diff_sum
 *
 */

struct Cache
{
  Cache(
      cache_result_t result
      , asks_t diff_pos
      , asks_t diff_neg
      , ConstraintStore diff_s
      , module_set_t ums
      , module_diff_t mod_diff
      , list<module_set_t> ims
      , list<ConstraintStore> ic
      , variable_set_t dvs
  ): cache_result(result)
     , diff_positive_asks(diff_pos)
     , diff_negative_asks(diff_neg)
     , diff_sum(diff_s)
     , unadopted_ms(ums)
     , module_diff(mod_diff)
     , inconsistent_module_sets(ims)
     , inconsistent_constraints(ic)
     , discrete_vs(dvs) {}
  ~Cache(){}

  cache_result_t              cache_result;
  variable_map_t              cache_vm;

  asks_t                      diff_positive_asks, diff_negative_asks;
  ConstraintStore             diff_sum;
  module_set_t                unadopted_ms;
  module_diff_t               module_diff;
  std::list<module_set_t>     inconsistent_module_sets;
  std::list<ConstraintStore>  inconsistent_constraints;
  variable_set_t              discrete_vs;
};

class CacheManager
{
public:
  CacheManager(backend_sptr_t backend);
  ~CacheManager();
  bool check_cache_consistency(PhaseType type, asks_t pos_asks, module_set_t parent_unadopted, ConstraintStore diff_sum);
  void set_phase_result(phase_result_sptr_t phase);
  void new_cache(const asks_t asks, const phase_result_sptr_t phase);
  void dump_cache(std::ostream& s);
  int get_cache_hit_count();

private:
  backend_sptr_t        backend_;
  vector<Cache*>        consistent_cache_list_;
  cache_map_t           cache_map_point_;
  cache_map_t           cache_map_interval_;
  int                   cache_hit_count_;
};

} // namespace simulator
} // namespace hydla
