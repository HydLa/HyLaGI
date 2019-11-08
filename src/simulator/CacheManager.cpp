#include "Parameter.h"
#include "Simulator.h"
#include "Backend.h"
#include "HydLaError.h"
#include "CacheManager.h"
#include <iostream>

using namespace std;


namespace hydla {
namespace simulator {

CacheManager::CacheManager(backend_sptr_t backend):backend_(backend){}
CacheManager::~CacheManager(){}

/*
std::vector<*Cache> CacheManager::get_current_cache(PhaseType phase_type, asks_t entailed, asks_t unentailed, module_set_t parent_unadopted, ConstraintStore diff_sum){
  
}
*/

//void CacheManager::new_cache(PhaseType phase_type, asks_t diff_pos, asks_t diff_neg, module_set_t parent_unadopted, ConstraintStore diff_sum, module_set_t unadopted_module_set, module_diff_t, list<module_set_t> inconsistent_module_sets, list<ConstraintStore> inconsistent_constraints){
void CacheManager::new_cache(const asks_t pos_asks, const phase_result_sptr_t phase){
  pair<ConstraintStore, ConstraintStore> cache_result;
  ConstraintStore cache_constraint;
  backend_->call("popCache", true, 0, "", "cr", &cache_result);
  if(!phase->in_following_step()){
    return;
  }
  cache_key_t key = make_tuple(pos_asks, phase->parent->unadopted_ms, phase->phase_type == POINT_PHASE ? ConstraintStore() : phase->parent->diff_sum);
  Cache* cache = new Cache(
      cache_result
      , phase->get_diff_positive_asks()
      , phase->get_diff_negative_asks()
      , phase->diff_sum
      , phase->unadopted_ms
      , phase->module_diff
      , phase->inconsistent_module_sets
      , phase->inconsistent_constraints
      , phase->discrete_differential_set
  );
  //std::cout << "hogehogehoge" << "\n" << *phase << std::endl;
  if(phase->phase_type == POINT_PHASE){
    cache_map_point_[key].push_back(cache);
  }else{
    cache_map_interval_[key].push_back(cache);
  }
}

bool CacheManager::check_cache_consistency(PhaseType type, asks_t pos_asks, module_set_t parent_unadopted, ConstraintStore diff_sum) {
  cache_key_t key = make_tuple(pos_asks, parent_unadopted, type == POINT_PHASE ? ConstraintStore() : diff_sum);

  vector<Cache*> cache_list;
  vector<variable_map_t> vm_list;
  bool hit_flag = false;

  if(type == POINT_PHASE){
    if(cache_map_point_.count(key) > 0){
      cache_list = cache_map_point_[key];
      for (auto &cache : cache_list) {
        backend_->call("createVariableMapFromCache", true, 1, "clp", "cv", &cache->cache_result, &vm_list);
        if(vm_list.size() == 0){
          continue;
        }
        /*
        for (auto vm : vm_list) {
          std::cout << vm << std::endl;
        }
        */
        hit_flag  = true;
        cache->cache_vm = vm_list[0];
        consistent_cache_list_.push_back(cache);
      }
    }else{
      return false;
    }
  }else{
    if(cache_map_interval_.count(key) > 0){
      cache_list = cache_map_interval_[key];
      /*
      std::cout << "----------- interval cache hit! -----------" << endl;
      std::cout << "  cond:" << endl;
      std::cout << "    " << get<0>(key) << "," << get<1>(key) << "," << get<2>(key) << endl;
      std::cout << "  constraint:" << endl;
      for (auto cache : cache_list) {
        std::cout << "    " << cache->cache_result.first << ":" << cache->cache_result.second << endl;
        std::cout << endl;
      }
      */
      for (auto &cache : cache_list) {
        backend_->call("createVariableMapFromCache", true, 1, "cli", "cv", &cache->cache_result, &vm_list);
        if(vm_list.size() == 0){
          continue;
        }
        /*
        for (auto vm : vm_list) {
          std::cout << vm << std::endl;
        }
        */
        hit_flag = true;
        cache->cache_vm = vm_list[0];
        consistent_cache_list_.push_back(cache);
      }
    }else{
      return false;
    }
  }
  if(hit_flag){
    cache_hit_count_++;
    return true;
  }else{
    return false;
  }
}

void CacheManager::set_phase_result(phase_result_sptr_t phase){
  Cache* cache = consistent_cache_list_[0];
  phase->variable_map = cache->cache_vm;

  phase->parent->children.push_back(phase);
  phase->simulation_state = SIMULATED;

  phase->add_diff_positive_asks(cache->diff_positive_asks);
  phase->add_diff_negative_asks(cache->diff_negative_asks);
  phase->diff_sum = cache->diff_sum;
  phase->unadopted_ms = cache->unadopted_ms;
  phase->unadopted_ms = cache->unadopted_ms;
  phase->module_diff = cache->module_diff;
  phase->inconsistent_module_sets = cache->inconsistent_module_sets;
  phase->inconsistent_constraints = cache->inconsistent_constraints;
  phase->discrete_differential_set  = cache->discrete_vs;

  phase->cache_hit = true;
  consistent_cache_list_.clear();
}

int CacheManager::get_cache_hit_count() {
  return cache_hit_count_;
}

void CacheManager::dump_cache(std::ostream& s){
  s << "---- point cache costraint ----" << endl;
  int cnt = 1;
  for (auto entry : cache_map_point_) {
    s << cnt << endl;
    s << "  cond:" << endl;
    s << "    " << get<0>(entry.first) << "," << get<1>(entry.first) << "," << get<2>(entry.first) << endl;
    s << "  constraint:" << endl;
    for (auto cache : entry.second) {
      s << "    " << cache->cache_result.first << ":" << cache->cache_result.second << endl;
    }
    s << endl;
    cnt++;
  }
  cnt = 1;

  s << "---- interval cache costraint ----" << endl;
  cnt = 1;
  for (auto entry : cache_map_interval_) {
    s << cnt << endl;
    s << "  cond:" << endl;
    s << "    " << get<0>(entry.first) << "," << get<1>(entry.first) << "," << get<2>(entry.first) << endl;
    s << "  constraint:" << endl;
    for (auto cache : entry.second) {
      s << "    " << cache->cache_result.first << ":" << cache->cache_result.second << endl;
    }
    s << endl;
    cnt++;
  }
}

} // namespace simulator
} // namespace hydla 
