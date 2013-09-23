#ifndef _INCLUDED_HYDLA_SOLVER_MATHEMATICA_PACKET_ERROR_HANDLER_H_
#define _INCLUDED_HYDLA_SOLVER_MATHEMATICA_PACKET_ERROR_HANDLER_H_


class MathLink;

namespace hydla {
namespace solver {
namespace mathematica {

class PacketErrorHandler
{
public:
  static bool handle(MathLink* ml);
};

} // namespace mathematica
} // namespace solver
} // namespace hydla 

#endif // include guard
