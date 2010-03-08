(*
 * �f�o�b�N�p���b�Z�[�W�o�͊֐�
 *)
If[optUseDebugPrint,
  debugPrint[arg___] := Print[InputForm[{arg}]],
  debugPrint[arg___] := Null];

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

(* Or�łȂ����Ă���i������������j�ꍇ�́A���̂�����1�݂̂�Ԃ��H *)
createVariableList[Or[expr_, others__], vars_, result_] := (
  createVariableList[expr, vars, result]
);

createVariableList[Equal[varName_, varValue_], vars_, result_] := (
  sol = (Solve[Equal[varName, varValue], vars])[[1]];
  createVariableList[sol, result]
);

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
  sol = Reduce[expr, vars];
  If[sol =!= False,
      (* true *)
      (* �O����Or[]�̂���ꍇ��List�Œu��������B�Ȃ��ꍇ��List�ň͂� *)
      sol = LogicalExpand[sol];
      sol = If[Head[sol] === Or, Apply[List, sol], {sol}];
      (* ����ꂽ���X�g�̗v�f�̃w�b�h��And�ł���ꍇ��List�Œu��������B�Ȃ��ꍇ��List�ň͂� *)
      sol = Map[(If[Head[#] === And, Apply[List, #], {#}]) &, sol];
      (* ��ԓ����̗v�f �i���x��2�j�𕶎���ɂ��� *)
      {1, Map[(ToString[FullForm[#]]) &, sol, {2}]},

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
isRequiredConstraint[cons_, tellVars_] := (
   consVar = cons /. x_[0] == y_ -> x;
   removeDash[consVar];
   If[MemberQ[tellVars, consVar], True, False]
);

(* tellVars�����ɁA���̕ϐ����K�v���ǂ����𒲂ׂ� *)
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
 * ���̃|�C���g�t�F�[�Y�Ɉڍs���鎞�������߂�
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
      (* ���Ȃ��Ƌ��E�l�̉�����ʂ��� *)  
      sol = Reduce[{timeMinCons && (maxTime>=t) && (integAsk)}, t];
      If[sol=!=False, 
         (* true *)
         (* ���藧��t�̍ŏ��l�����߂� *)
         minT = First[Quiet[Minimize[{t, timeMinCons && (sol)}, {t}], 
                            Minimize::wksol]],

         (* false *)
         minT = error],
      
      (* false *)
      minT=0];

    (* 0�b��̂��܂�ł͂����Ȃ� *)
    If[includeZero===False && minT===0, minT=error];
    (* 0�b��̗��U�ω����s���邩�̃`�F�b�N�Ȃ̂�0�łȂ���΃G���[ *)
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
 * ask�̓��o��Ԃ��ω�����܂Őϕ��������Ȃ�
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
