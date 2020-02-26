#pragma once

#include "Variable.h"
#include <memory>
#include <sstream>

namespace hydla {
namespace simulator {

class PhaseResult;
class Value;

class Parameter {
public:
  Parameter(const Variable &variable, const PhaseResult &phase);

  Parameter(const std::string &name, int diff_cnt, int id);

  /**
   * create Parameter from string (like parameter[x, 0, 1]")
   */
  Parameter(const std::string &variable_str);

  Value as_value() const;

  inline std::string get_name() const { return variable_name_; }
  inline int get_differential_count() const { return differential_count_; }
  inline int get_phase_id() const { return phase_id_; }

  inline void set_name(const std::string &str) { variable_name_ = str; }
  inline void set_differential_count(int d) { differential_count_ = d; }
  inline void set_phase_id(int p) { phase_id_ = p; }

  std::string to_string() const;

  std::ostream &dump(std::ostream &s) const;

  friend bool operator<(const Parameter &lhs, const Parameter &rhs);

private:
  std::string variable_name_;
  int differential_count_;
  int phase_id_;
};

class ParameterComparator {
public:
  bool operator()(const Parameter x, const Parameter y) const;
};

std::ostream &operator<<(std::ostream &s, const Parameter &p);

} // namespace simulator
} // namespace hydla
