(*
 apply specified value for expression
 *)

publicMethod[
  applyTime2Expr,
  expr, time,
  Module[
    {appliedExpr},
    appliedExpr = (expr /. t -> time);
    If[Element[appliedExpr, Reals] =!= False,
      toReturnForm[Simplify[appliedExpr]],
      Message[applyTime2Expr::nrls, appliedExpr]
    ]
  ]
];

applyTime2Expr::nrls = "`1` is not a real expression.";


(*
 shift time for expression
 *)

publicMethod[
  exprTimeShift,
  expr, time,
  toReturnForm[expr /. t -> t - time]
 (* toReturnForm[Simplify[expr /. t -> t - time]]*)
];

publicMethod[
  exprTimeShiftInverse,
  expr, time,
  toReturnForm[expr /. t -> t + time]
];


publicMethod[
  equivalent,
  lhs, rhs,
  Module[
    {},
    ToString[Reduce[Implies[lhs, rhs] && Implies[rhs, lhs], Reals] ]
  ]
];

publicMethod[
  toNumericalValue,
  expr,
  toReturnForm[Rationalize[N[expr], approxPrecision] ]
];

publicMethod[
  getSizeOfConstraint,
  ByteCount[constraint && initConstraint && prevConstraint && pConstraint]
];


(* translate given relational expression in the form of f(V) = 0 *)
publicMethod[
  relationToFunction,
  exp,
  Module[
    {
      rhs,
      lhs
      },
    If[!MemberQ[{Less, LessEqual, Equal, UnEqual, Greater, GreaterEqual}, Head[exp]],
      InvalidRelop,
      lhs = exp[[1]];
      rhs = exp[[2]];
      toReturnForm[timeConstrainedFullSimplify[lhs - rhs] ]
    ]
  ]
];

publicMethod[
  substituteVM,
  expr,
  variableMap,
  currentTime,
  Module[
    {tRemovedRules},
    tRemovedRules = Map[(Rule[#[[1]] /. x_[t] -> x, #[[2]]])&, variableMap];
    toReturnForm[((expr /. x_[t] /; isVariable[x] -> x) /. t -> t + currentTime) //. tRemovedRules]
  ]
];

publicMethod[
  differentiateWithTime,
  exp,
  toReturnForm[timeConstrainedFullSimplify[D[exp, t] ] ]
];

