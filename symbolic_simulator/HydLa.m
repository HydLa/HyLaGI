
(*
 * ある1つのask制約に関して、そのガードがtell制約からentailできるかどうかをチェック
 *)
checkEntailment[guard_, tells_, vars_] := (
  tmpSol = Reduce[Append[tells, guard], vars, Reals];
  If[tmpSol===False, Return[-1]];
  If[Reduce[Append[tells, Not[tmpSol]], vars, Reals]===False, 1, 0]
);

(*
 * 制約が無矛盾であるかをチェック
 *)
isConsistent[expr_, vars_] := (
  (*Print["isConsistent#expr:", expr];*)
  (*Print["isConsistent#vars:", vars];*)
  (*Return[116]*)
  (*Return[expr]*)
  If[Reduce[expr, vars] =!= False, 1, 0]
);

(* isConsistent[expr_, vars_] :=
 *  If[DSolve[expr, vars, t] != {}, 1, 0]; 
 *)

(* $MaxExtraPrecision = Infinity *)

(*
 * main function
 *)
If[optUseProfile, 
  HydLaMain[arg___] := (
    profile["Total", HydLaSolve[arg]];
    profilePrintResultAll[]
  ),
  HydLaMain[arg___] := HydLaSolve[arg]
];

(*
 * デバック用メッセージ出力関数
 *)
If[optUseDebugPrint,
  debugPrint[arg___] := Print[arg],
  debugPrint[arg___] := Null];

(*
 * profiling function
 * print CPU-time of function and return function's value
 *)
If[optUseProfile,
  (* True *)
  profile[label_String, func_] := Block[{time, val, indent},
    profCount = profCount + 1;
    If[MatchQ[profInfo[label], _profInfo],
      (* True *)(* {呼び出し回数, 実行CPU時間} *)
      profInfo[label] = {0, 0.0};
      AppendTo[profLabels, label],
      (* False *)
      Null
    ];
    {time, val} = Timing[func];
    profInfo[label] = {profInfo[label][[1]]+1, profInfo[label][[2]]+time};
    indent = Nest[(#<>"** ")&, "", profCount-1];
    Print["#*", indent, label, " : ", CForm[time], " ***"];
    profCount = profCount - 1;
    val
  ];
  profLabels = {};
  profCount = 0;
  SetAttributes[profile, {HoldAll, SequenceHold}];
  profilePrintResult[label_String] := 
    Print[
      "#  ",
      label,
      "\t-- count: ",
      profInfo[label][[1]],
      ",\ttime: ",
      NumberForm[CForm[profInfo[label][[2]]], 5],
      "[sec](",
      NumberForm[CForm[profInfo[label][[2]] * 100 / profInfo["Total"][[2]]], 5],
      "%)"
  ];
  profilePrintResultAll[] := Block[{},
    profLabels = Sort[profLabels, (profInfo[#1][[2]]>profInfo[#2][[2]])&];
    Print["#*** Profile Result ***"];
    Map[profilePrintResult, profLabels]
  ],
  (* False *)
  profile[label_String, func_] := func;
  profilePrintResult[___] := Null;
  profilePrintResultAll[] := Null];

(*
 * parallelmap
 *)
If[optParallel, 
   (* True *)
   CloseKernels[];
   Print["LaunchKernels:", LaunchKernels[]];
   pMap[f_, a_] := ParallelMap[f,a];
   pMap[f_, a_, level_] := ParallelMap[f, a, level],

   (* False *)
   pMap[f_, a_] := Map[f,a];
   pMap[f_, a_, level_] := Map[f, a, level]
];

(*
 * proxy of simplify
 *)
simplify[expr_] := 
  Simplify[expr];

(*
 * funcの結果がTrueとなる要素と、Falseとなる要素を分ける
 * Splitと違い、同一要素は連続している必要はない
 *
 * 戻り値 : {Trueとなる要素, Falseとなる要素}
 *)
split2[list_, func_] :=
  Fold[(If[func[#2], 
          {Append[#1[[1]], #2], #1[[2]]}, 
          {#1[[1]], Append[#1[[2]], #2]}])&, 
        {{}, {}}, list];

(*
 * 各変数を1次微分したものを追加したリストを返す
 *)
addDifferentialVar[vars_] :=
  Fold[(Join[#1, {#2, Derivative[1][#2]}])&, {}, vars];

(*
 * 入力された変数を内部で処理する形式に変換する
 *)
createUsrVar[var_] := 
  ToExpression["usrVar" <> ToString[var]];

createUsrPrevVar[var_] := 
  prev[createUsrVar[var]];

createUsrIntegVar[var_] := 
  ToExpression["usrIntegVar" <> ToString[var]];

var2UsrVar[vars_] := Map[(createUsrVar[#])&, vars];

(*
 * 与えられた関数を原始関数になるまで積分を行い、
 * 経過をリストとして返す
 *)
createIntegFuncList[func_] := {};
createIntegFuncList[Derivative[n_Integer][func_]] := 
  NestList[(#')&, func, n-1];

(*
 * 原始関数を取り出す
 *)
removeDerivative[Derivative[_][func_][_]] := func;
removeDerivative[func_[_]] := func;

(* 
 * 変数を時間関数にする 
 *)
var2TimeFunc[vars_] := Map[(createUsrVar[#][t])&, vars];

(* 
 * 変数を積分済み変数にする 
 *)
var2IntegVar[vars_] := Map[createUsrIntegVar, vars];

(*
 * expr内のvarsの表現形式をfuncに従って変換する
 *)
transExpr[expr_, vars_, func_] := expr /. Map[func, vars];  

(*
 * 与えられた式内の変数を時間関数に変換する
 * 例： x => x[t], y' => y'[t]
 *)
exprVar2TimeFunc[expr_, vars_] := 
  expr /. Map[(# -> createUsrVar[#][t])&, vars];

(*
 * 与えられた式内の時間関数を変数に変換する
 * 例： varx[t] => x, vary'[t] => y'
 *)
exprTimeFunc2Var[expr_, vars_] := 
  transExpr[expr, vars, (createUsrVar[#][t] -> #)&];

(*
 * 与えられた式内の時間関数を積分済み関数に変換する
 * 例： varx[t] => integVarx, vary'[t] => integVary'
 *)
exprTimeFunc2IntegFunc[expr_, vars_] := 
  transExpr[expr, vars, (createUsrVar[#][t] -> createUsrIntegVar[#][t])&];


(*
 * フレーム公理を満たすための制約を作成
 *)
createFrameAxiomsCons[vars_] :=
  Map[(unit[always[tell[createUsrVar[#'][t]==0]]])&, vars] /. List -> group;
(*
 * {"term", elem, ...} を term[elem, ...] の形式に変換する
 *)
parseCons[{"group", elem__}]       := Map[parseCons, group[elem]];
parseCons[{"ask", guard_, elem__}] := Insert[Map[parseCons, ask[elem]], guard, 1];
parseCons[{"order", elem__}]       := Map[parseCons, order[elem]];
parseCons[{"unit", elem__}]        := Map[parseCons, unit[elem]];
parseCons[{"always", elem___}]     := Map[parseCons, always[elem]];
parseCons[{"tell", elem__}]        := tell[elem];
parseCons[other___]                := Throw[{error, "unknown term", other}];

(* 
 * 現在の制約ストアからask制約がエンテール出来るかどうか 
 *)
askTestQ[cons_, asks_, vars_] := (
  tmpSol = Reduce[Append[cons, asks], vars, Reals];
  If[tmpSol===False, Return[False]];
  If[Reduce[Append[cons, Not[tmpSol]], vars, Reals]===False, True, False]	
);

(*
 * unitをtell制約とask制約に分ける
 * alwaysは取り除く
 *
 * 戻り値：{tell制約, ask制約のリスト}
 *)
splitTellAsk[unit_] := (
  debugPrint["splitTellAsk#unit:", unit];
  Fold[splitTellAsk, {{}, {}}, unit]
);
splitTellAsk[{tells_, asks_}, tell[elem__]] := 
  {Join[tells, {elem}], asks};
splitTellAsk[{tells_, asks_}, ask[elem__]] := 
  {tells, Append[asks, ask[elem]]};
splitTellAsk[acc_, always[elem__]] :=
  Fold[splitTellAsk, acc, {elem}];

(*
 * ask制約の適用
 *)
applyAsk[asks_, consStore_, posAsk_, changedAsk_, vars_] :=
  Fold[applyAskFold, 
       {unit[], posAsk, {}, False}, 
       pMap[(applyAskMap[#, consStore, changedAsk, vars])&, asks]];

applyAskMap[ask[guard_, elem__], consStore_, changedAsk_, vars_] :=
  If[MemberQ[changedAsk, {positive, guard}]=!=True && 
       askTestQ[consStore, guard, vars]===True,
      (* True *)
      {unit[elem],
       {positive, guard},
       True},
      (* False *)
      {unit[ask[guard, elem]],
       {negative, guard},
       False}];

applyAskFold[{table_, posAsk_, negAsk_, askSuc_}, {newunit_, {type_, guard_}, suc_}] :=
  {Join[table, newunit],
   If[type===positive, Append[posAsk, guard], posAsk],
   If[type===negative, Append[negAsk, guard], negAsk],
   askSuc || suc};
   

(* applyAsk[asks_, consStore_, posAsk_, changedAsk_, vars_] := *)
(*   Fold[(applyAsk[#1, #2, consStore, changedAsk, vars])&, {unit[], posAsk, {}, False}, asks] *)

(* applyAsk[{newValidModTable_, posAsk_, negAsk_, askSuc_}, ask[guard_, elem__], consStore_, changedAsk_, vars_] :=    *)
(*   If[MemberQ[changedAsk, {positive, guard}]=!=True && askTestQ[consStore, guard, vars]===True, *)
(*       {Join[newValidModTable, unit[elem]], *)
(*        Append[posAsk, guard], *)
(*        negAsk, *)
(*        True}, *)
(*       {Append[newValidModTable, ask[guard, elem]], *)
(*        posAsk, *)
(*        Append[negAsk, guard], *)
(*        askSuc}] *)

chSolveUnit[validModTable_, constraintStore_, positiveAsk_, negativeAsk_, changedAsk_, vars_] := Block[{
  posAsk    = positiveAsk,
  negAsk    = negativeAsk,
  consStore = constraintStore,
  askSuc,
  tells,
  table    = validModTable,
  asks,
  solve
},
  solve[] := ( 
    {tells, asks} = splitTellAsk[table];
    debugPrint["chSolveUnit#tells:", tells];
    debugPrint["chSolveUnit#asks:", asks];

    If[tells=!={}, 
        consStore = profile["chSolveUnit#Reduce",
                            {Reduce[Join[consStore, tells], vars]}]];

    debugPrint["chSolveUnit#consStore:", consStore];
    If[consStore=!={False},
      (* true *)
      {table, posAsk, negAsk, askSuc} = 
        profile["applyASK", 
                applyAsk[asks, consStore, posAsk, changedAsk, vars]];
      debugPrint["chSolveUnit#table:", table];
      debugPrint["chSolveUnit#posAsk:", posAsk];
      debugPrint["chSolveUnit#negAsk:", negAsk];
      debugPrint["chSolveUnit#askSuc:", askSuc];
      If[askSuc===True, solve[], {table, consStore, posAsk, negAsk}],
      (* false *)
      False]
  );
  solve[]
];

chSolve[consTable_, consStore_, changedAsk_, vars_] := Block[{
  nacc, 
  nsuc,
  solve
},
  solve[group[], acc_, suc_] := {acc, suc};
  solve[group[head_, tail___], acc_, suc_] := (
    {nacc, nsuc} = solve[head, acc, False];
    If[nsuc=!=False,
        solve[group[tail], nacc, True],
        solve[group[tail], acc, suc]]
  );

  solve[order[], acc_, suc_] := {acc, suc};
  solve[order[head_, tail___], acc_, suc_] := (
    {nacc, nsuc} = solve[head, acc, False];
    If[nsuc=!=False,
        solve[order[tail], nacc, True],
        {acc, suc}]
  );

  solve[unit[elem__], {validModTable_, store_, posAsk_, negAsk_}, suc_] := (
    nacc = profile["chSOLVEUNIT",chSolveUnit[Join[validModTable, unit[elem]], store, posAsk, negAsk, changedAsk, vars]];
    If[nacc=!=False, 
        {nacc, True},
        {{validModTable, store, posAsk, negAsk}, False}]
  );

  {nacc, nsuc} =
    solve[consTable, {unit[], consStore, {}, {}}, False];
  nacc[[2;;4]]
];

(********)


exDSolve[expr_, vars_] := (
  debugPrint["--- exDSolve ---"];
  debugPrint["expr:", expr];
  debugPrint["vars:", vars];

  Quiet[Check[
        Check[DSolve[expr, vars, t],
                underconstraint,
                {DSolve::underdet, Solve::svars, DSolve::deqx, 
                 DSolve::bvnr, DSolve::bvsing}],
        overconstraint,
        {DSolve::overdet, DSolve::bvnul, DSolve::dsmsm}],
      {DSolve::underdet, DSolve::overdet, DSolve::deqx, 
       Solve::svars, DSolve::bvnr, DSolve::bvsing, 
       DSolve::bvnul, DSolve::dsmsm}] 
);

 (* , Solve::verif, Solve::tdep *)

(* exDSolve[expr_, vars_] := *)
(*   DSolve[expr, vars, t] *)

(*
 * 有効なaskを適用し、中身を取り出す
 * その際、alwaysでない中身は削除する 
 *)
applyAskInterval[asks_, posAsk_, changedAsk_] :=
  Fold[(applyAskInterval[#1, #2, posAsk, changedAsk])&, {unit[], False}, asks];

applyAskInterval[{table_, askSuc_}, ask[guard_, elem__], posAsk_, changedAsk_] :=
  If[MemberQ[posAsk, guard],
      If[MemberQ[changedAsk, {positive, guard}],
        {Join[table, removeNonAlwaysTuple[unit[elem]]], True},
        {Join[table, unit[elem]], True}],
      {table, askSuc}];

(* validVars[expr_] := *)
(*   Cases[cs, _[___, usrVarf'[0], ___], Infinity] *)

(*   validVars[expr_, vars_] := *)
(*     Select[vars, (MemberQ[expr, #, Infinity])&] *)


approxExpr[expr_] :=
(*   FromDigits[RealDigits[expr, 10, 3]] * If[expr < 0, -1, 1] *)
  Quiet[
    FromContinuedFraction[ContinuedFraction[expr, 3]],
    ContinuedFraction::incomp];


validVars[expr_] :=
  Union[Flatten[
    Cases[expr, symbol_Symbol[t] | symbol_Symbol'[t] -> {symbol[t], symbol'[t]}, Infinity]]];

initialVals[expr_, consStore_] := 
  Flatten[Map[(Cases[consStore, _[#1[0], Except[prev[_][0]]] | _[Except[prev[_][0]], #1[0]], Infinity])&,
                Union[Flatten[Cases[expr, symbol_Symbol'[t] -> symbol, Infinity]]]]];

(*   Flatten[Map[(Cases[consStore, _[#1[0], Except[prev[_][0]]] | _[Except[prev[_][0]], #1[0]], Infinity])&, *)
(*                 Union[Flatten[{Cases[expr, symbol_Symbol'[t] -> {symbol, symbol'}, Infinity], *)
(*                                Cases[expr, symbol_Symbol[t] -> symbol, Infinity]}]]]] *)



chSolveUnitInterval[validModTable_, consStore_, posAsk_, changedAsk_, vars_] := Block[{
  askSuc,
  sol,
  tells,
  asks, 
  solve
},

  solve[table_] := ( 
    debugPrint["--- chSolveUnitInterval#solve ---"];
    debugPrint["chSolveUnitInterval#table:", table];
    {tells, asks} = splitTellAsk[table];
    debugPrint["chSolveUnitInterval#tells:", tells];
    debugPrint["chSolveUnitInterval#asks:", asks];
    debugPrint["chSolveUnitInterval#consStore:", consStore];

    If[tells=!={},
        (* true *)
        tells = profile["chSolveUnitInterval#Reduce",
                        {Reduce[tells, vars]}];
        debugPrint["chSolveUnitInterval#after reduce tells:", tells];
        If[tells === {False},
            debugPrint["reduce error!!"];
            Return[False]];

        (* prevに関する制約削除 *)
        tells = DeleteCases[tells, _[prev[_][_], _] | _[_, prev[_][_]], Infinity];
        debugPrint["remove prev cons:", tells];

        If[tells=!={True},
          sol = profile["chSolveUnitInterval#exDSOLVE",
                  exDSolve[Join[tells, initialVals[tells, consStore]], validVars[tells]]];
          debugPrint["chSolveUnitInterval#sol:", sol];
          If[Length[$MessageList]>0, Throw[{error, "cannot solve ODEs", tells, $MessageList}]];
          If[sol===overconstraint, Return[False]]]];

(*        tmpNewVars = Union[Cases[tells, Derivative[n_Integer][sym_Symbol][t] -> Derivative[n][sym], Infinity]]; *)
(*        tmpDiffTest = Fold[(FreeQ[consStore, #2] && #1)&, True, *)
(*                   Map[(#[t])&, addDifferentialVar[tmpNewVars]]]; *)
(*        tmpIntegTest = Fold[(FreeQ[consStore, #2[t] == Except[_[t]] | Except[_[t]] == #2[t], Infinity] && #1)&, True, *)
(*                   Flatten[Map[createIntegFuncList, tmpNewVars]]]; *)
(*       debugPrint["tmpNewVars:", tmpNewVars]; *)
(*       debugPrint["tmpDiffTest:", tmpDiffTest]; *)
(*       debugPrint["tmpIntegTest:", tmpIntegTest]; *)
(*        If[tmpDiffTest && tmpIntegTest, ok, Return[False]]]; *)
         
   {asks, askSuc} = applyAskInterval[asks, posAsk, changedAsk];
   If[askSuc===True,
        If[tells=!={},
            solve[Append[asks, tell @@ tells]],
            solve[asks]],
        table]
  );
  solve[validModTable]
];

chSolveInterval[consTable_, consStore_, posAsk_, changedAsk_, vars_] := Block[{
  nacc, 
  nsuc,
  solve
},
  solve[group[], acc_, suc_] := {acc, suc};
  solve[group[head_, tail___], acc_, suc_] := (
    {nacc, nsuc} = solve[head, acc, False];
    If[nsuc=!=False,
        solve[group[tail], nacc, True],
        solve[group[tail], acc, suc]]
  );

  solve[order[], acc_, suc_] := {acc, suc};
  solve[order[head_, tail___], acc_, suc_] := (
    {nacc, nsuc} = solve[head, acc, False];
    If[nsuc=!=False,
        solve[order[tail], nacc, True],
        {acc, suc}]
  );

  solve[unit[elem__], validModTable_, suc_] := (
    nacc = profile["chSOLVEUNITINTERVAL",chSolveUnitInterval[Join[validModTable, unit[elem]], consStore, posAsk, changedAsk, vars]];
    debugPrint["chSolveInterval#solve$unit#nacc:", nacc];
    If[nacc=!=False, 
        {nacc, True},
        {validModTable, False}]
  );

  {nacc, nsuc} =
    solve[consTable, unit[], False];
  nacc
];

collectTell[{elem___}]              := Fold[collectTell, {}, unit[elem]];
collectTell[terms_, tell[elem_]]    := Append[terms, elem];
collectTell[terms_, ask[__]]        := terms;
collectTell[terms_, always[elem__]] := Fold[collectTell, terms, {elem}];


(* chSolveInterval[unit[elem__], consStore_, newConsTable_, suc_, vars_] := ( *)
(*   tmpSol = Fold[chSolveUnit, {}, {elem}]; *)

(*   tmpNewVars = Union[Cases[tmpSol, Derivative[n_Integer][sym_Symbol][t] -> Derivative[n][sym], Infinity]]; *)
(*   tmpDiffTest = Fold[(FreeQ[consStore, #2] && #1)&, True,  *)
(*                   Map[(#[t])&, addDifferentialVar[tmpNewVars]]]; *)
(*   tmpIntegTest = Fold[(FreeQ[consStore, #2[t] == Except[_[t]] | Except[_[t]] == #2[t], Infinity] && #1)&, True,  *)
(*                   Flatten[Map[createIntegFuncList, tmpNewVars]]]; *)

(*   debugPrint["-------"];      *)
(*   debugPrint["appendCons:", tmpSol];    *)
(*   debugPrint["consStore :", consStore];    *)
(*   debugPrint["tmpNewVars:", tmpNewVars];   *)
(*   debugPrint["tmpDiffTest:", tmpDiffTest];     *)
(*   debugPrint["tmpIntegTest:", tmpIntegTest]; *)
(*   If[tmpSol=!={} && tmpDiffTest && tmpIntegTest, *)
(*       tmpSol = {Reduce[Join[consStore, tmpSol], vars]}, *)
(*       tmpSol = consStore]; *)
(*   If[tmpSol=!={False}, {tmpSol, unit[elem], True}, {consStore, {}, suc}] *)
(* ) *)

(********)

(*
 * 有効なask中の制約がalwaysの場合はaskの外から出す
 *
 * 戻り値 : 展開済みの制約テーブル
 *)
expandAsks[group[elem___], posAsk_, vars_] := 
  Map[(expandAsks[#, posAsk, vars])&, group[elem]];

expandAsks[order[elem___], posAsk_, vars_] := 
  Map[(expandAsks[#, posAsk, vars])&, order[elem]];

expandAsks[unit[elem___], posAsk_, vars_] := 
  expandAsksUnit[{elem}, posAsk, vars, unit[]];

expandAsksUnit[{}, posAsk_, vars_, unitTable_] := unitTable;

expandAsksUnit[{tell[elem_], tail___}, posAsk_, vars_, unitTable_] :=
   expandAsksUnit[{tail}, posAsk, vars, Append[unitTable, tell[elem]]];

expandAsksUnit[{ask[guard_, elem__], tail___}, posAsk_, vars_, unitTable_] := (
  If[MemberQ[posAsk, guard], 
    (* guardが成り立ったとき *)
    {tmpAlwaysElem, tmpNonAlwaysElem} = 
      split2[expandAsksUnit[{elem}, posAsk, vars, {}], (Head[#]===always)&];
    expandAsksUnit[{tail}, posAsk, vars,     
                    Join[If[Length[tmpNonAlwaysElem]>0, (* alwaysでないものはaskの中へ *) 
                            Append[unitTable, Join[ask[guard], ask @@ tmpNonAlwaysElem]], 
                            unitTable],
                         always @@ tmpAlwaysElem]], (* alwaysはaskの外へ *)
    (* guardが成り立たなかったとき *)
    expandAsksUnit[{tail}, posAsk, vars, Append[unitTable, ask[guard, elem]]]
  ]
);

expandAsksUnit[{always[elem__], tail___}, posAsk_, vars_, unitTable_] := (
  tmpNewUnitTable= expandAsksUnit[{elem}, posAsk, vars, always[]];
  expandAsksUnit[{tail}, posAsk, vars, Append[unitTable, tmpNewUnitTable]]
);

(*
 * alwaysに囲まれていないタプルを削除する
 * いちばん外側のalwaysに囲まれているものは
 * 中身を外に出してalwaysタプルを消す
 * 空のgroup, order, unitは削除
 * 要素が1のgroup, orderは中身を親のタプルに移譲
 *)
removeNonAlwaysTuple[group[elem__]] := (
  Fold[(tmpSol=removeNonAlwaysTuple[#2];
          If[Length[tmpSol]>0, Append[#1, tmpSol], #1])&, group[], group[elem]]

(*   tmpSol = Fold[(tmpSol=removeNonAlwaysTuple[#2];  *)
(*               If[Length[tmpSol]>0, Append[#1, tmpSol], #1])&, group[], group[elem]]; *)
(*   If[Length[tmpSol]==1, group[tmpSol]=tmpSol; tmpSol, tmpSol] *)
);

removeNonAlwaysTuple[order[elem__]] := (
 Fold[(tmpSol=removeNonAlwaysTuple[#2];
              If[Length[tmpSol]>0, Append[#1, tmpSol], #1])&, order[], order[elem]]

(*   tmpSol = Fold[(tmpSol=removeNonAlwaysTuple[#2];  *)
(*               If[Length[tmpSol]>0, Append[#1, tmpSol], #1])&, order[], order[elem]]; *)
(*   If[Length[tmpSol]==1, order[tmpSol]=tmpSol; tmpSol, tmpSol] *)
);

removeNonAlwaysTuple[unit[elem__]]  :=
  Select[unit[elem], (Head[#]===always)&];


(*
 * 次のポイントフェーズに移行するTを求める
 *)
findNextPointPhaseTime[includeZero_, maxTime_,
                        posAsk_, negAsk_, changedAsk_] := Block[{
  removeDisableAsk,
  calcMinTime,
  sol,
  minT
},
  removeDisableAsk[type_, ask_] :=
    Select[ask, (!(includeZero===False && #[[1]]===False)
                  && Not[MemberQ[changedAsk, {type, #[[2]]}]])&];

  calcMinTime[{type_, integAsk_, ask_}] := (
    debugPrint["----- findNextPointPhaseTime -----"];
    debugPrint["type:", type];
    debugPrint["includeZero:", includeZero];
    debugPrint["changedAsk:", changedAsk];
    debugPrint["integAsk:", integAsk];
    debugPrint["ask:", ask];

    (* 未採用のask *)
    If[integAsk=!=False,
      (* true *)
      sol = Reduce[{If[includeZero===True, (t>=0), (t>0)] && (maxTime>=t) && (integAsk)}, t];
      debugPrint["sol:", sol];
      minT = If[sol =!= False, (* 解なしと境界値の解を区別するため *)
                 First[Quiet[Minimize[{t, If[includeZero===True, t>=0, t>0] && (sol)}, {t}], Minimize::wksol]],
                 error];
      (* 最大値が0でない場合、一瞬のみ成り立つaskではない  *)
(*       If[includeZero===True, *)
(*         tmpMaxT = First[Quiet[Maximize[{t, t>=0 && (sol)}, {t}], Maximize::wksol]]; *)
(*         debugPrint["tmpMaxT:", tmpMaxT]; *)
(*         If[tmpMaxT === 0, minT = error]]; *)
      debugPrint["Minimize#minT:", minT];
      If[Length[$MessageList]>0,
         Throw[{error, "cannot solve min time", minT, $MessageList}]],
        
      (* false *)
      minT=0];

    (* 0秒後のを含んではいけない *)
    If[includeZero===False && minT===0, minT=error];
    (* 0秒後の離散変化が行われるかのチェックなので0でなければエラー *)
    If[includeZero===True && minT=!=0, minT=error];

    debugPrint["minT:", minT];

    {minT, {type, ask}}
  );

  minimumTime[{currentMinT_, currentMinAsk_}, {time_, ask_}] :=
    Which[time === error,      {currentMinT, currentMinAsk},
          time <  currentMinT, {time,        {ask}},
          time == currentMinT, {time,        Append[currentMinAsk, ask]},
          True,                {currentMinT, currentMinAsk}];
    
  Fold[minimumTime,
        {maxTime, {}},
        pMap[calcMinTime,
              Join[Map[({positive, Not[#[[1]]], #[[2]]})&, removeDisableAsk[positive, posAsk]],
                   Map[({negative,     #[[1]],  #[[2]]})&, removeDisableAsk[negative, negAsk]]]]]
];

(* findNextPointPhaseTime[type_, includeZero_, {}, changedAsk_, maxTime_, currentMinT_, currntMinAsk_] := *)
(*   {currentMinT, currentMinAsk}; *)

(* findNextPointPhaseTime[type_, includeZero_, {{integAsk_, ask_}, tail___}, changedAsk_, *)
(*                           maxTime_, currentMinT_, currentMinAsk_] := ( *)
(*   debugPrint["----- findNextPointPhaseTime -----"]; *)
(*   debugPrint["type:", type]; *)
(*   debugPrint["includeZero:", includeZero]; *)
(*   debugPrint["changedAsk:", changedAsk]; *)
(*   debugPrint["integAsk:", integAsk]; *)
(*   debugPrint["ask:", ask]; *)

(*   If[MemberQ[changedAsk, {type, ask}]===False, *)
(*       (\* 未採用のask *\) *)
(*       If[integAsk=!=False, *)
(*         (\* true *\) *)
(*         tmpSol = Reduce[{If[includeZero===True, (t>=0), (maxTime>=t && t>0)] && *)
(*                          (If[type===negative, integAsk, Not[integAsk]])}, t]; *)
(*         debugPrint["tmpSol:", tmpSol]; *)
(*         tmpMinT = If[tmpSol =!= False, (\* 解なしと境界値の解を区別するため *\) *)
(*                    First[Quiet[Minimize[{t, If[includeZero===True, t>=0, t>0] && (tmpSol)}, {t}], Minimize::wksol]], *)
(*                    error]; *)
(*         debugPrint["Minimize#tmpMinT:", tmpMinT]; *)
(*         If[Length[$MessageList]>0, *)
(*            Throw[{error, "cannot solve min time", tmpMinT, $MessageList}]], *)
        
(*         (\* false *\) *)
(*         tmpMinT=0], *)
   
(*       (\* すでに採用済のask *\) *)
(*       tmpMinT = error *)
(*     ]; *)

(*   (\* 0秒後のを含んではいけない *\) *)
(*   If[includeZero===False && tmpMinT===0, tmpMinT=error]; *)
(*   (\* 0秒後の離散変化が行われるかのチェックなので0でなければエラー *\) *)
(*   If[includeZero===True && tmpMinT=!=0, tmpMinT=error]; *)

(*   debugPrint["tmpMinT:", tmpMinT]; *)
 
(*   {tmpNewMinT, tmpNewMinAsk} = *)
(*     Which[tmpMinT === error,      {currentMinT, currentMinAsk}, *)
(*           tmpMinT <  currentMinT, {tmpMinT,     {{type, ask}}}, *)
(*           tmpMinT == currentMinT, {tmpMinT,     Append[currentMinAsk, {type, ask}]}, *)
(*           True,                   {currentMinT, currentMinAsk}]; *)
(*   findNextPointPhaseTime[type, includeZero, {tail}, changedAsk, maxTime, tmpNewMinT, tmpNewMinAsk] *)
(* ); *)

(*
 * ポイントフェーズの処理 
 *)
pointPhase[consTable_, consStore_, changedAsk_, vars_] := (
  debugPrint["***** point phase *****"];
  debugPrint["pointPhase#consTable:", consTable];

  profile["chSolve",chSolve[consTable, consStore, changedAsk /. sym_[t] -> sym[0] , vars]]
);

(*
 * インターバルフェイズの処理
 *)
intervalPhase[consTable_, consStore_, askList_, posAsk_, negAsk_, changedAsk_, includeZero_, 
              ruleNow2IntegNow_, rulePrev2IntegNow_,
              vars_, ftvars_, varsND_, integVars_, prevVars_,
              maxTime_, currentTime_] := (

  debugPrint["***** interval phase *****"];
  debugPrint["prevVars:", prevVars];  
  debugPrint["consTable:", consTable];
  debugPrint["posAsk:", posAsk];
  debugPrint["negAsk:", negAsk];
  debugPrint["changedAsk:", changedAsk];

(*   (\* prevに関する制約削除 *\) *)
(*   tmpSol = Fold[(DeleteCases[#1, #2[_]==_ | _==#2[_], Infinity])&, consTable, prevVars]; *)
(*   debugPrint["remove prev cons:", tmpSol]; *)

  tmpSol = profile["chSOLVEINTERVAL",chSolveInterval[consTable, consStore, posAsk, changedAsk, vars]];
  debugPrint["chSolveResult:", tmpSol];
  tmpTells = profile["collectTELL",collectTell[List @@ tmpSol]];
  debugPrint["tmpTells:", tmpTells];

  tmpTells = {Reduce[tmpTells, vars]};

  (* prevに関する制約削除 *)
  tmpTells = DeleteCases[tmpTells, prev[_][_]==_ | _==prev[_][_], Infinity];
  debugPrint["remove prev cons:", tmpTells];

  (* ODE求解 *)
  debugPrint["--- DSolve ---"];
  profile["VALS",
    tmpValidVars = validVars[tmpTells];
    tmpTells = Join[tmpTells, initialVals[tmpTells, consStore]]
  ];

  (* 解の近似（精度保証ではなくなる） *)
(*   tmpTells = Join[tmpTells, *)
(*                     Map[(Head[#][#[[1]], approxExpr[#[[2]]]])&, *)
(*                           initialVals[tmpTells, consStore]]]; *)

  debugPrint["expr:", tmpTells];
  debugPrint["var:", varsND];

  profile["DSOLVE",
    tmpIntegSol = DSolve[tmpTells, tmpValidVars, t];
    If[Length[$MessageList]>0, Throw[{error, "cannot solve ODEs", tmpIntegSol, $MessageList}]];
    tmpIntegSol = First[tmpIntegSol]
  ];

  debugPrint["askList:", askList];
  debugPrint["tmpIntegSol:", tmpIntegSol];
  debugPrint["rulePrev2IntegNow:", rulePrev2IntegNow];
  debugPrint["ruleNow2IntegNow:", ruleNow2IntegNow];

  (* 積分済み変数の割り当て *)
  debugPrint["integVars:", integVars];
  debugPrint["varsND:", varsND];
  profile["MapTHREAD1",
    MapThread[(#1[t_] = simplify[(#2 /. tmpIntegSol)])&, {integVars, varsND}]
  ];

  profile["tmpASKS",
    tmpAsk = Map[({# /. Join[rulePrev2IntegNow, ruleNow2IntegNow], #})&, askList];
    tmpPosAsk = Map[({# /. Join[rulePrev2IntegNow, ruleNow2IntegNow], #})&, posAsk];
    tmpNegAsk = Map[({# /. Join[rulePrev2IntegNow, ruleNow2IntegNow], #})&, negAsk]
  ];
(*   tmpAsk = Map[({simplify[# /. Join[rulePrev2IntegNow, ruleNow2IntegNow]], #})&, askList]; *)
(*   tmpPosAsk = Map[({simplify[# /. Join[rulePrev2IntegNow, ruleNow2IntegNow]], #})&, posAsk]; *)
(*   tmpNegAsk = Map[({simplify[# /. Join[rulePrev2IntegNow, ruleNow2IntegNow]], #})&, negAsk]; *)
  debugPrint["tmpAsk:", tmpAsk];  
  debugPrint["tmpPosAsk:", tmpPosAsk];  
  debugPrint["tmpNegAsk:", tmpNegAsk];  

  tmpChangedAsk = Join[changedAsk, Map[({negative, #})&, posAsk], Map[({positive, #})&, negAsk]];
  debugPrint["tmpChangedAsk:", tmpChangedAsk];  
  
(*   tmpChangedAsk = If[includeZero, tmpChangedAsk, {}]; *)

  (* 次のPointPhaseまでの時間を求める *)
  profile["findNEXTPOINTPHASETIME",

          {tmpMinT, tmpMinAsk} =
            findNextPointPhaseTime[includeZero, maxTime,
                                    tmpPosAsk, tmpNegAsk,
                                    If[includeZero, tmpChangedAsk, {}]]
  ];

(*           {tmpMinT, tmpMinAsk} = findNextPointPhaseTime[ *)
(*                                   positive, includeZero, *)
(*                                   tmpPosAsk, If[includeZero, tmpChangedAsk, {}], *)
(*                                   maxTime, maxTime, {}]; *)
(*           {tmpMinT, tmpMinAsk} = findNextPointPhaseTime[ *)
(*                                   negative, includeZero, *)
(*                                   tmpNegAsk, If[includeZero, tmpChangedAsk, {}], *)
(*                                   maxTime, tmpMinT, tmpMinAsk]]; *)

  debugPrint["--- findNextPointPhaseTimeResult ---"];
  debugPrint["tmpMinT:", tmpMinT];
  debugPrint["tmpMinAsk:", tmpMinAsk];
  debugPrint["includeZero:", includeZero];

  profile["MapTHREAD2",
  If[tmpMinAsk==={} && includeZero == True, 
      tmpNewChangedAsk = tmpChangedAsk;
      tmpNewIncludeZero = False;
      tmpMinT = 0;
      tmpConsStore = MapThread[(#1 == simplify[(#2 /. t->tmpMinT)])&, 
                                    {ftvars, Flatten[Map[({#[t], #'[t]})&, integVars]]}] /. sym_[t] -> sym[0],

      If[tmpMinT===0,
          tmpNewChangedAsk = Join[tmpChangedAsk, tmpMinAsk];
          tmpNewIncludeZero = includeZero;
          tmpConsStore = MapThread[(#1 == simplify[(#2 /. t->tmpMinT)])&, 
                                    {ftvars, Flatten[Map[({#[t], #'[t]})&, integVars]]}] /. sym_[t] -> sym[0],
          
          tmpNewChangedAsk = {};
          tmpNewIncludeZero = True;
          tmpConsStore = {}]]];

  debugPrint["tmpConsStore:", tmpConsStore];

  tmpPrevConsTable = profile["MapTHREAD4",
    group @@ MapThread[(unit[tell[#1[t] == simplify[(#2 /. t->tmpMinT)]]])&,
                            {prevVars, Flatten[Map[({#[t], #'[t]})&, integVars]]}]];

  (* 変数の値の出力
   *  
   * optOutputFormat変数がfmtTFunctionの場合にはtの関数による表示
   * fmtNumericの場合には今まで通りの表示
   *)
  debugPrint["begin answer"];
  profile["PRINT",
   Switch[optOutputFormat, 
    fmtTFunction,
     If[tmpMinT>0,
        Print[CForm[tmpPrevTime], " <= t <=",
              CForm[tmpMinT+currentTime], "\t", 
              Fold[(#1<>ToString[CForm[simplify[#2[t] ]]]<>"\t")&, "",integVars]]];
     tmpPrevTime = tmpMinT+currentTime, 
    fmtNumeric,
     For[i=0, i<tmpMinT, i+=1/10,
       Print[N[i+currentTime, 5], "\t",
             Fold[(#1<>ToString[N[simplify[#2[t] /. t->i], 5]]<>"\t")&, "", integVars]]];
     Print[N[tmpMinT+currentTime, 5], "\t" <>
           Fold[(#1<>ToString[N[simplify[#2[t] /. t->tmpMinT], 5]]<>"\t")&, "", integVars]];
   ]];
  debugPrint["end answer"];

  (* 積分済み変数の割り当て解除 *)
  Scan[(Clear[#])&, integVars];

  {tmpMinT, tmpNewIncludeZero, tmpNewChangedAsk, tmpPrevConsTable, tmpConsStore}
);

consStore2Table[consStore_, vars_] :=
  group @@ 
   DeleteCases[Map[(unit[tell[# == First[# /. Solve[consStore, #]]]])&, vars], unit[tell[True]]];

(*
 * 
 *)
HydLaSolve[cons_, argVars_, maxTime_] := Module[{
  sol,
  vars, 
  ftVars,
  ftVarsND,
  prevVars,
  consFrameAxioms,
  consTable,
  consStore = {},
  consStorePrev = {},
  currentTime = 0,
  includeZero = True,
  rval = {},
  pftVars,
  changedAsk = {}
},
  vars     = addDifferentialVar[argVars];
  consFrameAxioms = createFrameAxiomsCons[argVars];
  ftVars   = var2TimeFunc[vars];
  ftVarsND = var2TimeFunc[argVars];
  prevVars = Map[prev, Map[createUsrVar, vars]];
  pftVars  = Join[ftVars, Map[(#[t])&, prevVars]];

  (* 各変数の値をtの関数として表示用の変数初期化 *)
  tmpPrevTime = 0;

  gOutPut = {};
(*   Print[""]; *)
(*   Print["----begin----"]; *)
  Print["# time", Fold[(#1<> "\t" <> ToString[#2])&, "", argVars]];

  consPrevEqNow = 
    group @@ Map[(unit[tell[createUsrPrevVar[#][0] == createUsrVar[#][0]]])&, vars];

  rulePrev2Now      = Map[(createUsrPrevVar[#][t] -> createUsrVar[#][t])&, vars];
  rulePrev2IntegNow = Map[(createUsrPrevVar[#][t] -> createUsrIntegVar[#][t])&, vars];
  ruleNow2Prev      = Map[(createUsrVar[#][t]     -> createUsrPrevVar[#][t])&, vars];
  ruleNow2IntegNow  = Map[(createUsrVar[#][t]     -> createUsrIntegVar[#][t])&, vars];

(*   consTable = parseCons[cons /. Flatten[Map[({# -> createUsrVar[#][t], prev[#] -> prev[createUsrVar[#]][t]})&, vars]]]; *)
  consTable = cons /. Flatten[Map[({# -> createUsrVar[#][t], prev[#] -> prev[createUsrVar[#]][t]})&, vars]];

  debugPrint["--- Before MainLoop ---"];
  debugPrint["consTable:", consTable];
  debugPrint["consPrevEqNow:", consPrevEqNow];
  debugPrint["consFrameAxioms:", consFrameAxioms];
  debugPrint["rulePrev2IntegNow:", rulePrev2IntegNow];
  debugPrint["ruleNow2IntegNow:", ruleNow2IntegNow];

  While[True,

    tmpConsTable = 
      If[consStorePrev==={},
          order[consTable, consPrevEqNow] /. name_[t] -> name[0],
          order[consStorePrev, consTable, consPrevEqNow] /. name_[t] -> name[0]];

    {{consStore}, tmpPosAsk, tmpNegAsk} =
      profile["point-PHASE",
      pointPhase[tmpConsTable, consStore, changedAsk, pftVars]];


(*     Print[If[globalUseDebugPrint, "R:", ""], N[currentTime, 5], *)
(*             "\t" <> Fold[(#1<>ToString[N[simplify[First[#2 /. Solve[consStore, #2]]], 5]]<>"\t")&, *)
(*          "", *)
(*          ftVarsND /. name_[t] -> name[0]]]; *)

    debugPrint["--- After PointPhase ---"];
    debugPrint["consTable:", consTable];

    debugPrint["ret pointPhase:consStore:", consStore];

    If[currentTime >= maxTime,
      Return[gOutPut]];


    debugPrint["expandAsks:consTable:", consTable];
    consTable = profile["expandASKS",expandAsks[consTable, tmpPosAsk, pftVars]];
    consTable = profile["removeNONALWAYSTUPLE",removeNonAlwaysTuple[consTable]];
 
    {consTable, tmpPosAsk, tmpNegAsk} = {consTable, tmpPosAsk, tmpNegAsk} /. name_[0] -> name[t];
    debugPrint["tmpPosAsk:", tmpPosAsk];
    debugPrint["tmpNegAsk:", tmpNegAsk];
    debugPrint["changedAsk:", changedAsk];


    debugPrint["--- Before IntervalPhase ---"];
    debugPrint["consStore:", consStore];
    debugPrint["consTable:", consTable];

(*     tmpConsStoreTable = consStore2Table[consStore, pftVars /. name_[t] -> name[0]]; *)
(*     debugPrint["tmpConsStoreTable:", tmpConsStoreTable]; *)

    {tmpT, includeZero, changedAsk, tmpPrevConsTable, consStore} =
      profile["INTERVAL-PHASE",
      intervalPhase[
                    order[order[consTable, consFrameAxioms], consPrevEqNow],
(*                     order[order[order[consTable, consFrameAxioms], tmpConsStoreTable], consPrevEqNow], *)
                    consStore,
                    Join[tmpNegAsk, tmpPosAsk],
                    tmpPosAsk,
                    tmpNegAsk,
                    changedAsk,
                    includeZero,
                    ruleNow2IntegNow, rulePrev2IntegNow,
                    pftVars, ftVars, ftVarsND, var2IntegVar[argVars], prevVars, 
                    maxTime-currentTime, currentTime]];

    debugPrint["--- After IntervalPhase ---"];  
    debugPrint["changedAsk:", changedAsk];
    debugPrint["consStore:", consStore];

    currentTime = simplify[currentTime + tmpT];
(*     currentTime = FromDigits[RealDigits[currentTime + tmpT, 10, 10]]; *)

    If[tmpT=!=0 || consStorePrev === {}, consStorePrev = tmpPrevConsTable];
    debugPrint["currentTime:", currentTime];
    debugPrint["consStorePrev:", consStorePrev];
  ];
];

