(*
 * ¥Ç¥Ð¥Ã¥¯ÍÑ¥á¥Ã¥»¡¼¥¸½ÐÎÏ´Ø¿ô
 *)
If[optUseDebugPrint,
  debugPrint[arg___] := Print[InputForm[{arg}]],
  debugPrint[arg___] := Null];

(*
 * checkEntailment¤ÎIP¥Ð¡¼¥¸¥ç¥ó
 * 0 : Solver Error
 * 1 : Æ³½Ð²ÄÇ½
 * 2 : Æ³½ÐÉÔ²ÄÇ½
 *)
checkEntailmentInterval[guard_, store_, vars_, pars_] := Quiet[Check[Block[
  {tStore, sol, integGuard, otherExpr, minT},
  debugPrint["guard:", guard, "store:", store, "vars:", vars, "pars:", pars];
  sol = exDSolve[store, vars];
  (*debugPrint["sol:", sol];*)
  If[sol =!= overconstraint && sol =!= underconstraint,
    tStore = sol[[1]];
    otherExpr = sol[[2]];
    (* guard¤ÈotherExpr¤ËtStore¤òÅ¬ÍÑ¤¹¤ë *)
    integGuard = guard /. tStore;
    otherExpr = otherExpr /. tStore;
    If[integGuard =!= False,
      (* ¤½¤Î·ë²Ì¤Èt>0¤È¤òÏ¢Î© *)
      (*debugPrint["integGuard:", integGuard];*)
      sol = Quiet[Check[Reduce[{integGuard && t > 0 && (And@@otherExpr)}, t],
                        False, {Reduce::nsmet}], {Reduce::nsmet}];
      (* Inf¤ò¼è¤Ã¤Æ0¤Ë¤Ê¤ì¤ÐEntailed *)
      If[sol =!= False,
        minT = Quiet[First[Minimize[{t, sol}, Append[pars,t]]]];
        If[minT === 0,
          (* ½½Ê¬¾ò·ï¤«¤É¤¦¤«È½Äê¤·¤¿¤¤ *)
          (* MinValue¤Ç¤Ï¤Ê¤¯Minimize¤ò»ÈÍÑ¤¹¤ë *)
          (* If[Reduce[ForAll[pars, And@@otherExpr, MinValue[{t, sol}, t] == 0]] =!= False, *)
          If[Quiet[Reduce[ForAll[pars, And@@otherExpr, First[Minimize[{t, sol}, t]] == 0]]] =!= False,
            {1},
            {3}
          ],
          {2}
        ],
        {2}
      ],
      {2}
    ],
    {2}
  ]
],
  {0, $MessageList}
]];

(*
 * ¤¢¤ë1¤Ä¤ÎaskÀ©Ìó¤Ë´Ø¤·¤Æ¡¢¤½¤Î¥¬¡¼¥É¤¬À©Ìó¥¹¥È¥¢¤«¤éentail¤Ç¤­¤ë¤«¤É¤¦¤«¤ò¥Á¥§¥Ã¥¯
 *  0 : Solver Error
 *  1 : Æ³½Ð²ÄÇ½
 *  2 : Æ³½ÐÉÔ²ÄÇ½
 *  3 : ÉÔÌÀ ¡ÊÍ×Ê¬´ô¡Ë
 *)
checkEntailment[guard_, store_, vars_] := Quiet[Check[Block[
  {sol, nsol},
  debugPrint["guard:", guard, "store:", store, "vars:", vars];

  sol = Reduce[Append[store, guard], vars, Reals];
  If[sol=!=False, 
    nsol = Reduce[Append[store, Not[sol]], vars, Reals];
    If[nsol === False, 
      {1}, 
      {3}
    ],
    {2}
  ]
],
  {0, $MessageList}
]];

(* Print[checkEntailment[ht==0, {}, {ht}]] *)

renameVar[varName_] := Block[
  {renamedVarName, derivativeCount = 0, prevFlag = 0,
   getDerivativeCountPoint, removeUsrVar
  },  

  getDerivativeCountPoint[Derivative[n_][var_]] := n;
  removeUsrVar[var_] := First[StringCases[ToString[var], "usrVar" ~~ x__ -> x]];
  removeP[par_] := First[StringCases[ToString[par], "p" ~~ x__ -> x]];

  (* ÊÑ¿ôÌ¾¤Ë'¤¬¤Ä¤¯¾ì¹ç¤Î½èÍý *)
  If[MemberQ[{varName}, Derivative[n_][x_], Infinity],
    derivativeCount = getDerivativeCountPoint[varName];
    renamedVarName = removeDash[varName],
    renamedVarName = varName
  ];
  (* ÊÑ¿ôÌ¾¤Ëprev¤¬¤Ä¤¯¾ì¹ç¤Î½èÍý *)
  If[Head[renamedVarName] === prev,
    renamedVarName = First[renamedVarName];
    prevFlag = 1
  ];

  (*ÊÑ¿ôÌ¾¤ÎÆ¬¤Ë¤Ä¤¤¤Æ¤¤¤ë "usrVar"¤ò¼è¤ê½ü¤¯ ¡ÊÌ¾Á°¤ÎÆ¬¤¬usrVar¤¸¤ã¤Ê¤¤¤Î¤ÏinvalidVar¤«Äê¿ôÌ¾¡Ë *)
  If[StringMatchQ[ToString[renamedVarName], "usrVar" ~~ x__],
    (* true *)
    renamedVarName = removeUsrVar[renamedVarName];
    (* ¤³¤Î»þÅÀ¤ÇÃ±ÂÎ¤ÎÊÑ¿ôÌ¾¤Î¤Ï¤º¡¥ *)
    If[Length[renamedVarName] === 0,
      {renamedVarName, derivativeCount, prevFlag},
      (* ¼°·Á¼°¤Î¤â¤Î¤Ï¥Ï¥Í¤ë *)
      invalidVar],
    (* false *)
    If[StringMatchQ[ToString[renamedVarName], "p" ~~ x__],
     (*Äê¿ôÌ¾¤ÎÆ¬¤Ë¤Ä¤¤¤Æ¤¤¤ë "p"¤ò¼è¤ê½ü¤¯ Ì¾Á°¤ÎÆ¬¤¬p¤¸¤ã¤Ê¤¤¤Î¤ÏinvalidVar¡¥ *)
      renamedVarName = removeP[renamedVarName];
      (* prev¤¬-1¤Î¤â¤Î¤ÏÄê¿ô¤È¤·¤Æ°·¤¦¤³¤È¤Ë¤¹¤ë *)
      {renamedVarName, derivativeCount, -1},
      invalidVar
    ]
  ]
];

(* ÊÑ¿ô¤È¤½¤ÎÃÍ¤Ë´Ø¤¹¤ë¼°¤Î¥ê¥¹¥È¤ò¡¢ÊÑ¿ôÉ½Åª·Á¼°¤ËÊÑ´¹
 * 0:Equal
 * 1:Less
 * 2:Greater
 * 3:LessEqual
 * 4:GreaterEqual
*)
convertCSToVM[orExprs_] := Block[
  {andExprs, inequalityVariable,
   getExprCode, removeInequality},
      
  getExprCode[expr_] := Switch[Head[expr],
    Equal, 0,
    Less, 1,
    Greater, 2,
    LessEqual, 3,
    GreaterEqual, 4
  ];
   
  (* Inequality[a, relop, x, relop, b]¤Î·Á¤òÊÑ·Á *)
  removeInequality[ret_, expr_] := (
    If[Head[expr] === Inequality,
      Join[ret,
           {Reduce[expr[[2]][expr[[1]], expr[[3]]], expr[[3]]]},
           {expr[[4]][expr[[3]], expr[[5]]]}
      ],
      Append[ret, expr]
    ]
  );


  debugPrint["orExprs:", orExprs];
  If[Head[First[orExprs]] === Or,
    (* Or¤¬´Þ¤Þ¤ì¤ë¾ì¹ç¤Ï1¤Ä¤À¤±ºÎÍÑ *)
    (* TODO: Ê£¿ô²ò¤¢¤ë¾ì¹ç¤â¹Í¤¨¤ë *)
    andExprs = First[First[orExprs]],
    (* Or¤¬´Þ¤Þ¤ì¤Ê¤¤¾ì¹ç *)
    andExprs = First[orExprs]
  ];
  andExprs = applyList[andExprs];
  If[Cases[andExprs, Except[True]]==={},
    (* ¥¹¥È¥¢¤¬¶õ¤Î¾ì¹ç¤Ï¶õ½¸¹ç¤òÊÖ¤¹ *)
    {},
    andExprs = Fold[(removeInequality[#1, #2]) &, {}, andExprs];
    DeleteCases[Map[({renameVar[#[[1]]], 
                      getExprCode[#], 
                      ToString[FullForm[#[[2]]]]}) &, 
                    andExprs],
                {invalidVar, _, _}]]

];

(*
 * isConsistentÆâ¤ÎReduce¤Î·ë²ÌÆÀ¤é¤ì¤¿²ò¤ò {ÊÑ¿ôÌ¾, ÃÍ}¡¡¤Î¥ê¥¹¥È·Á¼°¤Ë¤¹¤ë
 *)
createVariableList[Rule[varName_, varValue_], result_] := Block[{
  name
},
  name = renameVar[varName];
  Append[result, pair[name, varValue]]
];

createVariableList[{expr_}, result_] := (
  createVariableList[expr, result]
);

createVariableList[{expr_, others__}, result_] := Block[{
  variableList
},
  variableList = createVariableList[expr, result];
  createVariableList[{others}, variableList]
];

createVariableList[And[expr__], vars_, result_] := Block[
  {sol},
  sol = (Solve[expr, vars])[[1]];
  createVariableList[sol, result]
];

(* Or¤Ç¤Ä¤Ê¤¬¤Ã¤Æ¤¤¤ë ¡ÊÊ£¿ô²ò¤¬¤¢¤ë¡Ë¾ì¹ç¤Ï¡¢¤½¤Î¤¦¤Á¤Î1¤Ä¤Î¤ß¤òÊÖ¤¹¡© *)
createVariableList[Or[expr_, others__], vars_, result_] := (
  createVariableList[expr, vars, result]
);

createVariableList[Equal[varName_, varValue_], vars_, result_] := Block[{
  sol
},
  sol = (Solve[Equal[varName, varValue], vars])[[1]];
  createVariableList[sol, result]
];

(* Reduce[{}, {}]¤Î¤È¤­ *)
createVariableList[True, vars_, result_] := (
  Return[result]
);

(* ¼°Ãæ¤ËÊÑ¿ôÌ¾¤¬½Ð¸½¤¹¤ë¤«ÈÝ¤« *)
hasVariable[exprs_] := Length[StringCases[ToString[exprs], "usrVar" ~~ LetterCharacter]] > 0;

(*
 * À©Ìó¤¬ÌµÌ·½â¤Ç¤¢¤ë¤«¤ò¥Á¥§¥Ã¥¯¤¹¤ë
 * Reduce¤Ç²ò¤¤¤¿·ë²Ì¡¢²ò¤¬ÆÀ¤é¤ì¤ì¤ÐÌµÌ·½â
 * ÆÀ¤é¤ì¤¿²ò¤òÍÑ¤¤¤Æ¡¢³ÆÊÑ¿ô¤Ë´Ø¤·¤Æ {ÊÑ¿ôÌ¾, ÃÍ}¡¡¤È¤¤¤¦·Á¼°¤ÇÉ½¤·¤¿¥ê¥¹¥È¤òÊÖ¤¹
 *
 * Ìá¤êÃÍ¤Î¥ê¥¹¥È¤ÎÀèÆ¬¡§
 *  0 : Solver Error
 *  1 : ½¼Â­
 *  2 : À©Ìó¥¨¥é¡¼
 *)
isConsistent[pexpr_, expr_, vars_] := Quiet[Check[Block[
{
  sol,
  getInverseRelOp, adjustExprs
},

  getInverseRelop[relop_] := Switch[relop,
                                    Equal, Equal,
                                    Less, Greater,
                                    Greater, Less,
                                    LessEqual, GreaterEqual,
                                    GreaterEqual, LessEqual];

  (* É¬¤º´Ø·¸±é»»»Ò¤Îº¸Â¦¤ËÊÑ¿ôÌ¾¤¬Æþ¤ë¤è¤¦¤Ë¤¹¤ë *)
  adjustExprs[andExprs_] := 
    Fold[(If[Not[hasVariable[#2[[1]]]],
            (* true *)
            If[hasVariable[#2[[2]]],
              (* true *)
              (* µÕ¤Ë¤Ê¤Ã¤Æ¤ë¤Î¤Ç¡¢±é»»»Ò¤òµÕ¤Ë¤·¤ÆÄÉ²Ã¤¹¤ë *)
              Append[#1, getInverseRelop[Head[#2]][#2[[2]], #2[[1]]]],
              (* false *)
              (* ¥Ñ¥é¥á¡¼¥¿À©Ìó¤Î¾ì¹ç¤Ë¤³¤³¤ËÆþ¤ë *)
              If[NumericQ[#2[[1]]],
                (* true *)
                (* µÕ¤Ë¤Ê¤Ã¤Æ¤ë¤Î¤Ç¡¢±é»»»Ò¤òµÕ¤Ë¤·¤ÆÄÉ²Ã¤¹¤ë *)
                Append[#1, getInverseRelop[Head[#2]][#2[[2]], #2[[1]]]],
                (* false *)
                Append[#1, #2]]],
            (* false *)
            Append[#1, #2]]) &,
         {}, andExprs];


  debugPrint["expr:", expr, "pexpr", pexpr, "vars:", vars];
  sol = Reduce[expr, vars, Reals];
  (*  debugPrint["sol after Reduce:", sol];*)
  If[sol =!= False,
    sol = Reduce[Append[pexpr,sol],vars, Reals];
    If[sol =!= False,
      (* true *)
      (* ³°Â¦¤ËOr[]¤Î¤¢¤ë¾ì¹ç¤ÏList¤ÇÃÖ¤­´¹¤¨¤ë¡£¤Ê¤¤¾ì¹ç¤âList¤Ç°Ï¤à *)
      sol = LogicalExpand[sol];
      (* debugPrint["sol after LogicalExpand:", sol];*)
      sol = If[Head[sol] === Or, Apply[List, sol], {sol}];
      (* debugPrint["sol after Apply Or List:", sol];*)
      (* ÆÀ¤é¤ì¤¿¥ê¥¹¥È¤ÎÍ×ÁÇ¤Î¥Ø¥Ã¥É¤¬And¤Ç¤¢¤ë¾ì¹ç¤ÏList¤ÇÃÖ¤­´¹¤¨¤ë¡£¤Ê¤¤¾ì¹ç¤âList¤Ç°Ï¤à *)
      sol = removeNotEqual[Map[(If[Head[#] === And, Apply[List, #], {#}]) &, sol]];
      (* debugPrint["sol after Apply And List:", sol];*)
      (* °ìÈÖÆâÂ¦¤ÎÍ×ÁÇ ¡Ê¥ì¥Ù¥ë2¡Ë¤òÊ¸»úÎó¤Ë¤¹¤ë *)
      {1, Map[(ToString[FullForm[#]]) &, 
              Map[(adjustExprs[#])&, sol], {2}]},
      {2}
    ],
    (* false *)
    {2}]
],
  {0, $MessageList}
]];

(* Print[isConsistent[s[x==2, 7==x*x], {x,y}]] *)

(* ÊÑ¿ôÌ¾¤«¤é ¡Ö\[CloseCurlyQuote]¡×¤ò¼è¤ë *)
removeDash[var_] := (
   var /. Derivative[_][x_] -> x
);

(* 
 * tellVars¤ò¸µ¤Ë¡¢¤½¤Î½é´üÃÍÀ©Ìó¤¬É¬Í×¤«¤É¤¦¤«¤òÄ´¤Ù¤ë
 * ½é´üÃÍÀ©ÌóÃæ¤Ë½Ð¸½¤¹¤ëÊÑ¿ô¤¬varsÆâ¤Ë¤Ê¤±¤ì¤ÐÉÔÍ×
 *)
isRequiredConstraint[cons_, tellVars_] := Block[{
   consVar
},
   consVar = cons /. x_[0] == y_ -> x;
   removeDash[consVar];
   If[MemberQ[tellVars, consVar], True, False]
];

(* tellVars¤ò¸µ¤Ë¡¢¤½¤ÎÊÑ¿ô¤¬É¬Í×¤«¤É¤¦¤«¤òÄ´¤Ù¤ë *)
isRequiredVariable[var_, tellVars_] := (
   If[MemberQ[tellVars, var], True, False]
);

removeNotEqual[sol_] := 
   DeleteCases[sol, Unequal[lhs_, rhs_], Infinity];

getNDVars[vars_] := Union[Map[(removeDash[#])&, vars]];

(* ¤¢¤ëÊÑ¿ôx¤Ë¤Ä¤¤¤Æ¡¢ *)
(* x[t]==a ¤È x[0]==a ¤¬¤¢¤Ã¤¿¾ì¹ç¤Ï¡¢x'[t]==0¤ò¾Ãµî ¡Ê¤¢¤ì¤Ð¡Ë *)
removeTrivialCons[cons_, consVars_] := Block[
  {exprRule, varSol, removedExpr,
   removeTrivialConsUnit, getRequiredVars},

  removeTrivialConsUnit[expr_, var_]:=(
    (* ÊýÄø¼°¤ò¥ë¡¼¥ë²½¤¹¤ë *)
    exprRule = Map[(Rule @@ #) &, expr];
    varSol = var[t] /. exprRule;
    If[MemberQ[expr, var[0] == varSol] && MemberQ[expr, var'[t] == 0],
      (* true *)
      removedExpr = DeleteCases[expr, var'[t] == 0],

      (* false *)
      removedExpr = expr
    ];
    removedExpr
  );

  getRequiredVars[vars_] := (
    Union[Map[(# /. Derivative[n_][x_][t] -> Derivative[n - 1][x]
                         /.  Derivative[0][x_][t] -> x) &, vars]]
  );

  Fold[(Intersection[#1, removeTrivialConsUnit[cons, #2]]) &,
       cons, getRequiredVars[consVars]]

];

(* Reduce¤ÇÆÀ¤é¤ì¤¿·ë²Ì¤ò¥ê¥¹¥È·Á¼°¤Ë¤¹¤ë *)
(* And¤Ç¤Ï¤Ê¤¯List¤Ç¤¯¤¯¤ë *)
applyList[reduceSol_] :=
  If[Head[reduceSol] === And, List @@ reduceSol, List[reduceSol]];

(* ¥Ñ¥é¥á¡¼¥¿¤ÎÀ©Ìó¤òÆÀ¤ë *)
getParamCons[cons_] := 
  Fold[(If[Not[hasVariable[#2]], Append[#1, #2], #1]) &, {}, cons];

exDSolve[expr_, vars_] := Block[
{sol, DExpr, DExprVars, NDExpr, otherExpr, paramCons},
  paramCons = getParamCons[expr];
  sol = LogicalExpand[removeNotEqual[Reduce[Cases[Complement[expr, paramCons], Except[True] | Except[False]], vars, Reals]]];

  If[sol===False,
    overconstraint,
    If[sol===True,
      {{}, {}},

      (* 1¤Ä¤À¤±ºÎÍÑ *)
      (* TODO: Ê£¿ô²ò¤¢¤ë¾ì¹ç¤â¹Í¤¨¤ë *)
      If[Head[sol]===Or, sol = First[sol]];

      sol = applyList[sol];

      (* Äê¿ô´Ø¿ô¤Î¾ì¹ç¤Ë²á¾ê·èÄê·Ï¤Î¸¶°ø¤È¤Ê¤ëÈùÊ¬À©Ìó¤ò¼è¤ê½ü¤¯ *)
      sol = removeTrivialCons[sol, vars];

      {DExpr, DExprVars, NDExpr, otherExpr} = splitExprs[sol];

      Quiet[Check[Check[If[Cases[DExpr, Except[True]] === {},
                          (* ¥¹¥È¥¢¤¬¶õ¤Î¾ì¹ç¤ÏDSolve¤¬²ò¤±¤Ê¤¤¤Î¤Ç¶õ½¸¹ç¤òÊÖ¤¹ *)
                          sol = {},
                          sol = DSolve[DExpr, DExprVars, t];
                          (* 1¤Ä¤À¤±ºÎÍÑ *)
                          (* TODO: Ê£¿ô²ò¤¢¤ë¾ì¹ç¤â¹Í¤¨¤ë *)
                          sol = First[sol]];

                        sol = Solve[Join[Map[(Equal @@ #) &, sol], NDExpr], getNDVars[vars]];
                        If[sol =!= {}, 
                          {First[sol], Join[otherExpr, paramCons]},
                          overconstraint],
                        underconstraint,
                        {DSolve::underdet, Solve::svars, DSolve::deqx, 
                         DSolve::bvnr, DSolve::bvsing}],
                  overconstraint,
                  {DSolve::overdet, DSolve::bvnul, DSolve::dsmsm}],
            {DSolve::underdet, DSolve::overdet, DSolve::deqx, 
             Solve::svars, DSolve::bvnr, DSolve::bvsing, 
             DSolve::bvnul, DSolve::dsmsm, Solve::incnst}]]]
];

(* DSolve¤Ç°·¤¨¤ë¼° (DExpr)¤È¤½¤¦¤Ç¤Ê¤¤¼° (NDExpr)¤È¤½¤ì°Ê³° ¡ÊotherExpr¡Ë¤ËÊ¬¤±¤ë *)
(* ÈùÊ¬ÃÍ¤ò´Þ¤Þ¤ºŽÿŽÿŽÿŽÿŽÿŽÿÊÑ¿ô¤¬2¼ïÎà°Ê¾å½Ð¤ë¼° (NDExpr)¤äÅù¼°°Ê³° ¡ÊotherExpr¡Ë¤ÏDSolve¤Ç°·¤¨¤Ê¤¤ *)
splitExprs[expr_] := Block[
  {NDExpr, DExpr, DExprVars, otherExpr},
  otherExpr = Select[expr, (Head[#] =!= Equal) &];
  NDExpr = Select[Complement[expr, otherExpr], 
                  (MemberQ[#, Derivative[n_][x_][t], Infinity] =!= True && Length[Union[Cases[#, _[t], Infinity]]] > 1) &];  
  DExpr = Complement[expr, Join[otherExpr, NDExpr]];
  DExprVars = Union[Fold[(Join[#1, Cases[#2, _[t] | _[0], Infinity] /. x_[0] -> x[t]]) &, 
                         {}, DExpr]];
  {DExpr, DExprVars, NDExpr, otherExpr}
];

(* isConsistentInterval[tells_, store_, tellsVars_, storeVars_] := ( *)
(*   If[store =!= {}, *)
(*      (\* À©Ìó¥¹¥È¥¢¤¬¶õ¤Ç¤Ê¤¤¾ì¹ç¡¢ÉÔÍ×¤Ê½é´üÃÍÀ©Ìó¤ò¼è¤ê½ü¤¯É¬Í×¤¬¤¢¤ë *\) *)
(*      newTellVars = Map[removeDash, Map[(# /. x_[t] -> x) &, tellsVars]]; *)
(*      (\* store¤¬List[And[]]¤Î·Á¤Ë¤Ê¤Ã¤Æ¤¤¤ë¾ì¹ç¤Ï¡¢°ìÃ¶¡¢Ãæ¤ÎAnd¤ò¼è¤ê½Ð¤¹ *\) *)
(*      newStore = If[Head[First[store]] === And, Apply[List, First[store]], store]; *)
(*      removedStore = {Apply[And, Select[newStore, (isRequiredConstraint[#, newTellVars]) &]]}; *)
(*      newStoreVars = Map[removeDash, Map[(# /. x_[t] -> x) &, storeVars]]; *)
(*      removedStoreVars = Map[(#[t])&, Select[newStoreVars, (isRequiredVariable[#, newTellVars])&]], *)

(*      (\* À©Ìó¥¹¥È¥¢¤¬¶õ¤Î¾ì¹ç *\) *)
(*      removedStore = store; *)
(*      removedStoreVars = storeVars]; *)

(*   If[sol =!= overconstraint, *)
(*      cons = Map[(ToString[FullForm[#]]) &, Join[tells, removedStore]]; *)
(*      vars = Join[tellsVars, removedStoreVars]; *)
(*      If[sol =!= underconstraint, {1, cons, vars}, {2, cons, vars}], *)
(*      0] *)
(* ); *)

(*
 * Ìá¤êÃÍ¤Î¥ê¥¹¥È¤ÎÀèÆ¬¡§
 *  0 : Solver Error
 *  1 : ½¼Â­
 *  2 : À©Ìó¥¨¥é¡¼
 *)
isConsistentInterval[expr_, vars_] :=  Block[
  {sol},
  Quiet[Check[
    debugPrint["expr:", expr, "vars:", vars];
    sol = exDSolve[expr, vars];
    If[sol=!=overconstraint,
      {1},
      {2}],
  {0, $MessageList}
]]];

(* Print[exDSolve[{True, True, Derivative[1][usrVarht][t] == usrVarv[t], Derivative[1][usrVarv][t] == -10, usrVarht[0] == 10, usrVarv[0] == 0},  *)
(* {usrVarht[t], Derivative[1][usrVarht][t], usrVarv[t], Derivative[1][usrVarv][t]}]]; *)

(* Print[isConsistentInterval[ *)
(* {Derivative[1][usrVarht][t] == usrVarv[t],  *)
(*   Derivative[1][usrVarv][t] == -10,  *)
(*   usrVarv[0] == Derivative[1][usrVarht]},  *)
(*   {Derivative[1][usrVarht][t], usrVarv[t], Derivative[1][usrVarv][t]}]]; *)

(*
 * ¼¡¤Î¥Ý¥¤¥ó¥È¥Õ¥§¡¼¥º¤Ë°Ü¹Ô¤¹¤ë»þ¹ï¤òµá¤á¤ë
 *)
calcNextPointPhaseTime[includeZero_, maxTime_, posAsk_, negAsk_, NACons_, otherExpr_] := Block[
{
  calcMinTime,
  sol, minT, paramVars, compareResult,
  timeMinCons = If[includeZero===True, (t>=0), (t>0)]
},
  calcMinTime[{currentMinT_, currentMinAsk_}, {type_, integAsk_, askID_}] := (
    If[integAsk=!=False,
      (* true *)
      (* ²ò¤Ê¤·¤È¶­³¦ÃÍ¤Î²ò¤ò¶èÊÌ¤¹¤ë *)  
      sol = Quiet[Check[Reduce[{timeMinCons && (maxTime>=t) && (integAsk) && (And @@ otherExpr)}, t],
                        errorSol,
                        {Reduce::nsmet}],
                  {Reduce::nsmet}];
      (*  debugPrint["calcMinTime#sol: ", sol]; *)
      If[sol=!=False && sol=!=errorSol, 
        (* true *)
        (* À®¤êÎ©¤Ät¤ÎºÇ¾®ÃÍ¤òµá¤á¤ë *)
        minT = First[Quiet[Minimize[{t, timeMinCons && (sol)}, {t}], 
                           Minimize::wksol]],

        (* false *)
        minT = error],
      
      (* false *)
      minT=0];

    debugPrint["calcMinTime#minT: ", minT];
    (* Piecewise¤Ø¤ÎÂÐ±þ *)
    If[Head[minT] === Piecewise, minT = First[First[First[minT]]]];
    (* 0ÉÃ¸å¤Î¤ò´Þ¤ó¤Ç¤Ï¤¤¤±¤Ê¤¤ *)
    If[includeZero===False && minT===0, minT=error];
    (* 0ÉÃ¸å¤ÎÎ¥»¶ÊÑ²½¤¬¹Ô¤ï¤ì¤ë¤«¤Î¥Á¥§¥Ã¥¯¤Ê¤Î¤Ç0¤Ç¤Ê¤±¤ì¤Ð¥¨¥é¡¼ *)
    If[includeZero===True && minT=!=0,
    minT=error];

    If[minT === error,
      (* true *)
      {currentMinT, currentMinAsk},

      (* false *)
      paramVars = Union[Fold[(Join[#1, getParamVar[#2]]) &, {}, otherExpr]];
      compareResult = Map[(Reduce[{#[minT, currentMinT] && (And @@ otherExpr)}, paramVars]) &, 
                          {Less, Equal, Greater}];


      If[Count[compareResult, Not[False]] > 1,
        (* Í×Ê¬´ô *)
        (* TODO: Å¬ÀÚ¤Ê½èÍý¤ò¤¹¤ë *)
        1,
        Switch[First[First[Position[compareResult, x_ /; x =!= False, {1}, Heads -> False]]],
          1, {minT, {{type, askID}}},
          2, {minT, Append[currentMinAsk, {type, askID}]},
          3, {currentMinT, currentMinAsk}]
      ]
    ]
  );


  (* ºÇ¾®ÃÍ¤È¾ò·ï¤ÎÁÈ¤ò¥ê¥¹¥È¥¢¥Ã¥×¤¹¤ë»þ¤Ë»È¤¦´Ø¿ô¡¥²¾ *)
  addMinTime[currentList_, {type_, integAsk_, askID_}] := (
    If[integAsk=!=False,
      (* true *)
      (* ²ò¤Ê¤·¤È¶­³¦ÃÍ¤Î²ò¤ò¶èÊÌ¤¹¤ë *)  
      sol = Quiet[Check[Reduce[{timeMinCons && (maxTime>=t) && (integAsk) && (And @@ otherExpr)}, t],
                        errorSol,
                        {Reduce::nsmet}],
                  {Reduce::nsmet}];
      (*  debugPrint["calcMinTime#sol: ", sol]; *)
      If[sol=!=False && sol=!=errorSol, 
        (* true *)
        (* À®¤êÎ©¤Ät¤ÎºÇ¾®ÃÍ¤òµá¤á¤ë *)
        minT = First[Quiet[Minimize[{t, timeMinCons && (sol)}, {t}], 
                           Minimize::wksol]],

        (* false *)
        minT = error
      ],
      (* false *)
      minT=0
    ];

    debugPrint["calcMinTime#minT: ", minT];
    (* 0ÉÃ¸å¤Î¤ò´Þ¤ó¤Ç¤Ï¤¤¤±¤Ê¤¤ *)
    If[includeZero===False && minT===0, minT=error];
    (* 0ÉÃ¸å¤ÎÎ¥»¶ÊÑ²½¤¬¹Ô¤ï¤ì¤ë¤«¤Î¥Á¥§¥Ã¥¯¤Ê¤Î¤Ç0¤Ç¤Ê¤±¤ì¤Ð¥¨¥é¡¼ *)
    If[includeZero===True && minT=!=0,
      (* Piecewise¤Ø¤ÎÂÐ±þ *)
      If[Head[minT] === Piecewise, minT = First[First[First[minT]]]];
    minT=error];

    If[minT === error,
      (* true *)
      currentList,

      (* false *)
      Append[currentList,{minT, type, askID}]
    ]
  );



  Fold[calcMinTime,
       {maxTime, {}},
       Join[Map[({pos2neg, Not[#[[1]]], #[[2]]})&, posAsk],
            Map[({neg2pos,     #[[1]],  #[[2]]})&, negAsk],
            Fold[(Join[#1, Map[({neg2pos, #[[1]], #[[2]]})&, #2]])&, {}, NACons]]]
  (* ¤Þ¤º¥ê¥¹¥È¥¢¥Ã¥× *)
  (*Fold[addMinTime,
       {},
       Join[Map[({pos2neg, Not[#[[1]]], #[[2]]})&, posAsk],
            Map[({neg2pos,     #[[1]],  #[[2]]})&, negAsk],
            Fold[(Join[#1, Map[({neg2pos, #[[1]], #[[2]]})&, #2]])&, {}, NACons]]]*)
   
];

(* Print[nextPointPhaseTime[False, 10, {}, {{t*t==2, c3}}]] *)

getVariableName[variable_[_]] := variable;
getVariableName[Derivative[n_][f_][_]] := f;

getDerivativeCount[variable_[_]] := 0;
getDerivativeCount[Derivative[n_][f_][_]] := n;

createIntegratedValue[variable_, integRule_] := (
  Simplify[
    variable /. Map[(Rule[#[[1]] /. x_[t]-> x, #[[2]]])&, integRule]
             /. Derivative[n_][f_] :> D[f, {t, n}] 
             /. x_[t] -> x]
);

(* Print[createIntegratedValue[ToExpression["{ht'[t]}"], ToExpression["{ht[t] -> 2t+Sin[t]}"]]] *)
(* Print[createIntegratedValue[ToExpression["{ht'[t]}"], ToExpression["{ht[t] -> 10}"]]] *)

(* ¥Ñ¥é¥á¡¼¥¿À©Ìó¤òÆÀ¤ë *)
getParamCons[cons_] := Cases[cons, x_ /; Not[hasVariable[x]], {1}];
(* ¼°Ãæ¤Î¥Ñ¥é¥á¡¼¥¿ÊÑ¿ô¤òÆÀ¤ë *)
getParamVar[paramCons_] := Cases[paramCons, x_ /; Not[NumericQ[x]], Infinity];

(*
 * ask¤ÎÆ³½Ð¾õÂÖ¤¬ÊÑ²½¤¹¤ë¤Þ¤ÇÀÑÊ¬¤ò¤ª¤³¤Ê¤¦
 *)
integrateCalc[cons_, 
              posAsk_, negAsk_, NACons_,
              vars_, 
              maxTime_] := Quiet[Check[Block[
{
  tmpIntegSol,
  tmpPosAsk,
  tmpNegAsk,
  tmpNACons,
  tmpMinT, 
  tmpMinAskIDs,
  tmpVarMap,
  endTimeFlag,
  tmpRet,
  NDExpr,
  DExpr,
  DExprVars,
  otherExpr,
  returnVars,
  solVars,
  paramCons,
  paramVars
(*   applyTime2VarMap *)
},
(*   applyTime2VarMap[{name_, derivative_, expr_}, time_, extrafunc_] := *)
(*     {name,  *)
(*      derivative,  *)
(*      extrafunc[expr /. t->time]}; *)
(* Map[(applyTime2VarMap[#,  *)
(*                                   tmpMinT,  *)
(*                                   Function[{expr}, ToString[FullForm[Simplify[expr]]]]])&,  *)

  debugPrint["cons:", cons, 
             "posAsk:", posAsk, 
             "negAsk:", negAsk, 
             "NACons:", NACons,
             "vars:", vars, 
             "maxTime:", maxTime];

  If[Cases[cons, Except[True]]=!={},
    (* true *)
    paramCons = getParamCons[cons];
    paramVars = Union[Fold[(Join[#1, getParamVar[#2]]) &, {}, paramCons]];
    tmpIntegSol = LogicalExpand[removeNotEqual[Reduce[Complement[cons, paramCons], Join[vars, paramVars], Reals]]];
    (* 1¤Ä¤À¤±ºÎÍÑ *)
    (* TODO: Ê£¿ô²ò¤¢¤ë¾ì¹ç¤â¹Í¤¨¤ë *)
    If[Head[tmpIntegSol]===Or, tmpIntegSol = First[tmpIntegSol]];

    tmpIntegSol = removeTrivialCons[applyList[tmpIntegSol], Join[vars, paramVars]];

    {DExpr, DExprVars, NDExpr, otherExpr} = splitExprs[tmpIntegSol];

    If[Cases[DExpr, Except[True]] === {},
      tmpIntegSol = {},
      tmpIntegSol = Quiet[DSolve[DExpr, DExprVars, t],
                        {Solve::incnst}];
      (* 1¤Ä¤À¤±ºÎÍÑ *)
      (* TODO:Ê£¿ô²ò¤¢¤ë¾ì¹ç¤â¹Í¤¨¤ë *)
      tmpIntegSol = First[tmpIntegSol]];

    (* debugPrint["tmpIntegSol: ", tmpIntegSol, "paramVars", paramVars "paramCons", paramCons]; *)
    (* tmpIntegSol¤ª¤è¤ÓNDExprÆâ¤Ë½Ð¸½¤¹¤ëNDÊÑ¿ô¤Î°ìÍ÷¤òÆÀ¤ë *)
    solVars = getNDVars[Union[Cases[Join[tmpIntegSol, NDExpr], _[t], Infinity]]];
    (* integrateCalc¤Î·×»»·ë²Ì¤È¤·¤ÆÉ¬Í×¤ÊÊÑ¿ô¤Î°ìÍ÷ *)
    returnVars = Select[vars, (MemberQ[solVars, removeDash[#]]) &];

    tmpIntegSol = First[Solve[Join[Map[(Equal @@ #) &, tmpIntegSol], NDExpr], 
                              getNDVars[returnVars]]];

    tmpPosAsk = Map[(# /. tmpIntegSol ) &, posAsk];
    tmpNegAsk = Map[(# /. tmpIntegSol) &, negAsk];
    tmpNACons = Map[(# /. tmpIntegSol) &, NACons];
    {tmpMinT, tmpMinAskIDs} = 
      calcNextPointPhaseTime[False, maxTime, tmpPosAsk, tmpNegAsk, tmpNACons, Join[otherExpr, paramCons]];

    tmpVarMap = 
      Map[({getVariableName[#], 
            getDerivativeCount[#], 
            createIntegratedValue[#, tmpIntegSol] // FullForm // ToString})&, 
          returnVars];
    endTimeFlag = If[Reduce[tmpMinT >= maxTime && (And @@ Join[otherExpr, paramCons]), paramVars] =!= False, 1, 0],

    (* false *)
    tmpMinT = maxTime;
    tmpVarMap = {};
    tmpMinAskIDs = {};
    endTimeFlag = 1
  ];
  tmpRet = {1,
            ToString[FullForm[tmpMinT]],
            tmpVarMap,
            tmpMinAskIDs, 
            endTimeFlag};
  debugPrint["tmpRet:", tmpRet];
  tmpRet
],
  (* debugPrint["MessageList:", $MessageList]; *)
  {0, $MessageList}
]];

(*
 * ¼°¤ËÂÐ¤·¤ÆÍ¿¤¨¤é¤ì¤¿»þ´Ö¤òÅ¬ÍÑ¤¹¤ë
 *)
applyTime2Expr[expr_, time_] := Block[
  {appliedExpr},
  debugPrint["expr:", expr,
             "time:", time];
  appliedExpr = Simplify[(expr /. t -> time)];
  If[Element[appliedExpr, Reals] =!= False,
    {1, appliedExpr  // FullForm // ToString},
    {0}]
];

(*
 * form¤«¤éto¤Þ¤Ç¤Î¥ê¥¹¥È¤òinterval´Ö³Ö¤ÇºîÀ®¤¹¤ë
 * ºÇ¸å¤ËÉ¬¤ºto¤¬Íè¤ë¤¿¤á¤Ë¡¤
 * ºÇ¸å¤Î´Ö³Ö¤Î¤ßinterval¤è¤ê¤âÃ»¤¯¤Ê¤ë²ÄÇ½À­¤¬¤¢¤ë
 *)
createValueList[from_, to_, interval_] := Block[
{sol},
  sol = NestWhileList[((#) + (interval))&, from, (# <= to)&, 1, Infinity, -1];
  If[Last[sol] =!= to, 
      Append[sol, to], 
      sol]
];

createOutputTimeList[from_, to_, interval_] :=
  Map[(ToString[#, InputForm])&, createValueList[from, to, interval]];

(* Print[createOutputTimeList[0, 5, 1/3]] *)

(* Print[integrateCalc[ *)
(* {True, Derivative[1][usrVarht][t] == usrVarv[t], *)
(* Derivative[1][usrVarv][t] == -10, usrVarht[0] == 10, usrVarv[0] == 0}, *)
(* {}, *)
(* {{usrVarht[t] == 0, 26}}, *)
(* {usrVarht[t], Derivative[1][usrVarht][t], usrVarv[t], Derivative[1][usrVarv][t]}, *)
(* 1]]; *)

(*  Print[integrateCalc[ *)
(* {True, True, True, Derivative[2][usrVarht][t] == 0, Derivative[2][usrVarv][t] == 0, usrVarht[0] == 10, usrVarv[0] == 0, Derivative[1][usrVarht][0] == 0, Derivative[1][usrVarv][0] == -10}, *)
(* {}, *)
(* {{usrVarht[t] == 0, 73}}, *)
(* {usrVarht[t], Derivative[1][usrVarht][t], Derivative[2][usrVarht][t], usrVarv[t], Derivative[1][usrVarv][t], Derivative[2][usrVarv][t]}, *)
(* 4]]; *)

(* Print[integrateCalc[{Equal[usrVarht[0], 10], Equal[usrVarv[0], 0], *)
(*     Equal[Derivative[1][usrVarht][t], usrVarv[t]], Equal[Derivative[1][usrVarv][t], -10]}, *)
(*   {}, *)
(*   {{usrVarht[t]==0, 10}}, *)
(*   {usrVarht[t], usrVarv[t], Derivative[1][usrVarht][t], Derivative[1][usrVarv][t]}, 10]]; *)

(* Print[integrateCalc[{Equal[usrVarht[0], 10], Equal[Derivative[1][usrVarht][0], 0], *)
(*     Equal[Derivative[2][usrVarht][t], -10]}, *)
(*   {}, *)
(*   {{usrVarht[t]==0, 10}}, *)
(*   {usrVarht[t], Derivative[1][usrVarht][t], Derivative[2][usrVarht][t]}, 10] // FullForm]; *)

(*
 * Í¿¤¨¤é¤ì¤¿¼°¤òÀÑÊ¬¤·¡¤ÊÖ¤¹
 *
 * 0: Solver Error
 * 1: Solved Successfully
 * 2: Under Constraint
 * 3: Over Constraint
 * 
 *)
integrateExpr[cons_, vars_] := Quiet[Check[Block[
{ sol },

(*  debugPrint["cons:", cons, "vars:", vars]; *)
 
  sol = exDSolve[cons, vars];
  Switch[sol,
    underconstraint, 
      {2},
    overconstraint,
      {3},
    _,
      {1,         
       Map[({getVariableName[#], 
             getDerivativeCount[#], 
             ToString[createIntegratedValue[#, sol], InputForm]})&, 
           vars]}]
],
  {0, $MessageList}
]];

(* Print["integ:", integrateExpr[{ht'[t]==v[t], v'[t]==-10, ht[0]==a, v[0]==b}, {ht[t], ht'[t], v[t], v'[t]}]]; *)
(* Print["integ:", integrateExpr[{ht'[t]==v[t], v'[t]==-10, v'[t]==-20, ht[0]==a, v[0]==b}, {ht[t], ht'[t], v[t], v'[t]}]]; *)
(* Print["integ:", integrateExpr[{ht'[t]==x[t], v'[t]==-10, ht[0]==a, v[0]==b}, {x[t], ht[t], ht'[t], v[t], v'[t]}]]; *)

(*
 * Í¿¤¨¤é¤ì¤¿¼°¤ò¶á»÷¤¹¤ë
 *)
approxExpr[precision_, expr_] :=
  Rationalize[N[Simplify[expr], precision + 3], 
              Divide[1, Power[10, precision]]];

(* 
 * Í¿¤¨¤é¤ì¤¿t¤Î¼°¤ò¥¿¥¤¥à¥·¥Õ¥È
 *)
exprTimeShift[expr_, time_] := ToString[FullForm[Simplify[expr /. t -> t - time ]]];
