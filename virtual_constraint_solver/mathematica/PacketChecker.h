#ifndef _INCLUDED_HYDLA_PACKET_CHECKER_H_
#define _INCLUDED_HYDLA_PACKET_CHECKER_H_

#include "mathlink_helper.h"

class PacketChecker {
public:
  PacketChecker(MathLink& ml);

  virtual ~PacketChecker();

  virtual void check();
  virtual void check_after_return();

  virtual void strCase();
  virtual void symCase();
  virtual void intCase();
  virtual void funcCase();

private:
  MathLink& ml_;

};

#endif //_INCLUDED_HYDLA_PACKET_CHECKER_H__

