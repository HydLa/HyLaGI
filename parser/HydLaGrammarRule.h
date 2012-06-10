#ifndef _INCLUDED_HYDLA_HYDLA_GRAMMAR_RULE_H_
#define _INCLUDED_HYDLA_HYDLA_GRAMMAR_RULE_H_

namespace hydla {
namespace grammer_rule {

typedef enum _RuleID {
  RI_Number = 1, 
  RI_Pi,
  RI_E,
  RI_PrevVariable,
  RI_BoundVariable,
  RI_Variable,
  RI_Identifier,
  RI_ProgramName,

  // 論理演算子
  RI_LogicalAnd,
  RI_LogicalOr,
  RI_LogicalNot,

  // 比較演算子
  RI_CompOp,
  RI_Less,
  RI_LessEqual,
  RI_Greater,
  RI_GreaterEqual,
  RI_Equal,
  RI_UnEqual,

  // 算術単項演算子
  RI_Negative,
  RI_Positive,
  
  // 関数
  RI_Function,
  // サポート外関数
  RI_UnsupportedFunction,
  
  // コマンド文
  RI_Command,

  // 算術二項演算子
  RI_Plus,
  RI_Subtract,
  RI_Times,
  RI_Divide,
  RI_Power,

  // 制約階層定義演算子
  RI_Weaker,
  RI_Parallel,
  

  // 微分
  RI_Differential,
  
  // 左極限
  RI_Previous,

  // 時相演算子 (Always = Globally)
  RI_Always,
  

  RI_Implies,
  RI_Equivalent,

  RI_Expression,
  RI_Factor,
  RI_Limit,
  RI_Diff,
  RI_Unary,
  RI_Arith_term,
  RI_Power_term,
  RI_Arithmetic,
  RI_AlwaysTerm,
  RI_Ask,
  RI_Tell,
  RI_Logical_term,
  RI_Logical,

  RI_Ask_Logical_Literal,
  RI_Ask_Logical_Term,
  RI_Ask_Logical,
  RI_Comparison,
  RI_Chain,

  RI_Program,
  RI_ProgramParallel,
  RI_ProgramOrdered,
  RI_ProgramFactor,

  RI_Module,
  RI_Constraint,
  RI_ConstraintName,
  RI_ConstraintCallee,
  RI_ConstraintCaller,
  RI_ProgramCallee,
  RI_ProgramCaller,
  RI_FormalArgs,
  RI_ActualArgs,

  RI_DefStatement,
  RI_ProgramDef,
  RI_ModuleDef,
  RI_ConstraintDef,
  
  RI_Assert,
  RI_Statements,
  RI_HydLaProgram,

} RuleID;

} //namespace hydla
} //namespace grammer_rule


#endif //_INCLUDED_HYDLA_HYDLA_GRAMMAR_RULE_H_
