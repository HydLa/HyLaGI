#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_PACKET_ERROR_HANDLER_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_PACKET_ERROR_HANDLER_H_


class MathLink;

namespace hydla {
namespace vcs {
namespace mathematica {

class PacketErrorHandler
{
public:
  static bool handle(MathLink* ml, int ret_code);
};

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_PACKET_ERROR_HANDLER_H_
