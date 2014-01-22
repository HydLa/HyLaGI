#ifndef _INCLUDED_MODULE_SET_CONTAINER_INITIALIZER_H_
#define _INCLUDED_MODULE_SET_CONTAINER_INITIALIZER_H_

#include "Node.h"
#include "ParseTree.h"
#include "ModuleSetContainer.h"
#include "ModuleSetGraph.h"
#include "ModuleSetList.h"
#include "IncrementalModuleSet.h"

#include "AskDisjunctionSplitter.h"
#include "AskDisjunctionFormatter.h"
#include "InitNodeRemover.h"
#include "ModuleSetContainerCreator.h"

namespace hydla{
namespace simulator{

class ModuleSetContainerInitializer {
public:
  typedef hydla::parse_tree::ParseTree parse_tree_t;
  typedef boost::shared_ptr<parse_tree_t> parse_tree_sptr;
  typedef hydla::simulator::module_set_container_sptr         module_set_container_sptr;
  template<typename MSCC>
    static void init(
        const parse_tree_sptr& parse_tree,
        module_set_container_sptr& msc_original, 
        module_set_container_sptr& msc_no_init,
        parse_tree_sptr& member_parse_tree)
    {
      hydla::ch::ModuleSetContainerCreator<MSCC> mcc;
      {
        parse_tree_sptr pt_original(boost::make_shared<parse_tree_t>(*parse_tree));
        AskDisjunctionFormatter().format(pt_original.get());
        AskDisjunctionSplitter().split(pt_original.get());
	msc_original = mcc.create(pt_original);
      }

      {
        parse_tree_sptr pt_no_init(boost::make_shared<parse_tree_t>(*parse_tree));
        InitNodeRemover().apply(pt_no_init.get());
        AskDisjunctionFormatter().format(pt_no_init.get());
        AskDisjunctionSplitter().split(pt_no_init.get());
        msc_no_init = mcc.create(pt_no_init);

        // 最適化された形のパースツリーを得る
        member_parse_tree = pt_no_init;
      }
    }
};

}
}

#endif // _INCLUDED_MODULE_SET_CONTAINER_INITIALIZER_H_
