#ifndef _INCLUDED_HYDLA_VCS_REDUCE_LINK_FACTORY_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_LINK_FACTORY_H_

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) 
#include "REDUCELinkTelnet.h"
#else
#include "REDUCELinkIpc.h"
#endif
#include "REDUCELink.h"

namespace hydla {
namespace vcs {
namespace reduce {

/**
 * REDUCELinkの生成
 * Simple Factoryパターン
 */
class REDUCELinkFactory{
public:
  REDUCELinkFactory(){}
  ~REDUCELinkFactory(){}

  REDUCELink* createInstance(){

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) 
    return new REDUCELinkTelnet();
#else
    return new REDUCELinkIpc();
#endif
  }
};

} // namespace reduce
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REDUCE_LINK_FACTORY_H_

