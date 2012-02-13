#include "ContinuityMapMaker.h"
#include "Logger.h"

#include <assert.h>
#include <iostream>

namespace hydla {
namespace simulator {
using namespace hydla::logger;
  
ContinuityMapMaker::ContinuityMapMaker()
{}

ContinuityMapMaker::~ContinuityMapMaker()
{}



// 制約式
void ContinuityMapMaker::visit(boost::shared_ptr<hydla::parse_tree::Constraint> node)
{
  accept(node->get_child());
}

// Ask制約
void ContinuityMapMaker::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{
}

// Tell制約
void ContinuityMapMaker::visit(boost::shared_ptr<hydla::parse_tree::Tell> node)
{
  accept(node->get_child());
}

// 論理積
void ContinuityMapMaker::visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// 時相演算子
void ContinuityMapMaker::visit(boost::shared_ptr<hydla::parse_tree::Always> node)
{
  accept(node->get_child());
}

// モジュールの弱合成
void ContinuityMapMaker::visit(boost::shared_ptr<hydla::parse_tree::Weaker> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// モジュールの並列合成
void ContinuityMapMaker::visit(boost::shared_ptr<hydla::parse_tree::Parallel> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// 制約呼び出し
void ContinuityMapMaker::visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node)
{
  accept(node->get_child());
}

// プログラム呼び出し
void ContinuityMapMaker::visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node)
{
  accept(node->get_child());
}



// 変数
void ContinuityMapMaker::visit(boost::shared_ptr<hydla::parse_tree::Variable> node)
{
  if(!differential_count_) return;
  continuity_map_t::iterator find = variables_.find(node->get_name());
  if(find == variables_.end() || find->second < differential_count_){
    if(negative_)
      variables_[node->get_name()] = -differential_count_ + 1;
    else
      variables_[node->get_name()] = differential_count_;
  }
}


// 微分
void ContinuityMapMaker::visit(boost::shared_ptr<hydla::parse_tree::Differential> node)
{
  differential_count_++;
  accept(node->get_child());
  differential_count_--;
}


// 左極限
void ContinuityMapMaker::visit(boost::shared_ptr<hydla::parse_tree::Previous> node)
{
  if(in_interval_){
    accept(node->get_child());
  }
}



#define DEFINE_DEFAULT_BINARY(NODE_NAME)                       \
void ContinuityMapMaker::visit(boost::shared_ptr<hydla::parse_tree::NODE_NAME> node)             \
{                                                                       \
  accept(node->get_lhs());                                              \
  accept(node->get_rhs());                                              \
}

#define DEFINE_DEFAULT_UNARY(NODE_NAME)                        \
void ContinuityMapMaker::visit(boost::shared_ptr<hydla::parse_tree::NODE_NAME> node)             \
{                                                                       \
  accept(node->get_child());                                            \
}

#define DEFINE_DEFAULT_FACTOR(NODE_NAME)                       \
void ContinuityMapMaker::visit(boost::shared_ptr<hydla::parse_tree::NODE_NAME> node)             \
{                                                                       \
}

DEFINE_DEFAULT_BINARY(Equal)
DEFINE_DEFAULT_BINARY(UnEqual)
DEFINE_DEFAULT_BINARY(Less)
DEFINE_DEFAULT_BINARY(LessEqual)
DEFINE_DEFAULT_BINARY(Greater)
DEFINE_DEFAULT_BINARY(GreaterEqual)
DEFINE_DEFAULT_BINARY(LogicalOr)
DEFINE_DEFAULT_BINARY(Plus)
DEFINE_DEFAULT_BINARY(Subtract)
DEFINE_DEFAULT_BINARY(Times)
DEFINE_DEFAULT_BINARY(Divide)
DEFINE_DEFAULT_BINARY(Power)
DEFINE_DEFAULT_BINARY(ArbitraryBinary)
DEFINE_DEFAULT_BINARY(Log)

DEFINE_DEFAULT_UNARY(Positive)
DEFINE_DEFAULT_UNARY(Negative)
DEFINE_DEFAULT_UNARY(Not)
DEFINE_DEFAULT_UNARY(Sin)
DEFINE_DEFAULT_UNARY(Cos)
DEFINE_DEFAULT_UNARY(Tan)
DEFINE_DEFAULT_UNARY(Asin)
DEFINE_DEFAULT_UNARY(Acos)
DEFINE_DEFAULT_UNARY(Atan)
DEFINE_DEFAULT_UNARY(ArbitraryUnary)
DEFINE_DEFAULT_UNARY(Ln)

DEFINE_DEFAULT_FACTOR(E)
DEFINE_DEFAULT_FACTOR(Pi)
DEFINE_DEFAULT_FACTOR(ArbitraryFactor)
DEFINE_DEFAULT_FACTOR(Number)
DEFINE_DEFAULT_FACTOR(Parameter)
DEFINE_DEFAULT_FACTOR(SymbolicT)
DEFINE_DEFAULT_FACTOR(Print)
DEFINE_DEFAULT_FACTOR(PrintPP)
DEFINE_DEFAULT_FACTOR(PrintIP)
DEFINE_DEFAULT_FACTOR(Scan)
DEFINE_DEFAULT_FACTOR(Abort)
DEFINE_DEFAULT_FACTOR(Exit)




} //namespace simulator
} //namespace hydla 
