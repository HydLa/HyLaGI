#pragma once

#include "Link.h"
#include "LinkError.h"
#include "wstp.h"
#include <string.h>

#ifdef _MSC_VER
#pragma comment(lib, "ml32i1m.lib")
#pragma comment(lib, "ml32i2m.lib")
#pragma comment(lib, "ml32i3m.lib")
#pragma comment(lib, "delayimp.lib")
#endif

namespace hydla {

namespace backend {
namespace mathematica {

class MathematicaLink : public Link {
public:
  MathematicaLink(const std::string &wstp_name, bool ignore_warnings,
                  const std::string &simplify_time, const int simplify_level,
                  const int dsolve_method, bool solve_over_reals);

  virtual ~MathematicaLink();

  /**
   * receive packets until next return packet received
   */
  bool receive_to_return_packet();

  /**
   * finalize the link
   */
  void clean();

  /**
   * reset the state of the link(to be able to send and receive new packet)
   */
  void reset();

  /**
   *   skip packets until given packet is returned
   */
  void skip_pkt_until(int pkt_name);

  inline WSENV get_env() { return env_; }
  inline WSLINK get_link() { return link_; }

  inline void put_function(const char *s, int n) { WSPutFunction(s, n); }

  inline void put_symbol(const char *s) { WSPutSymbol(s); }

  inline void put_string(const char* s) {
    WSPutString(s);
  }

  inline void put_number(const char *value) {
    WSPutFunction("ToExpression", 1);
    WSPutString(value);
  }

  void put_string(const std::string& s) { WSPutString(s.c_str()); }

  void put_integer(int i) { WSPutInteger(i); }

  void put_double(double num) { WSPutDouble(num); }

  void put_parameter(const std::string &name, int diff_count, int id);

  void get_function(std::string &name, int &cnt);

  std::string get_symbol();

  std::string get_string();

  int get_integer();

  double get_double();

  int get_arg_count();

  DataType get_type();

  std::string get_token_name(int tk_type);

  DataType get_next();

  void pre_send();

  void pre_receive();

  void post_receive();

  void set_maple_expression(const std::string &s);

  std::string get_debug_print() { return debug_print_; }

  std::string get_input_print() { return input_print_; }

  void _check();
  void check();
  void strCase();
  void symCase();
  void intCase();
  void funcCase();

  inline std::string backend_name() { return "Mathematica"; }

private:
  /////////// Mathematica Function /////////////
  int WSPutFunction(const char *s, int n) {
    return ::WSPutFunction(link_, s, n);
  }
  int WSGetFunction(const char **s, int *n) {
    return ::WSGetFunction(link_, s, n);
  }

  int WSPutInteger(int i) { return ::WSPutInteger(link_, i); }
  int WSGetInteger(int *i) { return ::WSGetInteger(link_, i); }

  int WSPutDouble(double d) { return ::WSPutDouble(link_, d); }
  int WSGetDouble(double *d) { return ::WSGetDouble(link_, d); }
  int WSPutSymbol(const char *s) { return ::WSPutSymbol(link_, s); }
  int WSGetSymbol(const char **s) { return ::WSGetSymbol(link_, s); }
  void WSReleaseSymbol(const char *s) { return ::WSReleaseSymbol(link_, s); }

  int WSPutString(const char *s) { return ::WSPutString(link_, s); }
  int WSGetString(const char **s) { return ::WSGetString(link_, s); }
  void WSReleaseString(const char *s) { return ::WSReleaseString(link_, s); }

  int WSPutNext(int type) { return ::WSPutNext(link_, type); }
  int WSGetNext() { return ::WSGetNext(link_); }

  int WSPutArgCount(int n) { return ::WSPutArgCount(link_, n); }
  int WSGetArgCount(int *n) { return ::WSGetArgCount(link_, n); }

  int WSEndPacket() { return ::WSEndPacket(link_); }
  int WSReady() { return ::WSReady(link_); }
  int WSNextPacket() { return ::WSNextPacket(link_); }
  int WSNewPacket() { return ::WSNewPacket(link_); }
  int WSGetType() { return ::WSGetType(link_); }
  int WSFlush() { return ::WSFlush(link_); }

  int WSError() { return ::WSError(link_); }

  std::string input_print_;
  std::string debug_print_;

  WSENV env_;
  WSLINK link_;

  bool on_next_;

  static const std::string par_prefix;
};

} // namespace mathematica
} // namespace backend
} // namespace hydla
