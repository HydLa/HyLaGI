#include "AskCollector.h"

#include <cassert>
#include <sstream>

#include <boost/bind.hpp>
#include <boost/iterator/indirect_iterator.hpp>

#include "Logger.h"

using namespace std;
using namespace hydla::symbolic_expression;
using namespace hydla::logger;

namespace hydla {
namespace simulator {
  
AskCollector::AskCollector()
{}

AskCollector::~AskCollector()
{}

void AskCollector::collect_ask(ConstraintStore &constraints,
                               const ask_set_t*   positive_asks,
                               const ask_set_t*   negative_asks,
                               ask_set_t*         unknown_asks)
{
  assert(negative_asks);
  assert(positive_asks);
  assert(unknown_asks);

  negative_asks_   = negative_asks;
  positive_asks_   = positive_asks;
  unknown_asks_    = unknown_asks;

  for(auto constraint : constraints)
  {
    in_positive_ask_    = false;
    accept(constraint);
  }   
}


// Ask制約
void AskCollector::visit(boost::shared_ptr<hydla::symbolic_expression::Ask> node)
{  
  if(positive_asks_->find(node) != positive_asks_->end()) 
  {
    // 既に展開済みのaskノードであった場合
    if(in_positive_ask_){
      accept(node->get_child());
    }else{
      in_positive_ask_ = true;
      accept(node->get_child());
      in_positive_ask_ = false;
    }
  }
  else if(negative_asks_->find(node) == negative_asks_->end())
  {
    // 導出されるとも矛盾するとも判定されていないaskノードであった場合
    unknown_asks_->insert(node);
  }
}

} //namespace simulator
} //namespace hydla
