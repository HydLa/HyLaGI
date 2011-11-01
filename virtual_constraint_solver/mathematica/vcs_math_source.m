(*
 * �f�o�b�O�p���b�Z�[�W�o��V��
 *)
If[optUseDebugPrint,
  debugPrint[arg___] := Print[InputForm[{arg}]],
  debugPrint[arg___] := Null];

(*
 * �O���[�o���ϐ�
 * constraints: ���݈����Ă��鐧��W���iIP�ł͕ϐ��ɂ��Ă̂݁j
 * pconstraints: IP�Ō��݈����Ă���萔�ɂ��Ă̏����̏W��
 * variables: ����W���ɏo������ϐ��̃��X�g�iPP�ł͒萔���܂ށj
 * parameters: ����W���ɏo������萔�̃��X�g
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
      Print["sol:", sol];
      If[sol === overconstraint,
        {2},
        If[sol === underconstraint || sol[[1]] === {} ,
          (* �x���o�����肵�������ǂ������H *)
          {1},
          tStore = sol[[1]];
          tStore = Map[(# -> createIntegratedValue[#, tStore])&, vars];
          otherExpr = Fold[(If[Head[#2] === And, Join[#1, List@@#2], Append[#1, #2]])&, {}, Simplify[expr]];
          otherExpr = Select[otherExpr, (MemberQ[{Or, Less, LessEqual, Greater, GreaterEqual, Inequality}, Head[#] ] || # === False)&];
          

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

removeP[par_] := First[StringCases[ToString[par], StartOfString ~~ "p" ~~ x__ -> x]];

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
                        adjustExprs[exprs] ],
                  {invalidVar, _, _}]
    ]
  );
  debugPrint["@convertCSToVM constraints:", constraints];
  constraints = LogicalExpand[constraints];
  If[Head[constraints] === Or,
    resultExprs = Map[(applyList[#])&, List@@constraints],
    resultExprs = Map[(applyList[#])&, {constraints}]
  ];
  resultExprs = Map[formatExprs, resultExprs];
  debugPrint["@convertCSToVM resultExprs:", resultExprs];
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

(* �����萔���̂��̂��ۂ� *)
isParameter[exprs_] := StringMatchQ[ToString[exprs], "p" ~~ __];



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
            If[isParameter[#2[[2]]],
              (* true *)
              (* �t�ɂȂ��Ă�̂ŁA���Z�q���t�ɂ��Ēǉ����� *)
              Append[#1, getInverseRelop[Head[#2]][#2[[2]], #2[[1]]]],
              (* false *)
              Append[#1, #2]]],
          (* false *)
          Append[#1, #2]]) &,
       {}, andExprs];





resetConstraint[] := (
  constraints = True;
  pconstraints = True;
  variables = {};
  parameters = {};
);

addConstraint[cons_, vars_] := (
  debugPrint["@addConstraint cons:", cons, "vars:", vars];
  constraints = constraints && cons;
  variables = Union[variables, vars];
  debugPrint["@addConstraint constraints:", constraints, "variables:", variables];
);

checkConsistency[] := Block[
  {sol},
  debugPrint["@checkConsistency constraints:", constraints, "variables:", variables];
  sol = checkConsistencyByReduce[constraints, variables];
  If[sol[[1]] == 1, constraints = sol[[2]]; sol = {1}  ];
  sol
];

checkConsistencyWithTemporaryConstraint[expr_, vars_] := (
  debugPrint["@checkConsistencyWTC constraints:", constraints, "variables:", variables, "expr:", expr, "vars", vars];
  { checkConsistencyByReduce[constraints && expr, Union[variables, vars] ] [[1]] }
);

checkConsistencyByReduce[expr_, vars_] := 
Quiet[
  Check[
    Block[
      {sol},
      sol = Reduce[expr, vars, Reals];
      If[sol === False,
        {2},
        {1, sol}
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


(* Piecewise�̑��v�f���C���̏����ƂƂ��ɑ��v�f�ɕt�����ă��X�g�ɂ���D������False�Ȃ�폜 *)
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
calculateNextPointPhaseTime[includeZero_, maxTime_, discCause_, otherExpr_] := Block[
{
  addMinTime, selectCondTime,
  sol, minT, paramVars, compareResult, resultList, condTimeList,
  calculateMinTimeList, convertExpr, removeInequalityInList, findMinTime, compareMinTime,
  compareMinTimeList, divideDisjunction
},
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
      (* ������0�ƂȂ�ꍇ����菜���D���S�̂��߂ɂ����������ǂ����C���z�I�ɂ͖����Ă������͂��H *)
      minT = Select[minT, (#[[1]] =!= 0)&];
      minT
    ]
  );
  
  (* �Q�̎����Ə����̑g���r���C�ŏ������Ƃ��̏����̑g�̃��X�g��Ԃ� *)
  compareMinTime[timeCond1_, timeCond2_] := ( Block[
      {
        case1, case2,
        andCond
      },
      andCond = Reduce[timeCond1[[2]]&&timeCond2[[2]], Reals];
      If[andCond === False, Return[{}] ];
      case1 = Quiet[Reduce[And[andCond,timeCond1[[1]] < timeCond2[[1]]], Reals]];
      If[ case1 === False, Return[{{timeCond2[[1]], andCond}} ] ];
      case2 = Reduce[andCond&&!case1];
      If[ case2 === False, Return[{{timeCond1[[1]], andCond}} ] ];
      Return[ {{timeCond2[[1]],  case2}, {timeCond1[[1]], case1}} ];
    ]
  );
  
  (* �Q�̎����Ə����̑g�̃��X�g���r���C�e�����g�ݍ��킹�ɂ����āC�ŏ��ƂȂ鎞���Ə����̑g�̃��X�g��Ԃ� *)
  compareMinTimeList[list1_, list2_] := ( Block[
      {resultList, i, j},
      If[list2 === {}, Return[list1] ];
      resultList = {};
      For[i = 1, i <= Length[list1], i++,
        For[j = 1, j <= Length[list2], j++,
          resultList = Join[resultList, compareMinTime[list1[[i]], list2[[j]] ] ]
        ]
      ];
      resultList
    ]
  );

  
  (* �ŏ������Ə����̑g�����X�g�A�b�v����֐� *)
  calculateMinTimeList[guardList_, condition_, maxT_] := (
    Block[
      {findResult, i},
      timeCaseList = {{maxT, And@@condition}};
      For[i = 1, i <= Length[guardList], i++,
        findResult = findMinTime[guardList[[i]], (And @@ condition), maxT];
        timeCaseList = compareMinTimeList[timeCaseList, findResult]
      ];
      timeCaseList
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
  
  (* �ŏ������Ə����̑g�̃��X�g�����߂� *)
  resultList = calculateMinTimeList[discCause, otherExpr, maxTime];

  (* ���`���Č��ʂ�Ԃ� *)
  resultList = Map[({#[[1]],LogicalExpand[#[[2]]]})&, resultList];
  resultList = Fold[(Join[#1, If[Head[#2[[2]]]===Or, divideDisjunction[#2], {#2}]])&,{}, resultList];
  resultList = Map[({#[[1]], applyList[#[[2]]]})&, resultList];
  resultList = Map[({ToString[FullForm[#[[1]]]], convertExpr[#[[2]]], If[Simplify[#[[1]]] === Simplify[maxTime], 1, 0]})&, resultList];
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
  debugPrint["@Integrate cons:", cons, 
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

     debugPrint["@Integrate tmpIntegSol before Solve: ", tmpIntegSol];
     debugPrint["@Integrate NDExpr before Solve: ", NDExpr];
     debugPrint["@Integrate returnVars before Solve: ", returnVars];

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

     debugPrint["@Integrate tmpIntegSol after Solve: ", tmpIntegSol];
    
    (* DSolve�̌��ʂɂ́Cy'[t]�Ȃǔ����l�ɂ��Ẵ��[�����܂܂�Ă��Ȃ��̂�returnVars�S�Ăɑ΂��ă��[������� *)
    tmpIntegSol = Map[(# -> createIntegratedValue[#, tmpIntegSol])&, returnVars];
    debugPrint["@Integrate tmpIntegSol", tmpIntegSol];

    tmpDiscCause = Map[(# /. tmpIntegSol) &, discCause];
    
    paramCons = {Reduce[paramCons]};
    
    debugPrint["@Integrate nextpointphase arg:", {False, maxTime, tmpDiscCause, paramCons}];
    tmpMinT = calculateNextPointPhaseTime[False, maxTime, tmpDiscCause, paramCons];

    tmpVarMap = 
      Map[({getVariableName[#], 
            getDerivativeCount[#], 
            createIntegratedValue[#, tmpIntegSol] // FullForm // ToString})&, 
          returnVars],

    (* false *)
    debugPrint["@Integrate tmpIntegSol before Solve: false"];
    debugPrint["@Integrate NDExpr before Solve: false"];
    debugPrint["@Integrate returnVars before Solve: false"];
    debugPrint["@Integrate tmpIntegSol after Solve: false"];
    debugPrint["@Integrate nextpointphase arg: no next point phase"];
    debugPrint["@Integrate tmpIntegSol", tmpIntegSol];
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
