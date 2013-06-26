#ifndef _INCLUDED_HYDLA_PARSE_ERROR_H_
#define _INCLUDED_HYDLA_PARSE_ERROR_H_

#include <string>
#include <sstream>
#include <stdexcept>

#include <boost/lexical_cast.hpp>

#include "Node.h"

namespace hydla {
namespace parse_error {

class ParseError : public std::runtime_error {
public:
  ParseError(const std::string& msg, 
             int line = -1) : 
    std::runtime_error(init("", msg, line))
  {}

  ParseError(const hydla::parse_tree::node_sptr& node, 
             int line = -1) : 
    std::runtime_error(
      init("", 
           boost::lexical_cast<std::string>(*node), 
           line))
  {}
  
  ParseError(const std::string& tag, 
             const hydla::parse_tree::node_sptr& node, 
             int line = -1) : 
    std::runtime_error(
      init(tag, 
           boost::lexical_cast<std::string>(*node), 
           line))
  {}

  ParseError(const std::string& tag, 
             const std::string& msg, 
             int line = -1) : 
    std::runtime_error(init(tag, msg, line))
  {}

private:
  std::string init(const std::string& tag, 
                   const std::string& msg, 
                   int line)
  {
    std::stringstream s;
    if(!tag.empty()) s << tag << " : ";
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
  SyntaxError(const std::string& name, int line = -1) :
    ParseError("syntax error", name, line)    
  {}
};

/**
 * �����v���O�����̑��d��`���Ɏg�p����N���X
 */
class MultipleDefinition : public ParseError {
public:
  MultipleDefinition(const std::string& name, 
                     int line = -1) :
    ParseError("multiple definition", name, line)    
  {}
};

/**
 * ��`����Ă��Ȃ������v���O�������Q�Ƃ������Ɏg�p����N���X
 */
class UndefinedReference : public ParseError {
public:
  UndefinedReference(const hydla::parse_tree::node_sptr& node, 
                     int line = -1) :
    ParseError("undefined reference", node, line)    
  {}
};

/**
 * �����v���O�����̏z�Q�Ƃ����݂����ۂɔ��������O�N���X
 */
class CircularReference : public ParseError {
public:
  CircularReference(const hydla::parse_tree::node_sptr& node, 
                    int line = -1) :
    ParseError("circular reference", node, line)
  {}
};


/**
 * �����ȃR�}���h���w�肳�ꂽ�Ƃ��ɔ�������N���X
 */
class InvalidCommand : public ParseError {
public:
  InvalidCommand(const std::string& name, 
                    int line = -1) :
    ParseError("InvalidCommand", line)
  {}
};

class BinNodeError : public ParseError {
public:
  BinNodeError(const std::string& tag,
               const hydla::parse_tree::node_sptr& lhs,
               const hydla::parse_tree::node_sptr& rhs, 
               int line = -1) :
    ParseError(tag,
               msg(lhs, rhs), 
               line)    
  {}

private:
  std::string msg(const hydla::parse_tree::node_sptr& lhs,
                  const hydla::parse_tree::node_sptr& rhs)
  {
    std::stringstream s;
    s << "between '" 
      << lhs
      <<"' and '" 
      << rhs 
      << "'";
    return s.str();
  }
};


/**
 * ����łȂ����́i�v���O�������j�̘A�����Ƃ낤�Ƃ����ۂɔ��������O�N���X
 */
class InvalidConjunction : public BinNodeError {
public:
  InvalidConjunction(const hydla::parse_tree::node_sptr& lhs,
                     const hydla::parse_tree::node_sptr& rhs, 
                     int line = -1) :
    BinNodeError("cannot conbine using conjunction", 
                 lhs, rhs, 
                 line)    
  {}
};

/**
 * �K�[�h�����łȂ����񓯎m��I���Ō������悤�Ƃ����ۂɔ��������O�N���X
 */
class InvalidDisjunction : public BinNodeError {
public:
  InvalidDisjunction(const hydla::parse_tree::node_sptr& lhs,
                     const hydla::parse_tree::node_sptr& rhs, 
                     int line = -1) :
    BinNodeError("cannot conbine using disjunction", 
                 lhs, rhs, 
                 line)    
  {}
};

/**
 * ������ŕ��񍇐��������Ȃ����Ƃ����ۂɔ��������O�N���X
 */
class InvalidParallelComposition : public BinNodeError {
public:
  InvalidParallelComposition(const hydla::parse_tree::node_sptr& lhs,
                             const hydla::parse_tree::node_sptr& rhs, 
                             int line = -1) :
    BinNodeError("invalid parallel composition",
                 lhs, rhs, 
                 line)    
  {}
};

/**
 * ������Ŏ㍇���������Ȃ����Ƃ����ۂɔ��������O�N���X
 */
class InvalidWeakComposition : public BinNodeError {
public:
  InvalidWeakComposition(const hydla::parse_tree::node_sptr& lhs,
                         const hydla::parse_tree::node_sptr& rhs, 
                         int line = -1) :
    BinNodeError("invalid weak composition",
                 lhs, rhs, 
                 line)    
  {}
};

/**
 * �K�[�h��������Always������g�p�����ۂɔ��������O�N���X
 */
class InvalidAlways : public ParseError {
public:
  InvalidAlways(const hydla::parse_tree::node_sptr& child, 
                int line = -1) :
    ParseError("invalid always. do not use always operator in guard constraints", 
                 child, 
                 line)    
  {}
};

/**
 * ���ɑ΂��Ĕ����������ۂɔ��������O�N���X
 */
class InvalidDifferential : public ParseError {
public:
  InvalidDifferential(const hydla::parse_tree::node_sptr& own, 
                          int line = -1) :
    ParseError("Sorry, applying differential operator to expression is not supported", 
                 own, 
                 line)    
  {}
};

/**
 * ���ɑ΂���prev��K�p�����ۂɔ��������O�N���X
 */
class InvalidPrevious : public ParseError {
public:
  InvalidPrevious(const hydla::parse_tree::node_sptr& own, 
                          int line = -1) :
    ParseError("Sorry, applying previous operator to expression is not supported", 
                 own, 
                 line)    
  {}
};



} //namespace parse_error
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_ERROR_H_
