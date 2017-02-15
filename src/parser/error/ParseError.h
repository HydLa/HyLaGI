#pragma once

#include <stdexcept>

#include "Node.h"

namespace hydla {
namespace parser {
namespace error {

class ParseError : public std::runtime_error {
public:
  ParseError(const std::string& msg, 
             int line = -1) : 
    std::runtime_error(init("", msg, line))
  {}

  ParseError(const symbolic_expression::node_sptr& node, 
             int line = -1) : 
    std::runtime_error(
      init("", 
           node->get_string(), 
           line))
  {}
  
  ParseError(const std::string& tag, 
             const symbolic_expression::node_sptr& node, 
             int line = -1) : 
    std::runtime_error(
      init(tag, 
           node->get_string(), 
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
 * 文法エラー時に使用するクラス
 */
class SyntaxError : public ParseError {
public:
  SyntaxError(const std::string& name, int line = -1) :
    ParseError("syntax error", name, line)    
  {}
};

/**
 * 制約やプログラムの多重定義時に使用するクラス
 */
class MultipleDefinition : public ParseError {
public:
  MultipleDefinition(const std::string& name, 
                     int line = -1) :
    ParseError("multiple definition", name, line)    
  {}
};

/**
 * 定義されていない制約やプログラムを参照した時に使用するクラス
 */
class UndefinedReference : public ParseError {
public:
  UndefinedReference(const symbolic_expression::node_sptr& node, 
                     int line = -1) :
    ParseError("undefined reference", node, line)    
  {}
};

/**
 * 制約やプログラムの循環参照が存在した際に発生する例外クラス
 */
class CircularReference : public ParseError {
public:
  CircularReference(const symbolic_expression::node_sptr& node, 
                    int line = -1) :
    ParseError("circular reference", node, line)
  {}
};


/**
 * 無効なコマンドが指定されたときに発生するクラス
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
               const symbolic_expression::node_sptr& lhs,
               const symbolic_expression::node_sptr& rhs, 
               int line = -1) :
    ParseError(tag,
               msg(lhs, rhs), 
               line)    
  {}

private:
  std::string msg(const symbolic_expression::node_sptr& lhs,
                  const symbolic_expression::node_sptr& rhs)
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
 * 制約でないもの（プログラム等）の連言をとろうとした際に発生する例外クラス
 */
class InvalidConjunction : public BinNodeError {
public:
  InvalidConjunction(const symbolic_expression::node_sptr& lhs,
                     const symbolic_expression::node_sptr& rhs, 
                     int line = -1) :
    BinNodeError("cannot conbine using conjunction", 
                 lhs, rhs, 
                 line)    
  {}
};

/**
 * ガード条件でない制約同士を選言で結合しようとした際に発生する例外クラス
 */
class InvalidDisjunction : public BinNodeError {
public:
  InvalidDisjunction(const symbolic_expression::node_sptr& lhs,
                     const symbolic_expression::node_sptr& rhs, 
                     int line = -1) :
    BinNodeError("cannot conbine using disjunction", 
                 lhs, rhs, 
                 line)    
  {}
};

/**
 * 制約内で並列合成をおこなおうとした際に発生する例外クラス
 */
class InvalidParallelComposition : public BinNodeError {
public:
  InvalidParallelComposition(const symbolic_expression::node_sptr& lhs,
                             const symbolic_expression::node_sptr& rhs, 
                             int line = -1) :
    BinNodeError("invalid parallel composition",
                 lhs, rhs, 
                 line)    
  {}
};

/**
 * 制約内で弱合成をおこなおうとした際に発生する例外クラス
 */
class InvalidWeakComposition : public BinNodeError {
public:
  InvalidWeakComposition(const symbolic_expression::node_sptr& lhs,
                         const symbolic_expression::node_sptr& rhs, 
                         int line = -1) :
    BinNodeError("invalid weak composition",
                 lhs, rhs, 
                 line)    
  {}
};

/**
 * ガード条件内でAlways制約を使用した際に発生する例外クラス
 */
class InvalidAlways : public ParseError {
public:
  InvalidAlways(const symbolic_expression::node_sptr& child, 
                int line = -1) :
    ParseError("invalid always. do not use always operator in guard constraints", 
                 child, 
                 line)    
  {}
};

/**
 * 式に対して微分をした際に発生する例外クラス
 */
class InvalidDifferential : public ParseError {
public:
  InvalidDifferential(const symbolic_expression::node_sptr& own, 
                          int line = -1) :
    ParseError("Sorry, applying differential operator to expression is not supported", 
                 own, 
                 line)    
  {}
};

/**
 * 式に対してprevを適用した際に発生する例外クラス
 */
class InvalidPrevious : public ParseError {
public:
  InvalidPrevious(const symbolic_expression::node_sptr& own, 
                          int line = -1) :
    ParseError("Sorry, applying previous operator to expression is not supported", 
                 own, 
                 line)    
  {}
};

/**
 * リストの添え字に無効なインデックスを指定した時に発生する例外
 */
class InvalidIndex : public ParseError {
public:
  InvalidIndex(const symbolic_expression::node_sptr& own, int line = -1) :
    ParseError("Invalid index is detected", own, line)
  {}
};


class InvalidParameter : public ParseError {
public:
  InvalidParameter(int line = -1) :
    ParseError("invalid parameter. parameter must be in form of \"p[(name), (differential_count), (phase_id)]\"", line)
  {}
};


} //namespace error
} //namespace parser
} //namespace hydla
