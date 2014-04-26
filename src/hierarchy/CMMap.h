#ifndef _CONDITION_MAP_H_
#define _CONDITION_MAP_H_

#include "ModuleSet.h"


namespace hydla{
  namespace hierarchy{
    typedef boost::shared_ptr<class CMMap> cm_map_sptr;
    typedef std::vector<module_set_sptr> module_set_list_t;
    typedef std::vector<cm_map_sptr> cm_map_list_t;

    class CMMap{
    public:
      CMMap();
      CMMap(hydla::symbolic_expression::node_sptr& cond);
      ~CMMap();

      // 探索済みならtrue
      bool is_searched();
      // 未探索にする
      void unsearched();
      // parents_にcmを追加
      void add_parents(const cm_map_sptr& cm);
      // children_にcmを追加
      void add_children(const cm_map_sptr& cm);
      // このCMMapのconditionを得る
      hydla::symbolic_expression::node_sptr& get_condition();
      // このCMMapがmsのsuper_cm(msを包含するモジュール集合を持っていればtrue)
      bool is_super_cm(const module_set_sptr& ms);
      // このCMMapがcmのparentsにあればtrue
      bool is_super_cm(const cm_map_sptr& cm);
      // このCMMapにモジュール集合msを追加
      void add_module_set(const module_set_sptr& ms);
      // メンバ変数を全てリセット
      void reset();
      // このCMMapのconditionをセット
      void set_condition(const hydla::symbolic_expression::node_sptr& cond);
      // parentsを得る
      cm_map_list_t get_parents();
      // childrenを再帰的に探索済みとする
      void mark_children();
      // module_set_listを返す
      module_set_list_t get_module_set_list();

      // ダンプ
      std::ostream& dump(std::ostream& s) const;

    private:
      hydla::symbolic_expression::node_sptr condition_;
      module_set_list_t ms_list_;
      cm_map_list_t parents_;
      cm_map_list_t children_;
      bool searched_;
    };
    std::ostream& operator<<(std::ostream& s, const CMMap& c);
  }
}

#endif
