#include "ConstraintStore.h"

using namespace std;

namespace hydla
{
namespace simulator
{

ConstraintStore::ConstraintStore():is_consistent(true)
{
}

constraints_t::iterator ConstraintStore::begin()
{
  return constraints.begin();
}
constraints_t::iterator ConstraintStore::end()
{
  return constraints.end();
}

constraints_t::const_iterator ConstraintStore::begin()const
{
  return constraints.begin();
}

constraints_t::const_iterator ConstraintStore::end()const
{
  return constraints.end();
}

void ConstraintStore::add_constraint(const constraint_t &constraint)
{
  constraints.push_back(constraint);
}
void ConstraintStore::add_constraint_store(const ConstraintStore &store)
{
  constraints.insert(constraints.end(), store.begin(), store.end());
}

void ConstraintStore::clear()
{
  constraints.clear();
}

size_t ConstraintStore::size()const
{
  return constraints.size();
}

bool ConstraintStore::consistent() const
{
  return is_consistent;
}
// return if this constraint store is always true
bool ConstraintStore::valid() const
{
  return consistent() && size() == 0;
}

void ConstraintStore::set_consistency(bool cons)
{
  is_consistent = cons;
}

std::ostream &operator<<(std::ostream &ost, const ConstraintStore &store)
{
  for(auto constraint : store)
  {
    ost << parse_tree::get_infix_string(constraint) << endl;
  }
  return ost;
}

}
}
