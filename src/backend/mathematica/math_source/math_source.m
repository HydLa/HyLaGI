(* ポイントフェーズにおける無矛盾性判定 *)

checkConsistencyPoint[] := (
  checkConsistencyPoint[constraint && initConstraint && prevConstraint, pConstraint, assumptions, Union[variables, prevVariables], parameters, currentTime ]
);

checkConsistencyPoint[tmpCons_] := (checkConsistencyPoint[(Assuming[assumptions, Simplify[tmpCons] ] /. prevRules) && constraint && (initConstraint /. prevRules) && prevConstraint, pConstraint, assumptions, Union[variables, prevVariables], parameters, currentTime]
);


publicMethod[
  checkConsistencyPoint,
  cons, pcons, assum, vars, pars, current,
  Module[
    {cpTrue, cpFalse, simplifiedCos},
    simplifiedCons = Assuming[assum, Simplify[cons]];
    Quiet[
      cpTrue = Reduce[Exists[vars, (simplifiedCons /. t -> current) && pcons], pars, Reals], {Reduce::useq}
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
  checkConsistencyInterval[constraint, initConstraint, assumptions, timeVariables, prevRules, prevConstraint, pConstraint, parameters]
);

checkConsistencyInterval[tmpCons_] :=  (checkConsistencyInterval[tmpCons && constraint, initConstraint, assumptions, timeVariables, prevRules, prevConstraint, pConstraint, parameters]
);

ccIntervalForEach[cond_, pCons_] :=
Module[
  {
    operator,
    lhs,
    eqSol,
    gtSol,
    ltSol,
    trueCond
  },
  inputPrint["ccIntervalForEach", cond, pCons];
  If[cond === True || cond === False, Return[cond]];
  operator = Head[cond];
  lhs = checkAndIgnore[(cond[[1]] - cond[[2]] ) /. t -> 0, Infinity, {Power::infy, Infinity::indet}];
  simplePrint[lhs, pCons];
  (* caused by underConstrained *)
  If[hasVariable[lhs], Return[pCons] ];

  trueCond = False;

  eqSol = Quiet[Reduce[lhs == 0 && pCons, Reals] ];
  If[eqSol =!= False,
    eqSol = ccIntervalForEach[operator[D[cond[[1]], t], D[cond[[2]], t]], eqSol];
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
  cons, initCons, assum, vars, prevRs, prevCons, pCons, pars,
  Module[
    {sol, timeVars, prevVars, tCons, tRules, i, j, conj, cpTrue, eachCpTrue, cpFalse},
      If[cons === True,
        {{LogicalExpand[pCons]}, {False}},
        Assuming[assum /. prevRs, 
          sol = exDSolve[cons, prevRs];
          simplePrint[sol];
          prevVars = Map[makePrevVar, vars];
          debugPrint["sol after exDSolve", sol];
          sol = Simplify[sol //. prevRs];
          If[sol === overConstrained,
            {{False}, {toReturnForm[LogicalExpand[pCons]]}},
            tRules = Map[((Rule[#[[1]] /. t-> t_, #[[2]]]))&, createDifferentiatedEquations[vars, sol[[3]] ] ];
            simplePrint[tRules];
            tCons = Map[(Join[#, and@@applyList[initCons] ])&, sol[[2]] ] /. tRules /. prevRs;
            
            simplePrint[tCons];
            
            cpTrue = False;
            For[i = 1, i <= Length[tCons], i++,
              conj = tCons[[i]];
              eachCpTrue = prevCons && pCons;
              For[j = 1, j <= Length[conj], j++,
                eachCpTrue = eachCpTrue && ccIntervalForEach[conj[[j]], eachCpTrue]
              ];
              cpTrue = cpTrue || eachCpTrue
            ];
            cpFalse = Reduce[!cpTrue && pCons && prevCons, Join[pars, prevVars], Reals];
            toReturnForm[{{LogicalExpand[cpTrue]}, {LogicalExpand[cpFalse]}}]
          ]
        ]
      ]
   ]
];

(* 変数もしくは記号定数とその値に関する式のリストを，表形式に変換 *)

createVariableMap[] := createVariableMap[constraint && pConstraint && (initConstraint /. prevRules), variables, assumptions, parameters, currentTime];

publicMethod[
  createVariableMap,
  cons, vars, assum, pars, current,
  Module[
    {ret, map, currentCons},
    currentCons = Assuming[assum, Simplify[cons] ] /. t -> current;
    map = removeUnnecessaryConstraints[currentCons, hasVariable];
    map = Reduce[map, vars, Reals];
    If[Head[map] === Or,
       (* try to solve with parameters *)
       map = removeUnnecessaryConstraints[currentCons, hasVariableOrParameter];
       map = Reduce[map, vars, Reals];
    ];

    map = removeUnnecessaryConstraints[map, hasVariable];
    If[map === True, 
      {{}},
      If[map === False,
        {},
        map = LogicalExpand[map];
        map = applyListToOr[map];
        map = Map[(applyList[#])&, map];
        map = Map[(adjustExprs[#, isVariable])&, map];
        debugPrint["map after adjustExprs in createVariableMap", map];
        ret = Map[(convertExprs[#])&, map];
        ret = Map[(Cases[#, Except[{p[___], _, _}] ])&, ret];
        ret = ruleOutException[ret];
        simplePrint[ret];
        ret
      ]
    ]
  ]
];



createVariableMapInterval[] := createVariableMapInterval[constraint, prevRules, timeVariables];

createVariableMapInterval[tvars_] := createVariableMapInterval[constraint, prevRules, tvars];

createVariableMapInterval::underconstrained = "The system should not be underconstrained here.";

publicMethod[
  createVariableMapInterval,
  cons, prevRs, vars,
  Module[
    {sol, tStore, ret},
    sol = exDSolve[cons, prevRs];
    sol = sol /. prevRs;
    debugPrint["sol after exDSolve", sol];
    If[sol === overConstrained || sol[[1]] === underConstrained,
      Message[createVariableMapInterval::underconstrained],
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

publicMethod[
  exactlyEqual,
  lhs, rhs,
  Simplify[lhs == rhs] === True
];

createParameterMaps[] := createParameterMaps[pConstraint, parameters];
createParameterMaps[pcons_] := createParameterMaps[pcons, parameters];

publicMethod[
  createParameterMaps,
  pCons, pars,
  Module[
    {map},
    map = removeUnnecessaryConstraints[pCons, hasParameter];
    map = Reduce[map, pars, Reals];
    map = removeUnnecessaryConstraints[map, hasParameter];
    
    If[map === True,
      {{}},
      If[map === False,
        {},
        map = LogicalExpand[map];
        map = applyListToOr[map];
        map = Map[(applyList[#])&, map];
        map = Map[(adjustExprs[#, isParameter])&, map];
        debugPrint["map after adjustExprs in createParameterMap", map];
        map = Map[(convertExprs[#])&, map];
        map
      ]
    ]
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
getDerivatives[exprs_] := Union[Cases[exprs, Derivative[_][_], Infinity], Cases[exprs, Derivative[_][_][_], Infinity]];
getVariablesWithDerivatives[exprs_] := Union[getVariables[expr], getDerivatives[exprs] ];


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
  pConstraint = True;
  initConstraint = True;
  assumptions = True;
];

publicMethod[
  clearPrevConstraint,
  prevConstraint = True;
  prevRules = {};
];

publicMethod[
  resetConstraintForVariable,
  constraint = True;
  initConstraint = True;
];

publicMethod[
  addAssumption,
  as,
  debugPrint["as in addAssumption:", as];
  assumptions = assumptions && as;
];

publicMethod[
  addConstraint,
  co,
  Module[
    {cons},
    cons = If[Head[co] === List, And@@co, co];
    cons = Assuming[assumptions, Simplify[cons]] //. prevRules;
    constraint = constraint && cons;
  ]
];

addInitConstraint[co_] := Module[
  {cons},
  cons = Assuming[assumptions, Simplify[And@@co]];
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

publicMethod[
  setCurrentTime,
  expr,
  currentTime = expr;
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
  lhs
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

calculateConsistentTime[cause_, lower_] := calculateConsistentTime[cause, lower, pConstraint, currentTime];

findMinTime::minimizeFailure = "failed to minimize `1`";


publicMethod[
  calculateConsistentTime,
  cause, lower, pCons, current, 
  Module[
    {
      resultCons
    },
    resultCons = ToRadicals[cause /. x_[t] /; isVariable[x] -> x] && lower < t;
    simplePrint[resultCons];
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
      necessaryPCons,
      consList,
      restPCons,
      parsInCons,
      maxCons,
      ret
    },
    (* Mathematica cannot solve some minimization problem with "t < Infinity", such 
    as "Minimize[{t, t > 0 && 10*t*Cos[(Pi*p[pangle, 0, 1])/180] > 10 && Inequality[10, Less, p[pangle, 0, 1], Less, 30] && t < Infinity}, {t}]" *)

    maxCons = If[maxTime === Infinity, True, t < maxTime];

    parsInCons = Union[getParameters[tCons], getParameters[maxTime] ];
    consList = applyList[pCons];
    necessaryPCons = Select[consList, (Length[Intersection[getParameters[#], parsInCons] ] > 0)&];
    restPCons = And@@Complement[consList, necessaryPCons];
    necessaryPCons = And@@necessaryPCons;
    simplePrint[necessaryPCons];
    simplePrint[restPCons];

    Quiet[Check[minT = Minimize[{t, t > startingTime && tCons && necessaryPCons && maxCons}, {t}],
         onTime = False,
         Minimize::wksol
       ],
       {Minimize::wksol, Minimize::infeas}
    ];
    (* TODO: 解が分岐していた場合、onTimeは必ずしも一意に定まらないため分岐が必要 *)
    debugPrint["minT after Minimize:", minT];
    If[Head[minT] === Minimize,
      Message[findMinTime::minimizeFailure, minT],
      minT = First[minT];
      If[minT === Infinity,
        {},
        ret = makeListFromPiecewise[minT, necessaryPCons];
        ret = Map[({#[[1]], #[[2]] && restPCons})&, ret];
        (* 時刻が0となる場合はinfとする．*)
        ret = Map[(If[#[[1]] =!= 0, #, ReplacePart[#, 1->Infinity]])&, ret];
        ret = Select[ret, (#[[2]] =!= False)&];

        (* 整形して結果を返す *)
        resultList = Map[({toReturnForm[#[[1]] ], If[onTime, 1, 0], {toReturnForm[LogicalExpand[#[[2]] ] ]}, -1})&, ret];
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
    {ret},
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
  @return overConstrained |
    {underConstrained, 変数値が満たすべき制約 （ルールに含まれているものは除く），各変数の値のルール} |
    {変数値が満たすべき制約 （ルールに含まれているものは除く），各変数の値のルール}
*)

exDSolve[expr_, prevRs_] :=
Module[
  {listExpr, reducedExpr, rules, tVars, resultCons, unsolvable = False, resultRule, searchResult, retCode, restCond},
  inputPrint["exDSolve", expr];
  listExpr = applyList[expr];
  sol = {};
  resultCons = Select[listExpr, (Head[#] =!= Equal)&];
  listExpr = Complement[listExpr, resultCons];
  (* add constraint "t > 0" to exclude past case *)
  tVars = Map[(#[t])&, getVariablesWithDerivatives[listExpr] ];
  resultCons = And@@resultCons && t > 0;
  resultRule = {};
  While[True,
    searchResult = searchExprsAndVars[listExpr];
    simplePrint[searchResult];
    If[searchResult === unExpandable,
      Break[],
      rules = solveByDSolve[searchResult[[1]], searchResult[[3]]];
      simplePrint[rules];
      If[rules === overConstrained || Length[rules] == 0, Return[overConstrained] ];
      (* TODO:rulesの要素数が2以上，つまり解が複数存在する微分方程式系への対応 *)
      If[Head[rules] === DSolve,
        resultCons = resultCons && And@@searchResult[[1]];
        listExpr = Complement[listExpr, searchResult[[1]] ];
        unsolvable = True;
        Continue[]
      ];
      resultRule = Union[resultRule, rules[[1]] ];
      listExpr = applyDSolveResult[searchResult[[2]], rules[[1]] ];
      listExpr = listExpr //. prevRs;
      If[MemberQ[listExpr, ele_ /; (ele === False || (!hasVariable[ele] && MemberQ[ele, t, Infinity]))], Return[overConstrained] ];
      listExpr = Select[listExpr, (#=!=True)&];
      tVars = Map[(#[t])&, getVariablesWithDerivatives[listExpr] ];
      simplePrint[listExpr, tVars];

      resultCons = applyDSolveResult[resultCons, resultRule] /. prevRs;
      If[resultCons === False, Return[overConstrained] ];
    ]
  ];
  retCode = If[Length[listExpr] > 0 || unsolvable, underConstrained, solved];
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

createPrevRules[var_] := Module[
  {tRemovedVar, dCount, i, varName, der},
  tRemovedVar = var /. x_[t] -> x;
  If[MatchQ[Head[tRemovedVar], Derivative[_]],
    varName = tRemovedVar[[1]];
    dCount = Head[tRemovedVar][[1]];
    ret = {varName[0] == makePrevVar[varName]};
    der = Derivative[i][varName];
    For[i = 1, i < dCount, i++,
      ret = Append[ret, der[0] == makePrevVar[der]];
    ];
    ret,
    {}
  ]
];


(* 渡された式をDSolveで解いて，結果のRuleを返す．
  @param expr: DSolveに渡す微分方程式系．形式はリスト．
  @param vars: exprに出現する変数のリスト
*)
solveByDSolve[expr_, vars_] :=
solveByDSolve[expr, vars] = (* for memoization *)
Module[
  {ini = {}, sol, derivatives, dCountList, i},
  tVars = Map[(#[t])&, vars];
  derivatives = getVariablesWithDerivatives[expr];
  For[i = 1, i <= Length[derivatives], i++,
    ini = Append[ini, createPrevRules[derivatives[[i]] ] ]
  ];
  sol = Quiet[
    Check[
      DSolve[Union[expr, ini], tVars, t],
          overConstrained,
      {DSolve::overdet, DSolve::bvimp}
    ],
  {DSolve::overdet, DSolve::bvimp, Solve::svars}
  ];
  (* remove solutions with imaginary numbers *)
  For[i = 1, i <= Length[sol], i++,
    If[Count[Map[(Simplify[Element[#[[2]], Reals]])&, sol[[i]] ], False] > 0,
      sol = Drop[sol, {i}];
      --i;
    ]
  ];
  If[Length[sol] > 0, sol, overConstrained]
];

alwaysLess[lhs_, rhs_, pCons_] := alwaysLess[lhs, rhs, pCons, parameters];

publicMethod[
  alwaysLess,
  lhs, rhs, pCons, pars,
  Reduce[ForAll[pars, pCons, lhs < rhs]] === True
];

exDSolve::unkn = "unknown error occurred in exDSolve";

borderIsIncluded[border_, lower_, upper_] := borderIsIncluded[border, lower, upper, parameters, pConstraint];

publicMethod[
  borderIsIncluded,
  border,
  lower,
  upper,
  pars,
  pCons,
  Module[
    {necessaryPCons, parsInCons},
    parsInCons = Union[getParameters[lower], getParameters[upper], getParameters[border]];
    necessaryPCons = And@@Select[applyList[pCons], (Length[Intersection[getParameters[#], parsInCons] ] > 0)&];
    Reduce[ForAll[parsInCons, necessaryPCons, lower <= border <= upper]] === True
  ]
];  
  

onBorder[borderExpr_, time_] := onBorder[borderExpr, time, parameters, pConstraint];

publicMethod[
  onBorder,
  borderExpr,
  time,
  pars,
  pCons,
  Module[
    {eq},
    If[borderExpr === True || borderExpr === False,
      False, (* exclude case which the trajectory is continuously on the border *)
      eq = Equal[borderExpr[[1]], borderExpr[[2]]];
      eq = borderExpr /. t -> time;
      simplePrint[eq];
      Reduce[ForAll[pars, pCons, eq]] === True
    ]
  ]
];

publicMethod[
  makeEquation,
  expr,
  Equal[expr[[1]], expr[[2]]]
];


solveTimeEquation[guard_, lowerBound_] :=
  solveTimeEquation[guard, lowerBound, pConstraint, parameters];

publicMethod[
  solveTimeEquation,
  guard, lowerBound, pCons, pars,
  Module[
  {rules, borderCond, sol, timeList},
    borderCond = Equal@@guard;
    sol = Solve[borderCond && t > 0, {t}];
    timeList = Map[(#[[1,2]])&, sol];
    timeList = Map[({toReturnForm[#], 1, {toReturnForm[pCons]}, -1})&, timeList];
    timeList
  ]
];

 (* TODO: consider case branching *)
sortWithParameters[timeList_, pCons_, pars_] := Sort[timeList, Reduce[#1 < #2 && pCons, Join[pars, t], Reals] =!= False];

trueAtInitialTime[guard_, initialTime_] := trueAtInitialTime[guard, initialTime, parameters, pConstraint];

publicMethod[
  trueAtInitialTime,
  guard, initialTime, pars, pCons, 
  Quiet[Reduce[(guard /. t -> initialTime) && pCons, pars, Reals] =!= False]
]

getMinimum[timeList_] := getMinimum[timeList, pConstraint, parameters]; 

publicMethod[
  getMinimum,
  timeList,
  pCons, 
  pars,
  Module[
    {minimum, i, minimumIndex},
    minimum = timeList[[1, 1]];
    minimumIndex = 0;
    (* TODO: deal with case branching *)
    For[i = 2, i <= Length[timeList], i++,
      If[Reduce[minimum <= timeList[[i, 1]] && pCons, pars, Reals] === False,
        minimum = timeList[[i, 1]];
        minimumIndex = i - 1;
      ];
    ];
    {{toReturnForm[minimum], 1, {toReturnForm[LogicalExpand[pCons] ]}, minimumIndex}}
  ]
];

publicMethod[
  isGuardSatisfied,
  guard,
  guard
];


productWithNegatedConstraint[cons_, toBeNegated_] := productWithNegatedConstraint[cons, toBeNegated, parameters];

publicMethod[
  productWithNegatedConstraint,
  cons,
  toBeNegated,
  pars,
(*  toReturnForm[{LogicalExpand[Reduce[cons && Not[toBeNegated], pars, Reals] ]} ] *)
  (* TODO: implement *)
  toReturnForm[{False}]
]