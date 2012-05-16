#ifndef _INCLUDED_MATHLINK_HELPER_H_
#define _INCLUDED_MATHLINK_HELPER_H_

//#include "mathlink.h"
#include "/Applications/Mathematica.app/SystemFiles/Links/MathLink/DeveloperKit/CompilerAdditions/mathlink.h"
#include <string.h>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include "Logger.h"

#ifdef _MSC_VER
#pragma comment(lib, "ml32i1m.lib")
#pragma comment(lib, "ml32i2m.lib")
#pragma comment(lib, "ml32i3m.lib")
#pragma comment(lib, "delayimp.lib")
#endif

class MathLinkError : public std::runtime_error {
public:
  MathLinkError(const std::string& msg, int code) : 
    std::runtime_error(init(msg, code))
  {}

private:
  std::string init(const std::string& msg, int code)
  {
    std::stringstream s;
    s << "mathlink error: " << msg << " : " << code;
    return s.str();
  }
};

class MathLink 
{
public:
  MathLink() : env_(0), link_(0)
  {}

  MathLink(const char *str) : env_(0), link_(0)
  {
    init(str);
  }


  MathLink(int argc, char *argv[]) : env_(0), link_(0)
  {
    init(argc, argv);
  }

  ~MathLink() 
  {
    clean();
  }

  bool init(const char *str)
  {
    if((env_ = MLInitialize(0)) == (MLENV)0) return false;
    
    int err;
    link_ = MLOpenString(env_, str, &err);
    if(link_ == (MLINK)0 || err != MLEOK) return false;

    if(!MLActivate(link_)) return false;
    return true;
  }

  bool init(int argc, char *argv[])
  {
    if((env_ = MLInitialize(0)) == (MLENV)0) return false;
    
    int err;
    link_ = MLOpenArgcArgv(env_, argc, argv, &err);
    if(link_ == (MLINK)0 || err != MLEOK) return false;

    if(!MLActivate(link_)) return false;
    return true;
  }
  
  
  /**
   * 次のリターンパケットまでデータを受信しておく．
   * 以前受信したデータは，開始時にすべて破棄する
   */
  bool receive();
  
  
  
  void strCase();
  void symCase();
  void intCase();
  void funcCase();

  
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

  int put_function(const char* s, int n) {
    return MLPutFunction(s, n);
  }
  
  int put_function(const std::string& s, int n) {
    return MLPutFunction(s.c_str(), n);
  }

  std::pair<std::string, int> get_function_()
  {
    const char *s;
    int n;
    if(MLGetFunction(&s, &n) == 0) {
      throw MathLinkError("get_function", MLError());
    }
    std::string sym(s);
    MLReleaseSymbol(s);
    return make_pair(sym, n);
  }

  int put_symbol(const char* s) {
    return MLPutSymbol(s);
  }
  
  int put_symbol(const std::string& s) {
    return MLPutSymbol(s.c_str());
  }

  std::string get_symbol()
  {
    if(token_list_.size() <= 0){
      throw MathLinkError("empty token list in get_symbol", 0);
    }
    if(token_list_.front()!=MLTKSYM){
      throw MathLinkError("illegal token in get_symbol", token_list_.front());
    }
    if(string_list_.size() <= 0){
      throw MathLinkError("empty string list in get_symbol", 0);
    }
    std::string str = string_list_.front();
    string_list_.pop_front();
    token_list_.pop_front();
    return str;
  }

  int put_string(const char* s) {
    return MLPutString(s);
  }
  
  int put_string(const std::string& s) {
    return MLPutString(s.c_str());
  }

  std::string get_string()
  {
    if(token_list_.size() <= 0){
      throw MathLinkError("empty token list in get_string", 0);
    }
    if(token_list_.front()!=MLTKSTR){
      throw MathLinkError("illegal token in get_string", token_list_.front());
    }
    if(string_list_.size()<=0){
      throw MathLinkError("empty string list in get_string", 0);
    }
    std::string str = string_list_.front();
    string_list_.pop_front();
    token_list_.pop_front();
    return str;
  }
  
  
  std::string get_string_()
  {
    const char *s;
    if(!MLGetString(&s)) {
      throw MathLinkError("get_string", MLError());
    }
    std::string str(s);
    MLReleaseString(s);
    return str;
  }


  int put_integer(int i) {
    return MLPutInteger(i);
  }

  int get_integer()
  {
    if(token_list_.size() <= 0){
      throw MathLinkError("empty token list in get_int", 0);
    }
    if(token_list_.front() != MLTKINT){
      throw MathLinkError("illegal token in get_integer", token_list_.front());
    }
    if(int_list_.size() <= 0){
      throw MathLinkError("empty int list in get_integer", 0);
    }
    int i = int_list_.front();
    int_list_.pop_front();
    token_list_.pop_front();
    return i;
  }
  
  
  int get_integer_()
  {
    int i;
    if(!MLGetInteger(&i)) {
      throw MathLinkError("get_integer", MLError());      
    }
    return i;
  }

  int get_arg_count()
  {
    if(token_list_.size() <= 0){
      throw MathLinkError("empty token list in get_arg_count", 0);
    }
    if(token_list_.front()!=MLTKFUNC){
      throw MathLinkError("illegal token in get_arg_count", token_list_.front());
    }
    if(int_list_.size()<=0){
      throw MathLinkError("empty int list in get_arg_count", 0);
    }
    int count = int_list_.front();
    int_list_.pop_front();
    token_list_.pop_front();
    return count;
  }

  int get_arg_count_()
  {
    int count;
    if(!MLGetArgCount(&count)){
      throw MathLinkError("get_arg_count", MLError());
    }
    return count;
  }
  
  int get_type(){
    if(token_list_.size() <= 0){
      throw MathLinkError("empty list in get_type", 0);
    }
    return token_list_.front();
  }
  
  int get_next(){
    if(token_list_.size() <= 0){
      throw MathLinkError("empty list in get_next", 0);
    }
    switch(token_list_.front()){
      case MLTKFUNC:
      case MLTKINT:
        if(int_list_.size() <= 0){
          throw MathLinkError("empty int list in get_next", 0);
        }
        int_list_.pop_front();
        break;
      case MLTKSYM:
      case MLTKSTR:
        if(string_list_.size() <= 0){
          throw MathLinkError("empty string list in get_next", 0);
        }
        string_list_.pop_front();
        break;
      default:
        throw MathLinkError("illegal element in get_next", 0);
        break;
    }
    token_list_.pop_front();
    return token_list_.front();
  }


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

private:
  std::list<std::string> string_list_;
  std::list<int>         int_list_;
  std::list<int>         token_list_;
  MLENV env_;
  MLINK link_;
};

#endif //_INCLUDED_MATHLINK_HELPER_H_
