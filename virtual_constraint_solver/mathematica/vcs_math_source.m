(* �ċA�񐔏�����グ�Ă݂� *)
$RecursionLimit = 1000;

(* �����ŗp���鐸�x���グ�Ă݂� *)
$MaxExtraPrecision = 1000;


(*
 * �O���[�o���ϐ�
 * constraint: ���݂̃t�F�[�Y�ł̐���
 * pConstraint: �萔�ɂ��Ă̐���
 * prevConstraint: ���Ɍ��l��ݒ肷�鐧��
 * initConstraint: �����l����
 * variables: ����ɏo������ϐ��̃��X�g
 * parameters: �L���萔�̃��X�g
 * isTemporary�F����̒ǉ����ꎞ�I�Ȃ��̂Ƃ��邩
 * tmpConstraint: �ꎞ�I�ɒǉ����ꂽ����
 * initTmpConstraint: �ꎞ�I�ɒǉ����ꂽ�����l����
 * tmpVariables: �ꎞ����ɏo������ϐ��̃��X�g
 * guard:
 * guardVars:
 * startTimes: �Ăяo���ꂽ�֐��̊J�n������ςރv���t�@�C�����O�p�X�^�b�N
 * profileList: �v���t�@�C�����O���ʂ̃��X�g
 * dList: �����������Ƃ��̈�ʉ���ێ����郊�X�g {�����������̃��X�g, ���̈�ʉ�, �ϐ��̒u�������K��}
 * createMapList: createMap�֐��ւ̓��͂Əo�͂̑g�̃��X�g
 * timeOutS: �^�C���A�E�g�܂ł̎��ԁD�b�P�ʁD
 * opt...: �e��I�v�V������ON/OFF
 *)


dList = {};
profileList = {};
createMapList = {};


(* �z��O�̃��b�Z�[�W���o�Ă��Ȃ����`�F�b�N�D�o�Ă����炻���ŏI���D
 �z��O�̌`���̌��ʂɂȂ��ĕςȌv�Z���n�߂ăG���[���b�Z�[�W���������邱�Ƃ������悤�ɂ��邽�߁D
 ���܂�ǂ��`�̎����ł͂Ȃ��CpublicMethod�ɂ��������Ă����`�Ŏ����ł���Ȃ瑽�����ꂪ�݌v�I�Ɉ�ԗǂ��͂��D
 ����ł́C��Ȃ��Ǝv�������ɒ��ꋲ��ł������ƂɂȂ�D *)
If[optIgnoreWarnings,
  checkMessage := (If[Length[Cases[$MessageList, Except[HoldForm[Minimize::ztest1], Except[HoldForm[Reduce::ztest1] ] ] ] ] > 0, Print[FullForm[$MessageList]];Abort[]]),
  checkMessage := (If[Length[$MessageList] > 0, Abort[] ])
];

publicMethod::timeout = "Calculation has reached to timeout";

(*
 * �v���t�@�C�����O�p�֐�
 * timeFuncStart: startTimes�Ɋ֐��̊J�n������ς�
 * timeFuncEnd: startTimes����J�n���������o���AprofileList�Ƀv���t�@�C�����ʂ��i�[
 * <�g����>
 *    �v���t�@�C�����O�������֐��̒�`�̐擪��timeFuncStart[];��
 *    ������timeFuncEnd["�֐���"];��ǉ�����.
 *    ������timeFuncEnd�̌�Œl��Ԃ��悤�ɂ��Ȃ��ƕԒl���ς���Ă��܂��̂Œ���.
 * <�v���t�@�C�����O���ʂ̌���>
 *    (���ݎ��s���I�������֐���) took (���̊֐����s�ɗv��������), elapsed time:(�v���O�������s����)
 *      function:(���܂łɌĂяo���ꂽ�֐���)  calls:(�Ăяo���ꂽ��)  total time of this function:(���̊֐��̍��v���s����)  average time:(���̊֐��̕��ώ��s����)  max time:(���̊֐��̍ō����s����)
 *    <��>
 *    calculateNextPointPhaseTime took 0.015635, elapsed time:1.006334
 *      function:checkConsistencyPoint  calls:1  total time of this function:0.000361  average time:0.000361  max time:0.000361
 *      function:createMap  calls:2  total time of this function:0.11461  average time:0.057304  max time:0.076988
 *      ...
 *)
timeFuncStart[] := (
  If[Length[startTimes]>0,
    startTimes = Append[startTimes,SessionTime[]];
  ,
    startTimes = {SessionTime[]};
  ];
);

timeFuncEnd[funcname_] := (
Module[{endTime,startTime,funcidx,i},
  endTime = SessionTime[];
  startTime = Last[startTimes];
  startTimes = Drop[startTimes,-1];
  If[Position[profileList, funcname] =!= {},
    funcidx = Flatten[Position[profileList,funcname]][[1]];
    profileList[[funcidx,2]] = profileList[[funcidx,2]] + 1;
    profileList[[funcidx,3]] = profileList[[funcidx,3]] + (endTime-startTime);
    profileList[[funcidx,4]] = profileList[[funcidx,3]] / profileList[[funcidx,2]];
    profileList[[funcidx,5]] = If[profileList[[funcidx,5]]<(endTime-startTime), endTime-startTime, profileList[[funcidx,5]]];
  ,
    profileList = Append[profileList, {funcname, 1, endTime-startTime, endTime-startTime, endTime-startTime}];
  ];
  profilePrint[funcname," took ",endTime-startTime,", elapsed time:",endTime];
  For[i=1,i<=Length[profileList],i=i+1,
    profilePrint["    function:",profileList[[i,1]],"  calls:",profileList[[i,2]],"  total time of this function:",profileList[[i,3]],"  average time:",profileList[[i,4]],"  max time:",profileList[[i,5]]];
  ];
];
);



(*
 * �f�o�b�O�p���b�Z�[�W�o�͊֐�
 * debugPrint�F�����Ƃ��ė^����ꂽ�v�f�v�f�𕶎���ɂ��ďo�͂���D�i�V���{���͕]�����Ă���\���j
 * simplePrint�F�����Ƃ��ė^����ꂽ�����C�u�i�]���O�j:�i�]����j�v�̌`���ŏo�͂���D
 *)
 
SetAttributes[simplePrint, HoldAll];

symbolToString := (StringJoin[ToString[Unevaluated[#] ], ": ", ToString[InputForm[Evaluate[#] ] ] ])&;

SetAttributes[symbolToString, HoldAll];

If[optUseDebugPrint || True,  (* �G���[���N�������̑Ή��̂��߁C���debugPrint��Ԃ��悤�ɂ��Ă����D������ɂ��낻��ȂɃR�X�g�͂�����Ȃ��H *)
  debugPrint[arg___] := Print[InputForm[{arg}]];
  simplePrint[arg___] := Print[delimiterAddedString[", ",
    List@@Map[symbolToString, Map[Unevaluated, Hold[arg]] ]
     ] ],
  
  debugPrint[arg___] := Null;
  simplePrint[arg___] := Null
];

profilePrint[arg___] := If[optUseProfilePrint, Print[InputForm[arg]], Null];

(*
 * �֐��Ăяo�����Č����邽�߂̕�����o�͂��s��
 *)
 
inputPrint[name_, arg___] := Print[StringJoin[name, "[", delimiterAddedString[",", Map[(ToString[InputForm[#] ])&,{arg}] ], "]" ] ];


delimiterAddedString[del_, {h_}] := h;

delimiterAddedString[del_, {h_, t__}] := StringJoin[h, del, delimiterAddedString[del, {t}] ];


SetAttributes[publicMethod, HoldAll];

(* C++�����璼�ڌĂяo���֐��́C�{�̕����̒�`���s���֐��D�f�o�b�O�o�͂Ƃ��C����I���̔���Ƃ��C��O�̈����Ƃ��𓝈ꂷ�� 
   �����ł����b�Z�[�W��f���\���̂���֐��́C���̊֐��Œ�`����悤�ɂ���D
   define��Return���܂܂�Ă���Ɛ���ɓ��삵�Ȃ��Ȃ�iReturn�̈��������̂܂ܕԂ邱�ƂɂȂ�j�̂Ŏg��Ȃ��悤�ɁI
*)

publicMethod[name_, args___, define_] := (
  name[Sequence@@Map[(Pattern[#, Blank[]])&, {args}]] := (
    inputPrint[ToString[name], args];
    CheckAbort[
      TimeConstrained[
        timeFuncStart[];
        Module[{publicRet},
          publicRet = define;
          simplePrint[publicRet];
          timeFuncEnd[name];
          checkMessage;
          {1, publicRet}
        ],
        Evaluate[timeOutS],
        {-1}
      ],
      debugPrint[$MessageList]; {0}
    ]
  )
);



(* �i�s�j�����̉E�ӂƍ��ӂ����ւ���ۂɁC�֌W���Z�q�̌��������]������DNot�Ƃ͈Ⴄ *)

getReverseRelop[relop_] := Switch[relop,
                                  Equal, Equal,
                                  Less, Greater,
                                  Greater, Less,
                                  LessEqual, GreaterEqual,
                                  GreaterEqual, LessEqual];



checkFalseConditions[] := (
  checkFalseConditions[prevConstraint, falseConditions, pConstraint, prevVariables]
);

publicMethod[
  checkFalseConditions,
  pCons, fCond, paramCons, vars,
  Module[
   {prevCons, falseCond, trueMap, falseMap, cpTrue, cpFalse, cpTmp},
    prevCons = pCons;
    prevCons = prevCons /. Rule->Equal;
    If[prevCons[[0]] == List, prevCons[[0]] = And;];
    falseCond = applyListToOr[LogicalExpand[fCond]];
    Quiet[
      cpTmp = Map[Reduce[Exists[vars, prevCons && #], Reals]&, falseCond], {Reduce::useq}
    ];
    If[cpTmp[[0]] == List, cpTmp[[0]] = Or;];
    cpTmp = Reduce[cpTmp,Reals];
    simplePrint[cpTmp];
    checkMessage;
    Quiet[
      cpTrue = Reduce[!cpTmp && paramCons, Reals], {Reduce::useq}
    ];
    checkMessage;
    simplePrint[cpTrue];
    Quiet[
      cpFalse = Reduce[cpTmp && paramCons, Reals], {Reduce::useq}
    ];
    checkMessage;
    simplePrint[cpFalse];
    {trueMap, falseMap} = Map[(createMap[#, isParameter, hasParameter, {}])&, {cpTrue, cpFalse}];
    simplePrint[trueMap, falseMap];
    {trueMap, falseMap}
  ]
];


(* ���񃂃W���[������������������Z�b�g���� *)
setFalseConditions[co_, va_] := Module[
  {cons, vars},
  cons = co;
  falseConditions = cons;
  simplePrint[cons, falseConditions];
];

(* �ϐ��̃��X�g����prev�ϐ�����菜�� *)
removePrevVariables[vars_] := Module[
  {ret,i},
  ret = {};
  For[i=1,i<=Length[vars],i++,
    If[!isPrevVariable[vars[[i]]], ret=Append[ret,vars[[i]]]];
  ];
  ret
];

(* ������������𐮌`���ĕԂ� *)
createPrevMap[cons_, vars_] := Module[
  {map},
  If[cons === True || cons === False, 
    cons,

    map = cons /. (expr_ /; (( Head[expr] === Inequality || Head[expr] === Equal || Head[expr] === LessEqual || Head[expr] === Less|| Head[expr] === GreaterEqual || Head[expr] === Greater) && (hasVariable[expr] || hasParameter[expr] || !hasPrevVariable[expr])) -> False);
    map = Reduce[map, vars, Reals];
    map = cons /. (expr_ /; (( Head[expr] === Inequality || Head[expr] === Equal || Head[expr] === LessEqual || Head[expr] === Less|| Head[expr] === GreaterEqual || Head[expr] === Greater) && (hasVariable[expr] || hasParameter[expr] || !hasPrevVariable[expr])) -> False);

    simplePrint[map];
    If[map =!= False, 
      map = LogicalExpand[map];
      map = applyListToOr[map];
      map = Map[(applyList[#])&, map];
      debugPrint["@createMap map after applyList", map];
 
      map = Map[(convertExprs[ adjustExprs[#, isPrevVariable] ])&, map];
    ];
    map
  ]
];


(* ���񃂃W���[����������������������邽�߂̖����������� *)
findFalseConditions[] := (
  findFalseConditions[constraint && tmpConstraint && guard && initConstraint && initTmpConstraint, guard, removePrevVariables[Union[variables, tmpVariables, guardVars]]]
);

publicMethod[
  findFalseConditions,
  cons, gua, vars,
  Module[
    {i, falseMap, cpFalse},
    Quiet[
      cpFalse = Reduce[!Reduce[Exists[vars, cons],Reals] && gua, Reals], {Reduce::useq}
    ];
    simplePrint[cpFalse];
    checkMessage;
    cpFalse = cpFalse /. (expr_ /; (( Head[expr] === Inequality || Head[expr] === Equal || Head[expr] === LessEqual || Head[expr] === Less|| Head[expr] === GreaterEqual || Head[expr] === Greater) && (hasVariable[expr] || hasParameter[expr] || !hasPrevVariable[expr])) -> False);

    (*    falseMap = createPrevMap[cpFalse, {}]; *)
    If[cpFalse =!= False && cpFalse =!= True,
      cpFalse = integerString[cpFalse];
      cpFalse = Simplify[cpFalse];
    ];
    simplePrint[cpFalse];
    cpFalse
  ]
];


(* �|�C���g�t�F�[�Y�ɂ����閳���������� *)

checkConsistencyPoint[] := (
  checkConsistencyPoint[constraint && tmpConstraint && guard && initConstraint && initTmpConstraint, pConstraint, Union[variables, tmpVariables, guardVars]]
);

publicMethod[
  checkConsistencyPoint,
  cons, pcons, vars,
  Module[
    {trueMap, falseMap, cpTrue, cpFalse},
    Quiet[
      cpTrue = Reduce[Exists[vars, cons && pcons], Reals], {Reduce::useq}
    ];
    simplePrint[cpTrue];
    checkMessage;
    Quiet[
      cpFalse = Reduce[pcons && !cpTrue, Reals], {Reduce::useq}
    ];
    checkMessage;
    simplePrint[cpFalse];
    {trueMap, falseMap} = Map[(createMap[#, isParameter, hasParameter, {}])&, {cpTrue, cpFalse}];
    simplePrint[trueMap, falseMap];
    {trueMap, falseMap}
  ]
];

(* �C���^�[�o���t�F�[�Y�ɂ����閳���������� *)

checkConsistencyInterval[] :=  (
  checkConsistencyInterval[constraint && tmpConstraint && guard, initConstraint && initTmpConstraint, pConstraint, Union[variables, tmpVariables, guardVars]]
);

appendZeroVars[vars_] := Join[vars, vars /. x_[t] -> x[0]];

publicMethod[
  checkConsistencyInterval,
  cons, initCons, pcons, vars,
  Module[
    {sol, otherCons, tCons, hasTCons, necessaryTCons, parList, tmpPCons, cpTrue, cpFalse, trueMap, falseMap},
    sol = exDSolve[cons, initCons];
    debugPrint["sol after exDSolve", sol];
    If[sol === overConstraint,
      {False, pcons},
      If[sol[[1]] === underConstraint,
        (* ����s���Ŕ��������������S�ɂ͉����Ȃ��Ȃ�C�P���Ɋe�ϐ��l����т��̔����l���������Ȃ����𒲂ׂ� *)
        (* Exists�̑�������Hold�iHoldAll?�j�����������Ă���炵���̂ŁCEvaluate�ŕ]������K�v������i�C������j *)
        tCons = Map[(# -> createIntegratedValue[#, sol[[3]] ])&, getTimeVars[vars]];
        tCons = sol[[2]] /. tCons;
        tmpPCons = If[getParameters[tCons] === {}, True, pcons];
        tCons = LogicalExpand[Quiet[Reduce[Exists[Evaluate[appendZeroVars[vars]], And@@tCons && tmpPCons], Reals]]],
        (* �������������������ꍇ *)
        tCons = Map[(# -> createIntegratedValue[#, sol[[2]] ])&, getTimeVars[vars]];
        tCons = applyList[sol[[1]] /. tCons];
        tmpPCons = If[getParameters[tCons] === {}, True, pcons];
        tCons = LogicalExpand[Quiet[Reduce[And@@tCons && tmpPCons, Reals]]]
      ];
      checkMessage;

      simplePrint[tCons];

      If[tCons === False,
        {False, pcons},
        
        hasTCons = tCons /. (expr_ /; (( Head[expr] === Equal || Head[expr] === LessEqual || Head[expr] === Less|| Head[expr] === GreaterEqual || Head[expr] === Greater) && (!hasSymbol[expr, {t}])) -> True);
        parList = getParameters[hasTCons];
        simplePrint[parList];
        necessaryTCons = tCons /. (expr_ /; (( Head[expr] === Equal || Head[expr] === LessEqual || Head[expr] === Less|| Head[expr] === GreaterEqual || Head[expr] === Greater) && (!hasSymbol[expr, {t}] && !hasSymbol[expr, parList])) -> True);
        
        simplePrint[necessaryTCons];
        cpTrue = Reduce[pcons && Quiet[Minimize[{t, necessaryTCons && t > 0}, t], {Minimize::wksol, Minimize::infeas}][[1]] == 0, Reals];
        cpFalse = Reduce[pcons && !cpTrue, Reals];

        simplePrint[cpTrue, cpFalse];

        checkMessage;
        {trueMap, falseMap} = Map[(createMap[#, isParameter,hasParameter, {}])&, {cpTrue, cpFalse}];
        simplePrint[trueMap, falseMap];
        {trueMap, falseMap}
      ]
    ]
  ]
];



(* �ϐ��������͋L���萔�Ƃ��̒l�Ɋւ��鎮�̃��X�g���C�\�`���ɕϊ� *)

createVariableMap[] := createVariableMap[constraint && pConstraint && initConstraint, variables];

publicMethod[
  createVariableMap,
  cons, vars,
  Module[
    {ret},
    ret = createMap[cons, isVariable, hasVariable, vars];
    debugPrint["ret after CreateMap", ret];
    ret = Map[(Cases[#, Except[{parameter[___], _, _}] ])&, ret];
    ret = ruleOutException[ret];
    simplePrint[ret];
    ret
  ]
];


createVariableMapInterval[] := createVariableMapInterval[constraint, initConstraint, variables, parameters];

publicMethod[
  createVariableMapInterval,
  cons, initCons, vars, pars,
  Module[
    {sol, tStore, tVars, ret},
    sol = exDSolve[cons, initCons];
    debugPrint["sol after exDSolve", sol];
    If[sol[[1]] === underConstraint, 
      underConstraint,
      tVars = getTimeVars[vars];
      tStore = Map[(# == createIntegratedValue[#, sol[[2]] ] )&, tVars];
      simplePrint[tStore];
      If[Length[Select[tStore, (hasVariable[ #[[2]] ])&, 1] ] > 0,
        (* �E�ӂɕϐ������c���Ă���C�܂�l�����S��t�̎��ɂȂ��Ă��Ȃ��ϐ����o�������ꍇ��underConstraint��Ԃ� *)
        underConstraint,
        ret = {convertExprs[tStore]};
        debugPrint["ret after convert", ret];
        ret = ruleOutException[ret];
        simplePrint[ret];
        ret
      ]
    ]
  ]
];


ruleOutException[list_] := Module[
  {ret},
  ret = Map[(Cases[#, {{_?isVariable, _}, _, _} ])&, list];
  ret = Map[(Cases[#, Except[{{t, 0}, _, _}] ])&, ret];
  ret = Map[(Cases[#, Except[{{prev[_, _], _}, _, _}] ])&, ret];
  ret
];


createParameterMap[] := createParameterMap[pConstraint];

publicMethod[
  createParameterMap,
  pcons,
  createMap[pcons, isParameter, hasParameter, {}];
]

createMap[cons_, judge_, hasJudge_, vars_] := Module[
  {map, idx},
  If[cons === True || cons === False, 
    cons,
    idx = {};
    If[optOptimizationLevel == 1 || optOptimizationLevel == 4,
      idx = Position[createMapList,{cons,judge,hasJudge,vars}];
      If[idx != {}, map = createMapList[[idx[[1]][[1]]]][[2]]];
    ];
    If[idx == {},
      (* TODO: ������prev�Ɋւ��鏈���͖{���Ȃ��Ă������͂��D����0�ł�prev�̈����������܂��ł���΂ǂ��ɂ��Ȃ�H *)
      map = cons /. (expr_ /; (( Head[expr] === Equal || Head[expr] === LessEqual || Head[expr] === Less|| Head[expr] === GreaterEqual || Head[expr] === Greater) && (!hasJudge[expr] || hasPrevVariable[expr])) -> True);
      map = Reduce[map, vars, Reals];
      (* TODO:2����������[���K�p���������Ȃ��D�ꍇ�̏d����C�s�v�ȏ����̔�����}���C�����ł��Ȃ����H *)
      map = map /. (expr_ /; (( Head[expr] === Equal || Head[expr] === LessEqual || Head[expr] === Less|| Head[expr] === GreaterEqual || Head[expr] === Greater) && (!hasJudge[expr] || hasPrevVariable[expr])) -> True);
      simplePrint[map];
      map = LogicalExpand[map];
      map = applyListToOr[map];
      map = Map[(applyList[#])&, map];
      debugPrint["@createMap map after applyList", map];
    
      map = Map[(convertExprs[ adjustExprs[#, judge] ])&, map];
      If[optOptimizationLevel == 1 || optOptimizationLevel == 4, createMapList = Append[createMapList,{{cons,judge,hasJudge,vars},map}]];
    ];
    map
  ]
];


(* �����ɕϐ������o�����邩�ۂ� *)

hasVariable[exprs_] := Length[StringCases[ToString[exprs], "usrVar" ~~ WordCharacter]] > 0;

(* �����ϐ��������͂��̔������̂��̂��ۂ� *)

isVariable[exprs_] := MatchQ[exprs, _Symbol] && StringMatchQ[ToString[exprs], "usrVar" ~~ WordCharacter__] || MatchQ[exprs, Derivative[_][_][_] ] || MatchQ[exprs, Derivative[_][_] ] ;

(* �����ɏo������ϐ����擾 *)

getVariables[exprs_] := ToExpression[StringCases[ToString[exprs], "usrVar" ~~ WordCharacter..]];

(* �����ɏo������L���萔���擾 *)

getParameters[exprs_] := Cases[exprs, parameter[_, _, _], Infinity];

(* ���ԕϐ����擾 *)
getTimeVars[list_] := Cases[list, _[t], Infinity];

(* �����ɒ萔�����o�����邩�ۂ� *)

hasParameter[exprs_] := Length[Cases[exprs, parameter[_, _, _], Infinity]] > 0;

(* �����萔���̂��̂��ۂ� *)

isParameter[exprs_] := Head[exprs] === parameter;

(* �����w�肳�ꂽ�V���{�������� *)
hasSymbol[exprs_, syms_List] := MemberQ[{exprs}, ele_ /; (MemberQ[syms, ele] || (!AtomQ[ele] && hasSymbol[Head[ele], syms]) ), Infinity ];

(* ����prev�ϐ����̂��̂��ۂ� *)
isPrevVariable[exprs_] := Head[exprs] === prev;

(* ����prev�ϐ������� *)
hasPrevVariable[exprs_] := Length[Cases[exprs, prev[_, _], Infinity]] > 0;


(* �K���֌W���Z�q�̍����ɕϐ�����萔��������悤�ɂ��� *)

adjustExprs[andExprs_, judgeFunction_] := 
Fold[
  (
   If[#2 === True,
    #1,
    If[Not[judgeFunction[#2[[1]] ] ] && judgeFunction[#2[[2]] ],
     (* �t�ɂȂ��Ă�̂ŁA���Z�q���t�ɂ��Ēǉ����� *)
     Append[#1, getReverseRelop[Head[#2] ][#2[[2]], #2[[1]] ] ],
     Append[#1, #2]]
   ]) &,
  {},
  andExprs
];


resetConstraint[] := (
  constraint = True;
  initConstraint = True;
  pConstraint = True;
  prevConstraint = {};
  initTmpConstraint = True;
  tmpConstraint = True;
  variables = tmpVariables = prevVariables = {};
  isTemporary = False;
  guard = True;
  guardVars = {};
  parameters = {};
);


resetConstraintForVariable[] := (
  constraint = True;
);

addGuard[gu_, vars_] := (
  guard = guard && gu;
  guardVars = Union[guardVars,vars];
  simplePrint[gu, vars, guard, guradVars];
);

addConstraint[co_, va_] := Module[
  {cons, vars},
  cons = co;
  cons = cons //. prevConstraint;
  vars = va;
  If[isTemporary,
    tmpVariables = Union[tmpVariables, vars];
    tmpConstraint = tmpConstraint && cons,
    variables = Union[variables, vars];
    constraint = constraint && cons
  ];
  simplePrint[cons, vars, constraint, variables, tmpConstraint, tmpVariables];
];


addInitConstraint[co_, va_] := Module[
  {cons, vars},
  cons = co;
  cons = cons //. prevConstraint;
  vars = va;
  If[isTemporary,
    tmpVariables = Union[tmpVariables, vars];
    initTmpConstraint = initTmpConstraint && cons,
    variables = Union[variables, vars];
    initConstraint = initConstraint && cons
  ];
  simplePrint[cons, vars, initConstraint, variables, initTmpConstraint, tmpVariables];
];

addPrevConstraint[co_, va_] := Module[
  {cons, vars},
  cons = co;
  vars = va;
  If[cons =!= True,
    prevConstraint = Union[prevConstraint, Map[(Rule@@#)&, applyList[cons]]]
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
  initTmpConstraint = True;
  tmpVariables = {};
  guard = True;
  guardVars = {};
);


resetConstraintForParameter[pcons_, pars_] := (
  pConstraint = True;
  parameters = {};
  addParameterConstraint[pcons, pars];
);

addParameterConstraint[pcons_, pars_] := (
  pConstraint = Reduce[pConstraint && pcons, Reals];
  parameters = Union[parameters, pars];
  simplePrint[pConstraint, pars];
);


(* �ϐ�������Derivative��t�����C�����񐔂ƂƂ��ɕԂ� *)
removeDash[var_] := Module[
   {ret},
   If[Head[var] === parameter || Head[var] === prev, Return[var]];
   ret = var /. x_[t] -> x;
   If[MatchQ[Head[ret], Derivative[_]],
     ret /. Derivative[d_][x_] -> {x, d},
     {ret, 0}
   ]
];


apply[AndreduceSol_] :=
  If[Head[reduceSol] === And, List @@ reduceSol, List[reduceSol]];

(* And�ł͂Ȃ�List�ł����� *)

applyList[reduceSol_] :=
  If[Head[reduceSol] === And, List @@ reduceSol, List[reduceSol]];


(* Or�ł͂Ȃ�List�ł����� *)

applyListToOr[reduceSol_] :=
  If[Head[reduceSol] === Or, List @@ reduceSol, List[reduceSol]];


(* Piecewise�̑��v�f���C���̏����ƂƂ��ɑ��v�f�ɕt�����ă��X�g�ɂ���D������False�Ȃ�폜 
   ���ł� others���e�����ɑ΂��ĕt�� *)

makeListFromPiecewise[minT_, others_] := Module[
  {tmpCondition = False, retMinT = minT[[1]]},
  tmpCondition = Or @@ Map[(#[[2]])&, minT[[1]]];
  tmpCondition = Reduce[And[others, Not[tmpCondition]], Reals];
  retMinT = Map[({#[[1]], Reduce[others && #[[2]] ]})&, retMinT];
  If[ tmpCondition === False,
    retMinT,
    Append[retMinT, {minT[[2]], tmpCondition}]
  ]
];


(*
 * ���̃|�C���g�t�F�[�Y�Ɉڍs���鎞�������߂�
 *)

calculateNextPointPhaseTime[maxTime_, discCause_] := 
  calculateNextPointPhaseTime[maxTime, discCause, constraint, initConstraint, pConstraint, variables];



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
  expr /. (Infinity :> inf)
       /. (x_ :> ToString[InputForm[x]] /; Head[x] === Root )
       /. (x_Rational :> Rational[replaceIntegerToString[Numerator[x] ], replaceIntegerToString[Denominator[x] ] ] )
       /. (x_Integer :> replaceIntegerToString[x])
);


(* ���X�g�𐮌`���� *)
(* FullSimplify���g���ƁCRoot&Function���o�Ă����Ƃ��ɂ����\�Ȗ�ł���D�Ƃ������Ȗ�ł��Ȃ��ƃG���[�ɂȂ�̂�TODO�ƌ�����TODO *)
(* TODO:���f���̗v�f�ɑ΂��Ă��C�C�Ӑ��x�ւ̑Ή��i������ւ̕ϊ��Ƃ��j���s�� *)

(* convertExprs[list_] := Map[({removeDash[ #[[1]] ], getExprCode[#], integerString[FullSimplify[#[[2]] ] ] } )&, list]; *)
convertExprs[list_] := Map[({removeDash[ #[[1]] ], getExprCode[#], integerString[#[[2]] ] } )&, list];


(* �����Ə����̑g�ŁC�������_���a�łȂ����Ă���ꍇ���ꂼ��ɕ������� *)
divideDisjunction[timeCond_] := Map[({timeCond[[1]], #, timeCond[[3]]})&, List@@timeCond[[2]]];


(* �ő厞���Ǝ����Ə����Ƃ̑g���r���C�ő厞���̕��������ꍇ��1��t���������̂𖖔��ɁC
  �����łȂ��ꍇ��0�𖖔��ɕt�����ĕԂ��D�����ɂ���ĕω�����ꍇ�́C�������i�荞��ł��ꂼ���Ԃ� *)
compareWithMaxTime[maxT_, timeCond_] := 
Module[
  {sol, tmpCond},
  sol = Reduce[maxT <= timeCond[[1]] && timeCond[[2]], Reals];
  If[sol === False,
    {{timeCond[[1]], timeCond[[2]], 0}},
    tmpCond = Reduce[(!sol && timeCond[[2]]), Reals];
    If[tmpCond === False,  (* �����𖞂����͈͂ŏ��maxT <= timeCond[[1]]�����藧�Ƃ� *)
      {{maxT, timeCond[[2]], 1}},
      {{maxT, sol, 1}, {timeCond[[1]], tmpCond, 0}}
    ]
  ]
];

publicMethod[
  calculateNextPointPhaseTime,
  maxTime, discCause, cons, initCons, pCons, vars,
  Module[
    {
      dSol,
      timeAppliedCauses,
      resultList,
      necessaryPCons,
      parameterList,
      originalOther,
      tmpMaxTime
    },
    
    (* �܂������������������D���܂�����checkConsistencyInterval�ŏo��������(tStore)�����̂܂܈����p�����Ƃ��ł���͂� *)
    dSol = exDSolve[cons, initCons];
    
    debugPrint["dSol after exDSolve", dSol];
    
    (* ���ɂ�����discCause�ɓK�p���� *)
    timeAppliedCauses = False;
    
    tStore = Map[(# -> createIntegratedValue[#, dSol[[2]] ])&, getTimeVars[vars]];
    timeAppliedCauses = Or@@(discCause /. tStore );
    simplePrint[timeAppliedCauses];
    
    parameterList = getParameters[timeAppliedCauses];
    
    (* �K�v��pCons������I�ԁD�s�v�Ȃ��̂������Ă����Minimze�̓��삪���������Ȃ�H *)
    
    necessaryPCons = LogicalExpand[pCons] /. (expr_ /; (( Head[expr] === Equal || Head[expr] === LessEqual || Head[expr] === Less|| Head[expr] === GreaterEqual || Head[expr] === Greater) && (!hasSymbol[expr, parameterList])) -> True);
    
    simplePrint[necessaryPCons];
    
    resultList = Quiet[Minimize[{t, (timeAppliedCauses) && necessaryPCons && t>0}, {t}], 
                           {Minimize::wksol, Minimize::infeas, Minimize::ztest}];
    debugPrint["resultList after Minimize", resultList];
    If[Head[resultList] === Minimize, Message[calculateNextPointPhaseTime::mnmz]];
    checkMessage;
    resultList = First[resultList];
    If[Head[resultList] === Piecewise, resultList = makeListFromPiecewise[resultList, pCons], resultList = {{resultList, pCons}}];
    
    resultList = Fold[(Join[#1, compareWithMaxTime[If[Quiet[Reduce[maxTime <= 0, Reals]] === True, 0, maxTime], #2] ])&,{}, resultList];
    (* resultList = Fold[(Join[#1, compareWithMaxTime[maxTime, #2] ])&,{}, resultList]; *)
    simplePrint[resultList];
    
    (* ���`���Č��ʂ�Ԃ� *)
    resultList = Map[({#[[1]],LogicalExpand[#[[2]] ], #[[3]]})&, resultList];
    resultList = Fold[(Join[#1, If[Head[#2[[2]]]===Or, divideDisjunction[#2], {#2}]])&,{}, resultList];
    resultList = Map[({#[[1]], Cases[applyList[#[[2]] ], Except[True]], #[[3]] })&, resultList];
    
    debugPrint["resultList after Format", resultList];
    
    resultList = Map[({integerString[FullSimplify[#[[1]] ] ], convertExprs[adjustExprs[#[[2]], isParameter ] ], #[[3]] })&, resultList];
    simplePrint[resultList];
    resultList
  ]
];


calculateNextPointPhaseTime::mnmz = "Failed to minimize in calculateNextPointPhaseTime";

getDerivativeCount[variable_[_]] := 0;

getDerivativeCount[Derivative[n_][f_][_]] := n;


applyDSolveResult[exprs_, integRule_] := (
  Simplify[
      exprs  /. integRule     (* �P���Ƀ��[����K�p *)
             /. Map[((#[[1]] /. x_[t]-> x) -> #[[2]] )&, integRule]
             /. (Derivative[n_][f_][t] /; !isVariable[f]) :> D[f, {t, n}] (* �����l�ɂ��Ă����[����K�p *)
  ]
);

createIntegratedValue[variable_, integRule_] := (
  Module[
    {tRemovedRule, ruleApplied, derivativeExpanded, tRemoved},
    tRemovedRule = Map[((#[[1]] /. x_[t]-> x) -> #[[2]] )&, integRule];
    tRemoved = variable /. x_Symbol[t] -> x;
    ruleApplied = tRemoved /. tRemovedRule;
    derivativeExpanded = ruleApplied /. Derivative[n_][f_][t] :> D[f, {t, n}];
    Simplify[derivativeExpanded]
  ]
);

(* �����������n�������D
  �P��DSolve�����̂܂܎g�p���Ȃ����R�͈ȉ��D
    ���R1: ���L�̂悤�Ȕ����������n�ɑ΂��Ďア�D
      DSolve[{usrVarz[t] == usrVarx[t]^2, usrVarx'[t] == usrVarx[t], usrVarx[0] == 1}, {usrVarx[t], usrVarz[t]}, t]
    ���R2: �s�����Ɏア
    ���R3: bvnul�Ȃǂ̗�O�����𓝈ꂵ����
  @param expr �����Ɋւ���ϐ��ɂ��Ă̐���
  @param initExpr �ϐ��̏����l�ɂ��Ă̐���
  @return overConstraint | 
    {underConstraint, �ϐ��l���������ׂ�����i���[���Ɋ܂܂�Ă�����̂͏����j�C�e�ϐ��̒l�̃��[��} |
    {�ϐ��l���������ׂ�����i���[���Ɋ܂܂�Ă�����̂͏����j�C�e�ϐ��̒l�̃��[��} 
    TODO: Subsets�Ƃ��g���Ă邩�玮�̐��ŊȒP�ɔ�������
*)

exDSolve[expr_, initExpr_] :=
Check[
  Module[
    {subsets, tmpExpr, excludingCons, tmpInitExpr, subset, tVars, ini, i, j, sol, resultCons, resultRule, idx, generalInitValue, swapValue},
    tmpExpr = applyList[expr];
    resultCons = Select[tmpExpr, (Head[#] =!= Equal)&];
    tmpExpr = Complement[tmpExpr, resultCons];
    tmpInitExpr = applyList[initExpr];
    subsets = Subsets[tmpExpr];
    resultCons = And@@resultCons;
    resultRule = {};
    For[i=2,i<=Length[subsets], i++, (* �Y������2����Ȃ͍̂ŏ��̋�W���𖳎����邽�� *)
      subset = subsets[[i]];
      tVars = Union[getVariables[subset]];
      If[Length[tVars] == Length[subset],
        ini = Select[tmpInitExpr, (hasSymbol[#, tVars ])& ];
        If[optOptimizationLevel == 1 || optOptimizationLevel == 4, 
          (* �����������̌��ʂ��ė��p����ꍇ *)
(*
          For[j=1,j<=Length[dList],j++,
            debugPrint["dList",j];
            debugPrint["  defferential equation", dList[[j, 1]]];
            debugPrint["  general solution", dList[[j, 2]]];
            debugPrint["  replaced Variable List", dList[[j, 3]]];
          ];
*)

          idx = Position[Map[(Sort[#])&,dList],Sort[subset]];
          If[idx == {},
            generalInitValue = ini;
            For[j=1,j<=Length[generalInitValue],j++,
              generalInitValue[[j, 1]] = initValue[j];
            ];
            sol = Check[
              DSolve[Union[subset, generalInitValue], Map[(#[t])&, tVars], t],
              overConstraint,
              {DSolve::overdet, DSolve::bvnul}
            ];
            For[j=1,j<=Length[generalInitValue],j++,
              generalInitValue[[j, 0]] = Rule;
            ];
            dList = Append[dList,{subset,sol,generalInitValue}];
            idx = Position[dList,subset],
            sol = dList[[idx[[1,1]],2]];
          ];
          For[j=1,j<=Length[ini],j++,
            swapValue = ini[[j, 2]];
            ini[[j, 2]] = ini[[j, 1]];
            ini[[j, 1]] = swapValue;
            ini[[j, 0]] = Rule;
          ];
          sol = sol /. (dList[[idx[[1, 1]], 3]] /. ini)
          ,
          simplePrint[subset, ini];
          sol = Quiet[Check[
              DSolve[Union[subset, ini], Map[(#[t])&, tVars], t],
              overConstraint,
              {DSolve::overdet, DSolve::bvnul}
            ],
            {DSolve::overdet, DSolve::bvnul}
          ]
        ];
        simplePrint[sol];
        checkMessage;
        If[sol === overConstraint || Head[sol] === DSolve || Length[sol] == 0, Return[overConstraint] ];
        tmpExpr = Complement[tmpExpr, subset];
        tmpExpr = applyDSolveResult[tmpExpr, sol[[1]] ];
        resultRule = Union[sol[[1]], resultRule];
        excludingCons = Select[tmpExpr, (Length[getVariables[#]] === 0)&];
        resultCons = resultCons && And@@excludingCons;
        If[resultCons === False, Return[overConstraint] ];
        (* TODO: DSolve�̌��ʂ���������ꍇ�ւ̑Ή� *)
        tmpExpr = Complement[tmpExpr, excludingCons];
        subsets = Subsets[tmpExpr];
        i = 1;
      ]
    ];
    simplePrint[resultCons];
    If[Length[subsets] > 1,
      {underConstraint, resultCons && And@@Map[(And@@#)&, subsets], resultRule},
      {resultCons, resultRule}
    ]
  ],
  Message[exDSolve::unkn]
];


exDSolve::unkn = "unknown error occurred in exDSolve";

(*
 * ���ɑ΂��ė^����ꂽ���Ԃ�K�p����
 *)

publicMethod[
  applyTime2Expr,
  expr, time,
  Module[
    {appliedExpr},
    (* FullSimplify���Ə������d�����CSimplify����Minimize:ztest���o�����₷�� *)
    appliedExpr = (expr /. t -> time);
    (* appliedExpr = FullSimplify[(expr /. t -> time)]; *)
    If[Element[appliedExpr, Reals] =!= False,
      integerString[appliedExpr],
      Message[applyTime2Expr::nrls, appliedExpr]
    ]
  ]
];

applyTime2Expr::nrls = "`1` is not a real expression.";

(*
 * �^����ꂽ�����ߎ�����
 *)
publicMethod[
  approxExpr,
  precision, expr,
  integerString[
    Rationalize[
      N[Simplify[expr], precision + 3],
      Divide[1, Power[10, precision]]
    ]
  ]
];


(* 
 * �^����ꂽt�̎����^�C���V�t�g
 *)

publicMethod[
  exprTimeShift,
  expr, time,
  integerString[expr /. t -> t - time]
];
