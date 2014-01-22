#ifndef _INCLUDED_HYDLA_BACKEND_REDUCE_LINK_FACTORY_H_
#define _INCLUDED_HYDLA_BACKEND_REDUCE_LINK_FACTORY_H_

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) 
#include "REDUCELinkTelnet.h"
#else
#include "REDUCELinkIpc.h"
#endif
#include "REDUCELink.h"

namespace hydla {
namespace backend {
namespace reduce {

/**
 * REDUCELinkの生成
 * Simple Factoryパターン
 */
class REDUCELinkFactory{
public:
  REDUCELinkFactory(){}
  ~REDUCELinkFactory(){}

  REDUCELink* createInstance(const simulator::Opts &opts){

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) 
    return new REDUCELinkTelnet(opts);
#else
    return new REDUCELinkIpc(opts);
#endif
  }
};

} // namespace reduce
} // namespace backend
} // namespace hydla

#endif // _INCLUDED_HYDLA_BACKEND_REDUCE_LINK_FACTORY_H_
