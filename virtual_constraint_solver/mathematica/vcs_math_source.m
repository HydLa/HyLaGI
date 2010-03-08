(*
 * デバック用メッセージ出力関数
 *)
If[optUseDebugPrint,
  debugPrint[arg___] := Print[InputForm[{arg}]],
  debugPrint[arg___] := Null];

(*
 * ある1つのask制約に関して、そのガードが制約ストアからentailできるかどうかをチェック
 *  0 : Solver Error
 *  1 : 導出可能
 *  2 : 導出不可能
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
 * 変数名の頭についている"usrVar"を取り除く
 *)
renameVar[varName_] := (
  ToExpression[First[StringCases[ToString[varName], "usrVar" ~~ x__ -> x]]]
);

(*
 * isConsistent内のReduceの結果得られた解を{変数名, 値}　のリスト形式にする
 *)
createVariableList[Rule[varName_, varValue_], result_] := (
  name = renameVar[varName];
  Append[result, pair[name, varValue]]
);

createVariableList[{expr_}, result_] := (
  createVariableList[expr, result]
);

createVariableList[{expr_, others__}, result_] := (
  variableList = createVariableList[expr, result];
  createVariableList[{others}, variableList]
);

createVariableList[And[expr__], vars_, result_] := (
  sol = (Solve[expr, vars])[[1]];
  createVariableList[sol, result]
);

(* Orでつながっている（複数解がある）場合は、そのうちの1つのみを返す？ *)
createVariableList[Or[expr_, others__], vars_, result_] := (
  createVariableList[expr, vars, result]
);

createVariableList[Equal[varName_, varValue_], vars_, result_] := (
  sol = (Solve[Equal[varName, varValue], vars])[[1]];
  createVariableList[sol, result]
);

(* Reduce[{}, {}]のとき *)
createVariableList[True, vars_, result_] := (
  Return[result]
);

(*
 * 制約が無矛盾であるかをチェックする
 * Reduceで解いた結果、解が得られれば無矛盾
 * 得られた解を用いて、各変数に関して{変数名, 値}　という形式で表したリストを返す
 *
 * 戻り値のリストの先頭：
 *  0 : Solver Error
 *  1 : 充足
 *  2 : 制約エラー
 *)
isConsistent[expr_, vars_] := Quiet[Check[Block[
{
  sol
},
  debugPrint["expr:", expr, "vars:", vars];
  sol = Reduce[expr, vars];
  If[sol =!= False,
      (* true *)
      (* 外側にOr[]のある場合はListで置き換える。ない場合もListで囲む *)
      sol = LogicalExpand[sol];
      sol = If[Head[sol] === Or, Apply[List, sol], {sol}];
      (* 得られたリストの要素のヘッドがAndである場合はListで置き換える。ない場合もListで囲む *)
      sol = Map[(If[Head[#] === And, Apply[List, #], {#}]) &, sol];
      (* 一番内側の要素 （レベル2）を文字列にする *)
      {1, Map[(ToString[FullForm[#]]) &, sol, {2}]},

      (* false *)
      {2}]
],
  {0, $MessageList}
]];


(* Print[isConsistent[s[x==2, 7==x*x], {x,y}]] *)

(* 変数名から 「’」を取る *)
removeDash[var_] := (
   var /. Derivative[_][x_] -> x
);

(* 
 * tellVarsを元に、その初期値制約が必要かどうかを調べる
 * 初期値制約中に出現する変数がvars内になければ不要
 *)
isRequiredConstraint[cons_, tellVars_] := (
   consVar = cons /. x_[0] == y_ -> x;
   removeDash[consVar];
   If[MemberQ[tellVars, consVar], True, False]
);

(* tellVarsを元に、その変数が必要かどうかを調べる *)
isRequiredVariable[var_, tellVars_] := (
   If[MemberQ[tellVars, var], True, False]
);

exDSolve[expr_, vars_] := Block[
{sol},
  sol = Reduce[Cases[expr, Except[True] | Except[False]], vars];
  If[sol===False,
      overconstraint,

      Quiet[Check[
            Check[DSolve[sol, vars, t],
                    underconstraint,
                    {DSolve::underdet, Solve::svars, DSolve::deqx, 
                     DSolve::bvnr, DSolve::bvsing}],
            overconstraint,
            {DSolve::overdet, DSolve::bvnul, DSolve::dsmsm}],
          {DSolve::underdet, DSolve::overdet, DSolve::deqx, 
           Solve::svars, DSolve::bvnr, DSolve::bvsing, 
           DSolve::bvnul, DSolve::dsmsm}]]
];


(* isConsistentInterval[tells_, store_, tellsVars_, storeVars_] := ( *)
(*   If[store =!= {}, *)
(*      (\* 制約ストアが空でない場合、不要な初期値制約を取り除く必要がある *\) *)
(*      newTellVars = Map[removeDash, Map[(# /. x_[t] -> x) &, tellsVars]]; *)
(*      (\* storeがList[And[]]の形になっている場合は、一旦、中のAndを取り出す *\) *)
(*      newStore = If[Head[First[store]] === And, Apply[List, First[store]], store]; *)
(*      removedStore = {Apply[And, Select[newStore, (isRequiredConstraint[#, newTellVars]) &]]}; *)
(*      newStoreVars = Map[removeDash, Map[(# /. x_[t] -> x) &, storeVars]]; *)
(*      removedStoreVars = Map[(#[t])&, Select[newStoreVars, (isRequiredVariable[#, newTellVars])&]], *)

(*      (\* 制約ストアが空の場合 *\) *)
(*      removedStore = store; *)
(*      removedStoreVars = storeVars]; *)

(*   If[sol =!= overconstraint, *)
(*      cons = Map[(ToString[FullForm[#]]) &, Join[tells, removedStore]]; *)
(*      vars = Join[tellsVars, removedStoreVars]; *)
(*      If[sol =!= underconstraint, {1, cons, vars}, {2, cons, vars}], *)
(*      0] *)
(* ); *)

(*
 * 戻り値のリストの先頭：
 *  0 : Solver Error
 *  1 : 充足
 *  2 : 制約エラー
 *)
isConsistentInterval[expr_, vars_] :=  Quiet[Check[(
  debugPrint["expr:", expr, "vars:", vars];
  If[expr === {},
    (* true *)
    {1}, 

    (* false *)
    If[exDSolve[expr, vars]=!=overconstraint,
        {1},
        {2}]]
),
  {0, $MessageList}
]];

(* Print[exDSolve[{True, True, Derivative[1][usrVarht][t] == usrVarv[t], Derivative[1][usrVarv][t] == -10, usrVarht[0] == 10, usrVarv[0] == 0},  *)
(* {usrVarht[t], Derivative[1][usrVarht][t], usrVarv[t], Derivative[1][usrVarv][t]}]]; *)

(* Print[isConsistentInterval[ *)
(* {Derivative[1][usrVarht][t] == usrVarv[t],  *)
(*   Derivative[1][usrVarv][t] == -10,  *)
(*   usrVarv[0] == Derivative[1][usrVarht]},  *)
(*   {Derivative[1][usrVarht][t], usrVarv[t], Derivative[1][usrVarv][t]}]]; *)

(*
 * 次のポイントフェーズに移行する時刻を求める
 *)
calcNextPointPhaseTime[
  includeZero_, maxTime_, posAsk_, negAsk_] := 
Block[{
  calcMinTime,
  sol,
  minT,
  timeMinCons = If[includeZero===True, (t>=0), (t>0)]
},
  calcMinTime[{currentMinT_, currentMinAsk_}, {type_, integAsk_, ask_}] := (
    If[integAsk=!=False,
      (* true *)
      (* 解なしと境界値の解を区別する *)  
      sol = Reduce[{timeMinCons && (maxTime>=t) && (integAsk)}, t];
      If[sol=!=False, 
         (* true *)
         (* 成り立つtの最小値を求める *)
         minT = First[Quiet[Minimize[{t, timeMinCons && (sol)}, {t}], 
                            Minimize::wksol]],

         (* false *)
         minT = error],
      
      (* false *)
      minT=0];

    (* 0秒後のを含んではいけない *)
    If[includeZero===False && minT===0, minT=error];
    (* 0秒後の離散変化が行われるかのチェックなので0でなければエラー *)
    If[includeZero===True && minT=!=0, minT=error];

    Which[minT === error,      {currentMinT, currentMinAsk},
          minT <  currentMinT, {minT,        {{type, ask}}},
          minT == currentMinT, {minT,        Append[currentMinAsk, {type, ask}]},
          True,                {currentMinT, currentMinAsk}]
  );

  Fold[calcMinTime,
       {maxTime, {}},
        Join[Map[({pos2neg, Not[#[[1]]], #[[2]]})&, posAsk],
             Map[({neg2pos,     #[[1]],  #[[2]]})&, negAsk]]]
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
 * askの導出状態が変化するまで積分をおこなう
 *)
integrateCalc[cons_, 
              posAsk_, negAsk_, 
              vars_, 
              maxTime_] := Quiet[Check[Block[
{
  tmpIntegSol,
  tmpPosAsk,
  tmpNegAsk,
  tmpMinT, 
  tmpMinAskIDs,
  tmpVarMap,
  tmpRet
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
             "vars:", vars, 
             "maxTime:", maxTime];

  tmpIntegSol = First[DSolve[Reduce[cons, vars], vars, t]];
  tmpPosAsk = Map[(# /. tmpIntegSol ) &, posAsk];
  tmpNegAsk = Map[(# /. tmpIntegSol) &, negAsk];
  {tmpMinT, tmpMinAskIDs} = 
    calcNextPointPhaseTime[False, maxTime, tmpPosAsk, tmpNegAsk];
  tmpVarMap = 
    Map[({getVariableName[#], 
          getDerivativeCount[#], 
          createIntegratedValue[#, tmpIntegSol] // FullForm // ToString})&, 
        vars];
  tmpRet = {1,
            ToString[tmpMinT, InputForm], 
            tmpVarMap,
            tmpMinAskIDs, 
            If[tmpMinT>=maxTime, 1, 0]};
  debugPrint["ret:", tmpRet];
  tmpRet
],
  {0, $MessageList}
]];

(*
 * 式に対して与えられた時間を適用する
 *)
applyTime2Expr[expr_, time_] := (
  (expr /. t -> time) // FullForm // ToString
);

(*
 * formからtoまでのリストをinterval間隔で作成する
 * 最後に必ずtoが来るために，
 * 最後の間隔のみintervalよりも短くなる可能性がある
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
 * 与えられた式を積分し，返す
 *
 * 0: Solver Error
 * 1: Solved Successfully
 * 2: Under Constraint
 * 3: Over Constraint
 * 
 *)
integrateExpr[cons_, vars_] := Quiet[Check[Block[
{ sol },
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
 * 与えられた式を近似する
 *)
approxExpr[precision_, expr_] :=
  Rationalize[N[Simplify[expr], precision + 3], 
              Divide[1, Power[10, precision]]];
