(* ポイントフェーズにおける無矛盾性判定 *)

checkConsistencyPoint[] := (
  checkConsistencyPoint[constraint && initConstraint && prevConstraint, pConstraint, Union[variables, prevVariables], parameters ]
);

checkConsistencyPoint[tmpCons_] := (checkConsistencyPoint[(tmpCons /. prevRules) && constraint && initConstraint && prevConstraint, pConstraint, Union[variables, prevVariables], parameters ]
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
  checkConsistencyInterval[constraint, initConstraint, prevConstraint, pConstraint, parameters]
);

checkConsistencyInterval[tmpCons_] :=  (checkConsistencyInterval[(tmpCons /. prevRules) && constraint, initConstraint, prevConstraint, pConstraint, parameters]
);

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
  lhs = checkAndIgnore[(cond[[1]] - cond[[2]] ) /. t -> 0 /. initRules, Infinity, {Power::infy, Infinity::indet}];
  simplePrint[lhs, pCons];
  (* caused by underConstraint *)
  If[hasVariable[lhs], Return[pCons] ];

  trueCond = False;

  eqSol = Quiet[Reduce[lhs == 0 && pCons, Reals] ];
  If[eqSol =!= False,
    eqSol = ccIntervalForEach[operator[D[cond[[1]], t], D[cond[[2]], t]], initRules, eqSol];
    trueCond = trueCond || eqSol
  ];
  If[MemberQ[{Unequal, Greater, GreaterEqual}, operator],
    gtSol = Quiet[Reduce[lhs > 0 && pCons, Reals] ];
    trueCond = trueCond || gtSol
  ];
  If[MemberQ[{Unequal, Less, LessEqual}, operator],
    ltSol = Quiet[Reduce[lhs < 0 && pCons, Reals] ];
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

createVariableMap[] := createVariableMap[constraint && pConstraint && initConstraint, variables, parameters];

publicMethod[
  createVariableMap,
  cons, vars, pars,
  Module[
    {ret, map},
    map = removeUnnecessaryConstraints[cons, hasVariableOrParameter];
    simplePrint[map];
    If[ Count[map, Less| Unequal| LessEqual| Greater| GreaterEqual, Infinity, Heads -> True] > 0,
      (* If the constraints include inequalities we use Reduce *)
      map = Reduce[map, vars, Reals];
      map = removeUnnecessaryConstraints[map, hasVariable];
      If[map === True, Return[{{}}] ];
      If[map === False, Return[{}] ];
      map = LogicalExpand[map];
      map = applyListToOr[map];
      map = Map[(applyList[#])&, map],
      (* else we use Solve *)
      (* Here, we use parameters because "Solve" cannot solve constraints without paramters such as 
      Solve[uy == (-8*Sqrt[-10 + p[py, 0, 1]])/Sqrt[5] && p[py, 0, 1] == 10, {uy}, Reals] *)
      map = Quiet[Solve[map, Join[vars, pars], Reals], Solve::svars]
    ];

    map = Map[(adjustExprs[#, isVariable])&, map];
    debugPrint["map after adjustExprs in createVariableMap", map];
    ret = Map[(convertExprs[#])&, map];
    ret = Map[(Cases[#, Except[{p[___], _, _}] ])&, ret];
    ret = ruleOutException[ret];
    simplePrint[ret];
    ret
  ]
];



createVariableMapInterval[] := createVariableMapInterval[constraint, initConstraint, timeVariables];

createVariableMapInterval[tvars_] := createVariableMapInterval[constraint, initConstraint, tvars];

publicMethod[
  createVariableMapInterval,
  cons, initCons, vars,
  Module[
    {sol, tStore, ret},
    sol = exDSolve[cons, initCons];

    debugPrint["sol after exDSolve", sol];
    If[sol === overConstraint || sol[[1]] === underConstraint,
      error,
      tStore = createDifferentiatedEquations[vars, sol[[3]] ];
      tStore = Select[tStore, (!hasVariable[ #[[2]] ])&];
      ret = {convertExprs[tStore]};
      ret = ruleOutException[ret];
      ret
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

productWithGlobalParameterConstraint[cons_] := productWithGlobalParameterConstraint[cons, pConstraint];

publicMethod[
  productWithGlobalParameterConstraint,
  map, pCons,
  {toReturnForm[LogicalExpand[map && pCons] ]}
];


publicMethod[
  getParameterConstraint,
  {toReturnForm[LogicalExpand[pConstraint] ]}
];

createParameterMaps[] := createParameterMaps[pConstraint];

publicMethod[
  createParameterMaps,
  pCons,
  Module[
    {map},
    map = removeUnnecessaryConstraints[pCons, hasParameter];
    map = Reduce[map, Reals];
    map = removeUnnecessaryConstraints[map, hasParameter];
    
    If[map === True, Return[{{}}] ];
    If[map === False, Return[{}] ];
      
    map = LogicalExpand[map];
    map = applyListToOr[map];
    map = Map[(applyList[#])&, map];
    map = Map[(adjustExprs[#, isParameter])&, map];
    debugPrint["map after adjustExprs in createParameterMap", map];
    map = Map[(convertExprs[#])&, map];
    map
  ]  
];

removeUnnecessaryConstraints[cons_, hasJudge_] :=
(
cons /. (expr_ /; ( MemberQ[{Equal, LessEqual, Less, Greater, GreaterEqual, Unequal, Inequality}, Head[expr] ] && (!hasJudge[expr] || hasPrevVariable[expr])) -> True)
);


(* 式中に変数名が出現するか否か *)

hasVariable[exprs_] := Length[getVariables[exprs] ] > 0;

(* 式が変数もしくはその微分そのものか否か *)

isVariable[exprs_] := MatchQ[exprs, _Symbol] && StringMatchQ[ToString[exprs], variablePrefix ~~ WordCharacter__] || MatchQ[exprs, Derivative[_][_][_] ] || MatchQ[exprs, Derivative[_][_] ] ;

(* 式中に出現する変数を取得 *)

getVariables[exprs_] := Cases[exprs, ele_ /; StringMatchQ[ToString[ele], variablePrefix ~~ WordCharacter..], Infinity, Heads->True];

(* 式中に出現するprev値を取得 *)
getPrevVariables[exprs_] := Cases[exprs, prev[_, _], Infinity];

(* 式中に出現する記号定数を取得 *)

getParameters[exprs_] := Cases[exprs, p[_, _, _], Infinity];

(* 式中に定数名が出現するか否か *)

hasParameter[exprs_] := Length[Cases[exprs, p[_, _, _], Infinity] ] > 0;

hasParameterOrPrev[exprs_] := Length[Cases[{exprs}, p[_, _, _] | prev[_, _], Infinity] ] > 0;

hasVariableOrParameter[exprs_] := hasParameter[exprs] || hasVariable[exprs];

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
  prevConstraint = True;
  prevRules = {};
];

publicMethod[
  resetConstraintForVariable,
  constraint = initConstraint = prevConstraint = True;
  prevRules = {};
];

publicMethod[
  addConstraint,
  co,
  Module[
    {cons},
    cons = If[Head[co] === List, And@@co, co] /. prevRules;
    constraint = constraint && cons;
  ]
];

addInitConstraint[co_] := Module[
  {cons, vars},
  cons = And@@co /. prevRules;
  initConstraint = initConstraint && cons;
];

addPrevLessEqual[var_, expr_] := addPrevConstraint[var <= expr];
addPrevLess[var_, expr_] := addPrevConstraint[var <= expr];
addPrevGreaterEqual[var_, expr_] := addPrevConstraint[var >= expr];
addPrevGreater[var_, expr_] := addPrevConstraint[var > expr];

addLessEqual[var_, expr_] := addConstraint[var <= expr];
addLess[var_, expr_] := addConstraint[var <= expr];
addGreaterEqual[var_, expr_] := addConstraint[var >= expr];
addGreater[var_, expr_] := addConstraint[var > expr];

publicMethod[
  addPrevEqual,
  var, expr,
  prevRules = Append[prevRules, var -> expr];
];

publicMethod[
  addPrevConstraint,
  co,
  prevConstraint = prevConstraint && co;
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
    Protect[variables, prevVariables, timeVariables, initVariables];
  ]
];


setVariables[vars_] := (
  Unprotect[variables, prevVariables, timeVariables, initVariables];
  variables = vars;
  prevVariables = Map[makePrevVar, vars];
  timeVariables = Map[(#[t])&, vars];
  initVariables = Map[(#[0])&, vars];
  Protect[variables, prevVariables, timeVariables, initVariables];
);


publicMethod[
  resetVariables,
  Unprotect[variables, prevVariables, timeVariables, initVariables];
  variables = {};
  prevVariables = {};
  timeVariables = {};
  initVariables = {};
  Protect[variables, prevVariables, timeVariables, initVariables];
];


publicMethod[
  resetConstraintForParameter,
  pCons,
  pConstraint = Reduce[pCons, Reals];
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
    constraint = constraint && cons;
  ]
];


publicMethod[
  addParameterConstraint,
  pCons,
  pConstraint = Reduce[pConstraint && pCons, Reals];
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


(* Piecewiseの第二要素（第一要素以外を除いた場合）に対し，
othersと第一要素中の全条件の否定の論理積を取って条件とし，第一要素の末尾に付加する*)

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

calculateConsistentTime[cause_, cons_] := calculateConsistentTime[cause, cons, pConstraint];

findMinTime::minimizeFailure = "failed to minimize t";


publicMethod[
  calculateConsistentTime,
  cause, cons, pCons,
  Module[
    {
      resultCons
    },
    tStore = Map[(Rule@@#)&, cons];
    resultCons = ToRadicals[cause /. tStore];
    simplePrint[timeAppliedCause];
    toReturnForm[LogicalExpand[resultCons]]
  ]
];

minimizeTime[tCons_, maxTime_] := minimizeTime[tCons, pConstraint, 0, maxTime];
minimizeTime[tCons_,startingTime_, maxTime_] := minimizeTime[tCons, pConstraint, startingTime, maxTime];

publicMethod[
  minimizeTime,
  tCons, pCons, startingTime, maxTime,
  Module[
    {
      minT,
      resultList,
      onTime = True,
      maxCons,
      ret
    },
    (* Mathematica cannot solve some minimization problem with "t < Infinity", such 
    as "Minimize[{t, t > 0 && 10*t*Cos[(Pi*p[pangle, 0, 1])/180] > 10 && Inequality[10, Less, p[pangle, 0, 1], Less, 30] && t < Infinity}, {t}]" *)
    maxCons = If[maxTime === Infinity, True, t < maxTime];

    Quiet[Check[minT = Minimize[{t, t > startingTime && tCons && pCons && maxCons}, {t}],
         onTime = False,
         Minimize::wksol
       ],
       {Minimize::wksol, Minimize::infeas}
    ];
    (* TODO: 解が分岐していた場合、onTimeは必ずしも一意に定まらないため分岐が必要 *)
    debugPrint["minT after Minimize:", minT];
    If[Head[minT] === Minimize,
      error,
      minT = First[minT];
      If[minT === Infinity,
        {},
        ret = makeListFromPiecewise[minT, pCons];
        (* 時刻が0となる場合はinfとする．*)

        ret = Map[(If[#[[1]] =!= 0, #, ReplacePart[#, 1->Infinity]])&, ret];
        ret = Select[ret, (#[[2]] =!= False)&];

        (* 整形して結果を返す *)
        resultList = Map[({toReturnForm[#[[1]] ], If[onTime, 1, 0], {toReturnForm[LogicalExpand[#[[2]] ] ]} })&, ret];
        resultList
      ]
    ]
  ]
];

createParameterMapList[cons_] := 
If[cons === False, {}, Map[(convertExprs[adjustExprs[#, isParameter]])&, Map[(applyList[#])&, applyListToOr[LogicalExpand[cons] ] ] ] ];

(* TODO: 場合分けをしていくだけで、併合はしないので最終的に冗長な場合分けが発生する可能性がある。 *)
(* @return {condition where the first argument is smaller, condition where the second argument is smaller, condition where the first argument equals to the second argument} *)
publicMethod[
  compareMinTime,
  time1, time2, pCons1, pCons2,
  Module[
    {
      andCond, caseEq, caseLe, caseGr, ret
    },
    andCond = Reduce[pCons1 && pCons2, Reals];
    If[andCond === False,
      {{False}, {False}, {False}},
      caseEq = Quiet[Reduce[And[andCond, time1 == time2], Reals]];
      caseLe = Quiet[Reduce[And[andCond, time1 < time2], Reals]];
      caseGr = Reduce[andCond && !caseLe && !caseEq];
      {{toReturnForm[LogicalExpand[caseLe] ]}, {toReturnForm[LogicalExpand[caseGr] ]}, {toReturnForm[LogicalExpand[caseEq] ]}}
    ]
  ]
];

publicMethod[
  isTriggerGuard,
  atomicGuard, cons, pCons, time,
  Module[
    {
      resultCons,
      lhs,
      reduced,
      negated
    },
    lhs = atomicGuard[[1]] - atomicGuard[[2]];
    simplePrint[lhs];
    tStore = Map[(Rule@@#)&, cons];
    lhs = lhs /. tStore;
    simplePrint[lhs];
    lhs = lhs /. t->time;
    reduced = Reduce[lhs == 0 && pCons];
    negated = Reduce[!reduced && pCons];
    (* Can result be some expression other than True or False ?*)
    If[negated === False, True, False]
  ]
];



(* 時刻と条件の組に対し，条件が論理和でつながっている場合それぞれの場合に分解する *)
divideDisjunction[timeCond_] := Map[({timeCond[[1]], timeCond[[2]], #})&, List@@timeCond[[3]]];

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

exDSolve::multi = "Solution of `1` is not unique.";

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
  {listExpr, reducedExpr, rules, tVars, resultCons, unsolvable = False, resultRule, searchResult, retCode, restCond},
  inputPrint["exDSolve", expr, initExpr];
  listExpr = applyList[expr];
  sol = {};
  resultCons = Select[listExpr, (Head[#] =!= Equal)&];
  listExpr = Complement[listExpr, resultCons];
  (* add constraint "t > 0" to exclude past case *)
  resultCons = And@@resultCons && t > 0;
  reducedExpr = Quiet[Check[Reduce[listExpr, Reals], listExpr], {Reduce::nsmet, Reduce::useq}];
  (* Reduceの結果が使えそうな場合のみ使う *)
  If[Head[reducedExpr] === And && MemberQ[reducedExpr, Element, Infinity, Heads->True], listExpr = applyList[reducedExpr] ];
  tmpInitExpr = applyList[initExpr];
  resultRule = {};
  While[True,
    searchResult = searchExprsAndVars[listExpr];
    simplePrint[searchResult];
    If[searchResult === unExpandable,
      Break[],
      rules = solveByDSolve[searchResult[[1]], tmpInitExpr, searchResult[[3]]];
      If[rules === overConstraint || Length[rules] == 0, Return[overConstraint] ];
      (* TODO:rulesの要素数が2以上，つまり解が複数存在する微分方程式系への対応 *)
      If[Head[rules] === DSolve,
        resultCons = resultCons && And@@searchResult[[1]];
        listExpr = Complement[listExpr, searchResult[[1]] ];
        unsolvable = True;
        Continue[]
      ];
      If[Length[rules] > 1, Message[exDSolve::multi, searchResult[[1]]];checkMessage ];
      resultRule = Union[resultRule, rules[[1]] ];
       listExpr = applyDSolveResult[searchResult[[2]], rules[[1]] ];
      If[MemberQ[listExpr, ele /; (ele === False || (!hasVariable[ele] && MemberQ[ele, t, Infinity]))], Return[overConstraint] ];
      listExpr = Select[listExpr, (#=!=True)&];
      If[Reduce[listExpr, tVars, Reals] == False, Return[overConstraint]];
      resultCons = applyDSolveResult[resultCons, resultRule];
      If[resultCons === False, Return[overConstraint] ];
    ]
  ];
  retCode = If[Length[listExpr] > 0 || unsolvable, underConstraint, solved];
  restCond = LogicalExpand[And@@listExpr && applyDSolveResult[resultCons, resultRule]];
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
  {ini, sol, idx, generalInitValue, swapValue, j},
  ini = Select[initExpr, (hasSymbol[#, tVars ])& ];
  sol = Quiet[
    Check[
      DSolve[Union[expr, ini], Map[(#[t])&, tVars], t],
          overConstraint,
      {DSolve::overdet, DSolve::bvimp}
    ],
  {DSolve::overdet, DSolve::bvimp, Solve::svars}
  ];
  sol
];

alwaysLess[lhs_, rhs_, pCons_] := alwaysLess[lhs, rhs, pCons, parameters];

publicMethod[
  alwaysLess,
  lhs, rhs, pCons, pars,
  Reduce[ForAll[pars, pCons, lhs < rhs]] === True
];

exDSolve::unkn = "unknown error occurred in exDSolve";
