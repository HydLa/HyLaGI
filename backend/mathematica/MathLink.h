#ifndef _INCLUDED_HYDLA_BACKEND_MATHEMATICA_MATHLINK_H_
#define _INCLUDED_HYDLA_BACKEND_MATHEMATICA_MATHLINK_H_

#include "mathlink.h"
#include "LinkError.h"
#include <string.h>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include "Logger.h"
#include "SymbolicLink.h"
#include <boost/bimap/bimap.hpp>


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

class MathLink : public SymbolicLink
{
public:
  MathLink(const hydla::simulator::Opts &opts) : env_(0), link_(0)
  {
    init(opts);
  }

  virtual ~MathLink() 
  {
    clean();
  }

  
  
  /**
   * 次のリターンパケットの直前までtextpktなどを受信しておく．
   */
  bool receive();
  
  void clean()
  {
    if(link_) {
      MLPutFunction("Exit", 0);
      MLEndPacket();
      MLClose(link_);
      link_ = 0;
    }
    if(env_)  {
      MLDeinitialize(env_);
      env_ = 0;
    }
  }

  /** 
   * 指定されたタイプのパケットが返ってくるまでスキップする
   */ 
  void skip_pkt_until(int pkt_name) 
  {
    int p;
    while ((p = MLNextPacket()) && p != pkt_name) {
      MLNewPacket();
    }
  }

  MLENV get_env()  {return env_;}
  MLINK get_link() {return link_;}

  /////////// Helper Function /////////////

  void put_function(const char* s, const int &n) {
    MLPutFunction(s, n);
  }
  
  int get_function(std::string &name, int &cnt)
  {
    HYDLA_LOGGER_FUNC_BEGIN(VCS);
    get_arg_count(cnt);
    get_symbol(name);
    HYDLA_LOGGER_VCS("%% cnt: ", cnt, ", name: ", name);
    HYDLA_LOGGER_FUNC_END(VCS);
    return 0;
  }

  void put_symbol(const char* s) {
    MLPutSymbol(s);
  }

  void put_number(const char* value)
  {
    MLPutFunction("ToExpression", 1);
    MLPutString(value);
  }
  
  void put_symbol(const std::string& s) {
    MLPutSymbol(s.c_str());
  }

  std::string get_symbol()
  {
    const char *s;
    if(!MLGetSymbol(&s)){
      throw LinkError("math", "get_symbol", MLError(), "input:\n" + input_print_ + "\n\ntrace:\n" + debug_print_);
    }
    std::string ret = s;
    HYDLA_LOGGER_LOCATION(VCS);
    HYDLA_LOGGER_VCS("%% symbol: ", s);
    MLReleaseSymbol(s);
    return ret;
  }


  int get_symbol(std::string& sym)
  {
    const char *s;
    if(!MLGetSymbol(&s)){
      throw LinkError("math", "get_symbol", MLError(), "input:\n" + input_print_ + "\n\ntrace:\n" + debug_print_);
    }
    sym = s;
    HYDLA_LOGGER_LOCATION(VCS);
    HYDLA_LOGGER_VCS("%% symbol: ", s);
    MLReleaseSymbol(s);
    return 0;
  }

  int put_string(const char* s) {
    return MLPutString(s);
  }
  
  int put_string(const std::string& s) {
    return MLPutString(s.c_str());
  }
  
  std::string get_string()
  {
    const char*s;
    if(!MLGetString(&s)) {
      throw LinkError("math", "get_string", MLError(), "input:\n" + input_print_ + "\n\ntrace:\n" + debug_print_);
    }
    std::string ret = s;
    HYDLA_LOGGER_LOCATION(VCS);
    HYDLA_LOGGER_VCS("%% string: ", s);
    MLReleaseString(s);
    return ret; 
  }
  
  int get_string(std::string& str)
  {
    const char *s;
    if(!MLGetString(&s)) {
      throw LinkError("math", "get_string", MLError(), "input:\n" + input_print_ + "\n\ntrace:\n" + debug_print_);
    }
    str = s;
    HYDLA_LOGGER_LOCATION(VCS);
    HYDLA_LOGGER_VCS("%%string: ", s);
    MLReleaseString(s);
    return 0;
  }


  void put_integer(const int &i) {
    MLPutInteger(i);
  } 
  
  int get_integer()
  {
    int i;
    if(!MLGetInteger(&i)) {
      throw LinkError("math", "get_integer", MLError(), "input:\n" + input_print_ + "\n\ntrace:\n" + debug_print_);
    }
    return i;
  }

  int get_arg_count(int &c)
  {
    if(!MLGetArgCount(&c)){
      throw LinkError("math", "get_arg_count", MLError(), "input:\n" + input_print_ + "\n\ntrace:\n" + debug_print_);
    }
    return 0;
  }
  
  int get_type(DataType& type){
    switch(MLGetType())
    {
    case MLTKFUNC:
      type = DT_FUNC;
      break;
    case MLTKSYM:
      type = DT_SYM;
      break;
    case MLTKSTR:
      type = DT_STR;
      break;
    case MLTKINT:
      type = DT_INT;
      break;
    default:
      assert(0);
    }
    return 0;
  }

  std::string get_token_name(const int &tk_type)
  {
    switch(tk_type)
    {
    case MLTKFUNC:
      return "MLTKFUNC";
    case MLTKSYM:
      return "MLTKSYM";
    case MLTKSTR:
      return "MLTKSTR";
    case MLTKINT:
      return "MLTKINT";
    default:
      return "unknown token";
    }
  }
  
  int get_next(DataType& type){
    int tk_type = MLGetNext();
    HYDLA_LOGGER_LOCATION(VCS);
    HYDLA_LOGGER_VCS("tk_type: ", get_token_name(tk_type));

    switch(tk_type)
    {
    case MLTKFUNC:
      type = DT_FUNC;
      break;
    case MLTKSYM:
      type = DT_SYM;
      break;
    case MLTKSTR:
      type = DT_STR;
      break;
    case MLTKINT:
      type = DT_INT;
      break;
    default:
      assert(0);
    }
    return 0;
  }
  

  int get_next(){
    int tk_type = MLGetNext();
    HYDLA_LOGGER_LOCATION(VCS);
    HYDLA_LOGGER_VCS("tk_type: ", get_token_name(tk_type));
    return 0;
  }
  

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

  bool convert(const std::string& orig, const int& orig_cnt, const bool &hydla2back, std::string& ret, int& ret_cnt);

  std::string backend_name(){return "Mathematica";}

private:

  void init(const hydla::simulator::Opts& opts);


  /////////// Mathematica Function /////////////
  int MLPutFunction(const char *s, int n)   {return ::MLPutFunction(link_, s, n);}
  int MLGetFunction(const char **s,int *n)  {return ::MLGetFunction(link_, s, n);}

  int MLPutInteger(int i)                 {return ::MLPutInteger(link_, i);}
  int MLGetInteger(int *i)                {return ::MLGetInteger(link_, i);}

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

  typedef std::pair<std::string, int> function_t;
  typedef boost::bimaps::bimap<function_t, function_t > function_map_t;
  function_map_t function_map_;
};

}
}
}

#endif // include guard
