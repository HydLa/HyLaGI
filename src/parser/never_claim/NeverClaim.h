#pragma once

#include <vector>
#include <memory>
#include "Automaton.h"

namespace hydla {
namespace never_claim {

class NeverClaim
{
  class NeverClaimLabel;

public:
  using GuardCondition = symbolic_expression::node_sptr;
  using DirectedEdge = std::pair<GuardCondition, std::weak_ptr<NeverClaimLabel>>;
  using BuchiAutomatonSptr = std::shared_ptr<simulator::Automaton>;

private:
  class NeverClaimLabel
  {
  public:
    NeverClaimLabel(std::string name) : name(name) { }
    std::vector<std::weak_ptr<NeverClaimLabel>> nextEdges;
    std::string getName() const { return name; }
    void addEdge(DirectedEdge edge);
    std::vector<DirectedEdge> directedEdges;

  private:
    std::string name;
  };

public:
  NeverClaim();
  void addNewLabel(std::string labelName);
  std::shared_ptr<NeverClaimLabel> getLabel(std::string labelName) const;
  void printAllLabels() const;
  void printAllEdges() const;
  bool isRegistered(std::string labelName) const;
  void addEdge(std::string from, std::string to, GuardCondition guard);
  BuchiAutomatonSptr createBA() const;

private:
  std::vector<std::shared_ptr<NeverClaimLabel>> nodes;

};
} // namespace never_claim
} // namespace hydla

