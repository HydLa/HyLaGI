#ifndef _INCLUDED_HYDLA_PARSE_ERROR_H_
#define _INCLUDED_HYDLA_PARSE_ERROR_H_

#include <string>
#include <sstream>
#include <stdexcept>

namespace hydla {
namespace parse_error {

class ParseError : public std::runtime_error {
public:
  ParseError(const std::string& msg) : 
    std::runtime_error(init(msg, -1))
  {}

  ParseError(const std::string& msg, int line) : 
    std::runtime_error(init(msg, line))
  {}

  ParseError(const std::string& tag, 
             const std::string& msg) : 
    std::runtime_error(init(tag + " : " + msg, -1))
  {}

  ParseError(const std::string& tag, 
             const std::string& msg, 
             int line) : 
    std::runtime_error(init(tag + " : " + msg, line))
  {}

private:
  std::string init(const std::string& msg, int line)
  {
    std::stringstream s;
    s << msg;
    if(line>0) s << " : line - " << line;
    return s.str();
  }
};

/**
 * 文法エラー時に使用するクラス
 */
class SyntaxError : public ParseError {
public:
  SyntaxError(const std::string& name) :
      ParseError("syntax error", name)    
  {}

  SyntaxError(const std::string& name, int line) :
    ParseError("syntax error", name, line)    
  {}
};

/**
 * 制約やプログラムの多重定義時に使用するクラス
 */
class MultipleDefinition : public ParseError {
public:
  MultipleDefinition(const std::string& name) :
      ParseError("multiple definition", name)    
  {}

  MultipleDefinition(const std::string& name, int line) :
    ParseError("multiple definition", name, line)    
  {}
};

/**
 * 定義されていない制約やプログラムを参照した時に使用するクラス
 */
class UndefinedReference : public ParseError {
public:
  UndefinedReference(const std::string& name) :
      ParseError("undefined reference", name)    
  {}

  UndefinedReference(const std::string& name, int line) :
    ParseError("undefined reference", name, line)    
  {}
};

/**
 * 制約やプログラムの循環参照が存在した際に発生する例外クラス
 */
class CircularReference : public ParseError {
public:
  CircularReference(const std::string& name) :
    ParseError("circular reference", name)
  {}

  CircularReference(const std::string& name, int line) :
    ParseError("circular reference", name, line)
  {}
};

/**
 * 制約でないもの（プログラム等）の連言をとろうとした際に発生する例外クラス
 */
class InvalidConjunction : public ParseError {
public:
  InvalidConjunction(const std::string& lhs,
                     const std::string& rhs) :
    ParseError(tag(), msg(lhs, rhs))    
  {}

  InvalidConjunction(const std::string& lhs,
                     const std::string& rhs, 
                     int line) :
    ParseError(tag(), msg(lhs, rhs), line)    
  {}

private:
  std::string tag()
  {
    return "cannot conbine using conjunction";
  }

  std::string msg(const std::string& lhs,
    const std::string& rhs)
  {
    return "between '" + lhs + "' and '" + rhs + "'";
  }
};

/**
 * ガード条件でない制約同士を選言で結合しようとした際に発生する例外クラス
 */
class InvalidDisjunction : public ParseError {
public:
  InvalidDisjunction(const std::string& lhs,
                     const std::string& rhs) :
    ParseError(tag(), msg(lhs, rhs))    
  {}

  InvalidDisjunction(const std::string& lhs,
                     const std::string& rhs, 
                     int line) :
    ParseError(tag(), msg(lhs, rhs), line)    
  {}

private:
  std::string tag()
  {
    return "cannot conbine using disjunction";
  }

  std::string msg(const std::string& lhs,
    const std::string& rhs)
  {
    return "between '" + lhs + "' and '" + rhs + "'";
  }
};

/**
 * 制約内で並列合成をおこなおうとした際に発生する例外クラス
 */
class InvalidParallelComposition : public ParseError {
public:
  InvalidParallelComposition(const std::string& lhs,
                           const std::string& rhs) :
    ParseError(tag(), msg(lhs, rhs))    
  {}

  InvalidParallelComposition(const std::string& lhs,
                           const std::string& rhs, 
                           int line) :
    ParseError(tag(), msg(lhs, rhs), line)    
  {}

private:
  std::string tag()
  {
    return "invalid parallel composition";
  }

  std::string msg(const std::string& lhs,
                  const std::string& rhs)
  {
    return "between '" + lhs + "' and '" + rhs + "'";
  }
};

/**
 * 制約内で弱合成をおこなおうとした際に発生する例外クラス
 */
class InvalidWeakComposition : public ParseError {
public:
  InvalidWeakComposition(const std::string& lhs,
                           const std::string& rhs) :
    ParseError(tag(), msg(lhs, rhs))    
  {}

  InvalidWeakComposition(const std::string& lhs,
                           const std::string& rhs, 
                           int line) :
    ParseError(tag(), msg(lhs, rhs), line)    
  {}

private:
  std::string tag()
  {
    return "invalid weak composition";
  }

  std::string msg(const std::string& lhs,
                  const std::string& rhs)
  {
    return "between '" + lhs + "' and '" + rhs + "'";
  }
};

/**
 * ガード条件内でAlways制約を使用した際に発生する例外クラス
 */
class InvalidAlways : public ParseError {
public:
  InvalidAlways(const std::string& child) :
    ParseError(tag(), msg(child))    
  {}

  InvalidAlways(const std::string& child, 
                int line) :
    ParseError(tag(), msg(child), line)    
  {}

private:
  std::string tag()
  {
    return "invalid always. do not use always operator in guard constraints";
  }

  std::string msg(const std::string& child)
  {
    return child;
  }
};

} //namespace parse_error
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_ERROR_H_
