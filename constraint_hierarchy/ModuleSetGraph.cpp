#include "ModuleSetGraph.h"

#include <boost/graph/graphviz.hpp>

using namespace std;
using namespace boost;

namespace hydla{
namespace ch {

ModuleSetGraph::ModuleSetGraph()
{}

ModuleSetGraph::ModuleSetGraph(module_set_sptr m)
{}

ModuleSetGraph::~ModuleSetGraph()
{}

void  ModuleSetGraph::add_parallel(ModuleSetGraph& parallel_module_set_graph)
{}

void  ModuleSetGraph::add_weak(ModuleSetGraph& parallel_module_set_graph)
{}

std::ostream&  ModuleSetGraph::dump(std::ostream& s)
{
  return s;
}

} // namespace ch
} // namespace hydla
