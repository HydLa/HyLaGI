(* （不）等式の右辺と左辺を入れ替える際に，関係演算子の向きも反転させる．Notとは違う *)

getReverseRelop[relop_] := Switch[relop,
                                  Equal, Equal,
                                  Less, Greater,
                                  Greater, Less,
                                  LessEqual, GreaterEqual,
                                  GreaterEqual, LessEqual];

checkConditions[] := (
  checkConditions[prevConstraint, falseConditions, pConstraint, prevVariables ]
);

publicMethod[
  checkConditions,
  pCons, fCond, paramCons, vars,
  Module[
  {prevCons, falseCond, trueMap, falseMap, cpTrue, cpFalse, cpTmp},
   debugPrint["fcond",fCond];
   If[fCond === 1, 
    {True,False},
    prevCons = pCons;
    prevCons = prevCons /. Rule->Equal;
    If[prevCons[[0]] == List, prevCons[[0]] = And;];
    Quiet[
      cpTrue = Reduce[Exists[vars, Simplify[prevCons&&fCond&&paramCons] ], Reals], {Reduce::useq}
    ];
    simplePrint[cpTrue];
    checkMessage;
    Quiet[
      cpFalse = Reduce[paramCons && !cpTrue, Reals], {Reduce::useq}
    ];
    checkMessage;
    simplePrint[cpFalse];
      (*
    Quiet[
      cpTmp = Reduce[Exists[vars, prevCons && fCond],Reals];
      cpTrue = Reduce[cpTmp && paramCons, Reals];
      cpFalse = Reduce[!cpTmp && paramCons, Reals];
    ];
       
    falseCond = applyListToOr[LogicalExpand[fCond]];
    Quiet[
      cpTmp = ParallelMap[Reduce[Exists[vars, prevCons && #], Reals]&, falseCond], {Reduce::useq}
    ];
    If[cpTmp[[0]] == List, cpTmp[[0]] = Or;];
    cpTmp = Reduce[cpTmp,Reals];
    simplePrint[cpTmp];
    checkMessage;
    Quiet[
      cpTrue = Reduce[cpTmp && paramCons, Reals], {Reduce::useq}
    ];
    checkMessage;
    simplePrint[cpTrue];
    Quiet[
      cpFalse = Reduce[!cpTmp && paramCons, Reals], {Reduce::useq}
    ];
    checkMessage;
    simplePrint[cpFalse];
       *)
    {trueMap, falseMap} = Map[(createMap[#, isParameter, hasParameter, {}])&, {cpTrue, cpFalse}];
    simplePrint[trueMap, falseMap];
    {trueMap, falseMap}
   ]
  ]
];

(* 制約モジュールが矛盾する条件をセットする *)
setConditions[co_, va_] := Module[
  {cons, vars},
  cons = co;
  falseConditions = cons;
  simplePrint[cons, falseConditions];
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
findConditions[] := (
  findConditions[constraint && tmpConstraint && initConstraint && initTmpConstraint, guard, variables ]
);

publicMethod[
  findConditions,
  cons, gua, vars,
  Module[
    {i, falseMap, cp},
    Quiet[
      cp = Reduce[Exists[vars, cons],Reals], {Reduce::useq}
    ];
    simplePrint[cp];
    checkMessage;
    If[cp =!= False && cp =!= True,
      cp = LogicalExpand[Simplify[cp] ];
      cp = integerString[cp];
    ];
    simplePrint[cp];
    cp
  ]
];

(* ポイントフェーズにおける無矛盾性判定 *)

checkConsistencyPoint[] := (
  checkConsistencyPoint[constraint && tmpConstraint && initConstraint && initTmpConstraint, pConstraint, Union[variables, prevVariables] ]
);

publicMethod[
  checkConsistencyPoint,
  cons, pcons, vars,
  Module[
    {trueMap, falseMap, cpTrue, cpFalse},

    (* ここでのSimplifyは不要な気がする *)
    Quiet[
      cpTrue = Reduce[Exists[vars, Simplify[cons&&pcons] ], Reals], {Reduce::useq}
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
  checkConsistencyInterval[constraint && tmpConstraint, initConstraint && initTmpConstraint, pConstraint, Union[timeVariables, prevVariables, initVariables] ]
);

appendZeroVars[vars_] := Join[vars, vars /. x_[t] -> x[0]];

publicMethod[
  checkConsistencyInterval,
  cons, initCons, pcons, vars,
  Module[
    {sol, otherCons, tCons, hasTCons, necessaryTCons, parList, tmpPCons, cpTrue, cpFalse, trueMap, falseMap},
    If[cons === True,
      {createMap[pcons, isParameter,hasParameter, {}], False},
      sol = exDSolve[cons, initCons];
      debugPrint["sol after exDSolve", sol];
      If[sol === overConstraint,
        {False, pcons},
        If[sol[[1]] === underConstraint,
          (* 制約不足で微分方程式が完全には解けないなら，単純に各変数値およびその微分値が矛盾しないかを調べる *)
          (* Existsの第一引数はHold （HoldAll?）属性を持っているらしいので，Evaluateで評価する必要がある *)
          tCons = Map[(# -> createIntegratedValue[#, sol[[3]] ])&, getTimeVars[vars]];
          tCons = sol[[2]] /. tCons;
          tmpPCons = If[getParameters[tCons] === {}, True, pcons];
          tCons = LogicalExpand[Quiet[Reduce[Exists[Evaluate[appendZeroVars[vars]], And@@applyList[tCons] && tmpPCons], Reals]]],
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

createVariableMapInterval[] := createVariableMapInterval[constraint, initConstraint, timeVariables, parameters];

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
];

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
      
      map = Map[(adjustExprs[#, judge])&, map];
      map = Map[(convertExprs[#])&, map];
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

getPrevs[exprs_] := Cases[exprs, prev[_, _], Infinity];

(* 時間変数を取得 *)
getTimeVars[list_] := Cases[list, _[t], Infinity];

(* 初期値変数を取得 *)
getInitVars[expr_] := Cases[expr, _[0], Infinity];

hasInitVars[expr_] := (Length[getInitVars[expr] ] > 0);

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

publicMethod[
  resetConstraint,
  constraint = True;
  initConstraint = True;
  pConstraint = True;
  prevConstraint = {};
  initTmpConstraint = True;
  tmpConstraint = True;
  isTemporary = False;
  parameters = {};
];

publicMethod[
  resetConstraintForVariable,
  constraint = True;
];

addConstraint[co_List] := addConstraint[And@@List];

publicMethod[
  addConstraint,
  co,
  Module[
    {cons},
    cons = co //. prevConstraint;
    If[isTemporary,
      tmpConstraint = tmpConstraint && cons,
      constraint = constraint && cons
      ];
    simplePrint[cons, constraint, tmpConstraint];
  ]
];

addInitConstraint[co_] := Module[
  {cons, vars},
  cons = co //. prevConstraint;
  If[isTemporary,
    initTmpConstraint = initTmpConstraint && cons,
    initConstraint = initConstraint && cons
  ];
  simplePrint[cons, initConstraint, initTmpConstraint];
];

publicMethod[
  addPrevConstraint,
  co,
  Module[
    {cons},
    cons = co;
    If[cons =!= True,
       prevConstraint = Union[prevConstraint, Map[(Rule@@#)&, applyList[cons]]]
       ];
    simplePrint[cons, prevConstraint];
  ]
];

publicMethod[
  simplify,
  arg,
  integerString[Simplify[arg]]
];


makePrevVar[var_] := Module[
  {name, dcount},
  If[MatchQ[var, Derivative[_][_] ],
    name = var[[1]];
    dcount = var[[0]][[1]],
    name = var;
    dcount = 0
  ];
  name = ToString[name];
  (* drop "usrVar" *)
  name = StringDrop[name, 6];
  name = ToExpression[name];
  prev[name, dcount]
];

publicMethod[
  addVariables,
  vars,
  Unprotect[variables, prevVariables, timeVariables, initVariables];
  variables = Union[variables, vars];
  prevVariables = Union[prevVariables,
    Map[makePrevVar, vars] ];
  timeVariables = Union[timeVariables, Map[(#[t])&, vars] ];
  initVariables = Union[initVariables, Map[(#[0])&, vars] ];
  simplePrint[variables, prevVariables, timeVariables, initVariables];
  Protect[variables, prevVariables, timeVariables, initVariables];
];


publicMethod[
  addVariable,
  var,
  Unprotect[variables, prevVariables, timeVariables, initVariables];
  variables = Union[variables, {var}];
  prevVariables = Union[prevVariables,
  {makePrevVar[var]} ];
  timeVariables = Union[timeVariables, {var[t] } ];
  initVariables = Union[initVariables, {var[t]} ];
  simplePrint[variables, prevVariables, timeVariables, initVariables];
  Protect[variables, prevVariables, timeVariables, initVariables];
];


setVariables[vars_] := (
  Unprotect[variables, prevVariables, timeVariables, initVariables];
  variables = vars;
  prevVariables = Map[makePrevVar, vars];
  timeVariables = Map[(#[t])&, vars];
  initVariables = Map[(#[0])&, vars];
  simplePrint[variables, prevVariables, timeVariables, initVariables];
  Protect[variables, prevVariables, timeVariables, initVariables];
);


publicMethod[
  startTemporary,
  isTemporary = True;
];

publicMethod[
  endTemporary,
  isTemporary = False;
  resetTemporaryConstraint[];
];

resetTemporaryConstraint[] := (
  tmpConstraint = True;
  initTmpConstraint = True;
);

resetConstraintForParameter[pcons_] := (
  pConstraint = True;
  addParameterConstraint[pcons];
);


publicMethod[
  addInitEquation,
  lhs, rhs,
  addInitConstraint[lhs == rhs]
];

publicMethod[
  addEquation,
  lhs, rhs,
  Module[
    {cons},
    cons = lhs == rhs //. prevConstraint;
    If[isTemporary,
      tmpConstraint = tmpConstraint && cons,
      constraint = constraint && cons
      ];
    simplePrint[cons, constraint, tmpConstraint];
  ]
];

publicMethod[
  addParameterConstraint,
  pcons,
  pConstraint = Reduce[pConstraint && pcons, Reals];
  simplePrint[pConstraint];
];

publicMethod[
  addParameter,
  par,
  parameters = Union[parameters, {par}]
];

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
  calculateNextPointPhaseTime[maxTime, discCause, constraint, initConstraint, pConstraint, timeVariables];

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
       /. (Derivative[cnt_, var_, ___]  :> derivative[cnt, var])
       /. (x_ :> ToString[InputForm[x]] /; Head[x] === Root )
       /. (x_Rational :> Rational[replaceIntegerToString[Numerator[x] ], replaceIntegerToString[Denominator[x] ] ] )
       /. (x_Integer :> replaceIntegerToString[x])
);

(* リストを整形する *)
(* FullSimplifyを使うと，Root&Functionが出てきたときにも結構簡約できる．というか簡約できないとエラーになるのでTODOと言えばTODO *)
(* TODO:複素数の要素に対しても，任意精度への対応 （文字列への変換とか）を行う *)

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
    
    (* まず微分方程式を解く．うまくやればcheckConsistencyIntervalで出した結果 (tStore)をそのまま引き継ぐこともできるはず *)
    dSol = exDSolve[cons, initCons];
    
    debugPrint["dSol after exDSolve", dSol];
    
    (* 次にそれらをdiscCauseに適用する *)
    timeAppliedCauses = False;
    
    tStore = Map[(# -> createIntegratedValue[#, dSol[[2]] ])&, getTimeVars[vars]];
    timeAppliedCauses = Or@@(applyList[discCause] /. tStore );
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
    simplePrint[resultList];
    
    resultList = Fold[(Join[#1, compareWithMaxTime[If[Quiet[Reduce[maxTime <= 0, Reals]] === True, 0, maxTime], #2] ])&,{}, resultList];
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
    理由1: 下記のような入力に対して弱い．
      DSolve[{z[t] == x[t]^2, Derivative[1][x][t] == x[t], x[0] == 1}, {x[t], z[t]}, t]
    理由2: 不等式に弱い
    理由3: bvnulなどの例外処理を統一したい
  @param expr 時刻に関する変数についての制約
  @param initExpr 変数の初期値についての制約
  @return overConstraint | 
    {underConstraint, 変数値が満たすべき制約 （ルールに含まれているものは除く），各変数の値のルール} |
    {変数値が満たすべき制約 （ルールに含まれているものは除く），各変数の値のルール} 
*)

exDSolve[expr_, initExpr_] := 
Check[
  Module[
    {tmpExpr, reducedExpr, rules, tVars, tVar, resultCons, exprSet, resultRule, idx, generalInitValue, swapValue, searchResult},
    inputPrint["exDSolve", expr, initExpr];
    tmpExpr = applyList[expr];
    sol = {};
    resultCons = Select[tmpExpr, (Head[#] =!= Equal)&];
    tmpExpr = Complement[tmpExpr, resultCons];
    reducedExpr = Quiet[Check[Reduce[tmpExpr, Reals], tmpExpr], {Reduce::nsmet, Reduce::useq}];
    (* Reduceの結果が使えそうな場合のみ使う *)
    If[Head[reducedExpr] === And && MemberQ[reducedExpr, Element, Infinity, Heads->True], tmpExpr = applyList[reducedExpr] ];
    tmpInitExpr = applyList[initExpr];
    resultRule = {};
    simplePrint[resultCons, tmpExpr];
    While[True, 
      searchResult = searchExprsAndVars[tmpExpr];
      If[searchResult === unExpandable,
        Break[],
        rules = solveByDSolve[searchResult[[1]], tmpInitExpr, searchResult[[3]]];
        If[rules === overConstraint || Head[rules] === DSolve || Length[rules] == 0, Return[overConstraint] ];
        (* TODO:rulesの要素数が2以上，つまり解が複数存在する微分方程式系への対応 *)
        resultRule = Union[resultRule, rules[[1]] ];
        tmpExpr = applyDSolveResult[searchResult[[2]], rules[[1]] ];
        If[MemberQ[tmpExpr, ele_ /; (ele === False || (!hasVariable[ele] && MemberQ[ele, t, Infinity]) )], Return[overConstraint] ];
        tmpExpr = Select[tmpExpr, (#=!=True)&];
        simplePrint[tmpExpr];
        resultCons = applyDSolveResult[resultCons, rules[[1]] ];
        If[MemberQ[resultCons, False], Return[overConstraint] ];
        resultCons = Select[resultCons, (#=!=True)&]
      ]
    ];
    
    resultCons = And@@resultCons;
    
    
    If[Length[tmpExpr ] > 0,
      {underConstraint, resultCons && And@@tmpExpr, resultRule},
      {resultCons && And@@tmpExpr, resultRule}
    ]
  ],
  Message[exDSolve::unkn]
];



(* 式の数とそこに出現するが一致するまで再帰的に呼び出す関数
  @param exprs 探索対象となる式集合
  @return unExpandable | {(DSolveすべき式の集合）, (残りの式の集合)， （DSolveすべき変数の集合）}
*)
searchExprsAndVars[exprs_] :=
Module[
  {tmpExprs, droppedExprs, searchResult = unExpandable, tVarsMap, tVars},
  
  For[i=1, i<=Length[exprs], i++,
    tVarsMap[ exprs[[i]] ] = Union[getVariables[exprs[[i]] ] ];
    tVars = tVarsMap[ exprs[[i]] ];
    If[Length[tVars] == 1,
      searchResult = {{ exprs[[i]] }, Drop[exprs, {i}], tVars}
    ]
  ];
  
  If[searchResult === unExpandable,
    tmpExprs = Sort[exprs, (Length[tVarsMap[#1] ] < Length[tVarsMap[#2] ])&];
    For[tmpExprs, Length[tmpExprs] > 0 && searchResult === unExpandable, tmpExprs = droppedExprs,
      droppedExprs = Drop[tmpExprs, 1];
      searchResult = searchExprsAndVars[{tmpExprs[[1]]}, tVarsMap[tmpExprs[[1]] ], droppedExprs, tVarsMap]
    ]
  ];
  simplePrint[searchResult];
  searchResult
];

(* 再帰的に呼び出す関数
  @param searchedExpr これまでに見つけた式の集合
  @param searchedVars これまでに見つけた変数の集合
  @param exprs 探索対象となる式集合
  @param tVarsMap exprsの各要素と，そこに出現する変数集合のマップ
  @return unExpandable | {(DSolveすべき式の集合）, （DSolveすべき変数の集合）}
*)
searchExprsAndVars[searchedExprs_, searchedVars_, exprs_, tVarsMap_] :=
Module[
  {tVar, tVarsInExpr, unionVars, i, j, k, appendExprs, searchResult},
  inputPrint["searchExprsAndVars", searchedExprs, searchedVars, exprs, tVarsMap];
  For[i=1, i<=Min[Length[searchedVars], 2], i++,
    (* 解けない変数が2つ以上含まれるなら候補には入らないはずなので，2とのMinをとる *)
    tVar = searchedVars[[i]];
    simplePrint[tVar];
    For[j=1, j<=Length[exprs], j++,
      tVarsInExpr = tVarsMap[ exprs[[j]] ];
      If[MemberQ[tVarsInExpr, tVar],
        unionVars = Union[tVarsInExpr, searchedVars];
        appendExprs = Append[searchedExprs, exprs[[j]] ];
        If[Length[unionVars] == Length[appendExprs],
          Return[{appendExprs, Drop[exprs, {j}], unionVars}],
          searchResult = searchExprsAndVars[appendExprs, unionVars, Drop[exprs, {j}], tVarsMap];
          If[searchResult =!= unExpandable, Return[searchResult] ]
        ]
      ]
    ]
  ];
  unExpandable
];

(* 渡された式をDSolveで解いて，結果のRuleを返す．
  @param expr: DSolveに渡す微分方程式系．形式はリスト．
  @param initExpr: 初期値制約のリスト．余計なものが含まれていてもよい
  @param tVars: exprに出現する変数のリスト
*)  
solveByDSolve[expr_, initExpr_, tVars_] :=
Module[
  {tmpExpr, ini, sol, idx, generalInitValue, swapValue, j},
  tmpExpr = expr;
  ini = Select[initExpr, (hasSymbol[#, tVars ])& ];
  simplePrint[tmpExpr, ini];
  
  If[optOptimizationLevel == 1 || optOptimizationLevel == 4, 
    (* 微分方程式の結果を再利用する場合 *)

    idx = Position[Map[(Sort[#])&,dList],Sort[tmpExpr]];
    If[idx == {},
      generalInitValue = ini;
      For[j=1,j<=Length[generalInitValue],j++,
        generalInitValue[[j, 1]] = ini[j];
      ];
      sol = Check[
        DSolve[Union[tmpExpr, generalInitValue], Map[(#[t])&, tVars], t],
        overConstraint,
        {DSolve::overdet, DSolve::bvnul}
      ];
      For[j=1,j<=Length[generalInitValue],j++,
        generalInitValue[[j, 0]] = Rule;
      ];
      dList = Append[dList,{tmpExpr,sol,generalInitValue}];
      idx = Position[dList,tmpExpr],
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
    sol = Quiet[Check[
        DSolve[Union[tmpExpr, ini], Map[(#[t])&, tVars], t],
            overConstraint,
        {DSolve::overdet, DSolve::bvnul}
      ],
     {DSolve::overdet, DSolve::bvnul}
    ]
  ];
  sol
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

makeIntervalRulesList[pcons_] := makeIntervalRulesList[pcons] = 
Module[
  {rules, iter, plist, pc, appearedp = {}, ret = {} var},
  plist = applyList[LogicalExpand[pcons] ];
  plist = adjustExprs[plist, isParameter];
  simplePrint[plist];
  For[iter = 1, iter <= Length[plist], iter++,
    pc = plist[[iter]];
    var = pc[[1]];
    
    If[Head[rules[var] ] =!= Rule,
      appearedp = Append[appearedp, var]
    ];
    
    If[ MemberQ[{Less, LessEqual}, Head[ pc ] ],
      If[Head[rules[var] ] === Rule,
        rules[var] = var -> Interval[{rules[var][[2]][[1]][[1]], pc[[2]] } ],
        rules[var] = var -> Interval[{-Infinity, pc[[2]]}]
      ]
    ];
    If[ MemberQ[{Greater, GreaterEqual}, Head[ pc ] ],
      If[Head[rules[var] ] === Rule,
        rules[var] = var -> Interval[{pc[[2]], rules[var][[2]][[1]][[2]] }],
        rules[var] = var -> Interval[{pc[[2]], Infinity}]
      ]
    ];
    If[ Head[ pc ]=== Equal,
      rules[var] = var -> Interval[pc[[2]] ]
    ];
  ];
  For[iter = 1, iter <= Length[appearedp], iter++,
    ret = Append[ret, rules[appearedp[[iter]] ] ]
  ];
  ret
];

approxValue[val_] := approxValue[val, pConstraint, approxMode, approxPrecision, approxThreshold];


approxValue[val_, mode_] := approxValue[val, pConstraint, mode, approxPrecision, 0];

(*
 * approx given value
 * approxMode === none: do nothing
 * approxMode === numeric: numeric->numeric，interval->interval (invalid for expressions with parameters)
 * approxMode === interval: numeric->interval，interval->interval
 *)
publicMethod[
  approxValue,
  val, pcons, mode, precision, threshold,
  Module[
    {lb, ub, itv},
    If[mode === none || LeafCount[val] <= 2*threshold || hasVariable[val],
      {0},
      If[mode === numeric,
        If[Length[val] == 1,
          {1, integerString[approxExpr[precision, val[[1]] ] ] },
          {1, integerString[approxExpr[precision, val[[1]] ] ], integerString[approxExpr[precision, val[[2]] ] ] }
        ],
        (* if approxMode === interval *)
        If[Length[val] == 1,
          (* make interval from exact value *)
          itv = getInterval[val[[1]], pcons, precision];
          simplePrint[itv];
          itv = integerString[{itv[[1]][[1]], itv[[1]][[2]]}];
          simplePrint[itv];
          Join[{1}, itv],
          (* make interval from interval *)
          lb = getInterval[val[[1]], pcons, precision];
          ub = getInterval[val[[2]], pcons, precision];
          simplePrint[lb, ub];
          Join[{1}, integerString[{Min[lb[[1]][[1]], ub[[1]][[1]] ], Max[lb[[1]][[2]], ub[[1]][[2]] ]}] ]
        ]
      ]
    ]
  ]
];

getInterval[expr_, pcons_, precision_] := Module[
  {tmp},
  tmp = If[pcons =!= True, expr /. makeIntervalRulesList[pcons], expr ];
  If[Head[tmp] =!= Interval, tmp = Interval[{tmp, tmp}] ];
  tmp = N[tmp, 10];
  tmp = Rationalize[tmp, 0];
  tmp
];


approxExpr[precision_, expr_] := (
  Rationalize[
    N[Simplify[expr], precision + 3],
    Divide[1, Power[10, precision] ]
  ]
);


linearApprox[val_, precision_] := linearApprox[val, pConstraint, precision];

(*
 * linear approximation
 *)

publicMethod[
  linearApprox,
  val, pcons, precision,
  Module[
    {res}, 
    res = primaryTaylorExpansion[val, pcons, precision];
    res = If[res[[2]] == 0, {integerString[res[[1]] ]},  {integerString[res[[1]] ], integerString[res[[2]] ], integerString[res[[3]] ]} ];
    res 
  ]
];

(* @return {linear approximated value, lb of interval, ub of interval} *)
primaryTaylorExpansion[expr_, pcons_, precision_] := Module[
  {i, tmp, pars, par, pRules, zeroRules, linear, coef, itv = 0},
  pRules = If[pcons =!= True, makeIntervalRulesList[pcons], {} ];
  pars = Union[getParameters[pRules]];
  simplePrint[pRules];
  (* calculate f(0) *)
  zeroRules = Map[(#[[1]] -> 0)&, pRules];
  simplePrint[zeroRules];
  linear = expr /. zeroRules;
  simplePrint[linear];
  For[i = 1, i <= Length[pars], i++,
    par = pars[[i]];
    coef = D[expr, par ];
    simplePrint[coef];
    coef = coef /. zeroRules;
    coef = Rationalize[N[coef, precision], 0];
    itv = itv + (Max[coef] - Min[coef])/2;
    coef = (Max[coef] + Min[coef])/2;
    simplePrint[itv];
    linear = linear + coef * par
  ];
  linear = linear + coef * par;
  simplePrint[linear, itv];
  {linear, -itv, itv}
];

(* 
 * 与えられたtの式をタイムシフト
 *)

publicMethod[
  exprTimeShift,
  expr, time,
  integerString[expr /. t -> t - time]
];

(*
 * 以下，HAConverter用
 * v1(now) in v2(past) => true
 *)

publicMethod[checkIncludeBound, v1, v2, 
  Module[{minPast, maxPast, minNow, maxNow, tmp, reduceExprPast, 
    reduceExprNow},
   minPast = 
    Quiet[Minimize[{v2, pConstraintPast}, 
      If[Union[getParameters[v2 && pConstraintPast]] =!= {}, 
       Union[getParameters[v2 && pConstraintPast]], {tmp}], 
      Reals], {Minimize::wksol, Minimize::infeas}];
   simplePrint[minPast];
   maxPast = 
    Quiet[Maximize[{v2, pConstraintPast}, 
      If[Union[getParameters[v2 && pConstraintPast]] =!= {}, 
       Union[getParameters[v2 && pConstraintPast]], {tmp}], 
      Reals], {Maximize::wksol, Maximize::infeas}];
   simplePrint[maxPast];
   minNow = 
    Quiet[Minimize[{v1, pConstraintNow}, 
      If[Union[getParameters[v1 && pConstraintNow]] =!= {}, 
       Union[getParameters[v1 && pConstraintNow]], {tmp}], 
      Reals], {Minimize::wksol, Minimize::infeas}];
   simplePrint[minNow];
   maxNow = 
    Quiet[Maximize[{v1, pConstraintNow}, 
      If[Union[getParameters[v1 && pConstraintNow]] =!= {}, 
       Union[getParameters[v1 && pConstraintNow]], {tmp}], 
      Reals], {Maximize::wksol, Maximize::infeas}];
   simplePrint[maxNow];
   (* not (minPast < tmp < maxPast) && minNow < tmp < 
   maxNow (=はcheckIncludeの戻り値で判断) *)
   minPast = checkInclude[minPast][[2]];
   maxPast = checkInclude[maxPast][[2]];
   minNow = checkInclude[minNow][[2]];
   maxNow = checkInclude[maxNow][[2]];
   If[minPast[[1]] == maxPast[[1]], 
    reduceExprPast = tmp == minPast[[1]];,
    If[minPast[[2]] == 0, reduceExprPast = minPast[[1]] < tmp;, 
     reduceExprPast = minPast[[1]] <= tmp;];
    If[maxPast[[2]] == 0, 
     reduceExprPast = reduceExprPast && tmp < maxPast[[1]];, 
     reduceExprPast = reduceExprPast && tmp <= maxPast[[1]];];
    ];
   If[minNow[[1]] == maxNow[[1]], reduceExprNow = tmp == minNow[[1]];,
    If[minNow[[2]] == 0, reduceExprNow = minNow[[1]] < tmp;, 
     reduceExprNow = minNow[[1]] <= tmp;];
    If[maxNow[[2]] == 0, 
     reduceExprNow = reduceExprNow && tmp < maxNow[[1]];, 
     reduceExprNow = reduceExprNow && tmp <= maxNow[[1]];];
    ];
   simplePrint[reduceExprPast && reduceExprNow];
   If[Reduce[
      Not[reduceExprPast] && reduceExprNow] === False, integerString[1], 
    integerString[0]]]
];

publicMethod[checkInclude, includeBound, 
 Module[{flg, flgInclude, res, tmpT0}, flg = True; flgInclude = True;
   If[Cases[includeBound[[1]], {_, _}] === {}, res = {includeBound[[1]], 1};,
     For[i = 1, i <= Length[includeBound[[1, 1]]], i = i + 1,
       If[Reduce[Not[includeBound[[1, 1, i, 2]]] && t == 0] === False,
         (* t==0 の場合があったら開区間、なかったら閉区間 （あやしい） *)
         flgInclude = False;
       ];
       If[Reduce[Not[includeBound[[1, 1, i, 2]]] && t > 0] === False,
         flg = False;
         res = {includeBound[[1, 1, i, 1]], 0};
       ];
     ](* For *);
     If[flg === True,
       If[flgInclude === True,
         res = {includeBound[[1, 2]], 1};,
         res = {includeBound[[1, 2]], 0};
       ];
     ];
   ](* If *);
   Limit[res, t -> 0, Direction -> -1]
 ](* Module *)
];

addParameterConstraintNow[pcons_, pars_] := (
     pConstraintNow = True;
     parametersNow = {};
     pConstraintNow = Reduce[pConstraintNow && pcons, Reals];
     parametersNow = Union[parametersNow, pars];
     simplePrint[pConstraintNow];
     );

addParameterConstraintPast[pcons_, pars_] := (
     pConstraintPast = True;
     parametersPast = {};
     pConstraintPast = Reduce[pConstraintPast && pcons, Reals];
     parametersPast = Union[parametersPast, pars];
     simplePrint[pConstraintPast];
     );
