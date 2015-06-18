#pragma once

#include "mathlink.h"
#include "LinkError.h"
#include <string.h>
#include "Link.h"

#ifdef _MSC_VER
#pragma comment(lib, "ml32i1m.lib")
#pragma comment(lib, "ml32i2m.lib")
#pragma comment(lib, "ml32i3m.lib")
#pragma comment(lib, "delayimp.lib")
#endif


namespace hydla{

namespace backend{
namespace mathematica{

class MathematicaLink : public Link
{
public:
  MathematicaLink(const std::string &mathlink_name, bool ignore_warnings);

  virtual ~MathematicaLink() ;

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

  inline MLENV get_env()  {return env_;}
  inline MLINK get_link() {return link_;}

  inline void put_function(const char* s, int n) {
    MLPutFunction(s, n);
  }

  inline void put_symbol(const char* s) {
    MLPutSymbol(s);
  }

  inline void put_number(const char* value)
  {
    MLPutFunction("ToExpression", 1);
    MLPutString(value);
  }
  
  void put_symbol(const std::string& s) {
    MLPutSymbol(s.c_str());
  }

  void put_string(const char* s) {
    MLPutString(s);
  }

  void put_integer(int i) {
    MLPutInteger(i);
  } 

  void put_double(double num)
  {
    MLPutDouble(num);
  }

  void put_parameter(const std::string& name, int diff_count, int id);
  
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


  std::string get_debug_print()
  {
    return debug_print_;
  }
  
  std::string get_input_print()
  {
    return input_print_;
  }

  void _check();
  void check();
  void strCase();
  void symCase();
  void intCase();
  void funcCase();

  

  inline std::string backend_name(){return "Mathematica";}

private:

  /////////// Mathematica Function /////////////
  int MLPutFunction(const char *s, int n)   {return ::MLPutFunction(link_, s, n);}
  int MLGetFunction(const char **s,int *n)  {return ::MLGetFunction(link_, s, n);}

  int MLPutInteger(int i)                 {return ::MLPutInteger(link_, i);}
  int MLGetInteger(int *i)                {return ::MLGetInteger(link_, i);}

  int MLPutDouble(double d)               {return ::MLPutDouble(link_, d);}
  int MLGetDouble(double *d)               {return ::MLGetDouble(link_, d);}
  int MLPutSymbol(const char *s)          {return ::MLPutSymbol(link_, s);}
  int MLGetSymbol(const char **s)         {return ::MLGetSymbol(link_, s);}
  void MLReleaseSymbol(const char *s)     {return ::MLReleaseSymbol(link_, s);}

  int MLPutString(const char*s)           {return ::MLPutString(link_, s);}
  int MLGetString(const char **s)         {return ::MLGetString(link_, s);}
  void MLReleaseString(const char *s)     {return ::MLReleaseString(link_, s);}

  int MLPutNext(int type)                 {return ::MLPutNext(link_, type);}
  int MLGetNext()                         {return ::MLGetNext(link_);}


  int MLPutArgCount(int n)                {return ::MLPutArgCount(link_, n);}
  int MLGetArgCount(int *n)               {return ::MLGetArgCount(link_, n);}

  int MLEndPacket()                       {return ::MLEndPacket(link_);}
  int MLReady()                           {return ::MLReady(link_);}
  int MLNextPacket()                      {return ::MLNextPacket(link_);}
  int MLNewPacket()                       {return ::MLNewPacket(link_);}
  int MLGetType()                         {return ::MLGetType(link_);}      
  int MLFlush()                           {return ::MLFlush(link_);}

  int MLError()                           {return ::MLError(link_);}

  std::string input_print_;
  std::string debug_print_;

  MLENV env_;
  MLINK link_;

  bool on_next_;

  static const std::string par_prefix;
};

}
}
}

