(*
 * 式に対して与えられた時間を適用する
 *)

publicMethod[
  applyTime2Expr,
  expr, time,
  Module[
    {appliedExpr},
    (* FullSimplifyだと処理が重いが，SimplifyだとMinimize:ztestが出現しやすい *)
    appliedExpr = (expr /. t -> time);
    (* appliedExpr = FullSimplify[(expr /. t -> time)]; *)
    If[Element[appliedExpr, Reals] =!= False,
      toReturnForm[appliedExpr],
      Message[applyTime2Expr::nrls, appliedExpr]
    ]
  ]
];

applyTime2Expr::nrls = "`1` is not a real expression.";


(* 
 * 与えられたtの式をタイムシフト
 *)

publicMethod[
  exprTimeShift,
  expr, time,
  toReturnForm[expr /. t -> t - time]
];

