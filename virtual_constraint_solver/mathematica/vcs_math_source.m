(*
 * ƒfƒoƒbƒN—pƒƒbƒZ[ƒWo—ÍŠÖ”
 *)
If[optUseDebugPrint,
  debugPrint[arg___] := Print[InputForm[{arg}]],
  debugPrint[arg___] := Null];

(*
 * checkEntailment‚ÌIPƒo[ƒWƒ‡ƒ“
 * 0 : Solver Error
 * 1 : “±o‰Â”\
 * 2 : “±o•s‰Â”\
 *)
checkEntailmentInterval[guard_, store_, vars_] := Quiet[Check[Block[
  {tStore, sol, integGuard, otherExpr, minT},
  debugPrint["guard:", guard, "store:", store, "vars:", vars];
  sol = exDSolve[store, vars];
  If[sol =!= overconstraint && sol =!= underconstraint,
    tStore = sol[[1]];
    otherExpr = sol[[2]];
    (* guard‚ÉtStore‚ğ“K—p‚·‚é *)
    integGuard = guard /. tStore;
    If[integGuard =!= False,
      (* ‚»‚ÌŒ‹‰Ê‚Æt>0‚Æ‚ğ˜A—§ *)
      (* debugPrint["integGuard:", integGuard]; *)
      sol = Quiet[Check[Reduce[{integGuard && t > 0 && (And@@otherExpr)}, t],
                        False, {Reduce::nsmet}], {Reduce::nsmet}];
      (* Inf‚ğæ‚Á‚Ä0‚É‚È‚ê‚ÎEntailed *)
      If[sol =!= False,
        minT = MinValue[{t, sol}, t];
        If[minT === 0,
          {1},
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
 * ‚ ‚é1‚Â‚Ìask§–ñ‚ÉŠÖ‚µ‚ÄA‚»‚ÌƒK[ƒh‚ª§–ñƒXƒgƒA‚©‚çentail‚Å‚«‚é‚©‚Ç‚¤‚©‚ğƒ`ƒFƒbƒN
 *  0 : Solver Error
 *  1 : “±o‰Â”\
 *  2 : “±o•s‰Â”\
 *  3 : •s–¾ i—v•ªŠòj
 *)
checkEntailment[guard_, store_, vars_] := Quiet[Check[Block[
  {sol, SAndG, SAndNotG},
  debugPrint["guard:", guard, "store:", store, "vars:", vars];

  sol = Reduce[Append[store, guard], vars, Reals];
  If[sol=!=False, 
    If[Reduce[Append[store, Not[sol]], vars, Reals]===False, 
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

  (* •Ï”–¼‚É'‚ª‚Â‚­ê‡‚Ìˆ— *)
  If[MemberQ[{varName}, Derivative[n_][x_], Infinity],
    derivativeCount = getDerivativeCountPoint[varName];
    renamedVarName = removeDash[varName],
    renamedVarName = varName
  ];
  (* •Ï”–¼‚Éprev‚ª‚Â‚­ê‡‚Ìˆ— *)
  If[Head[renamedVarName] === prev,
    renamedVarName = First[renamedVarName];
    prevFlag = 1
  ];

  (*•Ï”–¼‚Ì“ª‚É‚Â‚¢‚Ä‚¢‚é "usrVar"‚ğæ‚èœ‚­ i–¼‘O‚Ì“ª‚ªusrVar‚¶‚á‚È‚¢‚Ì‚ÍinvalidVarD’è”–¼‚Æ‚©j *)
   If[StringMatchQ[ToString[renamedVarName], "usrVar" ~~ x__],
     (* true *)
     renamedVarName = removeUsrVar[renamedVarName];
     (* ‚±‚Ì“_‚Å’P‘Ì‚Ì•Ï”–¼‚Ì‚Í‚¸D *)
     If[Length[renamedVarName] === 0,
       {renamedVarName, derivativeCount, prevFlag},
       (* ®Œ`®‚Ì‚à‚Ì‚Íƒnƒl‚é *)
       invalidVar],
     (* false *)
     invalidVar
   ]
];

(* •Ï”‚Æ‚»‚Ì’l‚ÉŠÖ‚·‚é®‚ÌƒŠƒXƒg‚ğA•Ï”•\“IŒ`®‚É•ÏŠ·
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
   
  (* Inequality[a, relop, x, relop, b]‚ÌŒ`‚ğ•ÏŒ` *)
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
    (* Or‚ªŠÜ‚Ü‚ê‚éê‡‚Í1‚Â‚¾‚¯Ì—p *)
    (* TODO: •¡”‰ğ‚ ‚éê‡‚àl‚¦‚é *)
    andExprs = First[First[orExprs]],
    (* Or‚ªŠÜ‚Ü‚ê‚È‚¢ê‡ *)
    andExprs = First[orExprs]
  ];
  andExprs = applyList[andExprs];
  If[Cases[andExprs, Except[True]]==={},
    (* ƒXƒgƒA‚ª‹ó‚Ìê‡‚Í‹óW‡‚ğ•Ô‚· *)
    {},
    andExprs = Fold[(removeInequality[#1, #2]) &, {}, andExprs];
    DeleteCases[Map[({renameVar[#[[1]]], 
                      getExprCode[#], 
                      ToString[FullForm[#[[2]]]]}) &, 
                    andExprs],
                {invalidVar, _, _}]]

];

(*
 * isConsistent“à‚ÌReduce‚ÌŒ‹‰Ê“¾‚ç‚ê‚½‰ğ‚ğ {•Ï”–¼, ’l}@‚ÌƒŠƒXƒgŒ`®‚É‚·‚é
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

createVariableList[And[expr__], vars_, result_] := Block[{
  sol
}
  sol = (Solve[expr, vars])[[1]];
  createVariableList[sol, result]
];

(* Or‚Å‚Â‚È‚ª‚Á‚Ä‚¢‚é i•¡”‰ğ‚ª‚ ‚éjê‡‚ÍA‚»‚Ì‚¤‚¿‚Ì1‚Â‚Ì‚İ‚ğ•Ô‚·H *)
createVariableList[Or[expr_, others__], vars_, result_] := (
  createVariableList[expr, vars, result]
);

createVariableList[Equal[varName_, varValue_], vars_, result_] := Block[{
  sol
},
  sol = (Solve[Equal[varName, varValue], vars])[[1]];
  createVariableList[sol, result]
];

(* Reduce[{}, {}]‚Ì‚Æ‚« *)
createVariableList[True, vars_, result_] := (
  Return[result]
);

(* ®’†‚É•Ï”–¼‚ªoŒ»‚·‚é‚©”Û‚© *)
hasVariable[exprs_] := Length[StringCases[ToString[exprs], "usrVar" ~~ LetterCharacter]] > 0;

(*
 * §–ñ‚ª–³–µ‚‚Å‚ ‚é‚©‚ğƒ`ƒFƒbƒN‚·‚é
 * Reduce‚Å‰ğ‚¢‚½Œ‹‰ÊA‰ğ‚ª“¾‚ç‚ê‚ê‚Î–³–µ‚
 * “¾‚ç‚ê‚½‰ğ‚ğ—p‚¢‚ÄAŠe•Ï”‚ÉŠÖ‚µ‚Ä {•Ï”–¼, ’l}@‚Æ‚¢‚¤Œ`®‚Å•\‚µ‚½ƒŠƒXƒg‚ğ•Ô‚·
 *
 * –ß‚è’l‚ÌƒŠƒXƒg‚Ìæ“ªF
 *  0 : Solver Error
 *  1 : [‘«
 *  2 : §–ñƒGƒ‰[
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

  (* •K‚¸ŠÖŒW‰‰Zq‚Ì¶‘¤‚É•Ï”–¼‚ª“ü‚é‚æ‚¤‚É‚·‚é *)
  adjustExprs[andExprs_] := 
    Fold[(If[Not[hasVariable[#2[[1]]]],
            (* true *)
            If[hasVariable[#2[[2]]],
              (* true *)
              (* ‹t‚É‚È‚Á‚Ä‚é‚Ì‚ÅA‰‰Zq‚ğ‹t‚É‚µ‚Ä’Ç‰Á‚·‚é *)
              Append[#1, getInverseRelop[Head[#2]][#2[[2]], #2[[1]]]],
              (* false *)
              (* ƒpƒ‰ƒ[ƒ^§–ñ‚Ìê‡‚É‚±‚±‚É“ü‚é *)
              If[NumericQ[#2[[1]]],
                (* true *)
                (* ‹t‚É‚È‚Á‚Ä‚é‚Ì‚ÅA‰‰Zq‚ğ‹t‚É‚µ‚Ä’Ç‰Á‚·‚é *)
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
    If[Reduce[Append[pexpr,sol],vars, Reals] =!= False,
      (* true *)
      (* ŠO‘¤‚ÉOr[]‚Ì‚ ‚éê‡‚ÍList‚Å’u‚«Š·‚¦‚éB‚È‚¢ê‡‚àList‚ÅˆÍ‚Ş *)
      sol = LogicalExpand[sol];
      (* debugPrint["sol after LogicalExpand:", sol];*)
      sol = If[Head[sol] === Or, Apply[List, sol], {sol}];
      (* debugPrint["sol after Apply Or List:", sol];*)
      (* “¾‚ç‚ê‚½ƒŠƒXƒg‚Ì—v‘f‚Ìƒwƒbƒh‚ªAnd‚Å‚ ‚éê‡‚ÍList‚Å’u‚«Š·‚¦‚éB‚È‚¢ê‡‚àList‚ÅˆÍ‚Ş *)
      sol = removeNotEqual[Map[(If[Head[#] === And, Apply[List, #], {#}]) &, sol]];
      (* debugPrint["sol after Apply And List:", sol];*)
      (* ˆê”Ô“à‘¤‚Ì—v‘f iƒŒƒxƒ‹2j‚ğ•¶š—ñ‚É‚·‚é *)
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

(* •Ï”–¼‚©‚ç u\[CloseCurlyQuote]v‚ğæ‚é *)
removeDash[var_] := (
   var /. Derivative[_][x_] -> x
);

(* 
 * tellVars‚ğŒ³‚ÉA‚»‚Ì‰Šú’l§–ñ‚ª•K—v‚©‚Ç‚¤‚©‚ğ’²‚×‚é
 * ‰Šú’l§–ñ’†‚ÉoŒ»‚·‚é•Ï”‚ªvars“à‚É‚È‚¯‚ê‚Î•s—v
 *)
isRequiredConstraint[cons_, tellVars_] := Block[{
   consVar
},
   consVar = cons /. x_[0] == y_ -> x;
   removeDash[consVar];
   If[MemberQ[tellVars, consVar], True, False]
];

(* tellVars‚ğŒ³‚ÉA‚»‚Ì•Ï”‚ª•K—v‚©‚Ç‚¤‚©‚ğ’²‚×‚é *)
isRequiredVariable[var_, tellVars_] := (
   If[MemberQ[tellVars, var], True, False]
);

removeNotEqual[sol_] := 
   DeleteCases[sol, Unequal[lhs_, rhs_], Infinity];

getNDVars[vars_] := Union[Map[(removeDash[#])&, vars]];

(* ‚ ‚é•Ï”x‚É‚Â‚¢‚ÄA *)
(* x[t]==a ‚Æ x[0]==a ‚ª‚ ‚Á‚½ê‡‚ÍAx'[t]==0‚ğÁ‹ i‚ ‚ê‚Îj *)
removeTrivialCons[cons_, consVars_] := Block[
  {exprRule, varSol, removedExpr,
   removeTrivialConsUnit, getRequiredVars},

  removeTrivialConsUnit[expr_, var_]:=(
    (* •û’ö®‚ğƒ‹[ƒ‹‰»‚·‚é *)
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

(* Reduce‚Å“¾‚ç‚ê‚½Œ‹‰Ê‚ğƒŠƒXƒgŒ`®‚É‚·‚é *)
(* And‚Å‚Í‚È‚­List‚Å‚­‚­‚é *)
applyList[reduceSol_] :=
  If[Head[reduceSol] === And, List @@ reduceSol, List[reduceSol]];

(* ƒpƒ‰ƒ[ƒ^‚Ì§–ñ‚ğ“¾‚é *)
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

      (* 1‚Â‚¾‚¯Ì—p *)
      (* TODO: •¡”‰ğ‚ ‚éê‡‚àl‚¦‚é *)
      If[Head[sol]===Or, sol = First[sol]];

      sol = applyList[sol];

      (* ’è”ŠÖ”‚Ìê‡‚É‰ßèŒˆ’èŒn‚ÌŒ´ˆö‚Æ‚È‚é”÷•ª§–ñ‚ğæ‚èœ‚­ *)
      sol = removeTrivialCons[sol, vars];

      {DExpr, DExprVars, NDExpr, otherExpr} = splitExprs[sol];

      Quiet[Check[Check[If[Cases[DExpr, Except[True]] === {},
                          (* ƒXƒgƒA‚ª‹ó‚Ìê‡‚ÍDSolve‚ª‰ğ‚¯‚È‚¢‚Ì‚Å‹óW‡‚ğ•Ô‚· *)
                          sol = {},

                          sol = DSolve[DExpr, DExprVars, t];
                          (* 1‚Â‚¾‚¯Ì—p *)
                          (* TODO: •¡”‰ğ‚ ‚éê‡‚àl‚¦‚é *)
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

(* DSolve‚Åˆµ‚¦‚é® (DExpr)‚Æ‚»‚¤‚Å‚È‚¢® (NDExpr)‚Æ‚»‚êˆÈŠO iotherExprj‚É•ª‚¯‚é *)
(* ”÷•ª’l‚ğŠÜ‚Ü‚¸ÿÿÿÿÿÿ•Ï”‚ª2í—ŞˆÈão‚é® (NDExpr)‚â“™®ˆÈŠO iotherExprj‚ÍDSolve‚Åˆµ‚¦‚È‚¢ *)
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
(*      (\* §–ñƒXƒgƒA‚ª‹ó‚Å‚È‚¢ê‡A•s—v‚È‰Šú’l§–ñ‚ğæ‚èœ‚­•K—v‚ª‚ ‚é *\) *)
(*      newTellVars = Map[removeDash, Map[(# /. x_[t] -> x) &, tellsVars]]; *)
(*      (\* store‚ªList[And[]]‚ÌŒ`‚É‚È‚Á‚Ä‚¢‚éê‡‚ÍAˆê’UA’†‚ÌAnd‚ğæ‚èo‚· *\) *)
(*      newStore = If[Head[First[store]] === And, Apply[List, First[store]], store]; *)
(*      removedStore = {Apply[And, Select[newStore, (isRequiredConstraint[#, newTellVars]) &]]}; *)
(*      newStoreVars = Map[removeDash, Map[(# /. x_[t] -> x) &, storeVars]]; *)
(*      removedStoreVars = Map[(#[t])&, Select[newStoreVars, (isRequiredVariable[#, newTellVars])&]], *)

(*      (\* §–ñƒXƒgƒA‚ª‹ó‚Ìê‡ *\) *)
(*      removedStore = store; *)
(*      removedStoreVars = storeVars]; *)

(*   If[sol =!= overconstraint, *)
(*      cons = Map[(ToString[FullForm[#]]) &, Join[tells, removedStore]]; *)
(*      vars = Join[tellsVars, removedStoreVars]; *)
(*      If[sol =!= underconstraint, {1, cons, vars}, {2, cons, vars}], *)
(*      0] *)
(* ); *)

(*
 * –ß‚è’l‚ÌƒŠƒXƒg‚Ìæ“ªF
 *  0 : Solver Error
 *  1 : [‘«
 *  2 : §–ñƒGƒ‰[
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
 * Ÿ‚Ìƒ|ƒCƒ“ƒgƒtƒF[ƒY‚ÉˆÚs‚·‚é‚ğ‹‚ß‚é
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
      (* ‰ğ‚È‚µ‚Æ‹«ŠE’l‚Ì‰ğ‚ğ‹æ•Ê‚·‚é *)  
      sol = Quiet[Check[Reduce[{timeMinCons && (maxTime>=t) && (integAsk) && (And @@ otherExpr)}, t],
                        errorSol,
                        {Reduce::nsmet}],
                  {Reduce::nsmet}];
      (*  debugPrint["calcMinTime#sol: ", sol]; *)
      If[sol=!=False && sol=!=errorSol, 
        (* true *)
        (* ¬‚è—§‚Ât‚ÌÅ¬’l‚ğ‹‚ß‚é *)
        minT = First[Quiet[Minimize[{t, timeMinCons && (sol)}, {t}], 
                           Minimize::wksol]],

        (* false *)
        minT = error],
      
      (* false *)
      minT=0];

    (* Piecewise‚Ö‚Ì‘Î‰ *)
    If[Head[minT] === Piecewise, minT = First[First[First[minT]]]];
    (* debugPrint["calcMinTime#minT: ", minT];*)
    (* 0•bŒã‚Ì‚ğŠÜ‚ñ‚Å‚Í‚¢‚¯‚È‚¢ *)
    If[includeZero===False && minT===0, minT=error];
    (* 0•bŒã‚Ì—£U•Ï‰»‚ªs‚í‚ê‚é‚©‚Ìƒ`ƒFƒbƒN‚È‚Ì‚Å0‚Å‚È‚¯‚ê‚ÎƒGƒ‰[ *)
    If[includeZero===True && minT=!=0, minT=error];

    If[minT === error,
      (* true *)
      {currentMinT, currentMinAsk},
 
      (* false *)
      paramVars = Union[Fold[(Join[#1, getParamVar[#2]]) &, {}, otherExpr]];
      compareResult = Map[(Reduce[{#[minT, currentMinT] && (And @@ otherExpr)}, paramVars]) &, 
                          {Less, Equal, Greater}];


      If[Count[compareResult, Not[False]] > 1,
        (* —v•ªŠò *)
        (* TODO: “KØ‚Èˆ—‚ğ‚·‚é *)
        1,
        Switch[First[First[Position[compareResult, x_ /; x =!= False, {1}, Heads -> False]]],
          1, {minT, {{type, askID}}},
          2, {minT, Append[currentMinAsk, {type, askID}]},
          3, {currentMinT, currentMinAsk}]]]
  );


  Fold[calcMinTime,
       {maxTime, {}},
       Join[Map[({pos2neg, Not[#[[1]]], #[[2]]})&, posAsk],
            Map[({neg2pos,     #[[1]],  #[[2]]})&, negAsk],
            Fold[(Join[#1, Map[({neg2pos, #[[1]], #[[2]]})&, #2]])&, {}, NACons]]]
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

(* ƒpƒ‰ƒ[ƒ^§–ñ‚ğ“¾‚é *)
getParamCons[cons_] := Cases[cons, x_ /; Not[hasVariable[x]], {1}];
(* ®’†‚Ìƒpƒ‰ƒ[ƒ^•Ï”‚ğ“¾‚é *)
getParamVar[paramCons_] := Cases[paramCons, x_ /; Not[NumericQ[x]], Infinity];

(*
 * ask‚Ì“±oó‘Ô‚ª•Ï‰»‚·‚é‚Ü‚ÅÏ•ª‚ğ‚¨‚±‚È‚¤
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
    tmpIntegSol = LogicalExpand[removeNotEqual[Reduce[Complement[cons, paramCons], vars, Reals]]];
    (* 1‚Â‚¾‚¯Ì—p *)
    (* TODO: •¡”‰ğ‚ ‚éê‡‚àl‚¦‚é *)
    If[Head[tmpIntegSol]===Or, tmpIntegSol = First[tmpIntegSol]];

    tmpIntegSol = removeTrivialCons[applyList[tmpIntegSol], vars];

    {DExpr, DExprVars, NDExpr, otherExpr} = splitExprs[tmpIntegSol];

    If[Cases[DExpr, Except[True]] === {},
      tmpIntegSol = {},
      tmpIntegSol = Quiet[DSolve[DExpr, DExprVars, t],
                        {Solve::incnst}];
      (* 1‚Â‚¾‚¯Ì—p *)
      (* TODO:•¡”‰ğ‚ ‚éê‡‚àl‚¦‚é *)
      tmpIntegSol = First[tmpIntegSol]];

    (* debugPrint["tmpIntegSol: ", tmpIntegSol]; *)
    (* tmpIntegSol‚¨‚æ‚ÑNDExpr“à‚ÉoŒ»‚·‚éND•Ï”‚Ìˆê——‚ğ“¾‚é *)
    solVars = getNDVars[Union[Cases[Join[tmpIntegSol, NDExpr], _[t], Infinity]]];
    (* integrateCalc‚ÌŒvZŒ‹‰Ê‚Æ‚µ‚Ä•K—v‚È•Ï”‚Ìˆê—— *)
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
            ToString[tmpMinT, InputForm], 
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
 * ®‚É‘Î‚µ‚Ä—^‚¦‚ç‚ê‚½ŠÔ‚ğ“K—p‚·‚é
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
 * form‚©‚çto‚Ü‚Å‚ÌƒŠƒXƒg‚ğintervalŠÔŠu‚Åì¬‚·‚é
 * ÅŒã‚É•K‚¸to‚ª—ˆ‚é‚½‚ß‚ÉC
 * ÅŒã‚ÌŠÔŠu‚Ì‚İinterval‚æ‚è‚à’Z‚­‚È‚é‰Â”\«‚ª‚ ‚é
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
 * —^‚¦‚ç‚ê‚½®‚ğÏ•ª‚µC•Ô‚·
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
 * —^‚¦‚ç‚ê‚½®‚ğ‹ß—‚·‚é
 *)
approxExpr[precision_, expr_] :=
  Rationalize[N[Simplify[expr], precision + 3], 
              Divide[1, Power[10, precision]]];

(* 
 * —^‚¦‚ç‚ê‚½t‚Ì®‚ğƒ^ƒCƒ€ƒVƒtƒg
 *)
exprTimeShift[expr_, time_] := ToString[FullForm[Simplify[ToExpression[expr] /. t -> t - ToExpression[time] ]]];
