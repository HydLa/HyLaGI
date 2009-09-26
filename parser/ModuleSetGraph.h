#ifndef _INCLUDED_HTDLA_MODULE_SET_GRAPH_H_
#define _INCLUDED_HTDLA_MODULE_SET_GRAPH_H_

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>

namespace hydla {
class ModuleSetGraph{
public:
  ModuleSetGraph();
  ~ModuleSetGraph();

  void dump();

private:

  //  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::property<boost::vertex_name_t, std::string> > Graph;
  typedef boost::adjacency_list<boost::vecS, boost::vecS> Graph;
  Graph graph_;

};

}

#endif // _INCLUDED_HTDLA_MODULE_SET_GRAPH_H_
