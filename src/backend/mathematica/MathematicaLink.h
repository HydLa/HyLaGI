#pragma once

#include "mathlink.h"
#include "LinkError.h"
#include <string.h>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include "Logger.h"
#include "Link.h"


#ifdef _MSC_VER
#pragma comment(lib, "ml32i1m.lib")
#pragma comment(lib, "ml32i2m.lib")
#pragma comment(lib, "ml32i3m.lib")
#pragma comment(lib, "delayimp.lib")
#endif


namespace hydla{

namespace simulator{
  struct Opts;
}

namespace backend{
namespace mathematica{

class MathematicaLink : public Link
{
public:
  MathematicaLink(const hydla::simulator::Opts &opts);

  virtual ~MathematicaLink() ;

  /**
   * 次のリターンパケットの直前までtextpktなどを受信する
   */
  bool receive_to_return_packet();
  
  void clean();

  /** 
   * 指定されたタイプのパケットが返ってくるまでスキップする
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

  void put_float(double num)
  {
    MLPutDouble(num);
  }

  void put_variable(const std::string &name, int diff_count, const variable_form_t &variable_arg);

  void put_parameter(const std::string& name, int diff_count, int id);
  
  void get_function(std::string &name, int &cnt);

  std::string get_symbol();
  
  std::string get_string();
  
  int get_integer();

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

  void init(const hydla::simulator::Opts& opts);


  /////////// Mathematica Function /////////////
  int MLPutFunction(const char *s, int n)   {return ::MLPutFunction(link_, s, n);}
  int MLGetFunction(const char **s,int *n)  {return ::MLGetFunction(link_, s, n);}

  int MLPutInteger(int i)                 {return ::MLPutInteger(link_, i);}
  int MLGetInteger(int *i)                {return ::MLGetInteger(link_, i);}

  int MLPutDouble(double d)               {return ::MLPutDouble(link_, d);}

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


  static const std::string prev_prefix;
  static const std::string par_prefix;
  static const std::string var_prefix;

};

}
}
}

