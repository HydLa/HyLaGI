#include "ModuleSetGraph.h"

#include <boost/graph/graphviz.hpp>

using namespace std;
using namespace boost;

namespace hydla{

ModuleSetGraph::ModuleSetGraph()
{
   add_vertex(graph_);
   //boost::put(boost::vertex_name_t, graph_, 0, "A");
}
  
ModuleSetGraph::~ModuleSetGraph()
{}

void ModuleSetGraph::dump()
{
  // 頂点インデックスのためのプロパティマップを得る
  typedef property_map<Graph, vertex_index_t>::type IndexMap;
  IndexMap index = get(vertex_index, graph_);

  std::cout << "vertices(g) = ";
  typedef graph_traits<Graph>::vertex_iterator vertex_iter;
  std::pair<vertex_iter, vertex_iter> vp;
  for (vp = vertices(graph_); vp.first != vp.second; ++vp.first)
    std::cout << index[*vp.first] <<  " ";
  std::cout << std::endl;


  std::cout << "edges(g) = ";
  graph_traits<Graph>::edge_iterator ei, ei_end;
  for (tie(ei, ei_end) = edges(graph_); ei != ei_end; ++ei)
    std::cout << "(" << index[source(*ei, graph_)]
  << "," << index[target(*ei, graph_)] << ") ";
  std::cout << std::endl;

  write_graphviz(std::cout, graph_);
}

}
