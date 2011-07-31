(*
 * �f�o�b�O�p���b�Z�[�W�o��V��
 *)
If[optUseDebugPrint,
  debugPrint[arg___] := Print[InputForm[{arg}]],
  debugPrint[arg___] := Null];

(*
 * �O���[�o���ϐ�
 * constraints: ���݈����Ă��鐧��W��
 * vars: ����W���ɏo������ϐ��̃��X�g
 * pars: ����W���ɏo������萔�̃��X�g
 *)

(* ���[���̃��X�g���󂯂āCt�ɂ��Ẵ��[�������������̂�Ԃ��֐� *)
removeRuleForTime[ruleList_] := DeleteCases[ruleList, t -> _];

getInverseRelop[relop_] := Switch[relop,
                                  Equal, Equal,
                                  Less, Greater,
                                  Greater, Less,
                                  LessEqual, GreaterEqual,
                                  GreaterEqual, LessEqual];

(* Inequality[a, relop, x, relop, b]�̌`��ό`����֐� *)
removeInequality[ret_, expr_] := (
  If[Head[expr] === Inequality,
    Join[ret,
         {Reduce[expr[[2]][expr[[1]], expr[[3]]], expr[[3]]]},
         {expr[[4]][expr[[3]], expr[[5]]]}
    ],
    Append[ret, expr]
  ]
);

(* ����1�������2�������C�ϐ��\�`���ɂ��ĕԂ� *)
getComplementCondition[cond1_, cond2_] := (
  debugPrint["cond1:", cond1, "cond2:", cond2];
  convertCSToVM[Complement[applyList[cond1[[1]] ], applyList[cond2[[1]] ] ] ]
);

(* �^����ꂽ���̒��́CC�Ɋւ��鐧��ƁCt�̉����ƂȂ鎮�𔲂��o���֐��D
 * �S��And�łȂ���Ă���̂Ƃ����O��D
 * Inequality�͖����D
 * IP�Ȃ̂�t�Ɋւ��铙������������D����C��A���I�Ȓl�Ƃ���t�Ƃ̓����ɂ��Ă����ꍇ�͑ʖڂȋC�����邯�ǁC��������͖����͂��D
 * ���ƁCC��t�Ɋւ��鐧��́C���ꂼ��ɂ��ĉ�����Ă���Ƃ����O��D�������͌��\���������D
 *)

getValidExpression[expr_, pars_] := Block[
  {underBounds, constraintsForC, listOfC, tmpExpr, parameterConstraints},
  underBounds = {};
  constraintsForC = {};
  parameterConstraints = {};
  listOfC = {};
  For[i=1, i <= Length[expr], i++,
    tmpExpr = expr[[i]];
    (* �܂��C�s�����͑S��Less��������LessEqual�ɂ���D *)
    If[Head[tmpExpr] === Greater || Head[tmpExpr] === GreaterEqual,
      tmpExpr = getInverseRelop[Head[tmpExpr] ] [tmpExpr[[2]], tmpExpr[[1]] ]
    ];
    (* �E�ӂ�t�Ȃ牺�����X�g�ɒǉ��D�����łȂ��Ȃ�萔���C�����łȂ��Ȃ�C�ɂ��Ă̐��񂩂��`�F�b�N���� *)
    If[tmpExpr[[2]] === t,
      (* t�ɂ��Ă͕s�����݂̂��󂯕t���� *)
      If[Head[tmpExpr] === Less || Head[tmpExpr] === LessEqual,
        underBounds = Append[underBounds, tmpExpr[[1]] ]
      ],
      If[MemberQ[pars, tmpExpr[[1]] ] || MemberQ[pars, tmpExpr[[2]] ], 
        parameterConstraints = Append[parameterConstraints, tmpExpr],
        If[Head[ tmpExpr[[1]] ] === C,
          constraintsForC = Append[constraintsForC, tmpExpr];
          listOfC = Union[listOfC, {tmpExpr[[1]] } ],
          If[Head[ tmpExpr[[2]] ] === C,
            constraintsForC = Append[constraintsForC, tmpExpr];
            listOfC = Union[listOfC, {tmpExpr[[2]] } ]
          ]
        ]
      ]
    ]
  ];
  {underBounds, constraintsForC, listOfC, parameterConstraints}
];

(*
 * �_���ςłȂ��ꂽ�et�̉������X�g���C�ċA��1�����ׂĂ����֐��D
 * 1�ł��萔�l�Ɋ֌W�Ȃ�0�ȉ��ɂł��Ȃ����̂������2��Ԃ��D
 * �����łȂ��C1�ł��萔�l�ɂ���Ă�0�ȉ��ɂł��Ȃ����̂������3��Ԃ��D
 * ������ł��Ȃ��Ȃ�1��Ԃ��D
 *)
checkInfForEach[underBounds_, constraintsForC_, otherConstraints_, pars_, listOfC_] := Block[
  {ret, restRet, tmp, expr},
  If[underBounds === {},
    {1},
    expr = underBounds[[1]];
    If[pars==={}&&listOfC==={},
      (* �p�����[�^�������Ȃ�P���ɉ����Ō��܂� *)
      If[expr>0,
        {2},
        {1}
      ],
      (* 0�ȉ��ɂł���Ȃ�C���o�ł���\�������� *)
      tmp = Quiet[Reduce[Join[constraintsForC, otherConstraints, {expr <= 0}] , pars ] ];
      If[tmp === False,
        {2},
        (* �K�����o�ł��邩�ǂ����𔻒� *)
        If[Quiet[Reduce[ForAll[pars, And@@otherConstraints, Reduce[Join[constraintsForC, {expr <= 0}] ] ] ] ] =!= False,
          checkInfForEach[Rest[underBounds], constraintsForC, otherConstraints, pars, listOfC],
          restRet = checkInfForEach[Rest[underBounds], constraintsForC, otherConstraints, pars, listOfC];
          Switch[restRet[[1]],
            1, {3, tmp},
            2, {2},
            3, {3, Reduce[restRet[[2]]&&tmp, Reals]}
          ]
        ]
      ]
    ]
  ]
];

(*
 * �_���a�łȂ��ꂽ�et�̉��������C�ċA��1�����ׂĂ����֐��D
 * 1�ł��萔�l�Ɋ֌W�Ȃ�0�ȉ��ɂł�����̂������{1}��Ԃ��D
 * �����łȂ��Ȃ�C1�ł��萔�l�ɂ���Ă�0�ȉ��ɂł�����̂������{3}��Ԃ��D
 * ������ł��Ȃ��Ȃ�{2}��Ԃ��D
 *)

checkInf[candidates_, constraints_, pars_] := Block[
  {ret, restRet, underBounds, constraintsForC, listOfC},
  If[Length[candidates] == 0,
    {2},
    ret = Fold[removeInequality, {}, candidates[[1]] ];
    (* t�̍ŏ��l���𒲂ׂ邽�߁Ct�̉�����t�ȊO�̕ϐ��ɂ��Ă̐���Əo������ϐ��̃��X�g�𒊏o����D *)
    {underBounds, constraintsForC, listOfC, otherConstraints} = getValidExpression[ret, pars];
    otherConstraints = applyList[Reduce[Join[otherConstraints, constraints], Reals]];
    otherConstraints = Fold[removeInequality, {}, otherConstraints ];

    If[underBounds === {},
      (* ��₪���݂��Ȃ���Γ��o�s�\�Ȃ̂ŁC�������� *)
      checkInf[Rest[candidates], constraints, pars],
      (* ��₪���݂���Ȃ�C����������D*)
      ret = checkInfForEach[underBounds, constraintsForC, otherConstraints, pars, listOfC ];
      If[ret[[1]] != 2 && Quiet[Reduce[ForAll[pars, And@@constraints, Reduce[otherConstraints] ] ] ] === False,
        If[ret[[1]] == 1,
          ret = {3, And@@otherConstraints},
          ret = {3, Reduce[ret[[2]] && And@@otherConstraints, Reals]}
        ]
      ];
      Switch[ret[[1]],
        1, {1},
          (* �����Ȃ�C���������D *)
        2, checkInf[Rest[candidates], constraints, pars ],
        3,
          (* �萔�l�ɂ���Ă͉\�ȏꍇ�C���ȍ~��{1}�������{1}���D�����łȂ����{3}��Ԃ��D *)
          restRet = checkInf[Rest[candidates], constraints, pars];
          Switch[restRet[[1]],
            1, {1},
            2, {3, ret[[2]] },
            3, {3, Reduce[ ret[[2]] || restRet[[2]], Reals ] }
          ]
      ]
    ]
  ]
];


(*
 * �߂�l�̃��X�g�̐擪�F
 *  0 : Solver Error
 *  1 : �[��
 *  2 : ����
 *)

checkConsistencyInterval[expr_, pexpr_, vars_, pars_] :=  
Quiet[
  Check[
    Block[
      {tStore, sol, integGuard, otherExpr, condition},
      debugPrint["expr:", expr, "pexpr", pexpr, "vars:", vars, "pars:", pars, "all", expr, pexpr, vars, pars];
      sol = exDSolve[expr, vars];
      If[sol === overconstraint,
        {2},
        If[sol === underconstraint || sol[[1]] === {} ,
          (* �x���o�����肵�������ǂ������H *)
          {1},
          tStore = sol[[1]];
          tStore = Map[(# -> createIntegratedValue[#, tStore])&, vars];
          otherExpr = Fold[(If[Head[#2] === And, Join[#1, List@@#2], Append[#1, #2]])&, {},Simplify[expr]];
          otherExpr = Select[otherExpr, (MemberQ[{Or, Less, LessEqual, Greater, GreaterEqual}, Head[#] ] || # === False)&];


          (* otherExpr��tStore��K�p���� *)
          otherExpr = otherExpr /. tStore;
          (* �܂��Ct>0�ŏ����𖞂����\�������邩�𒲂ׂ� *)
          sol = LogicalExpand[Quiet[Check[Reduce[{And@@otherExpr && t > 0 && And@@pexpr}, t, Reals],
                        False, {Reduce::nsmet}], {Reduce::nsmet}]];

          If[sol === False,
            {2},
            (* ���X�g�̃��X�g�ɂ��� *)
            sol = applyListToOr[sol];
            sol = Map[applyList, sol];
            (* t�̍ő剺�E��0�Ƃł���\���𒲂ׂ�D *)
            sol = checkInf[sol, pexpr, pars];
            If[sol[[1]] == 3, sol[[2]] = ToString[FullForm[sol[[2]] ] ] ];
            sol
          ]
        ]
      ]
    ],
    {0, $MessageList}
  ]
];

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
      (* �萔��prev_flag��-1�ɂ��邱�Ƃŕ\�� *)
      {renamedVarName, derivativeCount, -1},
      invalidVar
    ]
  ]
];

(* �ϐ��Ƃ��̒l�Ɋւ��鎮�̃��X�g���A�ϐ��\�I�`���ɕϊ� *)
getExprCode[expr_] := Switch[Head[expr],
  Equal, 0,
  Less, 1,
  Greater, 2,
  LessEqual, 3,
  GreaterEqual, 4
];

convertCSToVM[] := Block[
  {resultExprs, inequalityVariable},
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

  debugPrint["constraints:", constraints];
  If[Head[First[constraints]] === Or,
    resultExprs = Map[(applyList[#])&, List@@First[constraints]],
    resultExprs = Map[(applyList[#])&, constraints]
  ];
  resultExprs = Map[formatExprs, resultExprs];
  debugPrint["resultExprs:", resultExprs];
  resultExprs
];

(* checkConsistency����Reduce�̌��ʓ���ꂽ����{�ϐ���, �l}�̃��X�g�`���ɂ��� *)
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


resetConstraint := (
  constraints = {True};
  variables = {};
);

addConstraint[cons_, vars_] := (
  debugPrint["cons:", cons, "vars:", vars];
  constraints = Union[constraints, cons];
  variables = Union[variables, vars];
  debugPrint["constraints:", constraints, "variables:", variables];
);

checkConsistency[] := Block[
  {sol},
  debugPrint["constraints:", constraints, "variables:", variables];
  sol = checkConsistencyByReduce[constraints, variables];
  If[sol[[1]] == 1, constraints = sol[[2]]; sol = {1}  ];
  sol
];

checkConsistencyWithTemporaryConstraint[expr_, vars_] := (
  debugPrint["constraints:", constraints, "variables:", variables, "expr:", expr, "vars", vars];
  { checkConsistencyByReduce[Union[constraints, expr], Union[variables, vars] ] [[1]] }
);

checkConsistencyByReduce[expr_, vars_] := 
Quiet[
  Check[
    Block[
      {sol},
      sol = Reduce[expr, vars, Reals];
    If[sol === False,
      {2},
      {1, {sol}}
      ]
    ],
    {0, $MessageList}
  ]
];

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

(* ����ϐ�x�ɂ��āAx[t]==a �� x[0]==a ���������ꍇ�́Ax'[t]==0������ �i����΁j *)
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
  
(* Or�ł͂Ȃ�List�ł����� *)
applyListToOr[reduceSol_] :=
  If[Head[reduceSol] === Or, List @@ reduceSol, List[reduceSol]];

exDSolve[expr_, vars_] := Block[
{sol, DExpr, DExprVars, NDExpr, otherExpr, paramCons},
  paramCons = getParamCons[expr];
  sol = LogicalExpand[removeNotEqual[Reduce[Cases[Complement[expr, paramCons],Except[True]], vars, Reals]]];

  If[sol===False || Reduce[expr, vars, Reals] === False,
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

      Quiet[
        Check[
          Check[
            
            If[Cases[DExpr, Except[True]] === {},
              (* �X�g�A����̏ꍇ��DSolve�������Ȃ��̂ŋ�W����Ԃ� *)
              sol = {},
              sol = DSolve[DExpr, DExprVars, t];
              (* 1�����̗p *)
              (* TODO: ����������ꍇ���l���� *)
              sol = First[sol]
            ];

            sol = Quiet[Solve[Join[Map[(Equal @@ #) &, sol], NDExpr], getNDVars[vars]]];
            If[sol =!= {},
              {First[sol], Join[otherExpr, paramCons]},
              overconstraint
            ],
            underconstraint,
            {DSolve::underdet, Solve::svars, DSolve::deqx, 
             DSolve::bvnr, DSolve::bvsing}],
                  overconstraint,
                  {DSolve::overdet, DSolve::bvnul, DSolve::dsmsm}],
            {DSolve::underdet, DSolve::overdet, DSolve::deqx, 
             Solve::svars, DSolve::bvnr, DSolve::bvsing, 
             DSolve::bvnul, DSolve::dsmsm, Solve::incnst}
          ]
        ]
      ]
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
calcNextPointPhaseTime[includeZero_, maxTime_, discCause_, otherExpr_] := Block[
{
  calcMinTime, addMinTime, selectCondTime,
  sol, minT, paramVars, compareResult, resultList, condTimeList,
  calcMinTimeList, convertExpr, removeInequalityInList, findMinTime, compareMinTime,
  compareMinTimeList, divideDisjunction, 
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
      If[Head[minT] === Piecewise, minT = makeListFromPiecewise[minT, condition], minT = {{minT, condition}}];
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
  
  
  (* �����Ə����̑g�ŁC�������_���a�łȂ����Ă���ꍇ���ꂼ��ɕ������� *)
  divideDisjunction[timeCond_] := Map[({timeCond[[1]], #})&, List@@timeCond[[2]]];


  (* �]���́C�ŏ��������P���������邽�߂̏���*)
  (*resultList = Fold[calcMinTime,
       {maxTime, {}},
       Join[Map[({pos2neg, Not[#[[1]]], #[[2]]})&, posAsk],
            Map[({neg2pos,     #[[1]],  #[[2]]})&, negAsk],
            Fold[(Join[#1, Map[({neg2pos, #[[1]], #[[2]]})&, #2]])&, {}, NACons]]];*)
            

  (* �ŏ������Ə����̑g�̃��X�g�����߂� *)
  resultList = calcMinTimeList[discCause, {{maxTime, And@@otherExpr}}, otherExpr, maxTime];

  (* ���`���Č��ʂ�Ԃ� *)
  
  resultList = Map[({#[[1]],LogicalExpand[#[[2]]]})&, resultList];
  resultList = Fold[(Join[#1, If[Head[#2[[2]]]===Or, divideDisjunction[#2], {#2}]])&,{}, resultList];
  resultList = Map[({#[[1]], applyList[#[[2]]]})&, resultList];
  
  resultList = Map[({ToString[FullForm[#[[1]]]], convertExpr[#[[2]]], If[#[[1]] === maxTime, 1, 0]})&, resultList];
  resultList
];


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
              discCause_,
              vars_, 
              maxTime_] := Quiet[Check[Block[
{
  tmpIntegSol,
  tmpDiscCause,
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
},
  debugPrint["cons:", cons, 
             "discCause:", discCause,
             "vars:", vars, 
             "maxTime:", maxTime,
             "all", cons, discCause, vars, maxTime];

  If[Cases[cons, Except[True]]=!={},
    (* true *)
    paramCons = getParamCons[cons];
    paramVars = Union[Fold[(Join[#1, getParamVar[#2]]) &, {}, paramCons]];
    tmpIntegSol = LogicalExpand[removeNotEqual[Reduce[Complement[cons, paramCons], vars, Reals]]];
    (* 1�����̗p *)
    (* TODO: ����������ꍇ���l���� *)
    If[Head[tmpIntegSol]===Or, tmpIntegSol = First[tmpIntegSol]];

    tmpIntegSol = removeTrivialCons[applyList[tmpIntegSol], Join[vars, paramVars]];

    {DExpr, DExprVars, NDExpr, otherExpr} = splitExprs[tmpIntegSol];


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

     debugPrint["tmpIntegSol before Solve: ", tmpIntegSol];
     debugPrint["NDExpr before Solve: ", NDExpr];
     debugPrint["returnVars before Solve: ", returnVars];

     (*improve by takeguchi*)
     tmpIntegSol = If[Length[NDExpr] == 0, tmpIntegSol,
          First[FullSimplify[ExpToTrig[Quiet[
              Solve[Join[Map[(Equal @@ #) &, tmpIntegSol], TrigToExp[NDExpr]], getNDVars[returnVars]],
          {Solve::incnst, Solve::ifun, Solve::svars}]]]]
     ];
    
     (*before improve :*)
     (*     
        tmpIntegSol=First[Quiet[Solve[Join[Map[(Equal@@#)&,tmpIntegSol],
                 NDExpr],getNDVars[returnVars]],{Solve::incnst,Solve::ifun}]];
     *)

     debugPrint["tmpIntegSol after Solve: ", tmpIntegSol];
    
    (* DSolve�̌��ʂɂ́Cy'[t]�Ȃǔ����l�ɂ��Ẵ��[�����܂܂�Ă��Ȃ��̂�returnVars�S�Ăɑ΂��ă��[������� *)
    tmpIntegSol = Map[(# -> createIntegratedValue[#, tmpIntegSol])&, returnVars];
    debugPrint["tmpIntegSol", tmpIntegSol];

    tmpDiscCause = Map[(# /. tmpIntegSol) &, discCause];
    
    paramCons = {Reduce[paramCons]};
    
    debugPrint["nextpointphase arg:", {False, maxTime, tmpDiscCause, paramCons}];
    tmpMinT = calcNextPointPhaseTime[False, maxTime, tmpDiscCause, paramCons];

    tmpVarMap = 
      Map[({getVariableName[#], 
            getDerivativeCount[#], 
            createIntegratedValue[#, tmpIntegSol] // FullForm // ToString})&, 
          returnVars],

    (* false *)
    debugPrint["tmpIntegSol before Solve: false"];
    debugPrint["NDExpr before Solve: false"];
    debugPrint["returnVars before Solve: false"];
    debugPrint["tmpIntegSol after Solve: false"];
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
