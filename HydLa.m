(* BeginPackage["HydLa`"] *)

(* HydLaSolve::usage = "HydLa" *)
(* PlotSamples::usage = "Plot samples" *)

(* Begin["`Private`"] *)

(*
 * デバック用メッセージを出力するかどうか
 *)
globalUseDebugPrint = False

(*
 * デバック用メッセージ出力関数
 *)
debugPrint[arg___] := 
  If[globalUseDebugPrint, Print[arg]]

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
        {{}, {}}, list]

(*
 * 各変数を1次微分したものを追加したリストを返す
 *)
addDifferentialVar[vars_] :=
  Fold[(Join[#1, {#2, Derivative[1][#2]}])&, {}, vars];

(*
 * 入力された変数を内部で処理する形式に変換する
 *)
createUsrVar[var_] := 
  ToExpression["usrVar" <> ToString[var]]

createUsrPrevVar[var_] := 
  prev[createUsrVar[var]]

createUsrIntegVar[var_] := 
  ToExpression["usrIntegVar" <> ToString[var]]

var2UsrVar[vars_] := Map[(createUsrVar[#])&, vars]

(*
 * 与えられた関数を原始関数になるまで積分を行い、
 * 経過をリストとして返す
 *)
createIntegFuncList[func_] := {}
createIntegFuncList[Derivative[n_Integer][func_]] := 
  NestList[(#')&, func, n-1]

(*
 * 原始関数を取り出す
 *)
removeDerivative[Derivative[_][func_][_]] := func
removeDerivative[func_[_]] := func

(* 
 * 変数を時間関数にする 
 *)
var2TimeFunc[vars_] := Map[(createUsrVar[#][t])&, vars]

(* 
 * 変数を積分済み変数にする 
 *)
var2IntegVar[vars_] := Map[createUsrIntegVar, vars]

(*
 * expr内のvarsの表現形式をfuncに従って変換する
 *)
transExpr[expr_, vars_, func_] := expr /. Map[func, vars]  

(*
 * 与えられた式内の変数を時間関数に変換する
 * 例： x => x[t], y' => y'[t]
 *)
exprVar2TimeFunc[expr_, vars_] := 
  expr /. Map[(# -> createUsrVar[#][t])&, vars]

(*
 * 与えられた式内の時間関数を変数に変換する
 * 例： varx[t] => x, vary'[t] => y'
 *)
exprTimeFunc2Var[expr_, vars_] := 
  transExpr[expr, vars, (createUsrVar[#][t] -> #)&]

(*
 * 与えられた式内の時間関数を積分済み関数に変換する
 * 例： varx[t] => integVarx, vary'[t] => integVary'
 *)
exprTimeFunc2IntegFunc[expr_, vars_] := 
  transExpr[expr, vars, (createUsrVar[#][t] -> createUsrIntegVar[#][t])&]


(*
 * フレーム公理を満たすための制約を作成
 *)
createFrameAxiomsCons[vars_] :=
  Map[(unit[tell[createUsrVar[#'][t]==0]])&, vars] /. List -> group
(*
 * {"term", elem, ...} を term[elem, ...] の形式に変換する
 *)
parseCons[{"group", elem__}]       := Map[parseCons, group[elem]]
parseCons[{"ask", guard_, elem__}] := Insert[Map[parseCons, ask[elem]], guard, 1]
parseCons[{"order", elem__}]       := Map[parseCons, order[elem]]
parseCons[{"unit", elem__}]        := Map[parseCons, unit[elem]]
parseCons[{"always", elem___}]     := Map[parseCons, always[elem]]
parseCons[{"tell", elem__}]        := tell[elem]
parseCons[other___]                := Throw[{error, "unknown term", other}]

(* 
 * 現在の制約ストアからask制約がエンテール出来るかどうか 
 *)
askTestQ[cons_, asks_, vars_] := (
  tmpSol = Reduce[Append[cons, asks], vars, Reals];
  If[tmpSol===False, Return[False]];
  If[Reduce[Append[cons, Not[tmpSol]], vars, Reals]===False, True, False]	
)

(*
 * unitをtell制約とask制約に分ける
 * alwaysは取り除く
 *
 * 戻り値：{tell制約, ask制約のリスト}
 *)
splitTellAsk[unit_] := 
  Fold[splitTellAsk, {{}, {}}, unit]
splitTellAsk[{tells_, asks_}, tell[elem__]] := 
  {Join[tells, {elem}], asks}
splitTellAsk[{tells_, asks_}, ask[elem__]] := 
  {tells, Append[asks, ask[elem]]}
splitTellAsk[acc_, always[elem__]] :=
  Fold[splitTellAsk, acc, {elem}]

(*
 * ask制約の適用
 *)
applyAsk[asks_, consStore_, posAsk_, vars_] :=
  Fold[(applyAsk[#1, #2, consStore, vars])&, {unit[], posAsk, {}, False}, asks]

applyAsk[{newValidModTable_, posAsk_, negAsk_, askSuc_}, ask[guard_, elem__], consStore_, vars_] :=   
  If[askTestQ[consStore, guard, vars]===True,
      {Join[newValidModTable, unit[elem]],
       Append[posAsk, guard],
       negAsk,
       True},
      {Append[newValidModTable, ask[guard, elem]],
       posAsk,
       Append[negAsk, guard],
       askSuc}]

chSolveUnit[validModTable_, constraintStore_, positiveAsk_, negativeAsk_, vars_] := Block[{
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

    If[tells=!={}, consStore = {Reduce[Join[consStore, tells], vars]}];
    debugPrint["chSolveUnit#consStore:", consStore];
    If[consStore=!={False},
      (* true *)
      {table, posAsk, negAsk, askSuc} = 
        applyAsk[asks, consStore, posAsk, vars];
      debugPrint["chSolveUnit#table:", table];
      debugPrint["chSolveUnit#posAsk:", posAsk];
      debugPrint["chSolveUnit#negAsk:", negAsk];
      debugPrint["chSolveUnit#askSuc:", askSuc];
      If[askSuc===True, solve[], {table, consStore, posAsk, negAsk}],
      (* false *)
      False]
  );
  solve[]
]

chSolve[consTable_, consStore_, vars_] := Block[{
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
    nacc = chSolveUnit[Join[validModTable, unit[elem]], store, posAsk, negAsk, vars];
    If[nacc=!=False, 
        {nacc, True},
        {{validModTable, store, posAsk, negAsk}, False}]
  );

  {nacc, nsuc} =
    solve[consTable, {unit[], consStore, {}, {}}, False];
  nacc[[2;;4]]
]

(********)

validVars[expr_, vars_] := With[{
  expr2  = expr /. name_[0] -> name[t]
},
  Select[vars, (MemberQ[expr2, #, Infinity])&]
]

exDSolve[expr_, vars_] := (
  debugPrint["--- exDSolve ---"];
  debugPrint["expr:", expr];
  debugPrint["vars:", vars];

  Quiet[Check[
        Check[DSolve[expr, vars, t],
                underconstraint,
                {DSolve::underdet, Solve::svars, DSolve::deqx, 
                 DSolve::bvnr, DSolve::bvsing, DSolve::dsmsm}],
        overconstraint,
        {DSolve::overdet, DSolve::bvnul}],
      {DSolve::underdet, DSolve::overdet, DSolve::deqx, 
       Solve::svars, DSolve::bvnr, DSolve::bvsing, 
       DSolve::dsmsm, DSolve::bvnul}] 
)

(* exDSolve[expr_, vars_] := *)
(*   DSolve[expr, vars, t] *)

(*
 * 有効なaskを適用し、中身を取り出す
 * その際、alwaysでない中身は削除する 
 *)
applyAskInterval[asks_, posAsk_] :=
  Fold[(applyAskInterval[#1, #2, posAsk])&, {unit[], False}, asks]

applyAskInterval[{table_, askSuc_}, ask[guard_, elem__], posAsk_] :=
  If[MemberQ[posAsk, guard],
      {Join[table, removeNonAlwaysTuple[unit[elem]]], True},
(*       {Join[table, unit[elem]], True}, *)
      {table, askSuc}]

chSolveUnitInterval[validModTable_, posAsk_, vars_] := Block[{
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

    If[tells=!={},
        (* true *)

        tells = {Reduce[tells, validVars[tells, vars]]};
        debugPrint["chSolveUnitInterval#after reduce tells:", tells];
        If[tells === {False},
            debugPrint["reduce error!!"]; 
            Return[False]];

        (* prevに関する制約削除 *)
        tells = DeleteCases[tells, prev[_][_]==_ | _==prev[_][_], Infinity];
        debugPrint["remove prev cons:", tells];

        sol = exDSolve[tells, validVars[tells, vars]];
        debugPrint["chSolveUnitInterval#sol:", sol];
        If[Length[$MessageList]>0, Throw[{error, "cannot solve ODEs", tells, $MessageList}]];
        If[sol===overconstraint, Return[False]]];

(*        tmpNewVars = Union[Cases[tells, Derivative[n_Integer][sym_Symbol][t] -> Derivative[n][sym], Infinity]]; *)
(*        tmpDiffTest = Fold[(FreeQ[consStore, #2] && #1)&, True, *)
(*                   Map[(#[t])&, addDifferentialVar[tmpNewVars]]]; *)
(*        tmpIntegTest = Fold[(FreeQ[consStore, #2[t] == Except[_[t]] | Except[_[t]] == #2[t], Infinity] && #1)&, True, *)
(*                   Flatten[Map[createIntegFuncList, tmpNewVars]]]; *)
(*       debugPrint["tmpNewVars:", tmpNewVars]; *)
(*       debugPrint["tmpDiffTest:", tmpDiffTest]; *)
(*       debugPrint["tmpIntegTest:", tmpIntegTest]; *)
(*        If[tmpDiffTest && tmpIntegTest, ok, Return[False]]]; *)
         
   {asks, askSuc} = applyAskInterval[asks, posAsk];
   If[askSuc===True,
        If[tells=!={},
            solve[Append[asks, tell @@ tells]],
            solve[asks]],
        table]
  );
  solve[validModTable]
]

chSolveInterval[consTable_, posAsk_, vars_] := Block[{
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
    nacc = chSolveUnitInterval[Join[validModTable, unit[elem]], posAsk, vars];
    debugPrint["chSolveInterval#solve$unit#nacc:", nacc];
    If[nacc=!=False, 
        {nacc, True},
        {validModTable, False}]
  );

  {nacc, nsuc} =
    solve[consTable, unit[], False];
  nacc
]

collectTell[{elem___}]              := Fold[collectTell, {}, unit[elem]]
collectTell[terms_, tell[elem_]]    := Append[terms, elem]
collectTell[terms_, ask[__]]        := terms
collectTell[terms_, always[elem__]] := Fold[collectTell, terms, {elem}]


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
  Map[(expandAsks[#, posAsk, vars])&, group[elem]]

expandAsks[order[elem___], posAsk_, vars_] := 
  Map[(expandAsks[#, posAsk, vars])&, order[elem]]

expandAsks[unit[elem___], posAsk_, vars_] := 
  expandAsksUnit[{elem}, posAsk, vars, unit[]]

expandAsksUnit[{}, posAsk_, vars_, unitTable_] := unitTable

expandAsksUnit[{tell[elem_], tail___}, posAsk_, vars_, unitTable_] :=
   expandAsksUnit[{tail}, posAsk, vars, Append[unitTable, tell[elem]]]

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
)

expandAsksUnit[{always[elem__], tail___}, posAsk_, vars_, unitTable_] := (
  tmpNewUnitTable= expandAsksUnit[{elem}, posAsk, vars, always[]];
  expandAsksUnit[{tail}, posAsk, vars, Append[unitTable, tmpNewUnitTable]]
)

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
)

removeNonAlwaysTuple[order[elem__]] := (
 Fold[(tmpSol=removeNonAlwaysTuple[#2];
              If[Length[tmpSol]>0, Append[#1, tmpSol], #1])&, order[], order[elem]]

(*   tmpSol = Fold[(tmpSol=removeNonAlwaysTuple[#2];  *)
(*               If[Length[tmpSol]>0, Append[#1, tmpSol], #1])&, order[], order[elem]]; *)
(*   If[Length[tmpSol]==1, order[tmpSol]=tmpSol; tmpSol, tmpSol] *)
)

removeNonAlwaysTuple[unit[elem__]]  :=
  Select[unit[elem], (Head[#]===always)&]  


(*
 * 次のポイントフェーズに移行するTを求める
 *)
findNextPointPhaseTime[type_, includeZero_, {}, changedAsk_, maxTime_, currentMinT_, currentMinAsk_] := 
  {currentMinT, currentMinAsk}

findNextPointPhaseTime[type_, includeZero_, {{integAsk_, ask_}, tail___}, changedAsk_, 
                          maxTime_, currentMinT_, currentMinAsk_] := (
  debugPrint["----- findNextPointPhaseTime -----"];
  debugPrint["type:", type];
  debugPrint["includeZero:", includeZero];
  debugPrint["changedAsk:", changedAsk];
  debugPrint["integAsk:", integAsk];
  debugPrint["ask:", ask];

  If[MemberQ[changedAsk, {type, ask}]===False,
    (* 未採用のask *)
     tmpSol = Reduce[{If[includeZero===True, t>=0, t>0] && 
                       (maxTime==t || If[type===negative, integAsk, Not[integAsk]])}, t];
     debugPrint["tmpSol:", tmpSol];
     tmpMinT = If[tmpSol =!= False, (* 解なしと境界値の解を区別するため *)
                 First[Quiet[Minimize[{t, tmpSol}, {t}], Minimize::wksol]],
                 error];
     If[Length[$MessageList]>0, 
         Throw[{error, "cannot solve min time", tmpMinT, $MessageList}]],
   
    (* すでに採用済のask *)
      tmpMinT = error
    ];

  debugPrint["tmpMinT:", tmpMinT];
 
  {tmpNewMinT, tmpNewMinAsk} = 
    Which[tmpMinT === error,      {currentMinT, currentMinAsk},
          tmpMinT <  currentMinT, {tmpMinT,     {{type, ask}}},
          tmpMinT == currentMinT, {tmpMinT,     Append[currentMinAsk, {type, ask}]},
          True,                   {currentMinT, currentMinAsk}];
  findNextPointPhaseTime[type, includeZero, {tail}, changedAsk, maxTime, tmpNewMinT, tmpNewMinAsk]
)

(*
 * ポイントフェーズの処理 
 *)
pointPhase[consTable_, consStore_, vars_] := (
  debugPrint["***** point phase *****"];
  chSolve[consTable, consStore, vars]
) 

(*
 * インターバルフェイズの処理
 *)
intervalPhase[consTable_, askList_, posAsk_, changedAsk_, includeZero_, 
              ruleNow2IntegNow_, rulePrev2IntegNow_,
              vars_, ftvars_, varsND_, integVars_, prevVars_,
              maxTime_, currentTime_] := (

  debugPrint["***** interval phase *****"];
  debugPrint["prevVars:", prevVars];  
  debugPrint["consTable:", consTable];
  debugPrint["posAsk:", posAsk];

(*   (\* prevに関する制約削除 *\) *)
(*   tmpSol = Fold[(DeleteCases[#1, #2[_]==_ | _==#2[_], Infinity])&, consTable, prevVars]; *)
(*   debugPrint["remove prev cons:", tmpSol]; *)

  tmpSol = chSolveInterval[consTable, posAsk, vars];
  debugPrint["chSolveResult:", tmpSol];
  tmpTells = collectTell[List @@ tmpSol];
  debugPrint["tmpTells:", tmpTells];

  tmpTells = {Reduce[tmpTells, vars]};

  (* prevに関する制約削除 *)
  tmpTells = DeleteCases[tmpTells, prev[_][_]==_ | _==prev[_][_], Infinity];
  debugPrint["remove prev cons:", tmpTells];

  (* ODE求解 *)
  debugPrint["--- DSolve ---"];
  debugPrint["expr:", tmpTells];
  debugPrint["var:", varsND];
  tmpIntegSol = DSolve[tmpTells, varsND, t];
  If[Length[$MessageList]>0, Throw[{error, "cannot solve ODEs", tmpIntegSol, $MessageList}]];
  tmpIntegSol = First[tmpIntegSol];

  debugPrint["askList:", askList];
  debugPrint["tmpIntegSol:", tmpIntegSol];
  debugPrint["rulePrev2IntegNow:", rulePrev2IntegNow];

  (* 積分済み変数の割り当て *)
  debugPrint["integVars:", integVars];
  debugPrint["varsND:", varsND];
  MapThread[(#1[t_] = (#2 /. tmpIntegSol))&, {integVars, varsND}];

  tmpAsk = Map[({# /. rulePrev2IntegNow, #})&, askList];
  debugPrint["tmpAsk:", tmpAsk];  
  
  (* 次のPointPhaseまでの時間を求める *)
  {tmpMinT, tmpMinAsk} = findNextPointPhaseTime[
                          positive, includeZero,
                          tmpAsk, changedAsk, 
                          maxTime, maxTime, {}];

  {tmpMinT, tmpMinAsk} = findNextPointPhaseTime[
                          negative, includeZero,
                          tmpAsk, changedAsk, 
                          maxTime, tmpMinT, tmpMinAsk];

  If[tmpMinAsk==={} && includeZero, 
      tmpNewChangedAsk = {};
      tmpNewIncludeZero = False;
      tmpMinT = 0,       

      If[tmpMinT===0,
          tmpNewChangedAsk = Join[changedAsk, tmpMinAsk];
          tmpNewIncludeZero = includeZero,
          
          tmpNewChangedAsk = {};
          tmpNewIncludeZero = True]];


  debugPrint["--- findNextPointPhaseTimeResult ---"];
  debugPrint["tmpMinT:", tmpMinT];
  debugPrint["tmpMinAsk:", tmpMinAsk];
 
  tmpPrevConsTable = group @@ MapThread[(unit[tell[#1[t] == (#2 /. t->tmpMinT)]])&, 
                                   {prevVars, Flatten[Map[({#[t], #'[t]})&, integVars]]}];

  (* 変数の値の出力 *)
  For[i=0, i<tmpMinT, i+=1/10,
    Print[N[i+currentTime, 5], "\t" <> Fold[(#1<>ToString[N[#2[t] /. t->i, 5]]<>"\t")&, "", integVars]];
    AppendTo[gOutPut, Fold[(Append[#1, N[#2[t] /. t->i, 5]])&, {N[i+currentTime, 5]}, integVars]];
  ];
  Print[N[tmpMinT+currentTime, 5], "\t" <> Fold[(#1<>ToString[N[#2[t] /. t->tmpMinT, 5]]<>"\t")&, "", integVars]];
  AppendTo[gOutPut, Fold[(Append[#1, N[#2[t] /. t->tmpMinT, 5]])&, {N[i+currentTime, 5]}, integVars]];


  (* 積分済み変数の割り当て解除 *)
  Scan[(Clear[#])&, integVars];

  {tmpMinT, tmpNewIncludeZero, tmpNewChangedAsk, tmpPrevConsTable}
)

consStore2Table[consStore_, vars_] :=
  group @@ 
   DeleteCases[Map[(unit[tell[# == First[# /. Solve[consStore, #]]]])&, vars], unit[tell[True]]]

(*
 * 
 *)
HydLaSolve[cons_, argVars_, maxTime_, debug_] := Module[{
  sol,
  vars, 
  ftVars,
  ftVarsND,
  prevVars,
  consFrameAxioms,
  consTable,
  consStore,
  consStorePrev = {},
  currentTime = 0,
  includeZero = True,
  rval = {},
  pftVars,
  changedAsk = {}
},
  globalUseDebugPrint = debug;

  vars     = addDifferentialVar[argVars];
  consFrameAxioms = createFrameAxiomsCons[argVars];
  ftVars   = var2TimeFunc[vars];
  ftVarsND = var2TimeFunc[argVars];
  prevVars = Map[prev, Map[createUsrVar, vars]];
  pftVars  = Join[ftVars, Map[(#[t])&, prevVars]];

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


  While[True,
    consTable = consTable /. name_[t] -> name[0];

    debugPrint["--- Before PointPhase ---"];
    debugPrint["consTable:", consTable];

    {{consStore}, tmpPosAsk, tmpNegAsk} = pointPhase[consTable, {}, pftVars];

    Print[If[globalUseDebugPrint, "R:", ""], N[currentTime, 5],
            "\t" <> Fold[(#1<>ToString[N[First[#2 /. Solve[consStore, #2]], 5]]<>"\t")&,
         "",
         ftVarsND /. name_[t] -> name[0]]];

    debugPrint["--- After PointPhase ---"];
    debugPrint["consTable:", consTable];

    debugPrint["ret pointPhase:consStore:", consStore];

    If[currentTime >= maxTime,   (*Print["----end----"];*)
      Return[gOutPut]];
(*      Return[currentTime]];*)

    debugPrint["expandAsks:consTable:", consTable];
    consTable = expandAsks[consTable, tmpPosAsk, pftVars];
    consTable = removeNonAlwaysTuple[consTable];
 
    {consTable, tmpPosAsk, tmpNegAsk} = {consTable, tmpPosAsk, tmpNegAsk} /. name_[0] -> name[t];
    debugPrint["tmpPosAsk:", tmpPosAsk];
    debugPrint["tmpNegAsk:", tmpNegAsk];
    debugPrint["changedAsk:", changedAsk];


    debugPrint["--- Before IntervalPhase ---"];
    debugPrint["consStore:", consStore];
    debugPrint["consTable:", consTable];

    tmpConsStoreTable = consStore2Table[consStore, pftVars /. name_[t] -> name[0]];
    debugPrint["tmpConsStoreTable:", tmpConsStoreTable];

    {tmpT, includeZero, changedAsk, tmpPrevConsTable} =
      intervalPhase[
(*                     order[group[order[consTable, consFrameAxioms], unit[tell[consStore]]], consPrevEqNow], *)
                    order[order[tmpConsStoreTable, order[consTable, consFrameAxioms]], consPrevEqNow],
(*                     order[order[order[consTable, consFrameAxioms], tmpConsStoreTable], consPrevEqNow], *)
                    Join[tmpNegAsk, tmpPosAsk],
                    tmpPosAsk,
                    changedAsk,
                    includeZero,
                    ruleNow2IntegNow, rulePrev2IntegNow,
                    pftVars, ftVars, ftVarsND, var2IntegVar[argVars], prevVars, 
                    maxTime-currentTime, currentTime];
  
    debugPrint["changedAsk:", changedAsk];

    currentTime += tmpT;

    If[tmpT=!=0 || consStorePrev === {}, consStorePrev = tmpPrevConsTable];

    consTable = order[consStorePrev, consTable, consPrevEqNow];
    debugPrint["consTable:", consTable];
  ];
]

(* End[] *)
(* EndPackage[] *)
