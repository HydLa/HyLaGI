(*
 * ��ǒ�В�Ò����ђ�ᒥÒ����������В�ϒ�ؒ��
 *)
If[optUseDebugPrint,
  debugPrint[arg___] := Print[InputForm[{arg}]],
  debugPrint[arg___] := Null];

(*
 * checkEntailment���IP��В�������璥�
 * 0 : Solver Error
 * 1 : �Ƴ��В�Ēǽ
 * 2 : �Ƴ��В�Ԓ�Ēǽ
 *)
checkEntailmentInterval[guard_, store_, vars_] := Quiet[Check[Block[
  {tStore, sol, integGuard},
  debugPrint["guard:", guard, "store:", store, "vars:", vars];
  tStore = exDSolve[store, vars];
  If[tStore =!= overconstraint && tStore =!= underconstraint,
    (* guard���tStore���Ŭ��ђ����� *)
    integGuard = guard /. tStore;
    If[integGuard =!= False,
      (* �����Β�뒲̒��t>0��Ȓ��Ϣ�Ω *)
      (* debugPrint["integGuard:", integGuard]; *)
      sol = Reduce[{integGuard && t > 0}, t];
      (* Inf���蒤Ò��0��˒�ʒ�쒤�Entailed *)
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
 * ������1��Ē��ask������˒�ؒ����ƒ�������Β�������ɒ�������󒥹��Ȓ��������entail��ǒ����뒤���ɒ��������������Ò��
 *  0 : Solver Error
 *  1 : �Ƴ��В�Ēǽ
 *  2 : �Ƴ��В�Ԓ�Ēǽ
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
 * ��ђ���̾��ΒƬ��˒�Ē����ƒ�����"usrVar"���蒤꒽����
 *)
renameVar[varName_] := (
  ToExpression[First[StringCases[ToString[varName], "usrVar" ~~ x__ -> x]]]
);

(*
 * isConsistent��⒤�Reduce��Β�뒲̒����钤쒤�����{��ђ���̾, ���}�����Β�꒥���Ȓ�������˒�����
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

(* Or��ǒ�Ē�ʒ����Ò�ƒ����뒡ʒʣ�����򒤬�����뒡˒�쒹璤ϒ�������Β��������1��Ē�Β�ߒ���֒����� *)
createVariableList[Or[expr_, others__], vars_, result_] := (
  createVariableList[expr, vars, result]
);

createVariableList[Equal[varName_, varValue_], vars_, result_] := Block[{
  sol
},
  sol = (Solve[Equal[varName, varValue], vars])[[1]];
  createVariableList[sol, result]
];

(* Reduce[{}, {}]��Β�Ȓ�� *)
createVariableList[True, vars_, result_] := (
  Return[result]
);

(*
 * �����󒤬�̵�̷��⒤ǒ����뒤����������Ò��������
 * Reduce��ǒ�򒤤�����뒲̒����򒤬�����钤쒤쒤В̵�̷���
 * �����钤쒤������ђ����ƒ����ƒ�ђ����˒�ؒ�����{��ђ���̾, ���}�����Ȓ�������������ǒɽ��������꒥���Ȓ���֒��
 *
 * ��ᒤ��͒�Β�꒥���Ȓ�Β��Ƭ���
 *  0 : Solver Error
 *  1 : ����­
 *  2 : �����󒥨��钡�
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
      (* ����¦���Or[]��Β����뒾쒹璤�List��ǒ�֒����������뒡���ʒ����쒹璤�List��ǒ�ϒ�� *)
      sol = LogicalExpand[sol];
     (*      debugPrint["sol after LogicalExpand:", sol];*)
      sol = If[Head[sol] === Or, Apply[List, sol], {sol}];
     (*      debugPrint["sol after Apply Or List:", sol];*)
      (* �����钤쒤���꒥���Ȓ�Β�ג�ǒ�Β�ؒ�Ò�ɒ��And��ǒ����뒾쒹璤�List��ǒ�֒����������뒡���ʒ����쒹璤�List��ǒ�ϒ�� *)
      sol = Map[(If[Head[#] === And, Apply[List, #], {#}]) &, sol];
     (*      debugPrint["sol after Apply And List:", sol];*)
      (* ����֒��¦��Β�ג�� ��ʒ�쒥ْ��2��˒��ʸ������˒����� *)
      {1, Map[(ToString[FullForm[#]]) &, removeNotEqual[sol], {2}]},

      (* false *)
      {2}]
],
  {0, $MessageList}
]];


(* Print[isConsistent[s[x==2, 7==x*x], {x,y}]] *)

(* ��ђ���̾������ ��֒�ǒ�ג��蒤� *)
removeDash[var_] := (
   var /. Derivative[_][x_] -> x
);

(* 
 * tellVars��򒸵��˒�������Β�钴���͒����󒤬�ɬ��ג����ɒ��������Ĵ��ْ��
 * ��钴���͒������撤˒�В���������ђ�����vars��⒤˒�ʒ����쒤В�Ԓ��
 *)
isRequiredConstraint[cons_, tellVars_] := Block[{
   consVar
},
   consVar = cons /. x_[0] == y_ -> x;
   removeDash[consVar];
   If[MemberQ[tellVars, consVar], True, False]
];

(* tellVars��򒸵��˒�������Β�ђ������ɬ��ג����ɒ��������Ĵ��ْ�� *)
isRequiredVariable[var_, tellVars_] := (
   If[MemberQ[tellVars, var], True, False]
);

removeNotEqual[sol_] := 
   DeleteCases[sol, Unequal[lhs_, rhs_], Infinity];

getNDVars[vars_] := Union[Map[(removeDash[#] /. x_[t] -> x) &, vars]];

(* �������ђ��x��˒�Ē����ƒ�� *)
(* x[t]==a ��� x[0]==a ��������Ò����쒹璤ϒ��x'[t]==0���Ò�� ��ʒ����쒤В�� *)
removeTrivialCons[expr_, var_] := Block[
   {exprRule, varSol, removedExpr},

   (* ������������뒡���뒲������� *)
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

(* Reduce��ǒ����钤쒤���뒲̒��꒥���Ȓ�������˒����� *)
(* And��ǒ�ϒ�ʒ��List��ǒ�������� *)
applyList[reduceSol_] :=
  If[Head[reduceSol] === And, List @@ reduceSol, List[reduceSol]];

exDSolve[expr_, vars_] := Block[
{sol, DExpr, DExprVars, NDExpr},
  sol = removeNotEqual[Reduce[Cases[expr, Except[True] | Except[False]], vars]];

  If[sol===False,
    overconstraint,
    If[sol===True,
      underconstraint,

      (* 1��Ē�������Β�� *)
      (* TODO: �ʣ�����򒤢��뒾쒹璤Ⓓ͒����� *)
      If[Head[sol]===Or, sol = First[sol]];

      sol = applyList[sol];

      (* ��꒿���ؒ����Β�쒹璤˒�ᒾ꒷��꒷ϒ�Β�������Ȓ�ʒ�����ʬ�������蒤꒽���� *)
      sol = Fold[(Intersection[#1, removeTrivialCons[sol, #2]]) &, sol, getNDVars[vars]]; 

      {DExpr, DExprVars, NDExpr} = splitExprs[sol];

      Quiet[Check[Check[sol = DSolve[DExpr, DExprVars, t];
                        (* 1��Ē�������Β�� *)
                        (* TODO: �ʣ�����򒤢��뒾쒹璤Ⓓ͒����� *)
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

(* DSolve��ǒ�������뒼� (DExpr)��Ȓ�������ǒ�ʒ����� (NDExpr)��˒ʬ������ *)
(* ����ʬ��͒��ޒ�ޒ���������ĉ���ђ�����2�������ʒ�咽В�뒼� (NDExpr)���DSolve��ǒ�������ʒ�� *)
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
(*      (\* �����󒥹��Ȓ����������ǒ�ʒ����쒹璡���Ԓ�ג�ʒ�钴���͒������蒤꒽�����ɬ��ג�������� *\) *)
(*      newTellVars = Map[removeDash, Map[(# /. x_[t] -> x) &, tellsVars]]; *)
(*      (\* store���List[And[]]��Β����˒�ʒ�Ò�ƒ����뒾쒹璤ϒ�����ö�����撤�And���蒤꒽В�� *\) *)
(*      newStore = If[Head[First[store]] === And, Apply[List, First[store]], store]; *)
(*      removedStore = {Apply[And, Select[newStore, (isRequiredConstraint[#, newTellVars]) &]]}; *)
(*      newStoreVars = Map[removeDash, Map[(# /. x_[t] -> x) &, storeVars]]; *)
(*      removedStoreVars = Map[(#[t])&, Select[newStoreVars, (isRequiredVariable[#, newTellVars])&]], *)

(*      (\* �����󒥹��Ȓ����������Β�쒹� *\) *)
(*      removedStore = store; *)
(*      removedStoreVars = storeVars]; *)

(*   If[sol =!= overconstraint, *)
(*      cons = Map[(ToString[FullForm[#]]) &, Join[tells, removedStore]]; *)
(*      vars = Join[tellsVars, removedStoreVars]; *)
(*      If[sol =!= underconstraint, {1, cons, vars}, {2, cons, vars}], *)
(*      0] *)
(* ); *)

(*
 * ��ᒤ��͒�Β�꒥���Ȓ�Β��Ƭ���
 *  0 : Solver Error
 *  1 : ����­
 *  2 : �����󒥨��钡�
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
 * �����Β�ݒ�����Ȓ�Ւ����������˒�ܒ�Ԓ����뒻����ᒤᒤ�
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
      (* ���ʒ����Ȓ�������͒�Β�����̒����� *)  
       sol = Quiet[Check[Reduce[{timeMinCons && (maxTime>=t) && (integAsk)}, t],
                         errorSol,
                         {Reduce::nsmet}],
                   {Reduce::nsmet}];
      (*  debugPrint["calcNextPointPhaseTime#sol: ", sol]; *)
      If[sol=!=False && sol=!=errorSol, 
         (* true *)
         (* ������Ω���t��Β�ǒ����͒��ᒤᒤ� *)
         minT = First[Quiet[Minimize[{t, timeMinCons && (sol)}, {t}], 
                            Minimize::wksol]],

         (* false *)
         minT = error],
      
      (* false *)
      minT=0];

      (* debugPrint["calcMinTime#minT: ", minT];*)
    (* 0��Ò�咤Β��ޒ��ǒ�ϒ�������ʒ�� *)
    If[includeZero===False && minT===0, minT=error];
    (* 0��Ò�咤ΒΥ�����ђ�������Ԓ�쒤뒤���Β�������Ò����ʒ�Β��0��ǒ�ʒ����쒤В����钡� *)
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
 * ask��ΒƳ��В����֒����ђ�������뒤ޒ�ǒ�ђʬ��򒤪�����ʒ��
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

    (* 1��Ē�������Β�� *)
    (* TODO: �ʣ�����򒤢��뒾쒹璤Ⓓ͒����� *)
    If[Head[tmpIntegSol]===Or, tmpIntegSol = First[tmpIntegSol]];
    tmpIntegSol = Quiet[DSolve[tmpIntegSol, DExprVars, t],
                        {Solve::incnst}];
    (* debugPrint["tmpIntegSol: ", tmpIntegSol]; *)

    (*1��Ē�������Β��*)
    (*TODO:�ʣ�����򒤢��뒾쒹璤Ⓓ͒�����*)
    tmpIntegSol = First[tmpIntegSol];
    tmpIntegSol = applyList[removeNotEqual[Reduce[Join[Map[(Equal @@ #) &, tmpIntegSol], NDExpr], vars]]];
    (* ������������뒡���뒲������� *)
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
 * �����˒�В����ƒͿ�����钤쒤������֒��Ŭ��ђ�����
 *)
applyTime2Expr[expr_, time_] := (
  (expr /. t -> time) // FullForm // ToString
);

(*
 * form������to��ޒ�ǒ�Β�꒥���Ȓ��interval��֒�֒�ǒ����������
 * ��ǒ�咤˒ɬ���to�����蒤뒤���ᒤ˒��
 * ��ǒ�咤Β�֒�֒�Β��interval��蒤꒤�û�����ʒ�뒲Ēǽ������������
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
 * �Ϳ�����钤쒤��������ђʬ��������֒��
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
 * �Ϳ�����钤쒤�������ᒻ�������
 *)
approxExpr[precision_, expr_] :=
  Rationalize[N[Simplify[expr], precision + 3], 
              Divide[1, Power[10, precision]]];
