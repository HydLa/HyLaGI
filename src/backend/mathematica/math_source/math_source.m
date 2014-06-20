
(* ポイントフェーズにおける無矛盾性判定 *)

checkConsistencyPoint[] := (
  checkConsistencyPoint[constraint && tmpConstraint && initConstraint && initTmpConstraint && prevIneqs, pConstraint, Union[variables, prevVariables], parameters ]
);

publicMethod[
  checkConsistencyPoint,
  cons, pcons, vars, pars,
  Module[
    {cpTrue, cpFalse},
    
    Quiet[
      cpTrue = Reduce[Exists[vars, cons && pcons], pars, Reals], {Reduce::useq}
    ];
    simplePrint[cpTrue];
    (* remove (Not)Element[] because it seems to be always true *)
    cpTrue = cpTrue /. {NotElement[_, _] -> True, Element[_, _] -> True};
    checkMessage;
    Quiet[
      cpFalse = Reduce[pcons && !cpTrue, pars, Reals], {Reduce::useq}
    ];
    checkMessage;
    simplePrint[cpFalse];
    toReturnForm[{{LogicalExpand[cpTrue]}, {LogicalExpand[cpFalse]}}]
  ]
];

(* インターバルフェーズにおける無矛盾性判定 *)

checkConsistencyInterval[] :=  (
  checkConsistencyInterval[constraint && tmpConstraint, initConstraint && initTmpConstraint, prevIneqs, pConstraint, parameters]
);

moveTermsToLeft[expr_] := Head[expr][expr[[1]] - expr[[2]], 0];

ccIntervalForEach[cond_, initRules_, pCons_] :=
Module[
  {
    operator,
    lhs,
    eqSol,
    gtSol,
    ltSol,
    trueCond
  },
  inputPrint["ccIntervalForEach", cond, initRules, pCons];
  If[cond === True || cond === False, Return[cond]];
  operator = Head[cond];
  lhs = (cond[[1]] - cond[[2]] ) /. t -> 0 /. initRules;
  simplePrint[lhs];
  (* caused by underConstraint *)
  If[hasVariable[lhs], Return[pCons] ];

  trueCond = False;

  eqSol = Quiet[Reduce[lhs == 0 && pCons, Reals] ];
  If[eqSol =!= False,
    eqSol = ccIntervalForEach[operator[D[cond[[1]], t], D[cond[[2]], t]], initRules, eqSol];
    simplePrint[eqSol];
    trueCond = trueCond || eqSol
  ];
  If[MemberQ[{Unequal, Greater, GreaterEqual}, operator],
    gtSol = Quiet[Reduce[lhs > 0 && pCons, Reals] ];
    simplePrint[gtSol];
    trueCond = trueCond || gtSol
  ];
  If[MemberQ[{Unequal, Less, LessEqual}, operator],
    ltSol = Quiet[Reduce[lhs < 0 && pCons, Reals] ];
    simplePrint[ltSol];
    trueCond = trueCond || ltSol
  ];
  trueCond
];


publicMethod[
  checkConsistencyInterval,
  cons, initCons, prevCons, pCons, pars,
  Module[
    {sol, timeVars, prevVars, tCons, i, j, conj, cpTrue, eachCpTrue, cpFalse},
    If[cons === True,
      {{LogicalExpand[pCons]}, {False}},
      sol = exDSolve[cons, initCons];
      timeVars = Map[(#[t])&, getVariables[cons] ];
      prevVars = getPrevVariables[initCons];
      debugPrint["sol after exDSolve", sol];
      If[sol === overConstraint,
        {{False}, {LogicalExpand[pCons]}},
        tCons = Map[(Rule@@#)&, createDifferentiatedEquations[timeVars, sol[[3]] ] ];
        tCons = sol[[2]] /. tCons;
        simplePrint[tCons];

        cpTrue = False;
        For[i = 1, i <= Length[tCons], i++,
          conj = tCons[[i]];
          eachCpTrue = prevCons && pCons;
          For[j = 1, j <= Length[conj], j++,
            eachCpTrue = eachCpTrue && ccIntervalForEach[conj[[j]], Map[(Rule@@#)&, applyList[initCons] ], eachCpTrue]
          ];
          cpTrue = cpTrue || eachCpTrue
        ];
        cpFalse = Reduce[!cpTrue && pCons && prevCons, Join[pars, prevVars], Reals];
        toReturnForm[{{LogicalExpand[cpTrue]}, {LogicalExpand[cpFalse]}}]
      ]
    ]
  ]
];


(* 変数もしくは記号定数とその値に関する式のリストを，表形式に変換 *)

createVariableMap[cons_] := createVariableMap[And@@cons && pConstraint && initConstraint, variables];

publicMethod[
  createVariableMap,
  cons, vars,
  Module[
    {ret},
    ret = createMap[cons, isVariable, hasVariable, vars];
    debugPrint["ret after CreateMap", ret];
    ret = Map[(Cases[#, Except[{p[___], _, _}] ])&, ret];
    ret = ruleOutException[ret];
    simplePrint[ret];
    ret
  ]
];

publicMethod[
  createVariableMapInterval,
  cons,
  Module[
    {sol, tStore, ret},
    tStore = Select[cons, (!hasVariable[ #[[2]] ])&];
    ret = {convertExprs[tStore]};
    ret = ruleOutException[ret];
    simplePrint[ret];
    ret
  ]
];


publicMethod[
  getConstraintStorePoint,
  {toReturnForm[LogicalExpand[constraint && pConstraint && initConstraint]]}
];


getConstraintStoreInterval[] := 
getConstraintStoreInterval[constraint, initConstraint, timeVariables];

publicMethod[
  getConstraintStoreInterval,
  cons, initCons, vars,
  Module[
    {sol, tStore},
    sol = exDSolve[cons, initCons];
    debugPrint["sol after exDSolve", sol];
    If[sol === overConstraint || sol[[1]] === underConstraint,
      error,
      tStore = createDifferentiatedEquations[vars, sol[[3]] ];
      tStore = Select[tStore, (!hasVariable[ #[[2]] ])&];
      toReturnForm[tStore]
    ]
  ]
];

(* 返す制約として不要なものを除く *)
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
  pCons,
  createMap[pCons, isParameter, hasParameter, {}];
];

createMap[cons_, judge_, hasJudge_, vars_] := 
createMap[cons, judge, hasJudge, vars] = (* for memoization *)
Module[
  {map},
  If[cons === True || cons === False, 
    cons,
    (* Remove unnecessary Constraints*)
    map = cons /. (expr_ /; ( MemberQ[{Equal, LessEqual, Less, Greater, GreaterEqual, Unequal}, Head[expr] ] && (!hasJudge[expr] || hasPrevVariable[expr])) -> True);
    map = Reduce[map, vars, Reals];

    (* Remove unnecessary Constraints*)
    map = map /. (expr_ /; ( MemberQ[{Equal, LessEqual, Less, Greater, GreaterEqual, Unequal}, Head[expr] ] && (!hasJudge[expr] || hasPrevVariable[expr])) -> True);

    simplePrint[map];
    map = LogicalExpand[map];
    map = applyListToOr[map];
    map = Map[(applyList[#])&, map];
    map = Map[(adjustExprs[#, judge])&, map];
    debugPrint["map after adjustExprs in createMap", map];
    map = Map[(convertExprs[#])&, map];
    map
  ]
];

(* 式中に変数名が出現するか否か *)

hasVariable[exprs_] := Length[StringCases[ToString[exprs], variablePrefix ~~ WordCharacter]] > 0;

(* 式が変数もしくはその微分そのものか否か *)

isVariable[exprs_] := MatchQ[exprs, _Symbol] && StringMatchQ[ToString[exprs], variablePrefix ~~ WordCharacter__] || MatchQ[exprs, Derivative[_][_][_] ] || MatchQ[exprs, Derivative[_][_] ] ;

(* 式中に出現する変数を取得 *)

getVariables[exprs_] := ToExpression[StringCases[ToString[exprs], variablePrefix ~~ WordCharacter..]];

(* 式中に出現するprev値を取得 *)
getPrevVariables[exprs_] := Cases[exprs, prev[_, _], Infinity];

(* 式中に出現する記号定数を取得 *)

getParameters[exprs_] := Cases[exprs, p[_, _, _], Infinity];

(* 式中に定数名が出現するか否か *)

hasParameter[exprs_] := Length[Cases[exprs, p[_, _, _], Infinity] ] > 0;
 
hasParameterOrPrev[exprs_] := Length[Cases[{exprs}, p[_, _, _] | prev[_, _], Infinity] ] > 0;

(* 式が定数そのものか否か *)

isParameter[exprs_] := Head[exprs] === p;

isParameterOrPrev[exprs_] := Head[exprs] === p || Head[exprs] === prev;

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
  prevIneqs = True;
  prevRules = {};
  initTmpConstraint = True;
  tmpConstraint = True;
  isTemporary = False;
];

publicMethod[
  resetConstraintForVariable,
  constraint = initConstraint = tmpConstraint = initTmpConstraint = True;
];

publicMethod[
  addConstraint,
  co,
  Module[
    {cons},
    cons = If[Head[co] === List, And@@co, co] //. prevRules;
    If[isTemporary,
      tmpConstraint = tmpConstraint && cons,
      constraint = constraint && cons
    ];
    simplePrint[cons, constraint, tmpConstraint];
  ]
];

addInitConstraint[co_] := Module[
  {cons, vars},
  cons = And@@co //. prevRules;
  If[isTemporary,
    initTmpConstraint = initTmpConstraint && cons,
    initConstraint = initConstraint && cons
  ];
  simplePrint[cons, initConstraint, initTmpConstraint, prevRules];
];

publicMethod[
  addPrevConstraint,
  co,
  Module[
    {land, cnf, eqs, ineqs},
    land = And@@co;
    (* x == 1 && y == 3 || x == 2 && y == 3を、(x == 1 || x == 2) && y == 3のようなCNFに変換して、yの値が一意に定まることを容易に判断できる形式にする *)
    cnf = And2and[BooleanConvert[land, "CNF"] ];
    eqs = Select[cnf, (Head[#]===Equal)&];
    ineqs = Complement[cnf, eqs];
    prevRules = Join[prevRules, List@@Map[(Rule@@#)&, eqs] ];
    prevIneqs = prevIneqs && And@@ineqs;
    simplePrint[prevRules, prevIneqs];
  ]
];


makePrevVar[var_] := Module[
  {name, dcount},
  If[MatchQ[var, Derivative[_][_] ],
    name = var[[1]];
    dcount = var[[0]][[1]],
    name = var;
    dcount = 0
  ];
  (* remove 'u' *)
  name = ToString[name];
  name = StringDrop[name, 1];
  name = StringJoin["p", name];
  name = ToExpression[name];
  prev[name, dcount]
];

publicMethod[
  addVariable,
  name,
  diffCnt,
  Module[
    {var},
    var = If[diffCnt > 0, Derivative[diffCnt][name], name];
    Unprotect[variables, prevVariables, timeVariables, initVariables];
    variables = Union[variables, {var}];
    prevVariables = Union[prevVariables,
      {makePrevVar[var]} ];
    timeVariables = Union[timeVariables, {var[t] } ];
    initVariables = Union[initVariables, {var[t]} ];
    simplePrint[variables, prevVariables, timeVariables, initVariables];
    Protect[variables, prevVariables, timeVariables, initVariables];
  ]
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

publicMethod[
  resetConstraintForParameter,
  pCons,
  pConstraint = True;
  pConstraint = Reduce[pConstraint && And@@pCons, Reals];
  simplePrint[pConstraint];
];


publicMethod[
  addInitEquation,
  lhs, rhs,
  addInitConstraint[{lhs == rhs}]
];

publicMethod[
  addEquation,
  lhs, rhs,
  Module[
    {cons},
    cons = lhs == rhs;
    If[isTemporary,
      tmpConstraint = tmpConstraint && cons,
      constraint = constraint && cons
      ];
    simplePrint[cons, constraint, tmpConstraint];
  ]
];

publicMethod[
  addParameterConstraint,
  pCons,
  pConstraint = Reduce[pConstraint && And@@pCons, Reals];
  simplePrint[pConstraint];
];


publicMethod[
  clearParameters,
  par,
  parameters = {}
];

publicMethod[
  addParameter,
  par,
  parameters = Union[parameters, {par}]
];

(* 変数名からDerivativeやtを取り，微分回数とともに返す *)
removeDash[var_] := Module[
   {ret},
   If[Head[var] === p || Head[var] === prev, Return[var]];
   ret = var /. x_[t] -> x;
   If[MatchQ[Head[ret], Derivative[_]],
     ret /. Derivative[d_][x_] -> {x, d},
     {ret, 0}
   ]
];


(* Piecewiseの第二要素を，その条件とともに第一要素に付加してリストにする．条件がFalseなら削除 
   ついでに othersを各条件に対して付加 *)

makeListFromPiecewise[minT_, others_] := Module[
  {tmpCondition = False, retMinT},
  If[Head[minT] =!= Piecewise, Return[{{minT, others}}] ];
  retMinT = minT[[1]];
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

calculateNextPointPhaseTime[maxTime_, discCauses_] := 
  calculateNextPointPhaseTime[maxTime, discCauses, constraint, initConstraint, pConstraint, timeVariables];


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


(* 条件を満たす最小の時刻と，その条件の組を求める *)
findMinTime[causeAndID_, condition_] := 
Module[
  {
    id,
    cause,
    minT,
    onTime = True,
    ret
  },
  id = causeAndID[[2]];
  cause = causeAndID[[1]];
  sol = Reduce[cause && condition && t > 0, t, Reals];
  checkMessage[];
  If[sol === False, Return[{}] ];
  (* 成り立つtの最小値を求める *)
  
  minT = Quiet[Check[minT = Minimize[{t, sol}, {t}],
                     onTime = False;minT,
                     Minimize::wksol],
         Minimize::wksol];
  (* TODO: 解が分岐していた場合、onTimeは必ずしも一意に定まらないため分岐が必要 *)
  minT = First[minT];
  Assert[Head[minT] =!= Piecewise];
  ret = makeListFromPiecewise[minT, condition];
  (* 時刻が0となる場合を取り除く．*)
  ret = Select[ret, (#[[1]] =!= 0)&];
  (* append id for each time *)
  ret = Map[({dcInfo[#[[1]], ids[id], If[onTime, 1, 0] ], #[[2]] })&, ret];
  ret
];

(* TODO: 場合分けをしていくだけで、併合はしないので最終的に冗長な場合分けが発生する可能性がある。 *)
(* ２つの時刻と条件の組を比較し，最小時刻とその条件の組のリストを返す *)
compareMinTime[timeCond1_, timeCond2_] := ( Block[
    {
      minTime1, minTime2,
      dcInfo1, dcInfo2,
      nonMinimum,
      caseEq,caseLe, caseGr,
      ret,
      andCond
    },
    (* assume that only timeCond1 has nonMinimum *)
    andCond = Reduce[timeCond1[[3]] && timeCond2[[2]], Reals];
    If[andCond === False, Return[{}] ];
    dcInfo1 = timeCond1[[1]];
    dcInfo2 = timeCond2[[1]];
    minTime1 = dcInfo1[[1]];
    minTime2 = dcInfo2[[1]];
    nonMinimum = timeCond1[[2]];
    caseEq = Quiet[Reduce[And[andCond, minTime1 == minTime2], Reals]];
    caseLe = Quiet[Reduce[And[andCond, minTime1 < minTime2], Reals]];
    caseGr = Reduce[andCond && !caseLe && !caseEq];
    ret = {};
    If[ caseEq =!= False,
      (* TODO: onTimeかどうかで真面目に場合分けをする *)
      ret = Append[ret,
        {dcInfo[minTime1, Join[dcInfo1[[2]], dcInfo2[[2]] ], dcInfo1[[3]] ], nonMinimum, caseEq}]
    ];
    If[ caseLe =!= False,
      ret = Append[ret, 
        {dcInfo1, Append[nonMinimum, dcInfo2], caseLe}]
    ];
    If[ caseGr =!= False, 
      ret = Append[ret,
        {dcInfo2, Append[nonMinimum, dcInfo1], caseGr}]
    ];
    Return[ ret ];
  ]
);

 
(* ２つの時刻と条件の組のリストを比較し，各条件組み合わせにおいて，最小となる時刻と条件の組のリストを返す *)
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

(* 最小時刻と条件の組をリストアップする関数 *)
calculateMinTimeList[causeAndIDList_, condition_, maxT_] := (
  Block[
    {findResult, i},
    (* -1 is regarded as the id of time limit *)
    timeCaseList = {{dcInfo[maxT, ids[-1], 0], {}, condition}};
    For[i = 1, i <= Length[causeAndIDList], i++,
      findResult = findMinTime[causeAndIDList[[i]], condition];
      timeCaseList = compareMinTimeList[timeCaseList, findResult]
    ];
    timeCaseList
  ]
);

(* 時刻と条件の組に対し，条件が論理和でつながっている場合それぞれの場合に分解する *)
divideDisjunction[timeCond_] := Map[({timeCond[[1]], timeCond[[2]], #})&, List@@timeCond[[3]]];

 
publicMethod[
  calculateNextPointPhaseTime,
  maxTime, causeAndIDs, cons, initCons, pCons, vars,
  Module[
    {
      timeAppliedCauses,
      resultList,
      necessaryPCons,
      parameterList,
      originalOther,
      tmpMaxTime
    },

    tStore = Map[(Rule@@#)&, createDifferentiatedEquations[vars, applyList[cons] ] ];
    timeAppliedCauses = causeAndIDs /. tStore;
    simplePrint[timeAppliedCauses];
    
    parameterList = getParameters[timeAppliedCauses];
    
    (* 必要なpConsだけを選ぶ．不要なものが入っているとMinimzeの動作がおかしくなる？ *)
    
    necessaryPCons = LogicalExpand[pCons] /. (expr_ /; (( Head[expr] === Equal || Head[expr] === LessEqual || Head[expr] === Less|| Head[expr] === GreaterEqual || Head[expr] === Greater) && (!hasSymbol[expr, parameterList])) -> True);
    
    simplePrint[necessaryPCons];
    
    resultList = calculateMinTimeList[timeAppliedCauses, necessaryPCons, maxTime];
    
    simplePrint[resultList];
    
    (* 整形して結果を返す *)
    resultList = Map[({#[[1]], #[[2]], LogicalExpand[#[[3]] ]})&, resultList];
    resultList = Fold[(Join[#1, If[Head[#2[[3]]]===Or, divideDisjunction[#2], {#2}]])&,{}, resultList];
    resultList = Map[({#[[1]], #[[2]], Cases[applyList[#[[3]] ], Except[True]]})&, resultList];
    
    debugPrint["resultList after Format", resultList];
    
    resultList = Map[
    ({dcInfoToReturn[#[[1]] ], Map[(dcInfoToReturn[#])&, #[[2]] ], convertExprs[adjustExprs[#[[3]], isParameter ] ] })&, resultList];
    simplePrint[resultList];
    resultList
  ]
];

dcInfoToReturn[ti_] := dcInfo[toReturnForm[ti[[1]] ], ti[[2]], ti[[3]] ];

applyDSolveResult[exprs_, integRule_] := (
  exprs  /. integRule     (* 単純にルールを適用 *)
         /. Map[((#[[1]] /. x_[t]-> x) -> #[[2]] )&, integRule]
         /. (Derivative[n_][f_][t] /; !isVariable[f]) :> D[f, {t, n}] (* 微分値についてもルールを適用 *)
);

createDifferentiatedEquations[vars_, integRules_] := (
  Module[
    {tRemovedRules, derivativeExpanded, ruleApplied, ruleVars, ret, tRemovedVars},
    ret = Fold[(Join[#1, createDifferentiatedEquation[#2, integRules] ])&, {}, vars];
    ret
  ]
);

removeDerivative[Derivative[_][var_][arg_]] := var[arg];
removeDerivative[var_] := var;

createDifferentiatedEquation[var_, integRules_] := (
  Module[
    {rule, result, tVar, dCount},
    (* exclude variables which have no rule *)
    tVar = removeDerivative[var];
    rule = Select[integRules, (#[[1]] === tVar)&];
    If[Length[rule] == 0, Return[{}], rule = rule[[1]] ];
    If[MatchQ[var, Derivative[n_][x_][t]], 
      dCount = Head[Head[var] ][[1]];
      result = Equal[var, D[rule[[2]], {t, dCount}] ],
      result = Equal[var, rule[[2]] ]
    ];
    {result}
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
Module[
  {tmpExpr, reducedExpr, rules, tVars, tVar, resultCons, unsolvable = False, resultRule, searchResult, retCode, restCond},
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
      If[rules === overConstraint || Length[rules] == 0, Return[overConstraint] ];
      (* TODO:rulesの要素数が2以上，つまり解が複数存在する微分方程式系への対応 *)
      If[Head[rules] === DSolve,
        resultCons = Union[resultCons, searchResult[[1]] ];
        tmpExpr = Complement[tmpExpr, searchResult[[1]] ];
        unsolvable = True;
        Continue[]
      ];
      resultRule = Union[resultRule, rules[[1]] ];
      simplePrint[resultRule];
      tmpExpr = applyDSolveResult[searchResult[[2]], rules[[1]] ];
      tmpExpr = Map[(If[!hasVariable[#], Reduce[#], #])&, tmpExpr];
      If[MemberQ[tmpExpr, ele /; (ele === False || (!hasVariable[ele] && MemberQ[ele, t, Infinity]))], Return[overConstraint] ];
      tmpExpr = Select[tmpExpr, (#=!=True)&];
      simplePrint[tmpExpr];
      resultCons = applyDSolveResult[resultCons, rules[[1]] ];
      If[MemberQ[resultCons, False], Return[overConstraint] ];
      resultCons = Select[resultCons, (#=!=True)&]
    ]
  ];
  retCode = If[Length[tmpExpr] > 0 || unsolvable, underConstraint, solved];
  restCond = LogicalExpand[And@@tmpExpr && And@@resultCons];
  restCond = Or2or[restCond];
  restCond = Map[(And2and[#])&, restCond];    
  { retCode, restCond, resultRule}
];


(* 与えられた微分方程式系から，独立に解くことのできる部分集合を探す関数
  方程式の数と集合中に出現する変数の数が一致するような部分集合を探す．
  @param exprs 探索対象となる式集合
  @return unExpandable | {(DSolveすべき式の集合）, (残りの式の集合)， （DSolveすべき変数の集合）}
*)
searchExprsAndVars[exprs_] :=
Module[
  {tmpExprs, droppedExprs, searchResult = unExpandable, tVarsMap, tVars},
  simplePrint[exprs];
  For[i=1, i<=Length[exprs], i++,
    tVarsMap[ exprs[[i]] ] = Union[getVariables[exprs[[i]] ] ];
    tVars = tVarsMap[ exprs[[i]] ];
    If[Length[tVars] == 1,
      searchResult = {{ exprs[[i]] }, Drop[exprs, {i}], tVars}
    ]
  ];
    simplePrint[searchResult];
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
solveByDSolve[expr, initExpr, tVars] = (* for memoization *)
Module[
  {tmpExpr, ini, sol, idx, generalInitValue, swapValue, j},
  tmpExpr = expr;
  ini = Select[initExpr, (hasSymbol[#, tVars ])& ];
  simplePrint[tmpExpr, ini, tVars];
  
  sol = Quiet[
    Check[
      DSolve[Union[tmpExpr, ini], Map[(#[t])&, tVars], t],
          overConstraint,
      {DSolve::overdet, DSolve::bvimp}
    ],
   {DSolve::overdet, DSolve::bvimp}
  ];
  simplePrint[sol];
  sol
];

exDSolve::unkn = "unknown error occurred in exDSolve";
