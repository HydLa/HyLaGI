(*
 * �f�o�b�N�p���b�Z�[�W�o��V��
 *)
If[optUseDebugPrint,
  debugPrint[arg___] := Print[InputForm[{arg}]],
  debugPrint[arg___] := Null];


(* ���[���̃��X�g���󂯂āCt�ɂ��Ẵ��[�������������̂�Ԃ��֐� *)
removeRuleForTime[ruleList_] := DeleteCases[ruleList, t -> _];

(*
 * checkEntailment��IP�o�[�W����
 * 0 : Solver Error
 * 1 : ���o�\
 * 2 : ���o�s�\
 * 3 : �v����
 *)
checkEntailmentInterval[guard_, store_, vars_, pars_] := Quiet[Check[Block[
  {tStore, sol, integGuard, otherExpr, minT},
  debugPrint["guard:", guard, "store:", store, "vars:", vars, "pars:", pars];
  sol = exDSolve[store, vars];
  (*debugPrint["sol:", sol];*)
  If[sol =!= overconstraint && sol =!= underconstraint,
    tStore = sol[[1]];
    otherExpr = sol[[2]];
    (* guard��otherExpr��tStore��K�p���� *)
    integGuard = guard /. tStore;
    otherExpr = otherExpr /. tStore;
    (* ���̌��ʂ�t>0�Ƃ�A�� *)
    (*debugPrint["integGuard:", integGuard];*)
    sol = Quiet[Check[Reduce[{integGuard && t > 0 && (And@@otherExpr)}, t, Reals],
                      False, {Reduce::nsmet}], {Reduce::nsmet}];
    If[sol =!= False,
      (* Inf�������0�ɂȂ�΁C���o�ł���\�������� *)
      minT = Quiet[Minimize[{t, sol}, Append[pars,t]]];
      If[First[minT] === 0 && Quiet[Reduce[And@@Map[(Equal@@#)&, removeRuleForTime[minT[[2]]]] && And@@otherExpr]] =!= False,
        (* ���2�Ԗڂ̏����͈ꌩ����ƕs�v�����CMinimize�����Ԃ̋��E�ɐG��Ă���ꍇ�ƒ萔�̋��E�ɐG��Ă���ꍇ����ʂł��Ȃ��炵���̂ŕK�v�D *)
        (* ���Ԃ̋��E�͂n�j�����C�萔�����E�ɏ���Ă�͖̂{�����肦�Ȃ��ꍇ�̂��� *)
        
        (* �K�����o�ł��邩�ǂ����𔻒� *)
        If[Quiet[Reduce[ForAll[pars, And@@otherExpr, First[Minimize[{t, sol}, t]] == 0]]] =!= False,
          {1},
          {3}
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
 * ����1��ask����Ɋւ��āA���̃K�[�h������X�g�A����entail�ł��邩�ǂ������`�F�b�N
 *  0 : Solver Error
 *  1 : ���o�\
 *  2 : ���o�s�\
 *  3 : �s�� �i�v����j
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

removeP[par_] := First[StringCases[ToString[par], "p" ~~ x__ -> x]];

renameVar[varName_] := Block[
  {renamedVarName, derivativeCount = 0, prevFlag = 0,
   getDerivativeCountPoint, removeUsrVar
  },  

  getDerivativeCountPoint[Derivative[n_][var_]] := n;
  removeUsrVar[var_] := First[StringCases[ToString[var], "usrVar" ~~ x__ -> x]];

  (* �ϐ�����'�����ꍇ�̏��� *)
  If[MemberQ[{varName}, Derivative[n_][x_], Infinity],
    derivativeCount = getDerivativeCountPoint[varName];
    renamedVarName = removeDash[varName],
    renamedVarName = varName
  ];
  (* �ϐ�����prev�����ꍇ�̏��� *)
  If[Head[renamedVarName] === prev,
    renamedVarName = First[renamedVarName];
    prevFlag = 1
  ];

  (*�ϐ����̓��ɂ��Ă��� "usrVar"����菜�� �i���O�̓���usrVar����Ȃ��̂�invalidVar���萔���j *)
  If[StringMatchQ[ToString[renamedVarName], "usrVar" ~~ x__],
    (* true *)
    renamedVarName = removeUsrVar[renamedVarName];
    (* ���̎��_�ŒP�̂̕ϐ����̂͂��D *)
    If[Length[renamedVarName] === 0,
      {renamedVarName, derivativeCount, prevFlag},
      (* ���`���̂��̂̓n�l�� *)
      invalidVar],
    (* false *)
    If[StringMatchQ[ToString[renamedVarName], "p" ~~ x__],
     (*�萔���̓��ɂ��Ă��� "p"����菜�� ���O�̓���p����Ȃ��̂�invalidVar�D *)
      renamedVarName = removeP[renamedVarName];
      (* prev��-1�̂��̂͒萔�Ƃ��Ĉ������Ƃɂ��� *)
      {renamedVarName, derivativeCount, -1},
      invalidVar
    ]
  ]
];

(* �ϐ��Ƃ��̒l�Ɋւ��鎮�̃��X�g���A�ϐ��\�I�`���ɕϊ�
 * 0:Equal
 * 1:Less
 * 2:Greater
 * 3:LessEqual
 * 4:GreaterEqual
*)

getExprCode[expr_] := Switch[Head[expr],
  Equal, 0,
  Less, 1,
  Greater, 2,
  LessEqual, 3,
  GreaterEqual, 4
];

convertCSToVM[orExprs_] := Block[
  {resultExprs, inequalityVariable,
   removeInequality},
  
  
  (* Inequality[a, relop, x, relop, b]�̌`��ό` *)
  removeInequality[ret_, expr_] := (
    If[Head[expr] === Inequality,
      Join[ret,
           {Reduce[expr[[2]][expr[[1]], expr[[3]]], expr[[3]]]},
           {expr[[4]][expr[[3]], expr[[5]]]}
      ],
      Append[ret, expr]
    ]
  );
  
  
  formatExprs[exprs_] := (
    If[Cases[exprs, Except[True]]==={},
      (* ����{�i�ϐ����j, �i�֌W���Z�q�R�[�h�j, (�l�̃t��������)�p�̌`���ɕϊ����� *)
      {},
      
      DeleteCases[Map[({renameVar[#[[1]]], 
                        getExprCode[#], 
                        ToString[FullForm[#[[2]]]]}) &, 
                      Fold[(removeInequality[#1, #2]) &, {}, exprs]],
                  {invalidVar, _, _}]
    ]
  );

  debugPrint["orExprs:", orExprs];
  If[Head[First[orExprs]] === Or,
    resultExprs = Map[(applyList[#])&, List@@First[orExprs]],
    resultExprs = Map[(applyList[#])&, orExprs]
  ];
  resultExprs = Map[formatExprs, resultExprs];
  debugPrint["resultExprs:", resultExprs];
  resultExprs
];

(*
 * isConsistent����Reduce�̌��ʓ���ꂽ����{�ϐ���, �l}�̃��X�g�`���ɂ���
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

(* Or�łȂ����Ă��� �i������������j�ꍇ�́A���̂�����1�݂̂�Ԃ��H *)
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

(* �����ɕϐ������o�����邩�ۂ� *)
hasVariable[exprs_] := Length[StringCases[ToString[exprs], "usrVar" ~~ LetterCharacter]] > 0;

getInverseRelop[relop_] := Switch[relop,
                                  Equal, Equal,
                                  Less, Greater,
                                  Greater, Less,
                                  LessEqual, GreaterEqual,
                                  GreaterEqual, LessEqual];

(* �K���֌W���Z�q�̍����ɕϐ���������悤�ɂ��� *)
adjustExprs[andExprs_] := 
  Fold[(If[Not[hasVariable[#2[[1]]]],
          (* true *)
          If[hasVariable[#2[[2]]],
            (* true *)
            (* �t�ɂȂ��Ă�̂ŁA���Z�q���t�ɂ��Ēǉ����� *)
            Append[#1, getInverseRelop[Head[#2]][#2[[2]], #2[[1]]]],
            (* false *)
            (* �p�����[�^����̏ꍇ�ɂ����ɓ��� *)
            If[NumericQ[#2[[1]]],
              (* true *)
              (* �t�ɂȂ��Ă�̂ŁA���Z�q���t�ɂ��Ēǉ����� *)
              Append[#1, getInverseRelop[Head[#2]][#2[[2]], #2[[1]]]],
              (* false *)
              Append[#1, #2]]],
          (* false *)
          Append[#1, #2]]) &,
       {}, andExprs];

(*
 * ���񂪖������ł��邩���`�F�b�N����
 * Reduce�ŉ��������ʁA낃F������Ζ�����
 * ����ꂽ���ʂ�p���āA�e�ϐ��Ɋւ��� {�ϐ���, �l}�@�Ƃ����`���ŕ\�������X�g��Ԃ�
 *
 * �߂�l�̃��X�g�̐擪�F
 *  0 : Solver Error
 *  1 : �[��
 *  2 : ����G���[
 *)
isConsistent[pexpr_, expr_, vars_] := Quiet[Check[Block[
{
  sol, ret
},

  debugPrint["pexpr:", pexpr, "expr", expr, "vars:", vars];
  sol = Reduce[expr, vars, Reals];
  (*  debugPrint["sol after Reduce:", sol];*)
  If[sol =!= False,
    sol = Reduce[Append[pexpr,sol],vars, Reals];
    If[sol =!= False,
      (* true *)
      (* �O����Or[]�̂���ꍇ��List�Œu��������B�Ȃ��ꍇ��List�ň͂� *)
      sol = LogicalExpand[sol];
      (* debugPrint["sol after LogicalExpand:", sol];*)
      sol = If[Head[sol] === Or, Apply[List, sol], {sol}];
      (* debugPrint["sol after Apply Or List:", sol];*)
      (* ����ꂽ���X�g�̗v�f�̃w�b�h��And�ł���ꍇ��List�Œu��������B�Ȃ��ꍇ��List�ň͂� *)
      sol = removeNotEqual[Map[(If[Head[#] === And, Apply[List, #], {#}]) &, sol]];
      (* debugPrint["sol after Apply And List:", sol];*)
      (* ��ԓ����̗v�f �i���x��2�j�𕶎���ɂ��� *)
      ret={1, Map[(ToString[FullForm[#]]) &, 
              Map[(adjustExprs[#])&, sol], {2}]},
      ret={2}
    ],
    (* false *)
    ret={2}
  ];
  debugPrint["ret:", ret];
  ret
],
  {0, $MessageList}
]];

(* Print[isConsistent[s[x==2, 7==x*x], {x,y}]] *)

(* �ϐ������� �u\[CloseCurlyQuote]�v����� *)
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

getNDVars[vars_] := Union[Map[(removeDash[#])&, vars]];

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

(* �p�����[�^�̐���𓾂� *)
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

      (* 1�����̗p *)
      (* TODO: ����������ꍇ���l���� *)
      If[Head[sol]===Or, sol = First[sol]];

      sol = applyList[sol];

      (* �萔�֐��̏ꍇ�ɉߏ茈��n�̌����ƂȂ�����������菜�� *)
      sol = removeTrivialCons[sol, vars];

      {DExpr, DExprVars, NDExpr, otherExpr} = splitExprs[sol];

      Quiet[Check[Check[If[Cases[DExpr, Except[True]] === {},
                          (* �X�g�A����̏ꍇ��DSolve�������Ȃ��̂ŋ�W����Ԃ� *)
                          sol = {},
                          sol = DSolve[DExpr, DExprVars, t];
                          (* 1�����̗p *)
                          (* TODO: ����������ꍇ���l���� *)
                          sol = First[sol]];

                        sol = Quiet[Solve[Join[Map[(Equal @@ #) &, sol], NDExpr], getNDVars[vars]]];
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

(* DSolve�ň����鎮 (DExpr)�Ƃ����łȂ��� (NDExpr)�Ƃ���ȊO �iotherExpr�j�ɕ����� *)
(* �����l���܂܂�/////�ϐ���2��ވȏ�o�鎮 (NDExpr)�ⓙ���ȊO �iotherExpr�j��DSolve�ň����Ȃ� *)
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



(* Piecewise�𕪉����ă��X�g�ɂ���D������False�Ȃ͍̂폜 *)
makeListFromPiecewise[minT_, others_] := Block[
  {tmpCondition = False},
  tmpCondition = Or @@ Map[(#[[2]])&, minT[[1]]];
  tmpCondition = Reduce[And[others, Not[tmpCondition]], Reals];
  If[ tmpCondition === False,
    minT[[1]],
    Append[minT[[1]], {minT[[2]], tmpCondition}]
  ]
];


(*
 * ���̃|�C���g�t�F�[�Y�Ɉڍs���鎞�������߂�
 *)
calcNextPointPhaseTime[includeZero_, maxTime_, posAsk_, negAsk_, NACons_, otherExpr_] := Block[
{
  calcMinTime, addMinTime, selectCondTime, removeInequality,
  sol, minT, paramVars, compareResult, resultList, condTimeList,
  timeMinCons = If[includeZero===True, (t>=0), (t>0)]
},
  calcMinTime[{currentMinT_, currentMinAsk_}, {type_, integAsk_, askID_}] := (
    If[integAsk=!=False,
      (* true *)
      (* ���Ȃ��Ƌ��E�l�̉�����ʂ��� *)  
      sol = Quiet[Check[Reduce[{timeMinCons && (maxTime>=t) && (integAsk) && (And @@ otherExpr)}, t],
                        errorSol,
                        {Reduce::nsmet}],
                  {Reduce::nsmet}];
      (*  debugPrint["calcMinTime#sol: ", sol]; *)
      If[sol=!=False && sol=!=errorSol, 
        (* true *)
        (* ���藧��t�̍ŏ��l�����߂� *)
        minT = First[Quiet[Minimize[{t, timeMinCons && (sol)}, {t}], 
                           Minimize::wksol]],

        (* false *)
        minT = error],
      
      (* false *)
      minT=0];

    debugPrint["calcMinTime#minT: ", minT];
    (* Piecewise�ւ̑Ή� *)
    If[Head[minT] === Piecewise, minT = First[First[First[minT]]]];
    (* 0�b��̂��܂�ł͂����Ȃ� *)
    If[includeZero===False && minT===0, minT=error];
    (* 0�b��̗��U�ω����s���邩�̃`�F�b�N�Ȃ̂�0�łȂ���΃G���[ *)
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
        (* �v���� *)
        (* TODO: �K�؂ȏ��������� *)
        1,
        Switch[First[First[Position[compareResult, x_ /; x =!= False, {1}, Heads -> False]]],
          1, {minT, {{type, askID}}},
          2, {minT, Append[currentMinAsk, {type, askID}]},
          3, {currentMinT, currentMinAsk}]
      ]
    ]
  );
  

  (* �����𖞂����ŏ��̎����ƁC���̏����̑g�����߂� *)
  (* maxT�͗��z�I�ɂ͖����Ă��\�����C�������������̂������ɂ����̂ƍ������������߂邩������Ȃ����ߒǉ� *)
  findMinTime[ask_, condition_, maxT_] := (
    sol = Quiet[Check[Reduce[ask&&condition&&t>0&&maxT>=t, t, Reals],
                      errorSol,
                      {Reduce::nsmet}],
                {Reduce::nsmet}];
    If[sol=!=False && sol=!=errorSol, 
      (* true *)
      (* ���藧��t�̍ŏ��l�����߂� *)
      minT = First[Quiet[Minimize[{t, sol}, {t}], 
                         Minimize::wksol]],
      (* false *)
      minT = error
    ];
    If[minT === error,
      {},
      (* Piecewise�Ȃ番��*)
      If[Head[minT] === Piecewise, minT = makeListFromPiecewise[minT, condition], minT = {{minT, {condition}}}];
      minT
    ]
  );
  
  (* �Q�̎����Ə����̑g���r���C�ŏ������Ƃ��̏����̑g�̃��X�g��Ԃ� *)
  compareMinTime[timeCond1_, timeCond2_] := ( Block[
      {
        sol, notSol,
        andCond, 
        currentMinTime
      },
      andCond = Reduce[timeCond1[[2]]&&timeCond2[[2]], Reals];
      If[andCond === False,
        {},
        sol = Quiet[Reduce[And[andCond,timeCond1[[1]] > timeCond2[[1]]], Reals]];
        If[ sol === False,
          {{timeCond1[[1]], andCond}},
          notSol = Reduce[andCond&&!sol];(* Implies���Ɗ��S�ɓ��^����Ȃ��Ɩ������ۂ��̂� *)
          If[ notSol === False,
            {{timeCond2[[1]], andCond}},
            {{timeCond1[[1]],  notSol}, {timeCond2[[1]], sol}}
          ]
        ]
      ]
    ]
    
  );
  
  (* �Q�̎����Ə����̑g�̃��X�g���r���C�e�����g�ݍ��킹�ɂ����āC�ŏ��ƂȂ鎞���Ə����̑g�̃��X�g��Ԃ� *)
  compareMinTimeList[{}, list2_] := {};
  compareMinTimeList[list1_, {}] := list1;
  compareMinTimeList[{h1_, t1___}, list2_] := ( Block[
      {
        resultList
      },
      resultList = Fold[(Join[#1, compareMinTime[h1, #2]])&,{}, list2];
      resultList = Join[resultList, compareMinTimeList[{t1}, list2]];
      resultList
    ]
  );

  (* �ŏ������Ə����̑g�����X�g�A�b�v����֐� *)
  calcMinTimeList[askList_, timeConditionList_, conditionForAll_, maxT_] := ( Block[
      {
        tmpList
      },
      If[askList === {},
        timeConditionList,
        tmpList = findMinTime[First[askList], (And @@ conditionForAll), maxT];
        tmpList = compareMinTimeList[timeConditionList, tmpList];
        tmpList = calcMinTimeList[Rest[askList], tmpList, conditionForAll, maxT];
        tmpList
        (*timeConditionList*)
      ]
    ]
  );
  
  (*  ���X�g����Inequality������ *)
  removeInequalityInList[{}] := {};
  removeInequalityInList[{h_,t___}] := ( Block[
      {
        resultList
      },
      If[Head[h] === Inequality,
        resultList = Join[{Reduce[h[[2]][h[[3]], h[[1]]], h[[3]]]},
             {h[[4]][h[[3]], h[[5]]]}
        ],
        If[h === True,(* ���ł�True������ *)
          resultList = {},
          resultList = {h}
        ];
      ];
      Join[resultList, removeInequalityInList[{t}]]
    ]
  );
  
  (* ���X�g�𐮌`����*)
  convertExpr[list_] := ( Block[
    {
      tmpList
    },
      tmpList = removeInequalityInList[list];
      tmpList = adjustExprs[tmpList];
      tmpList = Map[({removeP[#[[1]]], getExprCode[#], ToString[FullForm[#[[2]]]]})&, tmpList];
      tmpList
    ]
  );


  (* �]���́C�ŏ��������P���������邽�߂̏���*)
  (*resultList = Fold[calcMinTime,
       {maxTime, {}},
       Join[Map[({pos2neg, Not[#[[1]]], #[[2]]})&, posAsk],
            Map[({neg2pos,     #[[1]],  #[[2]]})&, negAsk],
            Fold[(Join[#1, Map[({neg2pos, #[[1]], #[[2]]})&, #2]])&, {}, NACons]]];*)

  (* �ŏ������Ə����̑g�̃��X�g�����߂� *)
  resultList = calcMinTimeList[ Join[Map[(Not[#[[1]]])&, posAsk], Map[(#[[1]])&, negAsk],
                                Fold[(Join[#1, Map[(#[[1]])&, #2]])&, {}, NACons]],
                                {{maxTime, And@@otherExpr}}, otherExpr, maxTime];

  (* �����Ə����̑g�ŁC�������_���a�łȂ����Ă���ꍇ���ꂼ��ɕ������� *)
  divideDisjunction[timeCond_] := Map[({timeCond[[1]], #})&, List@@timeCond[[2]]];

  (* ���`���Č��ʂ�Ԃ� *)
  
  resultList = Map[({#[[1]],LogicalExpand[#[[2]]]})&, resultList];
  resultList = Fold[(Join[#1, If[Head[#2[[2]]]===Or, divideDisjunction[#2], {#2}]])&,{}, resultList];
  resultList = Map[({#[[1]], applyList[#[[2]]]})&, resultList];
  
  resultList = Map[({ToString[FullForm[#[[1]]]], convertExpr[#[[2]]], If[#[[1]] === maxTime, 1, 0]})&, resultList];
  resultList
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

(* �p�����[�^����𓾂� *)
getParamCons[cons_] := Cases[cons, x_ /; Not[hasVariable[x]], {1}];
(* �����̃p�����[�^�ϐ��𓾂� *)
getParamVar[paramCons_] := Cases[paramCons, x_ /; Not[NumericQ[x]], Infinity];

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
    (*debugPrint["tmpIntegSol: ", tmpIntegSol, "paramVars", paramVars "paramCons", paramCons]; *)
    (* 1�����̗p *)
    (* TODO: ����������ꍇ���l���� *)
    If[Head[tmpIntegSol]===Or, tmpIntegSol = First[tmpIntegSol]];

    tmpIntegSol = removeTrivialCons[applyList[tmpIntegSol], Join[vars, paramVars]];

    {DExpr, DExprVars, NDExpr, otherExpr} = splitExprs[tmpIntegSol];
    (* debugPrint["DExpr ", DExpr, "DExprVars", DExprVars]; *)


    If[Cases[DExpr, Except[True]] === {},
      tmpIntegSol = {},
      tmpIntegSol = Quiet[DSolve[DExpr, DExprVars, t],
                        {Solve::incnst}];
      (* 1�����̗p *)
      (* TODO:����������ꍇ���l���� *)
      tmpIntegSol = First[tmpIntegSol]];
    (* tmpIntegSol�����NDExpr���ɏo������ND�ϐ��̈ꗗ�𓾂� *)
    solVars = getNDVars[Union[Cases[Join[tmpIntegSol, NDExpr], _[t], Infinity]]];
    (* integrateCalc�̌v�Z���ʂƂ��ĕK�v�ȕϐ��̈ꗗ *)
    returnVars = Select[vars, (MemberQ[solVars, removeDash[#]]) &];

    tmpIntegSol = First[Quiet[Solve[Join[Map[(Equal @@ #) &, tmpIntegSol], NDExpr], 
                              getNDVars[returnVars]], {Solve::incnst, Solve::ifun}]];
    
    (* DSolve�̌��ʂɂ́Cy'[t]�Ȃǔ����l�ɂ��Ẵ��[�����܂܂�Ă��Ȃ��̂�returnVars�S�Ăɑ΂��ă��[������� *)
    tmpIntegSol = Map[(# -> createIntegratedValue[#, tmpIntegSol])&, returnVars];
    debugPrint["tmpIntegSol", tmpIntegSol];

    tmpPosAsk = Map[(# /. tmpIntegSol) &, posAsk];
    tmpNegAsk = Map[(# /. tmpIntegSol) &, negAsk];
    tmpNACons = Map[(# /. tmpIntegSol) &, NACons];
    debugPrint["nextpointphase arg:", {False, maxTime, tmpPosAsk, tmpNegAsk, tmpNACons, paramCons}];
    tmpMinT = calcNextPointPhaseTime[False, maxTime, tmpPosAsk, tmpNegAsk, tmpNACons, paramCons];

    tmpVarMap = 
      Map[({getVariableName[#], 
            getDerivativeCount[#], 
            createIntegratedValue[#, tmpIntegSol] // FullForm // ToString})&, 
          returnVars],

    (* false *)
    debugPrint["nextpointphase arg: no next point phase"];
    debugPrint["tmpIntegSol", tmpIntegSol];
    tmpMinT = {{maxTime // FullForm // ToString, {}, 1}};
    tmpVarMap = {};
  ];
  tmpRet = {1,
            tmpVarMap,
            tmpMinT};
  debugPrint["tmpRet:", tmpRet];
  tmpRet
],
  (* debugPrint["MessageList:", $MessageList]; *)
  {0, $MessageList}
]];

(*
 * ���ɑ΂��ė^����ꂽ���Ԃ�K�p����
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

  debugPrint["cons:", cons, "vars:", vars]; 
 
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
             ToString[createIntegratedValue[#, sol[[1]]], InputForm]})&, 
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

(* 
 * �^����ꂽt�̎����^�C���V�t�g
 *)
exprTimeShift[expr_, time_] := ToString[FullForm[Simplify[expr /. t -> t - time ]]];
