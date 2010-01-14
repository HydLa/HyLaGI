#ifndef _INCLUDED_HYDLA_PACKET_CHECKER_H_
#define _INCLUDED_HYDLA_PACKET_CHECKER_H_

#include "mathlink_helper.h"

namespace hydla {
namespace symbolic_simulator {

class PacketChecker {
public:
  PacketChecker(MathLink& ml);

  virtual ~PacketChecker();

  virtual void check();
  virtual void check2();

  virtual void strCase();
  virtual void symCase();
  virtual void intCase();
  virtual void funcCase();

private:
  MathLink& ml_;

};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_PACKET_CHECKER_H__

