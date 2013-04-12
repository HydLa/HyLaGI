(* 再帰回数上限を上げてみる *)
$RecursionLimit = 1000;

(* 内部で用いる精度も上げてみる *)
$MaxExtraPrecision = 1000;


(*
 * グローバル変数
 * constraint: 現在のフェーズでの制約
 * pConstraint: 定数についての制約
 * prevConstraint: 左極限値を設定する制約
 * initConstraint: 初期値制約
 * variables: 制約に出現する変数のリスト
 * parameters: 記号定数のリスト
 * isTemporary：制約の追加を一時的なものとするか
 * tmpConstraint: 一時的に追加された制約
 * initTmpConstraint: 一時的に追加された初期値制約
 * tmpVariables: 一時制約に出現する変数のリスト
 * guard:
 * guardVars:
 * startTimes: 呼び出された関数の開始時刻を積むプロファイリング用スタック
 * profileList: プロファイリング結果のリスト
 * dList: 微分方程式とその一般解を保持するリスト {微分方程式のリスト, その一般解, 変数の置き換え規則}
 * createMapList: createMap関数への入力と出力の組のリスト
 * timeOutS: タイムアウトまでの時間．秒単位．
 * opt...: 各種オプションのON/OFF
 *)


dList = {};
profileList = {};
createMapList = {};


(* 想定外のメッセージが出ていないかチェック．出ていたらそこで終了．
 想定外の形式の結果になって変な計算を始めてエラーメッセージが爆発することが無いようにするため．
 あまり良い形の実装ではなく，publicMethodにだけ書いておく形で実装できるなら多分それが設計的に一番良いはず．
 現状では，危ないと思った個所に逐一挟んでおくことになる． *)
If[optIgnoreWarnings,
  checkMessage := (If[Length[Cases[$MessageList, Except[HoldForm[Minimize::ztest1], Except[HoldForm[Reduce::ztest1] ] ] ] ] > 0, Print[FullForm[$MessageList]];Abort[]]),
  checkMessage := (If[Length[$MessageList] > 0, Abort[] ])
];

publicMethod::timeout = "Calculation has reached to timeout";

(*
 * プロファイリング用関数
 * timeFuncStart: startTimesに関数の開始時刻を積む
 * timeFuncEnd: startTimesから開始時刻を取り出し、profileListにプロファイル結果を格納
 * <使い方>
 *    プロファイリングしたい関数の定義の先頭にtimeFuncStart[];を
 *    末尾でtimeFuncEnd["関数名"];を追加する.
 *    ただしtimeFuncEndの後で値を返すようにしないと返値が変わってしまうので注意.
 * <プロファイリング結果の見方>
 *    (現在実行が終了した関数名) took (その関数実行に要した時間), elapsed time:(プログラム実行時間)
 *      function:(今までに呼び出された関数名)  calls:(呼び出された回数)  total time of this function:(その関数の合計実行時間)  average time:(その関数の平均実行時間)  max time:(その関数の最高実行時間)
 *    <例>
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
 * デバッグ用メッセージ出力関数
 * debugPrint：引数として与えられた要素要素を文字列にして出力する．（シンボルは評価してから表示）
 * simplePrint：引数として与えられた式を，「（評価前）:（評価後）」の形式で出力する．
 *)
 
SetAttributes[simplePrint, HoldAll];

symbolToString := (StringJoin[ToString[Unevaluated[#] ], ": ", ToString[InputForm[Evaluate[#] ] ] ])&;

SetAttributes[symbolToString, HoldAll];

If[optUseDebugPrint || True,  (* エラーが起きた時の対応のため，常にdebugPrintを返すようにしておく．いずれにしろそんなにコストはかからない？ *)
  debugPrint[arg___] := Print[InputForm[{arg}]];
  simplePrint[arg___] := Print[delimiterAddedString[", ",
    List@@Map[symbolToString, Map[Unevaluated, Hold[arg]] ]
     ] ],
  
  debugPrint[arg___] := Null;
  simplePrint[arg___] := Null
];

profilePrint[arg___] := If[optUseProfilePrint, Print[InputForm[arg]], Null];

(*
 * 関数呼び出しを再現するための文字列出力を行う
 *)
 
inputPrint[name_, arg___] := Print[StringJoin[name, "[", delimiterAddedString[",", Map[(ToString[InputForm[#] ])&,{arg}] ], "]" ] ];


delimiterAddedString[del_, {h_}] := h;

delimiterAddedString[del_, {h_, t__}] := StringJoin[h, del, delimiterAddedString[del, {t}] ];


SetAttributes[publicMethod, HoldAll];

(* C++側から直接呼び出す関数の，本体部分の定義を行う関数．デバッグ出力とか，正常終了の判定とか，例外の扱いとかを統一する 
   少しでもメッセージを吐く可能性のある関数は，この関数で定義するようにする．
   defineにReturnが含まれていると正常に動作しなくなる（Returnの引数がそのまま返ることになる）ので使わないように！
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



(* （不）等式の右辺と左辺を入れ替える際に，関係演算子の向きも反転させる．Notとは違う *)

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


(* 制約モジュールが矛盾する条件をセットする *)
setFalseConditions[co_, va_] := Module[
  {cons, vars},
  cons = co;
  falseConditions = cons;
  simplePrint[cons, falseConditions];
];

(* 変数のリストからprev変数を取り除く *)
removePrevVariables[vars_] := Module[
  {ret,i},
  ret = {};
  For[i=1,i<=Length[vars],i++,
    If[!isPrevVariable[vars[[i]]], ret=Append[ret,vars[[i]]]];
  ];
  ret
];

(* 矛盾する条件を整形して返す *)
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


(* 制約モジュールが矛盾する条件を見つけるための無矛盾性判定 *)
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


(* ポイントフェーズにおける無矛盾性判定 *)

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

(* インターバルフェーズにおける無矛盾性判定 *)

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
        (* 制約不足で微分方程式が完全には解けないなら，単純に各変数値およびその微分値が矛盾しないかを調べる *)
        (* Existsの第一引数はHold（HoldAll?）属性を持っているらしいので，Evaluateで評価する必要がある（気がする） *)
        tCons = Map[(# -> createIntegratedValue[#, sol[[3]] ])&, getTimeVars[vars]];
        tCons = sol[[2]] /. tCons;
        tmpPCons = If[getParameters[tCons] === {}, True, pcons];
        tCons = LogicalExpand[Quiet[Reduce[Exists[Evaluate[appendZeroVars[vars]], And@@tCons && tmpPCons], Reals]]],
        (* 微分方程式が解けた場合 *)
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



(* 変数もしくは記号定数とその値に関する式のリストを，表形式に変換 *)

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
        (* 右辺に変数名が残っている，つまり値が完全にtの式になっていない変数が出現した場合はunderConstraintを返す *)
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
      (* TODO: ここでprevに関する処理は本来なくてもいいはず．時刻0でのprevの扱いさえうまくできればどうにかなる？ *)
      map = cons /. (expr_ /; (( Head[expr] === Equal || Head[expr] === LessEqual || Head[expr] === Less|| Head[expr] === GreaterEqual || Head[expr] === Greater) && (!hasJudge[expr] || hasPrevVariable[expr])) -> True);
      map = Reduce[map, vars, Reals];
      (* TODO:2回も同じルール適用をしたくない．場合の重複や，不要な条件の発生を抑えつつ，何かできないか？ *)
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


(* 式中に変数名が出現するか否か *)

hasVariable[exprs_] := Length[StringCases[ToString[exprs], "usrVar" ~~ WordCharacter]] > 0;

(* 式が変数もしくはその微分そのものか否か *)

isVariable[exprs_] := MatchQ[exprs, _Symbol] && StringMatchQ[ToString[exprs], "usrVar" ~~ WordCharacter__] || MatchQ[exprs, Derivative[_][_][_] ] || MatchQ[exprs, Derivative[_][_] ] ;

(* 式中に出現する変数を取得 *)

getVariables[exprs_] := ToExpression[StringCases[ToString[exprs], "usrVar" ~~ WordCharacter..]];

(* 式中に出現する記号定数を取得 *)

getParameters[exprs_] := Cases[exprs, parameter[_, _, _], Infinity];

(* 時間変数を取得 *)
getTimeVars[list_] := Cases[list, _[t], Infinity];

(* 式中に定数名が出現するか否か *)

hasParameter[exprs_] := Length[Cases[exprs, parameter[_, _, _], Infinity]] > 0;

(* 式が定数そのものか否か *)

isParameter[exprs_] := Head[exprs] === parameter;

(* 式が指定されたシンボルを持つか *)
hasSymbol[exprs_, syms_List] := MemberQ[{exprs}, ele_ /; (MemberQ[syms, ele] || (!AtomQ[ele] && hasSymbol[Head[ele], syms]) ), Infinity ];

(* 式がprev変数そのものか否か *)
isPrevVariable[exprs_] := Head[exprs] === prev;

(* 式がprev変数を持つか *)
hasPrevVariable[exprs_] := Length[Cases[exprs, prev[_, _], Infinity]] > 0;


(* 必ず関係演算子の左側に変数名や定数名が入るようにする *)

adjustExprs[andExprs_, judgeFunction_] := 
Fold[
  (
   If[#2 === True,
    #1,
    If[Not[judgeFunction[#2[[1]] ] ] && judgeFunction[#2[[2]] ],
     (* 逆になってるので、演算子を逆にして追加する *)
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


(* 変数名からDerivativeやtを取り，微分回数とともに返す *)
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

(* AndではなくListでくくる *)

applyList[reduceSol_] :=
  If[Head[reduceSol] === And, List @@ reduceSol, List[reduceSol]];


(* OrではなくListでくくる *)

applyListToOr[reduceSol_] :=
  If[Head[reduceSol] === Or, List @@ reduceSol, List[reduceSol]];


(* Piecewiseの第二要素を，その条件とともに第一要素に付加してリストにする．条件がFalseなら削除 
   ついでに othersを各条件に対して付加 *)

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
 * 次のポイントフェーズに移行する時刻を求める
 *)

calculateNextPointPhaseTime[maxTime_, discCause_] := 
  calculateNextPointPhaseTime[maxTime, discCause, constraint, initConstraint, pConstraint, variables];



(* 変数とその値に関する式のリストを、変数表的形式に変換 *)
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


(* リストを整形する *)
(* FullSimplifyを使うと，Root&Functionが出てきたときにも結構簡約できる．というか簡約できないとエラーになるのでTODOと言えばTODO *)
(* TODO:複素数の要素に対しても，任意精度への対応（文字列への変換とか）を行う *)

(* convertExprs[list_] := Map[({removeDash[ #[[1]] ], getExprCode[#], integerString[FullSimplify[#[[2]] ] ] } )&, list]; *)
convertExprs[list_] := Map[({removeDash[ #[[1]] ], getExprCode[#], integerString[#[[2]] ] } )&, list];


(* 時刻と条件の組で，条件が論理和でつながっている場合それぞれに分解する *)
divideDisjunction[timeCond_] := Map[({timeCond[[1]], #, timeCond[[3]]})&, List@@timeCond[[2]]];


(* 最大時刻と時刻と条件との組を比較し，最大時刻の方が早い場合は1を付加したものを末尾に，
  そうでない場合は0を末尾に付加して返す．条件によって変化する場合は，条件を絞り込んでそれぞれを返す *)
compareWithMaxTime[maxT_, timeCond_] := 
Module[
  {sol, tmpCond},
  sol = Reduce[maxT <= timeCond[[1]] && timeCond[[2]], Reals];
  If[sol === False,
    {{timeCond[[1]], timeCond[[2]], 0}},
    tmpCond = Reduce[(!sol && timeCond[[2]]), Reals];
    If[tmpCond === False,  (* 条件を満たす範囲で常にmaxT <= timeCond[[1]]が成り立つとき *)
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
    
    (* まず微分方程式を解く．うまくやればcheckConsistencyIntervalで出した結果(tStore)をそのまま引き継ぐこともできるはず *)
    dSol = exDSolve[cons, initCons];
    
    debugPrint["dSol after exDSolve", dSol];
    
    (* 次にそれらをdiscCauseに適用する *)
    timeAppliedCauses = False;
    
    tStore = Map[(# -> createIntegratedValue[#, dSol[[2]] ])&, getTimeVars[vars]];
    timeAppliedCauses = Or@@(discCause /. tStore );
    simplePrint[timeAppliedCauses];
    
    parameterList = getParameters[timeAppliedCauses];
    
    (* 必要なpConsだけを選ぶ．不要なものが入っているとMinimzeの動作がおかしくなる？ *)
    
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
    
    (* 整形して結果を返す *)
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
      exprs  /. integRule     (* 単純にルールを適用 *)
             /. Map[((#[[1]] /. x_[t]-> x) -> #[[2]] )&, integRule]
             /. (Derivative[n_][f_][t] /; !isVariable[f]) :> D[f, {t, n}] (* 微分値についてもルールを適用 *)
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

(* 微分方程式系を解く．
  単にDSolveをそのまま使用しない理由は以下．
    理由1: 下記のような微分方程式系に対して弱い．
      DSolve[{usrVarz[t] == usrVarx[t]^2, usrVarx'[t] == usrVarx[t], usrVarx[0] == 1}, {usrVarx[t], usrVarz[t]}, t]
    理由2: 不等式に弱い
    理由3: bvnulなどの例外処理を統一したい
  @param expr 時刻に関する変数についての制約
  @param initExpr 変数の初期値についての制約
  @return overConstraint | 
    {underConstraint, 変数値が満たすべき制約（ルールに含まれているものは除く），各変数の値のルール} |
    {変数値が満たすべき制約（ルールに含まれているものは除く），各変数の値のルール} 
    TODO: Subsetsとか使ってるから式の数で簡単に爆発する
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
    For[i=2,i<=Length[subsets], i++, (* 添え字が2からなのは最初の空集合を無視するため *)
      subset = subsets[[i]];
      tVars = Union[getVariables[subset]];
      If[Length[tVars] == Length[subset],
        ini = Select[tmpInitExpr, (hasSymbol[#, tVars ])& ];
        If[optOptimizationLevel == 1 || optOptimizationLevel == 4, 
          (* 微分方程式の結果を再利用する場合 *)
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
        (* TODO: DSolveの結果が複数ある場合への対応 *)
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
 * 式に対して与えられた時間を適用する
 *)

publicMethod[
  applyTime2Expr,
  expr, time,
  Module[
    {appliedExpr},
    (* FullSimplifyだと処理が重いが，SimplifyだとMinimize:ztestが出現しやすい *)
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
 * 与えられた式を近似する
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
 * 与えられたtの式をタイムシフト
 *)

publicMethod[
  exprTimeShift,
  expr, time,
  integerString[expr /. t -> t - time]
];
