#ifndef _INCLUDED_HTDLA_CH_MODULE_SET_H_
#define _INCLUDED_HTDLA_CH_MODULE_SET_H_

#include <vector>
#include <string>
#include <ostream>

#include <boost/shared_ptr.hpp>

#include "Node.h"

namespace hydla {
namespace ch {

typedef boost::shared_ptr<class ModuleSet> module_set_sptr;

/**
 * モジュールの集合を表すクラス
 *
 */
class ModuleSet {
public:
  typedef std::pair<std::string,
                    hydla::parse_tree::node_sptr> module_t;
  typedef std::vector<module_t>                   module_list_t;

  typedef struct ModuleComp_ {
    bool operator()(module_t& a, module_t& b)
    {
      return a.first < b.first;
    }
  } ModuleComp;

  ModuleSet();
  ModuleSet(std::string& name, hydla::parse_tree::node_sptr node);
  ModuleSet(ModuleSet& lhs, ModuleSet& rhs);

  ~ModuleSet();

  std::string get_name() const ;

  int compare(ModuleSet& rhs) const;

  std::ostream& dump(std::ostream& s)
  {
    s << get_name();
    return s;
  }

private:
  module_list_t module_list_;
};

bool operator<(ModuleSet& lhs, ModuleSet& rhs);
std::ostream& operator<<(std::ostream& s, ModuleSet& m);

} // namespace ch
} // namespace hydla

#endif //_INCLUDED_HTDLA_CH_MODULE_SET_H_
