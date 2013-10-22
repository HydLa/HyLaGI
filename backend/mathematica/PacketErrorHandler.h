#ifndef _INCLUDED_HYDLA_SOLVER_MATHEMATICA_PACKET_ERROR_HANDLER_H_
#define _INCLUDED_HYDLA_SOLVER_MATHEMATICA_PACKET_ERROR_HANDLER_H_

namespace hydla {
namespace backend {
namespace mathematica {

class MathLink;

class PacketErrorHandler
{
public:
  static bool handle(MathLink* ml);
};

} // namespace mathematica
} // namespace backend
} // namespace hydla 

#endif // include guard
