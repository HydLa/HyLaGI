#ifndef _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_H_
#define _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_H_

namespace hydla {
namespace ch {

class ModuleSetTester;

class ModuleSetContainer {
public:
  ModuleSetContainer() 
  {}
  
  virtual ~ModuleSetContainer()
  {}

  /**
   * ‹É‘å‚È§–ñƒ‚ƒWƒ…[ƒ‹W‡‚ğ–³–µ‚‚È‚à‚Ì‚ªŒ©‚Â‚©‚é‚Ü‚Å‚½‚ß‚·
   */
  virtual bool dispatch(ModuleSetTester* tester, int threads = 1) = 0;
};

} // namespace ch
} // namespace hydla

#endif // _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_H_
