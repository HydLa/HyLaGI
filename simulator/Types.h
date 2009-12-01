#ifndef _INCLUDED_HYDLA_SIMULATOR_TYPES_H_
#define _INCLUDED_HYDLA_SIMULATOR_TYPES_H_

#include <vector>
#include <set>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "ModuleSet.h"

namespace hydla {
namespace simulator {

typedef hydla::ch::ModuleSet module_set_t;
typedef std::vector<boost::shared_ptr<hydla::parse_tree::Always> > expanded_always_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Always> > visited_always_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Ask> > positive_asks_t;


typedef boost::shared_ptr<hydla::parse_tree::Ask>            negative_ask_t;
typedef std::set<negative_ask_t>                             negative_asks_t;

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_TYPES_H_

