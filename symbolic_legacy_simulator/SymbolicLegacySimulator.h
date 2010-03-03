#ifndef _INCLUDED_SYMBOLIC_LEGACY_SIMULATOR_H_
#define _INCLUDED_SYMBOLIC_LEGACY_SIMULATOR_H_

#include <string>

#include "ParseTree.h"

#include "mathlink_helper.h"

namespace hydla {
namespace symbolic_legacy_simulator {

typedef std::string symbolic_time_t;

class SymbolicLegacySimulator
{
public:
  typedef boost::shared_ptr<hydla::parse_tree::ParseTree>  parse_tree_sptr;

  typedef enum OutputFormat_ {
    fmtTFunction,
    fmtNumeric,
  } OutputFormat;

  typedef struct Opts_ {
    std::string     mathlink;
    bool            debug_mode;
    bool            profile_mode;
    bool            parallel_mode;
    OutputFormat    output_format;
    symbolic_time_t max_time;
    symbolic_time_t output_interval;
    int             output_precision;
    int             approx_precision;
  } Opts;

  SymbolicLegacySimulator(const Opts& opts);
  virtual ~SymbolicLegacySimulator();

  void initialize(const parse_tree_sptr& parse_tree);
  void simulate();

private:
  void init_mathlink();

  Opts            opts_;
  parse_tree_sptr pt_;
  MathLink        ml_; 
};

} // namespace symbolic_legacy_simulator
} // namespace hydla

#endif // _INCLUDED_SYMBOLIC_LEGACY_SIMULATOR_H_
