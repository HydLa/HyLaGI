(*
 * 蠑上↓蟇ｾ縺励※荳弱∴繧峨ｌ縺滓凾髢薙ｒ驕ｩ逕ｨ縺吶ｋ
 *)

publicMethod[
  applyTime2Expr,
  expr, time,
  Module[
    {appliedExpr},
    (* FullSimplify縺�縺ｨ蜃ｦ逅����′驥阪＞縺鯉ｼ郡implify縺�縺ｨMinimize:ztest縺悟����迴ｾ縺励ｄ縺吶＞ *)
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
 * 荳弱∴繧峨ｌ縺殳縺ｮ蠑上ｒ繧ｿ繧､繝�繧ｷ繝輔ヨ
 *)

publicMethod[
  exprTimeShift,
  expr, time,
  toReturnForm[expr /. t -> t - time]
];


publicMethod[
  equivalent,
  lhs, rhs,
  Module[
    {},
    ToString[Reduce[Implies[lhs, rhs] && Implies[rhs, lhs], Reals] ]
  ]
];
