(*
 apply specified value for expression
 *)

publicMethod[
  applyTime2Expr,
  expr, time,
  Module[
				 {appliedExpr,tmp},
    appliedExpr = (expr /. t -> time);
    If[Element[appliedExpr, Reals] =!= False,
			 debugPrint["arg",appliedExpr];
			 (*debugPrint["SimplifyCount",simplifyCount[appliedExpr][[2]]];*)
			 tmp = Timing[timeConstrainedSimplify[appliedExpr]];
			 debugPrint["tCS at aT2E",tmp[[1]]];
			 toReturnForm[tmp[[2]]],
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
	Module[
				 {tmp},
				 debugPrint["arg",expr /. t -> t - time];
				 (*debugPrint["SimplifyCount",simplifyCount[expr /. t -> t - time][[2]]];*)
				 tmp = Timing[timeConstrainedSimplify[expr /. t -> t - time]];
				 debugPrint["tCS at eTS",tmp[[1]]];
  toReturnForm[LogicalExpand[tmp[[2]]] ]
]
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
				lhs,
				tmp
      },
    If[!MemberQ[{Less, LessEqual, Equal, UnEqual, Greater, GreaterEqual}, Head[exp]],
      InvalidRelop,
      lhs = exp[[1]];
      rhs = exp[[2]];
			 debugPrint["arg",lhs - rhs];
			 (*debugPrint["SimplifyCount",simplifyCount[lhs - rhs][[2]]];*)
			 tmp = Timing[timeConstrainedSimplify[lhs - rhs]];
			 debugPrint["tCS at rTF",tmp[[1]]];
      toReturnForm[tmp[[2]]]
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
	Module[
				 {tmp},
				 debugPrint["arg",D[exp,t]];
				 (*debugPrint["SimplifyCount",simplifyCount[D[exp, t]][[2]]];*)
				 tmp = Timing[timeConstrainedSimplify[D[exp, t]]];
				 debugPrint["tCS at dWT",tmp[[1]]];
				  toReturnForm[tmp[[2]]]
]
];

