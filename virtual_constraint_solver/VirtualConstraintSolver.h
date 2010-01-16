#ifndef _INCLUDED_HYDLA_VCS_VIRTUAL_CONSTRAINT_SOLVER_H_
#define _INCLUDED_HYDLA_VCS_VIRTUAL_CONSTRAINT_SOLVER_H_

#include <vector>

#include "Types.h"


#include "Node.h"
#include "ParseTree.h"
#include "ModuleSetContainer.h"

#include "VariableMap.h"
#include "PhaseState.h"
#include "InitNodeRemover.h"

namespace hydla {
namespace vcs {

template<typename VariableT, typename ValueT, typename TimeT>
class VirtualConstraintSolver
{
public:  
  typedef VariableType                                       variable_t;
  typedef ValueType                                          value_t;
  typedef TimeT                                              time_t;
  typedef hydla::simulator::VariableMap<variable_t, value_t> variable_map_t;
  typedef hydla::simulator::tells_t                          tells_t;
  typedef hydla::simulator::positive_asks_t                  positive_asks_t;
  typedef hydla::simulator::negative_asks_t                  negative_asks_t;
  typedef hydla::simulator::changed_asks_t                   changed_asks_t;

  typedef struct IntegrateResult 
  {
    typedef struct NextPhaseState 
    {
      time_t         time;
      variable_map_t variable_map;
      bool           is_max_time;
    } next_phase_state_t;
    typedef std::vector<next_phase_state_t> next_phase_state_list_t;
  
    std::vector<next_phase_state_t> states;
    changed_asks_t                  changed_asks;
  } integrate_result_t;

  VirtualConstraintSolver();

  virtual ~VirtualConstraintSolver();

  /**
    ������������ 
  */
  enum Trivalent {Tri_FALSE, Tri_UNKNOWN, Tri_TRUE};

  /**
   * ���󥹥ȥ��ν�����򤪤��ʤ�
   */
  bool reset() = 0;

  /**
   * Ϳ����줿�ѿ�ɽ�򸵤ˡ����󥹥ȥ��ν�����򤪤��ʤ�
   */
  bool reset(const variable_map_t& vm) = 0;  

  /**
   * ���ߤ����󥹥ȥ������ѿ�ɽ���������
   */
  bool create_variable_map(variable_map_t& vm) = 0;

  /**
   * ������ɲä���
   */
  Trivalent add_constraint(const tells_t& collected_tells) = 0;
  
  /**
   * ���ߤ����󥹥ȥ�����Ϳ����ask��Ƴ�в�ǽ���ɤ���
   */
  Trivalent check_entailment(const boost::shared_ptr<Ask>& negative_ask) = 0;

  /**
   * ask��Ƴ�о��֤��Ѳ�����ޤ���ʬ�򤪤��ʤ�
   */
  bool integrate(
    IntegrateResult& integrate_result,
    const positive_asks_t& positive_asks,
    const negative_asks_t& negative_asks,
    const time_t& current_time,
    const time_t& max_time) = 0;
  

};

std::ostream& operator<<(std::ostream& s, 
                         const VirtualConstraintSolver::integrate_result_t & t);


} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_VIRTUAL_CONSTRAINT_SOLVER_H_
