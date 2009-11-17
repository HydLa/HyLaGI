#ifndef _INCLUDED_MATHLINK_HELPER_H_
#define _INCLUDED_MATHLINK_HELPER_H_

#include "mathlink.h"
#include <string.h>

#ifdef _MSC_VER
#pragma comment(lib, "ml32i1m.lib")
#pragma comment(lib, "ml32i2m.lib")
#pragma comment(lib, "ml32i3m.lib")
#endif

class MathLink {
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

  void clean()
  {
    if(link_) {
      MLClose(link_);
      link_ = 0;
    }
    if(env_)  {
      MLDeinitialize(env_);
      env_ = 0;
    }
  }  

  /** 
   * 指定されたタイプのパケット返ってくるまでスキップする
   */ 
  void skip_pkt_until(int pkt_name) 
  {
    int p;
    while ((p = MLNextPacket()) && p != pkt_name) {
      MLNewPacket();
    }
  }

  MLENV getEnv()  {return env_;}
  MLINK getLink() {return link_;}

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


private:
  MLENV env_;
  MLINK link_;
};

#endif //_INCLUDED_MATHLINK_HELPER_H_
