#pragma once

#include "Node.h"
#include "ParseTree.h"
#include "ModuleSetList.h"
#include "IncrementalModuleSet.h"
#include "ModuleSetGraph.h"

#include "ModuleSetContainerCreator.h"

namespace hydla {
namespace simulator {

class ModuleSetContainerInitializer
{
public:
  typedef parse_tree::ParseTree                             parse_tree_t;
  typedef boost::shared_ptr<parse_tree_t>                parse_tree_sptr;
  typedef simulator::module_set_container_sptr module_set_container_sptr;
  template<typename MSCC>
    static void init(
        const parse_tree_sptr& parse_tree,
        module_set_container_sptr& msc_original)
    {
      hierarchy::ModuleSetContainerCreator<MSCC> mcc;
      {
        parse_tree_sptr pt_original(boost::make_shared<parse_tree_t>(*parse_tree));
        msc_original = mcc.create(pt_original);
      }
    }
};

} // namespace simulator
} // namespace hydla
