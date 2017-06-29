#include "iostream"
#include "NeverClaim.h"
#include "PropertyNode.h"
#include <assert.h>

namespace hydla {
namespace never_claim {

using namespace std;
NeverClaim::NeverClaim()
{
}

void NeverClaim::addNewLabel(string labelName)
{
  if (isRegistered(labelName))
  {
    return;
  }

  nodes.push_back(make_shared<NeverClaimLabel>(labelName));
}

shared_ptr<NeverClaim::NeverClaimLabel> NeverClaim::getLabel(string labelName) const
{
  for (auto & x : nodes)
  {
    if (labelName == x->getName())
    {
      return x;
    }
  }

  return nullptr;
}

void NeverClaim::printAllLabels() const
{
  for (auto & x : nodes)
  {
    cout << x->getName() << endl;
  }
}

void NeverClaim::printAllEdges() const
{
  for (auto & n : nodes)
  {
    for (auto & edge : n->directedEdges)
    {
      cout << "[WARNING] This function needs to be fixed!" << endl;
      cout << "[" << n->getName() << " -> " << edge.second.lock()->getName()
      << " if (" << edge.first->get_node_type_name() << ")]" << endl;
    }
  }
}

bool NeverClaim::isRegistered(string labelName) const
{
  for (auto & x : nodes)
  {
    if (labelName == x->getName())
    {
      return true;
    }
  }

  return false;
}

void NeverClaim::addEdge(string from, string to, GuardCondition guard)
{
  assert(isRegistered(from));
  assert(isRegistered(to));

  auto fromLabel = getLabel(from);
  auto toLabel = getLabel(to);
  fromLabel->addEdge(DirectedEdge(guard, toLabel));

  return;
}

NeverClaim::BuchiAutomatonSptr NeverClaim::createBA() const
{
  using namespace simulator;
  using namespace symbolic_expression;

  using NamedNodePtr = pair<string, PropertyNode *>;

  auto buchiAutomaton = make_shared<simulator::Automaton>();
  int id = 0;
  shared_ptr<NeverClaimLabel> initLabel;
  PropertyNode *initNode;
  vector<NamedNodePtr> namedNodePtrList;

  // Construct a property node for each label
  for (auto & label : nodes)
  {
    PropertyNodeType type =
      label->getName().find("accept") != string::npos ? ACCEPTANCE_CYCLE : NORMAL;

    namedNodePtrList.push_back(NamedNodePtr(label->getName(), new PropertyNode(id++, type)));
    // cout << "Property node " << label->getName() << " was added to namedNodePtrList" << endl;
    
    // Look for the initial node (assumed to contain the substring "init")
    if (label->getName().find("init") != string::npos)
    {
      initLabel = label;
      initNode = namedNodePtrList.back().second;
      // cout << namedNodePtrList.back().first << endl;;
    }
  }

  {
    // cout << "Construct edges for each node\n";
    auto it = nodes.begin();
    for (auto & nodePtr : namedNodePtrList)
    {
      assert(it != nodes.end());
      for (auto & edge : (*it)->directedEdges)
      {
        string nextNodeName = edge.second.lock()->getName();
        PropertyNode *nextNode = nullptr; 

        // Look for a <PropertyNode *> object
        // with the name (= nextNodeName) in namedNodePtrList
        for (auto & x : namedNodePtrList)
        {
          if (x.first == nextNodeName)
          {
            nextNode = x.second;
            break;
          }
        }

        assert(nextNode != nullptr);
        // auto guard = GuardCondition(new True());
        auto guard = edge.first;
        // guard->dump(cout);
        // ここで第二引数として遷移条件 guard を渡す
        nodePtr.second->add_edge(nextNode, guard);
        // cout << "An edge (" << (*it)->getName() << " -> " << nextNodeName << ") was added." << endl;
      }

      ++it;
    }
  }

  buchiAutomaton->initial_node = initNode;

  return buchiAutomaton;
}

void NeverClaim::NeverClaimLabel::addEdge(DirectedEdge edge)
{
  // This function may create multiple edges, but it shouldn't.
  directedEdges.push_back(edge);
}

} //namespace never_claim
} //namespace hydla

