(*
 * �f�o�b�N�p���b�Z�[�W�o�͊֐�
 *)
If[optUseDebugPrint,
  debugPrint[arg___] := Print[InputForm[{arg}]],
  debugPrint[arg___] := Null];

(*
 * checkEntailment��IP�o�[�W����
 * 0 : Solver Error
 * 1 : ���o�\
 * 2 : ���o�s�\
 *)
checkEntailmentInterval[guard_, store_, vars_] := Quiet[Check[Block[
  {tStore, sol, integGuard},
  debugPrint["guard:", guard, "store:", store, "vars:", vars];
  tStore = exDSolve[store, vars];
  If[tStore =!= overconstraint && tStore =!= underconstraint,
    (* guard��tStore��K�p���� *)
    integGuard = guard /. tStore;
    If[integGuard =!= False,
      (* ���̌��ʂ�t>0�Ƃ�A�� *)
      (* debugPrint["integGuard:", integGuard]; *)
      sol = Quiet[Check[Reduce[{integGuard && t > 0}, t],
                        False, {Reduce::nsmet}], {Reduce::nsmet}];
      (* Inf�������0�ɂȂ��Entailed *)
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
 * ����1��ask����Ɋւ��āA���̃K�[�h������X�g�A����entail�ł��邩�ǂ������`�F�b�N
 *  0 : Solver Error
 *  1 : ���o�\
 *  2 : ���o�s�\
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
 * �ϐ����̓��ɂ��Ă���"usrVar"����菜��
 *)
renameVar[varName_] := (
  ToExpression[First[StringCases[ToString[varName], "usrVar" ~~ x__ -> x]]]
);

(*
 * isConsistent����Reduce�̌��ʓ���ꂽ����{�ϐ���, �l}�@�̃��X�g�`���ɂ���
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

(* Or�łȂ����Ă���i������������j�ꍇ�́A���̂�����1�݂̂�Ԃ��H *)
createVariableList[Or[expr_, others__], vars_, result_] := (
  createVariableList[expr, vars, result]
);

createVariableList[Equal[varName_, varValue_], vars_, result_] := Block[{
  sol
},
  sol = (Solve[Equal[varName, varValue], vars])[[1]];
  createVariableList[sol, result]
];

(* Reduce[{}, {}]�̂Ƃ� *)
createVariableList[True, vars_, result_] := (
  Return[result]
);

(*
 * ���񂪖������ł��邩���`�F�b�N����
 * Reduce�ŉ��������ʁA����������Ζ�����
 * ����ꂽ����p���āA�e�ϐ��Ɋւ���{�ϐ���, �l}�@�Ƃ����`���ŕ\�������X�g��Ԃ�
 *
 * �߂�l�̃��X�g�̐擪�F
 *  0 : Solver Error
 *  1 : �[��
 *  2 : ����G���[
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
      (* �O����Or[]�̂���ꍇ��List�Œu��������B�Ȃ��ꍇ��List�ň͂� *)
      sol = LogicalExpand[sol];
     (*      debugPrint["sol after LogicalExpand:", sol];*)
      sol = If[Head[sol] === Or, Apply[List, sol], {sol}];
     (*      debugPrint["sol after Apply Or List:", sol];*)
      (* ����ꂽ���X�g�̗v�f�̃w�b�h��And�ł���ꍇ��List�Œu��������B�Ȃ��ꍇ��List�ň͂� *)
      sol = Map[(If[Head[#] === And, Apply[List, #], {#}]) &, sol];
     (*      debugPrint["sol after Apply And List:", sol];*)
      (* ��ԓ����̗v�f �i���x��2�j�𕶎���ɂ��� *)
      {1, Map[(ToString[FullForm[#]]) &, removeNotEqual[sol], {2}]},

      (* false *)
      {2}]
],
  {0, $MessageList}
]];


(* Print[isConsistent[s[x==2, 7==x*x], {x,y}]] *)

(* �ϐ������� �u�f�v����� *)
removeDash[var_] := (
   var /. Derivative[_][x_] -> x
);

(* 
 * tellVars�����ɁA���̏����l���񂪕K�v���ǂ����𒲂ׂ�
 * �����l���񒆂ɏo������ϐ���vars���ɂȂ���Εs�v
 *)
isRequiredConstraint[cons_, tellVars_] := Block[{
   consVar
},
   consVar = cons /. x_[0] == y_ -> x;
   removeDash[consVar];
   If[MemberQ[tellVars, consVar], True, False]
];

(* tellVars�����ɁA���̕ϐ����K�v���ǂ����𒲂ׂ� *)
isRequiredVariable[var_, tellVars_] := (
   If[MemberQ[tellVars, var], True, False]
);

removeNotEqual[sol_] := 
   DeleteCases[sol, Unequal[lhs_, rhs_], Infinity];

getNDVars[vars_] := Union[Map[(removeDash[#] /. x_[t] -> x) &, vars]];

(* ����ϐ�x�ɂ��āA *)
(* x[t]==a �� x[0]==a ���������ꍇ�́Ax'[t]==0������ �i����΁j *)
removeTrivialCons[cons_, consVars_] := Block[
  {exprRule, varSol, removedExpr,
   removeTrivialConsUnit, getRequiredVars},

  removeTrivialConsUnit[expr_, var_]:=(
    (* �����������[�������� *)
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

(* Reduce�œ���ꂽ���ʂ����X�g�`���ɂ��� *)
(* And�ł͂Ȃ�List�ł����� *)
applyList[reduceSol_] :=
  If[Head[reduceSol] === And, List @@ reduceSol, List[reduceSol]];

exDSolve[expr_, vars_] := Block[
{sol, DExpr, DExprVars, NDExpr},
  sol = removeNotEqual[Reduce[Cases[expr, Except[True] | Except[False]], vars]];

  If[sol===False,
    overconstraint,
    If[sol===True,
      underconstraint,

      (* 1�����̗p *)
      (* TODO: ����������ꍇ���l���� *)
      If[Head[sol]===Or, sol = First[sol]];

      sol = applyList[sol];

      (* �萔�֐��̏ꍇ�ɉߏ茈��n�̌����ƂȂ�����������菜�� *)
      sol = removeTrivialCons[sol, vars];

      {DExpr, DExprVars, NDExpr} = splitExprs[sol];

      Quiet[Check[Check[sol = DSolve[DExpr, DExprVars, t];
                        (* 1�����̗p *)
                        (* TODO: ����������ꍇ���l���� *)
                        sol = First[sol];
                        sol = removeNotEqual[Reduce[Join[Map[(Equal @@ #) &, sol], NDExpr], 
                                                    Map[(#[t]) &, getNDVars[vars]]]];
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

(* DSolve�ň����鎮 (DExpr)�Ƃ����łȂ��� (NDExpr)�ɕ����� *)
(* �����l���܂܂�����¤�ϐ���2��ވȏ�o�鎮 (NDExpr)��DSolve�ň����Ȃ� *)
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
(*      (\* ����X�g�A����łȂ��ꍇ�A�s�v�ȏ����l�������菜���K�v������ *\) *)
(*      newTellVars = Map[removeDash, Map[(# /. x_[t] -> x) &, tellsVars]]; *)
(*      (\* store��List[And[]]�̌`�ɂȂ��Ă���ꍇ�́A��U�A����And�����o�� *\) *)
(*      newStore = If[Head[First[store]] === And, Apply[List, First[store]], store]; *)
(*      removedStore = {Apply[And, Select[newStore, (isRequiredConstraint[#, newTellVars]) &]]}; *)
(*      newStoreVars = Map[removeDash, Map[(# /. x_[t] -> x) &, storeVars]]; *)
(*      removedStoreVars = Map[(#[t])&, Select[newStoreVars, (isRequiredVariable[#, newTellVars])&]], *)

(*      (\* ����X�g�A����̏ꍇ *\) *)
(*      removedStore = store; *)
(*      removedStoreVars = storeVars]; *)

(*   If[sol =!= overconstraint, *)
(*      cons = Map[(ToString[FullForm[#]]) &, Join[tells, removedStore]]; *)
(*      vars = Join[tellsVars, removedStoreVars]; *)
(*      If[sol =!= underconstraint, {1, cons, vars}, {2, cons, vars}], *)
(*      0] *)
(* ); *)

(*
 * �߂�l�̃��X�g�̐擪�F
 *  0 : Solver Error
 *  1 : �[��
 *  2 : ����G���[
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
 * ���̃|�C���g�t�F�[�Y�Ɉڍs���鎞�������߂�
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
      (* ���Ȃ��Ƌ��E�l�̉�����ʂ��� *)  
       sol = Quiet[Check[Reduce[{timeMinCons && (maxTime>=t) && (integAsk)}, t],
                         errorSol,
                         {Reduce::nsmet}],
                   {Reduce::nsmet}];
      (*  debugPrint["calcNextPointPhaseTime#sol: ", sol]; *)
      If[sol=!=False && sol=!=errorSol, 
         (* true *)
         (* ���藧��t�̍ŏ��l�����߂� *)
         minT = First[Quiet[Minimize[{t, timeMinCons && (sol)}, {t}], 
                            Minimize::wksol]],

         (* false *)
         minT = error],
      
      (* false *)
      minT=0];

      (* debugPrint["calcMinTime#minT: ", minT];*)
    (* 0�b��̂��܂�ł͂����Ȃ� *)
    If[includeZero===False && minT===0, minT=error];
    (* 0�b��̗��U�ω����s���邩�̃`�F�b�N�Ȃ̂�0�łȂ���΃G���[ *)
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
 * ask�̓��o��Ԃ��ω�����܂Őϕ��������Ȃ�
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

    tmpIntegSol = removeTrivialCons[tmpIntegSol, vars];

    (* 1�����̗p *)
    (* TODO: ����������ꍇ���l���� *)
    If[Head[tmpIntegSol]===Or, tmpIntegSol = First[tmpIntegSol]];
    tmpIntegSol = Quiet[DSolve[tmpIntegSol, DExprVars, t],
                        {Solve::incnst}];
    (* debugPrint["tmpIntegSol: ", tmpIntegSol]; *)

    (*1�����̗p*)
    (*TODO:����������ꍇ���l����*)
    tmpIntegSol = First[tmpIntegSol];
    tmpIntegSol = applyList[removeNotEqual[Reduce[Join[Map[(Equal @@ #) &, tmpIntegSol], NDExpr], vars]]];
    (* �����������[�������� *)
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
 * ���ɑ΂��ė^����ꂽ���Ԃ�K�p����
 *)
applyTime2Expr[expr_, time_] := (
  (expr /. t -> time) // FullForm // ToString
);

(*
 * form����to�܂ł̃��X�g��interval�Ԋu�ō쐬����
 * �Ō�ɕK��to�����邽�߂ɁC
 * �Ō�̊Ԋu�̂�interval�����Z���Ȃ�\��������
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
 * �^����ꂽ����ϕ����C�Ԃ�
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
 * �^����ꂽ�����ߎ�����
 *)
approxExpr[precision_, expr_] :=
  Rationalize[N[Simplify[expr], precision + 3], 
              Divide[1, Power[10, precision]]];
