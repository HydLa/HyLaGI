#ifndef _INCLUDED_HYDLA_HYDLA_GRAMMAR_RULE_H_
#define _INCLUDED_HYDLA_HYDLA_GRAMMAR_RULE_H_

namespace hydla {
namespace grammer_rule {

typedef enum _RuleID {
  RI_Number = 1, 
  RI_PrevVariable,
  RI_BoundVariable,
  RI_Variable,
  RI_Identifier,
  RI_ProgramName,

  // ˜_—‰‰Zq
  RI_LogicalAnd,
  RI_LogicalOr,

  // ”äŠr‰‰Zq
  RI_CompOp,
  RI_Less,
  RI_LessEqual,
  RI_Greater,
  RI_GreaterEqual,
  RI_Equal,
  RI_UnEqual,

  // Zp’P€‰‰Zq
  RI_Negative,
  RI_Positive,

  // Zp“ñ€‰‰Zq
  RI_Plus,
  RI_Subtract,
  RI_Times,
  RI_Divide,

  // §–ñŠK‘w’è‹`‰‰Zq
  RI_Weaker,
  RI_Parallel,

  // ”÷•ª
  RI_Differential,
  
  // ¶‹ÉŒÀ
  RI_Previous,

  // ‘Š‰‰Zq (Always = Globally)
  RI_Always,

  RI_Implies,
  RI_Equivalent,

  RI_FuncCall,
  RI_Function,

  RI_Expression,
  RI_Factor,
  RI_Limit,
  RI_Diff,
  RI_Unary,
  RI_Arith_term,
  RI_Arithmetic,
  RI_AlwaysTerm,
  RI_Ask,
  RI_Tell,
  RI_Logical_term,
  RI_Logical,

  RI_Ask_Logical_Term,
  RI_Ask_Logical,
  RI_Comparison,

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

  RI_Statements,
  RI_HydLaProgram,

} RuleID;

} //namespace hydla
} //namespace grammer_rule


#endif //_INCLUDED_HYDLA_HYDLA_GRAMMAR_RULE_H_
