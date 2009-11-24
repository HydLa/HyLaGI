
#include <boost/shared_ptr.hpp>

#include "ModuleSetList.h"

#include "BPSimulator.h"

using namespace hydla::ch;
using namespace hydla::bp_simulator;

/**
 * RealPaver���g�p����Branch and Prune�ɂ��V�~�����[�V����
 */
void branch_and_prune_simulate(boost::shared_ptr<hydla::ch::ModuleSetContainer> msc) 
{
  BPSimulator bps;
  bps.simulate(msc);
}
