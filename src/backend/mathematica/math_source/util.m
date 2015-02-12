
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
  toReturnForm[Simplify[expr /. t -> t - time]]
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
