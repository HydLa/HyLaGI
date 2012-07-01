(* �ċA�񐔏�����グ�Ă݂� *)
$RecursionLimit = 1000;

(* �����ŗp���鐸�x���グ�Ă݂� *)
$MaxExtraPrecision = 1000;


(*
 * �f�o�b�O�p���b�Z�[�W�o�͊֐�
 *)
 
SetAttributes[simplePrint, HoldAll];

symbolToString := (StringJoin[ToString[Unevaluated[#] ], ": ", ToString[InputForm[Evaluate[#] ] ] ])&;

SetAttributes[symbolToString, HoldAll];

If[optUseDebugPrint,
  debugPrint[arg___] := Print[InputForm[{arg}]];
  simplePrint[arg___] := Print[delimiterAddedString[", ",
    List@@Map[symbolToString, Map[Unevaluated, Hold[arg]] ]
     ] ],
  
  debugPrint[arg___] := Null;
  simplePrint[arg___] := Null
];


(*
 * �֐��Ăяo�����Č����邽�߂̕�����o�͂��s��
 *)
 
inputPrint[name_, arg___] := Print[StringJoin[name, "[", delimiterAddedString[",", Map[(ToString[InputForm[#] ])&,{arg}] ], "]" ] ];


delimiterAddedString[del_, {h_}] := h;

delimiterAddedString[del_, {h_, t__}] := StringJoin[h, del, delimiterAddedString[del, {t}] ];


(*
 * �O���[�o���ϐ�
 * constraint: ���݂̃t�F�[�Y�ł̐���
 * pConstraint: �萔�ɂ��Ă̐���
 * prevConstraint: ���Ɍ��l��ݒ肷�鐧��D
 * variables: ����ɏo������ϐ��̃��X�g
 * parameters: �L���萔�̃��X�g
 * isTemporary�F����̒ǉ����ꎞ�I�Ȃ��̂Ƃ��邩�D
 * tmpConstraint: �ꎞ�I�ɒǉ����ꂽ����
 * tmpVariables: �ꎞ����ɏo������ϐ��̃��X�g
 * guard:
 * guardVars:
 *)


(* �i�s�j�����̉E�ӂƍ��ӂ����ւ���ۂɁC�֌W���Z�q�̌��������]������DNot�Ƃ͈Ⴄ *)

getReverseRelop[relop_] := Switch[relop,
                                  Equal, Equal,
                                  Less, Greater,
                                  Greater, Less,
                                  LessEqual, GreaterEqual,
                                  GreaterEqual, LessEqual];


(* �|�C���g�t�F�[�Y�ɂ����閳���������� *)

checkConsistencyPoint[] := (
  checkConsistencyPoint[constraint && tmpConstraint && guard, pConstraint, Union[variables, tmpVariables, guardVars]]
);

checkConsistencyPoint[cons_, pcons_, vars_] := (
  Check[
    Block[
      {trueMap, falseMap, cpTrue, cpFalse},
      inputPrint["checkConsistencyPoint", cons, pcons, vars];
      Quiet[
        cpTrue = Reduce[Exists[vars, cons && pcons], Reals], {Reduce::useq}
      ];
      simplePrint[cpTrue];
      Quiet[
        cpFalse = Reduce[pcons && !cpTrue, Reals], {Reduce::useq}
      ];
      simplePrint[cpFalse];
      {trueMap, falseMap} = Map[(createMap[#, isParameter, hasParameter, {}])&, {cpTrue, cpFalse}];
      simplePrint[trueMap, falseMap];
      {1, {trueMap, falseMap}}
    ],
    debugPrint[$MessageList]; {0}
  ]
);

(* �C���^�[�o���t�F�[�Y�ɂ����閳���������� *)

checkConsistencyInterval[] :=  (
  checkConsistencyInterval[constraint && tmpConstraint, pConstraint, guard, guardVars, Union[variables, tmpVariables]]
);

appendZeroVars[vars_] := Join[vars, vars /. x_[t] -> x[0]];

(* TODO ��������������������C���̌��ʂ��ė��p���� *)

checkConsistencyInterval[cons_, pcons_, gua_, gVars_, vars_] :=  
Check[
  Block[
    {tStore, sol, otherCons, originalOther, tCons, dVars, otherVars, i, cpTrue, cpFalse, trueMap, falseMap},
    inputPrint["checkConsistencyInterval", cons, pcons, gua, gVars, vars];
    sol = exDSolve[cons, vars];
    debugPrint["sol after exDSolve", sol];
    If[sol[[1]] === overConstraint,
      Return[{1, {False, pcons}}]
    ];
    
    If[sol[[1]] === underConstraint,
      (* ����s���Ŕ����������������Ȃ��ꍇ�́C�P���Ɋe�ϐ��l����т��̔����l���������Ȃ����𒲂ׂ� *)
      tStore = {};
      otherVars = vars;
      otherCons = cons && gua,
      
      (* �������������������ꍇ�́C�����������������̂Ɏg���Ȃ��������Ƃ̐������𒲂ׂ� *)
      originalOther = And[And@@sol[[2]], gua];
      otherCons = False;
      dVars = sol[[3]];
      otherVars = sol[[4]];
      For[i = 1, i <= Length[sol[[1]] ], i++,
        tStore = Map[(# -> createIntegratedValue[#, sol[[1]] [[i]]])&, dVars];
        otherCons = Or[otherCons, originalOther /. tStore ]
      ]
    ];
    
    simplePrint[tStore, otherCons];
    
    (* Exists�̑�������Hold�iHoldAll?�j�����������Ă���炵���̂ŁCEvaluate�ŕ]������K�v������i�C������j *)
    tCons = Reduce[Exists[Evaluate[Union[appendZeroVars[otherVars], gVars]], otherCons && pcons], Reals];

    simplePrint[tCons];

    If[tCons === False, Return[{1, {False, pcons}}] ];
    
    cpTrue = Reduce[Quiet[Minimize[{t, tCons && t > 0}, t], {Minimize::wksol, Minimize::infeas}][[1]] == 0, Reals];        
    cpFalse = Reduce[pcons && !cpTrue, Reals];

    simplePrint[cpTrue, cpFalse];

    {trueMap, falseMap} = Map[(createMap[#, isParameter,hasParameter, {}])&, {cpTrue, cpFalse}];
    trueMap = Map[(Cases[#, Except[{{prev[_, _], _}, _, _}] ])&, trueMap];
    falseMap = Map[(Cases[#, Except[{{prev[_, _], _}, _, _}] ])&, falseMap];
    simplePrint[trueMap, falseMap];
    {1, {trueMap, falseMap}}
  ],
  debugPrint[$MessageList]; {0}
]


(* �ϐ��������͋L���萔�Ƃ��̒l�Ɋւ��鎮�̃��X�g���C�\�`���ɕϊ� *)



createVariableMap[] := createVariableMap[constraint && pConstraint, variables];


createVariableMap[cons_, vars_] :=  
Check[
  Block[
    {ret},
    inputPrint["createVariableMap", cons, vars];
    ret = createMap[cons, isVariable, hasVariable, vars];
    debugPrint["ret after CreateMap", ret];
    ret = Map[(Cases[#, Except[{parameter[___], _, _}] ])&, ret];
    ret = ruleOutException[ret];
    simplePrint[ret];
    {1, ret}
  ],
  debugPrint[$MessageList]; {0}
];


createVariableMapInterval[] := createVariableMapInterval[constraint, variables, parameters];

createVariableMapInterval[cons_, vars_, pars_] := 
Check[
  Block[
  {sol, originalOther, otherCons, dVars, tStore, cStore, i, ret},
  inputPrint["createVariableMapInterval", cons, vars, pars];
  sol = exDSolve[cons, vars];
  debugPrint["sol after exDSolve", sol];
  (* �������������������ꍇ�́C�����������������̂Ɏg���Ȃ��������Ƃ̐������𒲂ׂ� *)
  originalOther = And@@sol[[2]];
  cStore = False;
  dVars = sol[[3]];
  otherVars = sol[[4]];
  For[i = 1, i <= Length[sol[[1]] ], i++,
    tStore = Map[(# -> createIntegratedValue[#, sol[[1]] [[i]]])&, dVars];
    cStore = Or[cStore, (originalOther /. tStore) && And@@Map[(Equal@@#)&, tStore] ]
  ];
  simplePrint[cStore];
  ret = createMap[cStore && t>0, isVariable, hasVariable, vars];
  debugPrint["ret after CreateMap", ret];
  ret = ruleOutException[ret];
  simplePrint[ret];
  {1, ret}
  ],
  debugPrint[$MessageList]; {0}
];


ruleOutException[list_] := Block[
  {ret},
  ret = Map[(Cases[#, {{_?isVariable, _}, _, _} ])&, list];
  ret = Map[(Cases[#, Except[{{t, 0}, _, _}] ])&, ret];
  ret = Map[(Cases[#, Except[{{prev[_, _], _}, _, _}] ])&, ret];
  ret
];


createParameterMap[] := createMap[pConstraint, isParameter, hasParameter, {}];

createMap[cons_, judge_, hasJudge_, vars_] := Block[
  {map},
  inputPrint["createMap", cons, judge, vars];
  If[cons === True || cons === False, Return[cons]];
  
  map = Reduce[Exists[Evaluate[Cases[vars, prev[_,_]]], cons], vars, Reals];
  debugPrint["@createMap map after Reduce", map];
  map = map /. (expr_ /;((Head[expr] === Equal || Head[expr] === LessEqual || Head[expr] === Less|| Head[expr] === GreaterEqual || Head[expr] === Greater) && !hasJudge[expr]) -> True);
  map = LogicalExpand[map];
  map = applyListToOr[map];
  map = Map[(applyList[#])&, map];
  debugPrint["@createMap map after applyList", map];
  
  map = Map[(convertExprs[ adjustExprs[#, judge] ])&, map];
  map
];


(* �����ɕϐ������o�����邩�ۂ� *)

hasVariable[exprs_] := Length[StringCases[ToString[exprs], "usrVar" ~~ LetterCharacter]] > 0;

(* �����ϐ��������͂��̔������̂��̂��ۂ� *)

isVariable[exprs_] := StringMatchQ[ToString[exprs], "usrVar" ~~ LetterCharacter__] || MatchQ[exprs, Derivative[_][_][_] ] || MatchQ[exprs, Derivative[_][_] ] ;

(* �����ɏo������ϐ����擾 *)

getVariables[exprs_] := ToExpression[StringCases[ToString[exprs], "usrVar" ~~ LetterCharacter..]];


(* �����ɒ萔�����o�����邩�ۂ� *)

hasParameter[exprs_] := Length[StringCases[ToString[exprs], "parameter[" ~~ LetterCharacter]] > 0;

isParameter[exprs_] := Head[exprs] === parameter;

(* �����萔���̂��̂��ۂ� *)

hasVariable[exprs_] := Length[StringCases[ToString[exprs], "usrVar" ~~ LetterCharacter]] > 0;



(* �K���֌W���Z�q�̍����ɕϐ�����萔��������悤�ɂ��� *)

adjustExprs[andExprs_, judgeFunction_] := 
  Fold[
    (If[Not[judgeFunction[#2[[1]] ] ] && judgeFunction[#2[[2]] ],
       (* �t�ɂȂ��Ă�̂ŁA���Z�q���t�ɂ��Ēǉ����� *)
       Append[#1, getReverseRelop[Head[#2] ][#2[[2]], #2[[1]] ] ],
       Append[#1, #2]]) &,
    {},
    andExprs
  ];


resetConstraint[] := (
  constraint = True;
  pConstraint = True;
  prevConstraint = {};
  tmpConstraint = True;
  variables = tmpVariables = prevVariables = {};
  isTemporary = False;
  guard = True;
  guardVars = {};
  parameters = {};
);


addConstraint[co_, va_] := Block[
  {cons, vars},
  cons = co;
  cons = cons /. prevConstraint;
  vars = va;
  If[isTemporary,
    tmpVariables = Union[tmpVariables, vars];
	  (* tmpConstraint = Reduce[Exists[Evaluate[prevVariables], prevConstraint && tmpConstraint && cons], tmpVariables, Reals], *)
	  tmpConstraint = Reduce[tmpConstraint && cons, tmpVariables, Reals],
	  variables = Union[variables, vars];
	  (* constraint = Reduce[Exists[Evaluate[prevVariables], prevConstraint && constraint && cons], variables, Reals] *)
	  constraint = Reduce[constraint && cons, variables, Reals]
  ];
  simplePrint[cons, vars, constraint, variables, tmpConstraint, tmpVariables];
];


addPrevConstraint[co_, va_] := Block[
  {cons, vars},
  cons = co;
  vars = va;
  If[cons =!= True,
    prevConstraint = Union[prevConstraint, Map[(Rule@@#)&, List@@cons]]
  ];
  prevVariables = Union[prevVariables, vars];
  simplePrint[cons, vars, prevConstraint, prevVariables];
];

addVariables[vars_] := (
  If[isTemporary,
    tmpVariables = Union[tmpVariables, vars],
	  variables = Union[variables, vars]
  ];
  simplePrint[vars, variables, tmpVariables];
);

setGuard[gu_, vars_] := (
  guard = gu;
  guardVars = vars;
  simplePrint[ gu, vars, guard, guardVars];
);

startTemporary[] := (
  isTemporary = True;
);

endTemporary[] := (
  isTemporary = False;
  resetTemporaryConstraint[];
);

resetTemporaryConstraint[] := (
  tmpConstraint = True;
  tmpVariables = {};
  guard = True;
  guardVars = {};
);


addParameterConstraint[pcons_, pars_] := (
  pConstraint = Reduce[pConstraint && pcons, Reals];
  parameters = Union[parameters, pars];
  simplePrint[pConstraint, pars];
);


(* �ϐ�������Derivative��t�����C�����񐔂ƂƂ��ɕԂ� *)
removeDash[var_] := Block[
   {ret},
   If[Head[var] === parameter, Return[var]];
   ret = var /. x_[t] -> x;
   If[MatchQ[Head[ret], Derivative[_]],
     ret /. Derivative[d_][x_] -> {x, d},
     {ret, 0}
   ]
];

(* And�ł͂Ȃ�List�ł����� *)

applyList[reduceSol_] :=
  If[Head[reduceSol] === And, List @@ reduceSol, List[reduceSol]];


(* Or�ł͂Ȃ�List�ł����� *)

applyListToOr[reduceSol_] :=
  If[Head[reduceSol] === Or, List @@ reduceSol, List[reduceSol]];


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
 
calculateNextPointPhaseTime[maxTime_, discCause_] := 
  calculateNextPointPhaseTime[maxTime, discCause, constraint, pConstraint, variables];


(* �����Ə����̑g�ŁC�������_���a�łȂ����Ă���ꍇ���ꂼ��ɕ������� *)

divideDisjunction[timeCond_] := Map[({timeCond[[1]], #})&, List@@timeCond[[2]]];

(* �ϐ��Ƃ��̒l�Ɋւ��鎮�̃��X�g���A�ϐ��\�I�`���ɕϊ� *)
getExprCode[expr_] := Switch[Head[expr],
  Equal, 0,
  Less, 1,
  Greater, 2,
  LessEqual, 3,
  GreaterEqual, 4
];


replaceIntegerToString[num_] := (If[num < 0, minus[IntegerString[num]], IntegerString[num] ]);
integerString[expr_] := (
  expr /. (x_ :> ToString[InputForm[x]] /; Head[x] === Root )
       /. (x_Rational :> Rational[replaceIntegerToString[Numerator[x] ], replaceIntegerToString[Denominator[x] ] ] )
       /. (x_Integer :> replaceIntegerToString[x])
);


(* ���X�g�𐮌`���� *)
(* FullSimplify���g���ƁCRoot&Function���o�Ă����Ƃ��ɂ����\�Ȗ�ł���D�Ƃ������Ȗ�ł��Ȃ��ƃG���[�ɂȂ�̂�TODO�ƌ�����TODO *)
(* TODO:���f���̗v�f�ɑ΂��Ă��C�C�Ӑ��x�ւ̑Ή��i������ւ̕ϊ��Ƃ��j���s�� *)
convertExprs[list_] := Map[({removeDash[ #[[1]] ], getExprCode[#], integerString[FullSimplify[#[[2]] ] ] } )&, list];

calculateNextPointPhaseTime[maxTime_, discCause_, cons_, pCons_, vars_] := Check[
  Block[
    {
      dSol,
      timeAppliedCauses,
      resultList,
      originalOther
    },
    
    inputPrint["calculateNextPointPhaseTime", maxTime, discCause, cons, pCons, vars];
    
    (* �܂������������������D���܂�����checkConsistencyInterval�ŏo��������(tStore)�����̂܂܈����p�����Ƃ��ł���͂� *)
    dSol = exDSolve[cons, vars];
    
    debugPrint["dSol after exDSolve", dSol];
    
    (* ���ɂ�����discCause�ɓK�p���� *)
    dVars = dSol[[3]];
    timeAppliedCauses = False;
    For[i = 1, i <= Length[dSol[[1]] ], i++,
      tStore = Map[(# -> createIntegratedValue[#, dSol[[1]] [[i]]])&, dVars];
      timeAppliedCauses = Or[timeAppliedCauses, Or@@discCause /. tStore ]
    ];
    
    timeAppliedCauses = Reduce[timeAppliedCauses, {t}, Reals];
    
    simplePrint[timeAppliedCauses];
    
    
    
    (* �Ō�ɁC���炩���ߋ��߂��Ă���͂���otherCons��pcons��t������Minimize *)  
    
    resultList = Quiet[Minimize[{t, (timeAppliedCauses || t == maxTime) && pCons && t>0}, {t}], 
                           {Minimize::wksol, Minimize::infeas}];
    debugPrint["resultList after Minimize", resultList];
    resultList = First[resultList];
    If[Head[resultList] === Piecewise, resultList = makeListFromPiecewise[resultList, pCons], resultList = {{resultList, pCons}}];
    
    (* ���`���Č��ʂ�Ԃ� *)
    resultList = Map[({#[[1]],LogicalExpand[#[[2]] ]})&, resultList];
    resultList = Fold[(Join[#1, If[Head[#2[[2]]]===Or, divideDisjunction[#2], {#2}]])&,{}, resultList];
    resultList = Map[({#[[1]], Cases[applyList[#[[2]] ], Except[True]] })&, resultList];
    
    debugPrint["resultList after Format", resultList];
    
    resultList = Map[({integerString[FullSimplify[#[[1]] ] ], convertExprs[adjustExprs[#[[2]], isParameter ] ], If[Quiet[Reduce[#[[1]] == maxTime && And@@#[[2]] ], {Reduce::useq}] =!= False, 1, 0]})&, resultList];
    simplePrint[resultList];
    {1, resultList}
  ],
  debugPrint[$MessageList]; {0}
];


getDerivativeCount[variable_[_]] := 0;

getDerivativeCount[Derivative[n_][f_][_]] := n;


createIntegratedValue[variable_, integRule_] := (
  Simplify[
    variable /. Map[(Rule[#[[1]] /. x_[t]-> x, #[[2]]])&, integRule]
             /. Derivative[n_][f_] :> D[f, {t, n}] 
             /. x_[t] -> x]
);


(* �ϐ��ɂ��Ă̐���̂����C������������������Ή����D 
   �����������������ɂ������Ďז��ɂȂ鎮�͉������ɁC�Ԃ�l�̗v�f�̑�2�v�f�Ƃ���
   ���񂪑������Ĕ����������������Ȃ��ꍇ�́CFalse��
   ���񂪏��Ȃ����Ēl������ł��Ȃ��ꍇ�́CTrue��Ԃ����̂Ƃ���
   �f�t�H���g�A������M�����āCdvnul�̉\���͍l���Ȃ����Ƃɂ���
   @return �l��������ƁC�����������̑g�̃��X�g
    *)

exDSolve[expr_, vars_] := 
Quiet[
  Block[
    {sol, dExpr, dVars, otherExpr, otherVars},
    
    sol = Reduce[Exists[Evaluate[Cases[vars, prev[_,_]]], expr], vars, Reals];
    
    sol = sol /. (expr_ /;((Head[expr] === Equal || Head[expr] === LessEqual || Head[expr] === Less|| Head[expr] === GreaterEqual || Head[expr] === Greater) && !hasJudge[hasVariable]) -> True);
    sol = LogicalExpand[sol];
    
    If[Head[sol]===Or, 
      sol = First[sol] 
    ];
    
    sol = applyList[sol];
      
    debugPrint["@exDSolve before splitExprs", sol];
    
    {dExpr, dVars, otherExpr, otherVars} = splitExprs[sol];
    
    debugPrint["@exDSolve after splitExprs", dExpr, dVars, otherExpr, otherVars];
    
    If[dExpr === {},
      (* ���������������݂��Ȃ� *)
      Return[{underConstraint, otherExpr}]
    ];
    Check[
      Check[
        sol = DSolve[dExpr, dVars, t];
        {sol, otherExpr, dVars, otherVars},
        {underConstraint, otherExpr},
        {DSolve::underdet, Solve::svars}
      ],
      {overConstraint, otherExpr},
      {DSolve::overdet}
    ]
  ]
];


(* �����������������ɂ�����C�ז��ɂȂ鎮�i���L�j������
   �E�s����
   �E�����ϐ��ɂ��ẮC�����l�Ŗ������y�̂����C�d��������́i��Fx[t] == 1 && x'[t] == 0��x'[t] == 0�j
     ���������C���ɐV�K�ϐ����o�����Ă���ꍇ�͏Ȃ��Ȃ��i��Fx[t] == 1 && x'[t] == y'[t]�j
   ���������͑�3�v�f�Ƃ��ĕԂ��D
   ��2�v�f�Ƒ�4�v�f�͂��ꂼ��C�K�v�Ȏ��Ǝז��Ȏ��Ɋ܂܂��ϐ��̃��X�g *)


(*
splitExprs[expr_] := Block[
  {dExprs, appendedTimeVars, dVars, iter, otherExprs, otherVars, getTimeVars, getNoInitialTimeVars, timeVars, exprStack},
  
  getTimeVars[list1_, list2_] := Union[list1, Cases[list2, _[t] | _[0], Infinity] /. x_[0] -> x[t]];
  getNoInitialTimeVars[list_] := Union[Cases[list, _[t], Infinity] /. Derivative[_][f_][_] -> f[t]];
  
  (* TODO: iter�Ƃ��g������_�� *)
  otherExprs = dExprs = appendedTimeVars = {};
  exprStack = List@@expr;
  iter = 0;
  While[Length[exprStack] > 0,
    iter++;
    timeVars = getNoInitialTimeVars[exprStack[[1]] ];
    (* Print["exprStack:", exprStack, ", otherExprs:", otherExprs, ", dExprs:", dExprs, ", timeVars:", timeVars, ", appendedTimeVars:", appendedTimeVars]; *)
    If[Length[Select[timeVars, (FreeQ[appendedTimeVars, #])& ] ] > 1 && iter<Length[exprStack] ,
      exprStack = Append[exprStack, exprStack[[1]] ],
      iter = 0;
      If[Head[exprStack[[1]] ] =!= Equal || (Length[timeVars] > 0 && Length[Select[timeVars, (FreeQ[appendedTimeVars, #])& ] ] == 0),
        otherExprs = Append[otherExprs, exprStack[[1]] ],
        dExprs = Append[dExprs, exprStack[[1]] ];
        appendedTimeVars = Union[appendedTimeVars, timeVars]
      ];
    ];
    exprStack = Delete[exprStack, 1]
  ];
  
  (* Print["dExprs:", dExprs, ", otherExprs:", otherExprs]; *)
  dVars = Fold[(getTimeVars[#1,#2])&, {}, dExprs];
  otherVars = Fold[(getTimeVars[#1,#2])&, {}, otherExprs];
  {dExprs, dVars, otherExprs, otherVars}
];
*)


(* DSolve�ň����鎮 �Ƃ���ȊO �iotherExpr�j�ɕ����� *)
(* �����l���܂܂�/////�ϐ���2��ވȏ�o��ⓙ���ȊO��DSolve�ň����Ȃ� *)
splitExprs[expr_] := Block[
  {dExprs, dVars, otherExprs, otherVars},
  
  getTimeVars[list1_, list2_] := Union[list1, Cases[list2, _[t] | _[0], Infinity] /. x_[0] -> x[t]];

  otherExprs = Select[expr, 
                  (Head[#] =!= Equal || MemberQ[#, Derivative[n_][x_][t], Infinity] =!= True && Length[Union[Cases[#, _[t], Infinity]]] > 1) &];      
  otherVars = Fold[(getTimeVars[#1,#2])&, {}, otherExprs];

  dExprs = Complement[expr, otherExprs];
  dVars = Union[Fold[(Join[#1, Cases[#2, _[t] | _[0], Infinity] /. x_[0] -> x[t]]) &, 
                         {}, dExprs]];
  {dExprs, dVars, otherExprs, otherVars}
];



(*
 * ���ɑ΂��ė^����ꂽ���Ԃ�K�p����
 *)

applyTime2Expr[expr_, time_] := Block[
  {appliedExpr},
  appliedExpr = FullSimplify[(expr /. t -> time)];
  If[Element[appliedExpr, Reals] =!= False,
    {1, integerString[appliedExpr]},
    {0}]
];

(*
 * �^����ꂽ�����ߎ�����
 *)
approxExpr[precision_, expr_] :=
  integerString[Rationalize[N[Simplify[expr], precision + 3], 
              Divide[1, Power[10, precision]]]];


(* 
 * �^����ꂽt�̎����^�C���V�t�g
 *)

exprTimeShift[expr_, time_] := integerString[Simplify[expr /. t -> t - time ]];