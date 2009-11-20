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
 * ���@�G���[���Ɏg�p����N���X
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
 * �����v���O�����̑��d��`���Ɏg�p����N���X
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
 * ��`����Ă��Ȃ������v���O�������Q�Ƃ������Ɏg�p����N���X
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
 * �����v���O�����̏z�Q�Ƃ����݂����ۂɔ��������O�N���X
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
 * ����łȂ����́i�v���O�������j�̘A�����Ƃ낤�Ƃ����ۂɔ��������O�N���X
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
 * �K�[�h�����łȂ����񓯎m��I���Ō������悤�Ƃ����ۂɔ��������O�N���X
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
 * ������ŕ��񍇐��������Ȃ����Ƃ����ۂɔ��������O�N���X
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
 * ������Ŏ㍇���������Ȃ����Ƃ����ۂɔ��������O�N���X
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
 * �K�[�h��������Always������g�p�����ۂɔ��������O�N���X
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
