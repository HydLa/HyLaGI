(*
 * ’¥Ç’¥Ð’¥Ã’¥¯’ÍÑ’¥á’¥Ã’¥»’¡¼’¥¸’½Ð’ÎÏ’´Ø’¿ô
 *)
If[optUseDebugPrint,
  debugPrint[arg___] := Print[InputForm[{arg}]],
  debugPrint[arg___] := Null];

(*
 * checkEntailment’¤ÎIP’¥Ð’¡¼’¥¸’¥ç’¥ó
 * 0 : Solver Error
 * 1 : ’Æ³’½Ð’²Ä’Ç½
 * 2 : ’Æ³’½Ð’ÉÔ’²Ä’Ç½
 *)
checkEntailmentInterval[guard_, store_, vars_] := Quiet[Check[Block[
  {tStore, sol, integGuard},
  debugPrint["guard:", guard, "store:", store, "vars:", vars];
  tStore = exDSolve[store, vars];
  If[tStore =!= overconstraint && tStore =!= underconstraint,
    (* guard’¤ËtStore’¤ò’Å¬’ÍÑ’¤¹’¤ë *)
    integGuard = guard /. tStore;
    If[integGuard =!= False,
      (* ’¤½’¤Î’·ë’²Ì’¤Èt>0’¤È’¤ò’Ï¢’Î© *)
      (* debugPrint["integGuard:", integGuard]; *)
      sol = Reduce[{integGuard && t > 0}, t];
      (* Inf’¤ò’¼è’¤Ã’¤Æ0’¤Ë’¤Ê’¤ì’¤ÐEntailed *)
      If[sol =!= False && MinValue[{t, sol}, t] === 0,
        {1},
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
 * ’¤¢’¤ë1’¤Ä’¤Îask’À©’Ìó’¤Ë’´Ø’¤·’¤Æ’¡¢’¤½’¤Î’¥¬’¡¼’¥É’¤¬’À©’Ìó’¥¹’¥È’¥¢’¤«’¤éentail’¤Ç’¤­’¤ë’¤«’¤É’¤¦’¤«’¤ò’¥Á’¥§’¥Ã’¥¯
 *  0 : Solver Error
 *  1 : ’Æ³’½Ð’²Ä’Ç½
 *  2 : ’Æ³’½Ð’ÉÔ’²Ä’Ç½
 *)
checkEntailment[guard_, store_, vars_] := Quiet[Check[Block[
{sol},
  debugPrint["guard:", guard, "store:", store, "vars:", vars];

  sol = Reduce[Append[store, guard], vars, Reals];
  If[sol=!=False, 
      If[Reduce[Append[store, Not[sol]], vars, Reals]===False, 
          {1}, 
          {2}],
      {2}]
],
  {0, $MessageList}
]];


(* Print[checkEntailment[ht==0, {}, {ht}]] *)

(*
 * ’ÊÑ’¿ô’Ì¾’¤Î’Æ¬’¤Ë’¤Ä’¤¤’¤Æ’¤¤’¤ë"usrVar"’¤ò’¼è’¤ê’½ü’¤¯
 *)
renameVar[varName_] := (
  ToExpression[First[StringCases[ToString[varName], "usrVar" ~~ x__ -> x]]]
);

(*
 * isConsistent’Æâ’¤ÎReduce’¤Î’·ë’²Ì’ÆÀ’¤é’¤ì’¤¿’²ò’¤ò{’ÊÑ’¿ô’Ì¾, ’ÃÍ}’¡¡’¤Î’¥ê’¥¹’¥È’·Á’¼°’¤Ë’¤¹’¤ë
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

(* Or’¤Ç’¤Ä’¤Ê’¤¬’¤Ã’¤Æ’¤¤’¤ë’¡Ê’Ê£’¿ô’²ò’¤¬’¤¢’¤ë’¡Ë’¾ì’¹ç’¤Ï’¡¢’¤½’¤Î’¤¦’¤Á’¤Î1’¤Ä’¤Î’¤ß’¤ò’ÊÖ’¤¹’¡© *)
createVariableList[Or[expr_, others__], vars_, result_] := (
  createVariableList[expr, vars, result]
);

createVariableList[Equal[varName_, varValue_], vars_, result_] := Block[{
  sol
},
  sol = (Solve[Equal[varName, varValue], vars])[[1]];
  createVariableList[sol, result]
];

(* Reduce[{}, {}]’¤Î’¤È’¤­ *)
createVariableList[True, vars_, result_] := (
  Return[result]
);

(*
 * ’À©’Ìó’¤¬’Ìµ’Ì·’½â’¤Ç’¤¢’¤ë’¤«’¤ò’¥Á’¥§’¥Ã’¥¯’¤¹’¤ë
 * Reduce’¤Ç’²ò’¤¤’¤¿’·ë’²Ì’¡¢’²ò’¤¬’ÆÀ’¤é’¤ì’¤ì’¤Ð’Ìµ’Ì·’½â
 * ’ÆÀ’¤é’¤ì’¤¿’²ò’¤ò’ÍÑ’¤¤’¤Æ’¡¢’³Æ’ÊÑ’¿ô’¤Ë’´Ø’¤·’¤Æ{’ÊÑ’¿ô’Ì¾, ’ÃÍ}’¡¡’¤È’¤¤’¤¦’·Á’¼°’¤Ç’É½’¤·’¤¿’¥ê’¥¹’¥È’¤ò’ÊÖ’¤¹
 *
 * ’Ìá’¤ê’ÃÍ’¤Î’¥ê’¥¹’¥È’¤Î’Àè’Æ¬’¡§
 *  0 : Solver Error
 *  1 : ’½¼’Â­
 *  2 : ’À©’Ìó’¥¨’¥é’¡¼
 *)
isConsistent[expr_, vars_] := Quiet[Check[Block[
{
  sol
},
  debugPrint["expr:", expr, "vars:", vars];
  sol = Reduce[expr, vars, Reals];
(*  debugPrint["sol after Reduce:", sol];*)
  If[sol =!= False,
      (* true *)
      (* ’³°’Â¦’¤ËOr[]’¤Î’¤¢’¤ë’¾ì’¹ç’¤ÏList’¤Ç’ÃÖ’¤­’´¹’¤¨’¤ë’¡£’¤Ê’¤¤’¾ì’¹ç’¤âList’¤Ç’°Ï’¤à *)
      sol = LogicalExpand[sol];
     (*      debugPrint["sol after LogicalExpand:", sol];*)
      sol = If[Head[sol] === Or, Apply[List, sol], {sol}];
     (*      debugPrint["sol after Apply Or List:", sol];*)
      (* ’ÆÀ’¤é’¤ì’¤¿’¥ê’¥¹’¥È’¤Î’Í×’ÁÇ’¤Î’¥Ø’¥Ã’¥É’¤¬And’¤Ç’¤¢’¤ë’¾ì’¹ç’¤ÏList’¤Ç’ÃÖ’¤­’´¹’¤¨’¤ë’¡£’¤Ê’¤¤’¾ì’¹ç’¤âList’¤Ç’°Ï’¤à *)
      sol = Map[(If[Head[#] === And, Apply[List, #], {#}]) &, sol];
     (*      debugPrint["sol after Apply And List:", sol];*)
      (* ’°ì’ÈÖ’Æâ’Â¦’¤Î’Í×’ÁÇ ’¡Ê’¥ì’¥Ù’¥ë2’¡Ë’¤ò’Ê¸’»ú’Îó’¤Ë’¤¹’¤ë *)
      {1, Map[(ToString[FullForm[#]]) &, removeNotEqual[sol], {2}]},

      (* false *)
      {2}]
],
  {0, $MessageList}
]];


(* Print[isConsistent[s[x==2, 7==x*x], {x,y}]] *)

(* ’ÊÑ’¿ô’Ì¾’¤«’¤é ’¡Ö’¡Ç’¡×’¤ò’¼è’¤ë *)
removeDash[var_] := (
   var /. Derivative[_][x_] -> x
);

(* 
 * tellVars’¤ò’¸µ’¤Ë’¡¢’¤½’¤Î’½é’´ü’ÃÍ’À©’Ìó’¤¬’É¬’Í×’¤«’¤É’¤¦’¤«’¤ò’Ä´’¤Ù’¤ë
 * ’½é’´ü’ÃÍ’À©’Ìó’Ãæ’¤Ë’½Ð’¸½’¤¹’¤ë’ÊÑ’¿ô’¤¬vars’Æâ’¤Ë’¤Ê’¤±’¤ì’¤Ð’ÉÔ’Í×
 *)
isRequiredConstraint[cons_, tellVars_] := Block[{
   consVar
},
   consVar = cons /. x_[0] == y_ -> x;
   removeDash[consVar];
   If[MemberQ[tellVars, consVar], True, False]
];

(* tellVars’¤ò’¸µ’¤Ë’¡¢’¤½’¤Î’ÊÑ’¿ô’¤¬’É¬’Í×’¤«’¤É’¤¦’¤«’¤ò’Ä´’¤Ù’¤ë *)
isRequiredVariable[var_, tellVars_] := (
   If[MemberQ[tellVars, var], True, False]
);

removeNotEqual[sol_] := 
   DeleteCases[sol, Unequal[lhs_, rhs_], Infinity];

getNDVars[vars_] := Union[Map[(removeDash[#] /. x_[t] -> x) &, vars]];

(* ’¤¢’¤ë’ÊÑ’¿ôx’¤Ë’¤Ä’¤¤’¤Æ’¡¢ *)
(* x[t]==a ’¤È x[0]==a ’¤¬’¤¢’¤Ã’¤¿’¾ì’¹ç’¤Ï’¡¢x'[t]==0’¤ò’¾Ã’µî ’¡Ê’¤¢’¤ì’¤Ð’¡Ë *)
removeTrivialCons[expr_, var_] := Block[
   {exprRule, varSol, removedExpr},

   (* ’Êý’Äø’¼°’¤ò’¥ë’¡¼’¥ë’²½’¤¹’¤ë *)
   exprRule = Map[(Rule @@ #) &, expr];
   varSol = var[t] /. exprRule;
   If[MemberQ[expr, var[0] == varSol] && MemberQ[expr, var'[t] == 0],
     (* true *)
     removedExpr = DeleteCases[expr, var'[t] == 0],

     (* false *)
     removedExpr = expr
   ];
   removedExpr
];

(* Reduce’¤Ç’ÆÀ’¤é’¤ì’¤¿’·ë’²Ì’¤ò’¥ê’¥¹’¥È’·Á’¼°’¤Ë’¤¹’¤ë *)
(* And’¤Ç’¤Ï’¤Ê’¤¯List’¤Ç’¤¯’¤¯’¤ë *)
applyList[reduceSol_] :=
  If[Head[reduceSol] === And, List @@ reduceSol, List[reduceSol]];

exDSolve[expr_, vars_] := Block[
{sol, DExpr, DExprVars, NDExpr},
  sol = removeNotEqual[Reduce[Cases[expr, Except[True] | Except[False]], vars]];

  If[sol===False,
    overconstraint,
    If[sol===True,
      underconstraint,

      (* 1’¤Ä’¤À’¤±’ºÎ’ÍÑ *)
      (* TODO: ’Ê£’¿ô’²ò’¤¢’¤ë’¾ì’¹ç’¤â’¹Í’¤¨’¤ë *)
      If[Head[sol]===Or, sol = First[sol]];

      sol = applyList[sol];

      (* ’Äê’¿ô’´Ø’¿ô’¤Î’¾ì’¹ç’¤Ë’²á’¾ê’·è’Äê’·Ï’¤Î’¸¶’°ø’¤È’¤Ê’¤ë’Èù’Ê¬’À©’Ìó’¤ò’¼è’¤ê’½ü’¤¯ *)
      sol = Fold[(Intersection[#1, removeTrivialCons[sol, #2]]) &, sol, getNDVars[vars]]; 

      {DExpr, DExprVars, NDExpr} = splitExprs[sol];

      Quiet[Check[Check[sol = DSolve[DExpr, DExprVars, t];
                        (* 1’¤Ä’¤À’¤±’ºÎ’ÍÑ *)
                        (* TODO: ’Ê£’¿ô’²ò’¤¢’¤ë’¾ì’¹ç’¤â’¹Í’¤¨’¤ë *)
                        sol = First[sol];
                        sol = Reduce[Join[Map[(Equal @@ #) &, sol], NDExpr], 
                                     Map[(#[t]) &, getNDVars[vars]]];
                        If[sol =!= False, 
                           Map[(Rule @@ #) &, applyList[sol]], 
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

(* DSolve’¤Ç’°·’¤¨’¤ë’¼° (DExpr)’¤È’¤½’¤¦’¤Ç’¤Ê’¤¤’¼° (NDExpr)’¤Ë’Ê¬’¤±’¤ë *)
(* ’Èù’Ê¬’ÃÍ’¤ò’´Þ’¤Þ’¤º‰¤’¤«’¤Ä‰¤’ÊÑ’¿ô’¤¬2’¼ï’Îà’°Ê’¾å’½Ð’¤ë’¼° (NDExpr)’¤ÏDSolve’¤Ç’°·’¤¨’¤Ê’¤¤ *)
splitExprs[expr_] := Block[
  {NDExpr, DExpr, DExprVars},
  NDExpr = Select[expr, (MemberQ[#, _'[t], Infinity] =!= True &&
                         Length[Union[Cases[#, _[t], Infinity]]] > 1) &];  
  DExpr = Complement[expr, NDExpr];
  DExprVars = Union[Fold[(Join[#1, Cases[#2, _[t] | _[0], Infinity] /. x_[0] -> x[t]]) &, 
                         {}, DExpr]];
  {DExpr, DExprVars, NDExpr}
];

(* isConsistentInterval[tells_, store_, tellsVars_, storeVars_] := ( *)
(*   If[store =!= {}, *)
(*      (\* ’À©’Ìó’¥¹’¥È’¥¢’¤¬’¶õ’¤Ç’¤Ê’¤¤’¾ì’¹ç’¡¢’ÉÔ’Í×’¤Ê’½é’´ü’ÃÍ’À©’Ìó’¤ò’¼è’¤ê’½ü’¤¯’É¬’Í×’¤¬’¤¢’¤ë *\) *)
(*      newTellVars = Map[removeDash, Map[(# /. x_[t] -> x) &, tellsVars]]; *)
(*      (\* store’¤¬List[And[]]’¤Î’·Á’¤Ë’¤Ê’¤Ã’¤Æ’¤¤’¤ë’¾ì’¹ç’¤Ï’¡¢’°ì’Ã¶’¡¢’Ãæ’¤ÎAnd’¤ò’¼è’¤ê’½Ð’¤¹ *\) *)
(*      newStore = If[Head[First[store]] === And, Apply[List, First[store]], store]; *)
(*      removedStore = {Apply[And, Select[newStore, (isRequiredConstraint[#, newTellVars]) &]]}; *)
(*      newStoreVars = Map[removeDash, Map[(# /. x_[t] -> x) &, storeVars]]; *)
(*      removedStoreVars = Map[(#[t])&, Select[newStoreVars, (isRequiredVariable[#, newTellVars])&]], *)

(*      (\* ’À©’Ìó’¥¹’¥È’¥¢’¤¬’¶õ’¤Î’¾ì’¹ç *\) *)
(*      removedStore = store; *)
(*      removedStoreVars = storeVars]; *)

(*   If[sol =!= overconstraint, *)
(*      cons = Map[(ToString[FullForm[#]]) &, Join[tells, removedStore]]; *)
(*      vars = Join[tellsVars, removedStoreVars]; *)
(*      If[sol =!= underconstraint, {1, cons, vars}, {2, cons, vars}], *)
(*      0] *)
(* ); *)

(*
 * ’Ìá’¤ê’ÃÍ’¤Î’¥ê’¥¹’¥È’¤Î’Àè’Æ¬’¡§
 *  0 : Solver Error
 *  1 : ’½¼’Â­
 *  2 : ’À©’Ìó’¥¨’¥é’¡¼
 *)
isConsistentInterval[expr_, vars_] :=  Block[
  {sol},
  Quiet[Check[(
    debugPrint["expr:", expr, "vars:", vars];
    If[expr === {},
      (* true *)
      {1}, 

      (* false *)
      sol = exDSolve[expr, vars];
      If[sol=!=overconstraint,
        {1},
        {2}]]
  ),
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
 * ’¼¡’¤Î’¥Ý’¥¤’¥ó’¥È’¥Õ’¥§’¡¼’¥º’¤Ë’°Ü’¹Ô’¤¹’¤ë’»þ’¹ï’¤ò’µá’¤á’¤ë
 *)
calcNextPointPhaseTime[
                       includeZero_, maxTime_, posAsk_, negAsk_, NACons_] := 
Block[{
  calcMinTime,
  sol,
  minT,
  timeMinCons = If[includeZero===True, (t>=0), (t>0)]
},
  calcMinTime[{currentMinT_, currentMinAsk_}, {type_, integAsk_, askID_}] := (
    If[integAsk=!=False,
      (* true *)
      (* ’²ò’¤Ê’¤·’¤È’¶­’³¦’ÃÍ’¤Î’²ò’¤ò’¶è’ÊÌ’¤¹’¤ë *)  
       sol = Quiet[Check[Reduce[{timeMinCons && (maxTime>=t) && (integAsk)}, t],
                         errorSol,
                         {Reduce::nsmet}],
                   {Reduce::nsmet}];
      (*  debugPrint["calcNextPointPhaseTime#sol: ", sol]; *)
      If[sol=!=False && sol=!=errorSol, 
         (* true *)
         (* ’À®’¤ê’Î©’¤Ät’¤Î’ºÇ’¾®’ÃÍ’¤ò’µá’¤á’¤ë *)
         minT = First[Quiet[Minimize[{t, timeMinCons && (sol)}, {t}], 
                            Minimize::wksol]],

         (* false *)
         minT = error],
      
      (* false *)
      minT=0];

      (* debugPrint["calcMinTime#minT: ", minT];*)
    (* 0’ÉÃ’¸å’¤Î’¤ò’´Þ’¤ó’¤Ç’¤Ï’¤¤’¤±’¤Ê’¤¤ *)
    If[includeZero===False && minT===0, minT=error];
    (* 0’ÉÃ’¸å’¤Î’Î¥’»¶’ÊÑ’²½’¤¬’¹Ô’¤ï’¤ì’¤ë’¤«’¤Î’¥Á’¥§’¥Ã’¥¯’¤Ê’¤Î’¤Ç0’¤Ç’¤Ê’¤±’¤ì’¤Ð’¥¨’¥é’¡¼ *)
    If[includeZero===True && minT=!=0, minT=error];

    Which[minT === error,      {currentMinT, currentMinAsk},
          minT <  currentMinT, {minT,        {{type, askID}}},
          minT == currentMinT, {minT,        Append[currentMinAsk, {type, askID}]},
          True,                {currentMinT, currentMinAsk}]
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

(*
 * ask’¤Î’Æ³’½Ð’¾õ’ÂÖ’¤¬’ÊÑ’²½’¤¹’¤ë’¤Þ’¤Ç’ÀÑ’Ê¬’¤ò’¤ª’¤³’¤Ê’¤¦
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
  tmpRet,
  NDExpr,
  DExpr,
  DExprVars
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

  If[cons=!={},
    (* true *)
    {DExpr, DExprVars, NDExpr} = splitExprs[cons];
    tmpIntegSol = applyList[removeNotEqual[Reduce[DExpr, DExprVars]]];

    tmpIntegSol = Fold[(Intersection[#1, removeTrivialCons[tmpIntegSol, #2]]) &, 
                       tmpIntegSol, getNDVars[vars]]; 

    (* 1’¤Ä’¤À’¤±’ºÎ’ÍÑ *)
    (* TODO: ’Ê£’¿ô’²ò’¤¢’¤ë’¾ì’¹ç’¤â’¹Í’¤¨’¤ë *)
    If[Head[tmpIntegSol]===Or, tmpIntegSol = First[tmpIntegSol]];
    tmpIntegSol = Quiet[DSolve[tmpIntegSol, DExprVars, t],
                        {Solve::incnst}];
    (* debugPrint["tmpIntegSol: ", tmpIntegSol]; *)

    (*1’¤Ä’¤À’¤±’ºÎ’ÍÑ*)
    (*TODO:’Ê£’¿ô’²ò’¤¢’¤ë’¾ì’¹ç’¤â’¹Í’¤¨’¤ë*)
    tmpIntegSol = First[tmpIntegSol];
    tmpIntegSol = applyList[removeNotEqual[Reduce[Join[Map[(Equal @@ #) &, tmpIntegSol], NDExpr], vars]]];
    (* ’Êý’Äø’¼°’¤ò’¥ë’¡¼’¥ë’²½’¤¹’¤ë *)
    tmpIntegSol = Map[(Rule @@ #) &, tmpIntegSol];

    tmpPosAsk = Map[(# /. tmpIntegSol ) &, posAsk];
    tmpNegAsk = Map[(# /. tmpIntegSol) &, negAsk];
    tmpNACons = Map[(# /. tmpIntegSol) &, NACons];
    {tmpMinT, tmpMinAskIDs} = 
      calcNextPointPhaseTime[False, maxTime, tmpPosAsk, tmpNegAsk, tmpNACons];
    tmpVarMap = 
      Map[({getVariableName[#], 
            getDerivativeCount[#], 
            createIntegratedValue[#, tmpIntegSol] // FullForm // ToString})&, 
            vars],

    (* false *)
    tmpMinT = maxTime;
    tmpVarMap = {};
    tmpMinAskIDs = {}
  ];
  tmpRet = {1,
            ToString[tmpMinT, InputForm], 
            tmpVarMap,
            tmpMinAskIDs, 
            If[tmpMinT>=maxTime, 1, 0]};
  debugPrint["ret:", tmpRet];
  tmpRet
],
  (* debugPrint["MessageList:", $MessageList]; *)
  {0, $MessageList}
]];

(*
 * ’¼°’¤Ë’ÂÐ’¤·’¤Æ’Í¿’¤¨’¤é’¤ì’¤¿’»þ’´Ö’¤ò’Å¬’ÍÑ’¤¹’¤ë
 *)
applyTime2Expr[expr_, time_] := (
  (expr /. t -> time) // FullForm // ToString
);

(*
 * form’¤«’¤éto’¤Þ’¤Ç’¤Î’¥ê’¥¹’¥È’¤òinterval’´Ö’³Ö’¤Ç’ºî’À®’¤¹’¤ë
 * ’ºÇ’¸å’¤Ë’É¬’¤ºto’¤¬’Íè’¤ë’¤¿’¤á’¤Ë’¡¤
 * ’ºÇ’¸å’¤Î’´Ö’³Ö’¤Î’¤ßinterval’¤è’¤ê’¤â’Ã»’¤¯’¤Ê’¤ë’²Ä’Ç½’À­’¤¬’¤¢’¤ë
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
 * ’Í¿’¤¨’¤é’¤ì’¤¿’¼°’¤ò’ÀÑ’Ê¬’¤·’¡¤’ÊÖ’¤¹
 *
 * 0: Solver Error
 * 1: Solved Successfully
 * 2: Under Constraint
 * 3: Over Constraint
 * 
 *)
integrateExpr[cons_, vars_] := Quiet[Check[Block[
{ sol },
(*
  debugPrint["cons:", cons,
             "vars:", vars];
 *)
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
             ToString[createIntegratedValue[#, First[sol]], InputForm]})&, 
           vars]}]
],
  {0, $MessageList}
]];

(* Print["integ:", integrateExpr[{ht'[t]==v[t], v'[t]==-10, ht[0]==a, v[0]==b}, {ht[t], ht'[t], v[t], v'[t]}]]; *)
(* Print["integ:", integrateExpr[{ht'[t]==v[t], v'[t]==-10, v'[t]==-20, ht[0]==a, v[0]==b}, {ht[t], ht'[t], v[t], v'[t]}]]; *)
(* Print["integ:", integrateExpr[{ht'[t]==x[t], v'[t]==-10, ht[0]==a, v[0]==b}, {x[t], ht[t], ht'[t], v[t], v'[t]}]]; *)

(*
 * ’Í¿’¤¨’¤é’¤ì’¤¿’¼°’¤ò’¶á’»÷’¤¹’¤ë
 *)
approxExpr[precision_, expr_] :=
  Rationalize[N[Simplify[expr], precision + 3], 
              Divide[1, Power[10, precision]]];
