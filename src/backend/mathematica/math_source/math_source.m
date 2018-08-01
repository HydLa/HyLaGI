trySolve[cons_, vars_] :=
  Module[
    {i, eachCons, sol = True, solved = False, consList, consToSolve, trivialCons = True, inequalities = True},
    If[Head[cons] =!= And,
      consToSolve = {cons},
      consList = applyList[cons];
      consToSolve = Select[consList, (Head[#] === Equal)&];
      inequalities = Complement[consList, consToSolve];
      inequalities = Reduce[inequalities, vars];
      For[i = 1, i <= Length[consToSolve], i++,
        eachCons = consToSolve[[i]];
        If[Head[eachCons] === Equal,
          (* swap lhs and rhs if rhs is variable *)
          If[isVariable[eachCons[[2]] ], eachCons = (eachCons[[2]] == eachCons[[1]])];
          If[isVariable[eachCons[[1]] ] && !hasVariable[eachCons[[2]] ],
            trivialCons = trivialCons && eachCons;
            consToSolve = Drop[consToSolve, {i}];
            consToSolve = consToSolve /. eachCons[[1]] -> eachCons[[2]];
            i = 0;
          ]
        ]
      ]
    ];
    simplePrint[consToSolve];
    If[freeFromInequalities[consToSolve],
      sol = Quiet[solveOverRorC[consToSolve, vars], {Solve::svars, PolynomialGCD::lrgexp, Solve::fulldim}];
      If[FreeQ[sol, ConditionalExpression] && Length[sol] === 1 && inequalities === True, sol = And@@Map[(Equal@@#)&, sol[[1]] ]; solved = True]
    ];
    If[solved =!= True, sol = And@@consToSolve];
    {trivialCons && sol && inequalities, solved}
  ];


checkConsistencyPoint[] := (
  Module[
    {tm, ret},
    {tm, ret} = Timing[checkConsistencyPoint[constraint && prevConstraint, initConstraint, pConstraint, assumptions, prevRules, variables, prevVariables, parameters, currentTime ]];
    pointTime = pointTime + tm;
    simplePrint[pointTime];
    ret
  ]
);

checkConsistencyPoint[tmpCons_] := (
  Module[
    {tm, ret},
    {tm, ret} = Timing[checkConsistencyPoint[tmpCons && constraint  && prevConstraint, initConstraint, pConstraint, assumptions, prevRules, variables, prevVariables, parameters, currentTime] ];
    pointTime = pointTime + tm;
    simplePrint[pointTime];
    ret
  ]
);

publicMethod[
  checkConsistencyPoint,
  cons, init, pcons, assum, prevRs, vars, prevs, pars, current,
  Module[
    {cpTrue, cpFalse, initRules, initSubsituted, sol, solved = False, tmpCache, resolvedPrevCons, tm},
    initRules = Map[(Rule@@#)&, applyList[init] ];
    debugPrint["assum: ", assum];
    initSubsituted = And@@Map[(Assuming[assum, timeConstrainedSimplify[# /. t->current /. initRules]])&, applyList[cons]];
    {sol, solved} = trySolve[initSubsituted, vars];
    sol = sol /. Element[_,_] -> True;
    resultConstraint = And@@Map[(Assuming[assum, timeConstrainedSimplify[# //. prevRs]])&, applyList[sol]];
    simplePrint[sol];
    If[checkFirstFlag === True,
      tmpCache = Last[tmpCacheStack];
      tmpCacheStack = Most[tmpCacheStack];
      {tm, {resolvedPrevCons, givenCons}} = Timing[resolvePrev[applyList[sol] ] ];
      exTime = tm + exTime;
      simplePrint[exTime];
    ];
    simplePrint[solved, resultConstraint];
    If[solved,
      If[resultConstraint =!= False && (Length[sol] > 0 || sol === True), 
        cpTrue = pcons;
        cpFalse = False,
        cpTrue = False;
        cpFalse = pcons;
        resolvedPrevCons = !resolvedPrevCons
      ],
      Quiet[
        cpTrue = Reduce[Exists[Evaluate[Join[vars, prevs] ], (resultConstraint /. t -> current) && pcons], pars, Reals], {Reduce::useq}
      ];
      simplePrint[cpTrue];
      (* remove (Not)Element[] because it seems to be always true *)
      cpTrue = cpTrue /. {NotElement[_, _] -> True, Element[_, _] -> True};
      checkMessage;
      Quiet[
        cpFalse = Reduce[pcons && !cpTrue, pars, Reals], {Reduce::useq}
      ];
      (* prev condition not to satisfy *)
      If[cpTrue === False && checkFirstFlag,
        resolvedPrevCons = !resolvedPrevCons
      ];
      (* prev condition to satisfy and not *)
      If[cpTrue =!= False && cpFalse =!= False && checkFirstFlag,
        {tm, resolvedPrevCons} = Timing[Resolve[Exists[getVariablesWithDerivatives[resolvedPrevCons], resolvedPrevCons]]];
        exTime = tm + exTime;
        tmpCacheStack = Append[tmpCacheStack, LogicalExpand[tmpCache && !resolvedPrevCons] ];
      ];
      If[cpTrue =!= False && !checkFirstFlag,
        {tm, {resolvedPrevCons, givenCons}} = Timing[resolvePrev[applyList[sol] ] ];
        exTime = tm + exTime;
        {tm, resolvedPrevCons} = Timing[Resolve[Exists[getVariablesWithDerivatives[resolvedPrevCons], resolvedPrevCons] ] ];
        exTime = tm + exTime;
        tmpCacheStack = Append[tmpCacheStack, LogicalExpand[Last[tmpCacheStack] && !resolvedPrevCons] ];
        tmpCacheStack = Append[tmpCacheStack, LogicalExpand[Last[tmpCacheStack] && resolvedPrevCons] ];
      ];
    ];
    checkMessage;
    simplePrint[cpFalse];
    simplePrint[checkFirstFlag];
    simplePrint[resolvedPrevCons];
    simplePrint[tmpCache];
    If[checkFirstFlag === True,
      tmpCacheStack = Append[tmpCacheStack, LogicalExpand[tmpCache && resolvedPrevCons]];
      checkFirstFlag = False;
    ];
    simplePrint[tmpCacheStack];
    toReturnForm[{{LogicalExpand[cpTrue]}, {LogicalExpand[cpFalse]}}]
  ]
];

(* インターバルフェーズにおける無矛盾性判定 *)

checkConsistencyInterval[] :=  (
  Module[
    {tm, ret},
    {tm, ret} = Timing[checkConsistencyInterval[constraint, initConstraint, assumptions, timeVariables, prevRules, prevConstraint, pConstraint, parameters]];
    intervalTime = intervalTime + tm;
    simplePrint[intervalTime];
    ret
  ]
);

checkConsistencyInterval[tmpCons_] :=  (
  Module[
    {tm, ret},
    {tm, ret} = Timing[checkConsistencyInterval[tmpCons && constraint, initConstraint, assumptions, timeVariables, prevRules, prevConstraint, pConstraint, parameters] ];
    intervalTime = intervalTime + tm;
    simplePrint[intervalTime];
    ret
  ]
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
  simplePrint[lhs];
  (* On the case when the variables are underconstrained *)
  If[hasVariable[lhs], Return[True] ];

  trueCond = False;
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
    {sol, timeVars, prevVars, tCons, tRules, i, j, conj, cpTrue, eachCpTrue, cpFalse, initRules, substitutedInit, prevExcludedRules, eachTmpCache, tmpCache, topCache},
      If[cons === True,
        toReturnForm[{{LogicalExpand[pCons]}, {False}}],
        Assuming[assum,
        sol = exDSolve[Simplify[cons] ];
        simplePrint[sol];
        prevVars = Map[makePrevVar, vars];
        debugPrint["sol after exDSolve", sol];
        prevExcludedRules = sol;
        If[sol === overConstrained,
          toReturnForm[{{False}, {LogicalExpand[pCons]}}],
          If[(sol[[2]] //. prevRs) === False,
            toReturnForm[{{False}, {LogicalExpand[pCons]}}],
            sol = sol //. prevRs;
            tRules = Map[((Rule[#[[1]], #[[2]]]))&, createDifferentiatedEquations[vars, sol[[3]] ] ];
            simplePrint[tRules];
            substitutedInit = initCons /. prevRs;
            debugPrint["hoge"];
            debugPrint[cons];
            debugPrint[initCons];
            debugPrint[prevRs];
            debugPrint[substitutedInit];
            debugPrint[tRules];
            debugPrint[(initCons /. (tRules /. t -> 0))];
            debugPrint[(substitutedInit /. (tRules /. t -> 0))];
            If[(initCons /. (tRules /. t -> 0)) === False, 
              toReturnForm[{{False}, {LogicalExpand[pCons]}}],
              tCons = sol[[2]] /. tRules;
              initRules = makeRulesForVariable[substitutedInit];
              prevExcludedRules = makeRulesForVariable[initCons];
              simplePrint[tCons];
              simplePrint[initRules];
              simplePrint[prevExcludedRules];
              cpTrue = False;
              tmpCache = False;
              For[i = 1, i <= Length[tCons], i++,
                conj = tCons[[i]];
                eachCpTrue = prevCons && pCons;
                eachTmpCache = True;
                For[j = 1, j <= Length[conj], j++,
                  eachCpTrue = eachCpTrue && ccIntervalForEach[conj[[j]], initRules, eachCpTrue];
                  eachTmpCache = eachTmpCache && ccIntervalForEach[conj[[j]], prevExcludedRules, True];
                  simplePrint[eachCpTrue];
                  simplePrint[eachTmpCache];
                ];
                tmpCache = tmpCache || eachTmpCache;
                cpTrue = cpTrue || eachCpTrue
              ];
              cpTrue = Reduce[cpTrue, Join[pars, prevVars], Reals];
              cpFalse = Reduce[!cpTrue && pCons && prevCons, Join[pars, prevVars], Reals];
              If[checkFirstFlag,
                simplePrint[tmpCacheStack];
                topCache = Last[tmpCacheStack];
                tmpCacheStack = Most[tmpCacheStack];
                If[cpTrue =!= False && cpFalse =!= False,
                  tmpCacheStack = Append[tmpCacheStack, topCache && LogicalExpand[!tmpCache]];
                ];
                tmpCacheStack = Append[tmpCacheStack, topCache && tmpCache];
              ];
              toReturnForm[{{LogicalExpand[cpTrue]}, {LogicalExpand[cpFalse]}}]
            ]
          ]
        ]
      ]
    ]
  ]
];

(* 変数もしくは記号定数とその値に関する式のリストを，表形式に変換 *)

createVariableMap[] := createVariableMap[resultConstraint && pConstraint, variables, assumptions, parameters, currentTime];

containsAny[expr_, list_List] := 
Module[
  {i},
  For[i = 0, i <= Length[list], i++,
    If[MemberQ[expr, list[[i]], {0, Infinity}], Return[True]]
  ];
  Return[False]
];


freeFromInequalities[expr_] := FreeQ[expr, Less] && FreeQ[expr, LessEqual] && FreeQ[expr, Greater] && FreeQ[expr, GreaterEqual] && FreeQ[expr, Unequal];

(* 
   consList: list of atomic constraints
   vars: set of variables
   return: {resultList, succeeded}
           resultList: list of constraints that contain at least one constraint for each variable
                       (the meaning is equal to cons)
           succeeded: True if it succeeded in transformation, otherwise False
 *)
tryToTransformConstraints[consList_, vars_] :=
Module[
  {i, processedVars = {}, newVars, resultCons = True, listToProcess = consList, tmpCons},
  For[i = 1, i <= Length[listToProcess], ++i, 
    newVars = Complement[getVariablesWithDerivatives[listToProcess[[i]] ], processedVars];
    If[Length[newVars] == 1,
      processedVars = Append[processedVars, newVars[[1]] ];
      simplePrint[listToProcess[[i]], newVars[[1]] ];
      tmpCons = Quiet[Check[If[listToProcess[[i]] =!= newVars[[1]],
        newVars[[1]] == Solve[listToProcess[[i]], newVars[[1]]][[1, 1, 2]],
        listToProcess[[i]]
      ], listToProcess[[i]] ] ];
      If[Head[tmpCons] === ConditionalExpression, tmpCons = listToProcess[[i]] ]; (* revert tmpCons *)
      simplePrint[tmpCons];
      If[!MemberQ[{Unequal, Less, LessEqual, Equal, Greater, GreaterEqual}, tmpCons], succeeded = false];
      resultCons = resultCons && tmpCons;
      listToProcess = Drop[listToProcess, {i}];
      i = 0;
    ];
    If[Length[newVars] == 0,
      resultCons = resultCons && listToProcess[[i]];
      listToProcess = Drop[listToProcess, {i}];
      --i;
    ];
  ];
  succeeded = If[Length[listToProcess] =!= 0, False, True];
  {resultCons, succeeded}
];

publicMethod[
  createVariableMap,
  cons, vars, assum, pars, current,
  Module[
    {map, tmpCons=cons, succeeded=False},

    If[cons === True, Return[{{}}]];
    If[cons === False, Return[{}]];

    tmpCons = removeUnnecessaryConstraints[LogicalExpand[cons], hasVariable];
    If[Head[tmpCons] =!= LogicalOr,
      {tmpCons, succeeded} = tryToTransformConstraints[applyList[LogicalExpand[tmpCons] ], vars]
    ];
    simplePrint[tmpCons, succeeded];
    If[!succeeded,
        (* try to solve with parameters *)
        tmpCons = removeUnnecessaryConstraints[cons, hasVariableOrParameter];
        tmpCons = Reduce[tmpCons, vars, Reals];
        tmpCons = removeUnnecessaryConstraints[tmpCons, hasVariable];
        tmpCons = LogicalExpand[tmpCons];
    ];

    simplePrint[Last[tmpCacheStack] ];
    cache = cache && givenCons;
    simplePrint[cache];

    tmpCons = applyListToOr[tmpCons];
    tmpCons = Map[(applyList[#])&, tmpCons];
    tmpCons = Map[(adjustExprs[#, isVariable])&, tmpCons];
    debugPrint["tmpCons after createMap in createVariableMap", tmpCons];
    map = Map[(convertExprs[#])&, tmpCons];
    map = Map[(Cases[#, Except[{p[___], _, _}] ])&, map];
    map = ruleOutException[map];
    simplePrint[map];
    map
  ]
];

createVariableMapFromCache[{prevCond_, cons_, pointFlag_}] := (
  Module[
    {tm, ret},
    {tm, ret} = Timing[createVariableMapFromCache[prevCond, cons, prevConstraint, prevRules, pConstraint, assumptions, currentTime, pointFlag] ];
    cacheTime = tm + cacheTime;
    simplePrint[cacheTime];
    ret
  ]
);


publicMethod[
  createVariableMapFromCache,
  pCond, constraint, prevCons, prevRs, pCons, assum, current, pointFlag,
  Module[
    {cond=pCond, cons=constraint, vars=getVariables[cons], map, tm},

    If[pointFlag,
      cond = cond /. t -> current;
      cons = cons /. t -> current;
    ];
    
    cond = (cond  //. prevRs) && prevConstraint && pConstraint;

    If[cond === False, Return[{}]];

    If[pointFlag,
      {tm, cons} = Timing[And@@Map[(timeConstrainedSimplify[#])&, applyList[(cons  //. prevRs)]] && prevConstraint && pConstraint];
      simpTime = tm + simpTime,
      cons = (cons  //. prevRs) && prevConstraint && pConstraint
    ];
    simplePrint[simpTime];
    simplePrint[cons];
    {cons, succeeded} = tryToTransformConstraints[applyList[cons] , vars];

    simplePrint[cons, succeeded];
    If[!succeeded,
        (* try to solve with parameters *)
        cons = removeUnnecessaryConstraints[cons, hasVariableOrParameter];
        cons = Reduce[cons, vars, Reals];
        cons = removeUnnecessaryConstraints[cons, hasVariable];
        cons = LogicalExpand[cons];
    ];
    simplePrint[cons];
    cons = And@@Map[(Expand[#])&, applyList[cons] ];
  
    cons = applyListToOr[cons];
    cons = Map[(applyList[#])&, cons];
    cons = Map[(adjustExprs[#, isVariable])&, cons];
    debugPrint["cons after createMap in createVariableMap", cons];
    map = Map[(convertExprs[#])&, cons];
    map = Map[(Cases[#, Except[{p[___], _, _}] ])&, map];
    map = ruleOutException[map];
    simplePrint[map];
    map
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
    sol = exDSolve[cons];
    debugPrint["sol after exDSolve", sol];
    If[sol === overConstrained || sol[[1]] === underConstrained,
      Message[createVariableMapInterval::underconstrained],
      tStore = createDifferentiatedEquations[vars, sol[[3]] ];
      tStore = Select[tStore, (!hasVariable[ #[[2]] ])&];
      cache = cache && And@@tStore;
      simplePrint[cache];
      tStore = tStore /. prevRs;
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
  {toReturnForm[LogicalExpand[Reduce[map && pCons] ] ]}
];


publicMethod[
  getParameterConstraint,
  {toReturnForm[LogicalExpand[pConstraint] ]}
];

publicMethod[
  exactlyEqual,
  lhs, rhs,
  TimeConstrained[Simplify[lhs] === Simplify[rhs], 1, False]
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
         map = applyListToOr[map];
         map = Union[Flatten[Map[(createMap[#, isParameter, {}, getParameters[pCons]])&, map], 1]];
         debugPrint["map after createMap in createParameterMap", map];
         map = Map[(convertExprs[#])&, map];
         map
      ]
    ]   
  ]
];

removeUnnecessaryConstraints[cons_, hasJudge_] :=
(
cons /. (expr_ /; ( MemberQ[{Equal, Element, NotElement, LessEqual, Less, Greater, GreaterEqual, Unequal, Inequality}, Head[expr] ] && (!hasJudge[expr] || hasPrevVariable[expr])) -> True)
);


(* 式中に変数名が出現するか否か *)

hasVariable[exprs_] := Length[getTimeVariablesWithDerivatives[exprs] ] > 0;

(* 式が変数もしくはその微分そのものか否か *)

isVariable[exprs_] := MatchQ[exprs, _Symbol] && (StringMatchQ[ToString[exprs], variablePrefix ~~ WordCharacter__] || StringMatchQ[ToString[exprs], derivativePrefix ~~ WordCharacter__] )|| MatchQ[exprs, Derivative[_][_][_] ] || MatchQ[exprs, Derivative[_][_] ] ;

(* 式中に出現する変数を取得 *)

getVariables[exprs_] := Cases[exprs, ele_ /; StringMatchQ[ToString[ele], variablePrefix ~~ WordCharacter..], {0, Infinity}, Heads->True];
getDerivatives[exprs_] := Union[Cases[exprs, Derivative[_][_], {0, Infinity}], Cases[exprs, Derivative[_][_][_], {0, Infinity}]];
getVariablesWithDerivatives[exprs_] := Union[getVariables[exprs], getDerivatives[exprs] ];
getTimeVariablesWithDerivatives[exprs_] := Union[getTimeVariables[exprs], getDerivatives[exprs] ];
getTimeVariables[exprs_] := Map[(#[t])&, getVariables[exprs]];

(* 式中に出現するprev値を取得 *)
getPrevVariables[exprs_] := Cases[exprs, prev[_, _], {0, Infinity}];

(* 式中に出現する記号定数を取得 *)
getParameters[exprs_] := Union[Cases[exprs, p[_, _, _], {0, Infinity}]];

getRelativeVars[vars_, consList_, getFunc_] :=
  Module[
    {nvars, npConsList},
    If[vars === {},
      {},
      nvars = Complement[Flatten[Select[Map[(getFunc[#])&, consList], (Length[Intersection[#, vars]]>0)&]], vars];
      nconsList = Select[Map[(getFunc[#])&, consList], (Length[Intersection[#, vars]]==0)&];
      Union[vars, getRelativeVars[nvars,nconsList,getFunc]]
    ]
  ]



(* 式中に定数名が出現するか否か *)

hasParameter[exprs_] := Length[Cases[exprs, p[_, _, _], {0, Infinity}] ] > 0;

hasParameterOrPrev[exprs_] := Length[Cases[{exprs}, p[_, _, _] | prev[_, _], {0, Infinity}] ] > 0;

hasVariableOrParameter[exprs_] := hasParameter[exprs] || hasVariable[exprs];


(* 式が定数そのものか否か *)

isParameter[exprs_] := Head[exprs] === p;

isParameterOrPrev[exprs_] := Head[exprs] === p || Head[exprs] === prev;

(* 式が指定されたシンボルを持つか *)
hasSymbol[exprs_, syms_List] := MemberQ[{exprs}, ele_ /; (MemberQ[syms, ele] || (!AtomQ[ele] && hasSymbol[Head[ele], syms]) ), {0, Infinity} ];

(* 式がprev変数そのものか否か *)
isPrevVariable[exprs_] := Head[exprs] === prev;

(* 式がprev変数を持つか *)
hasPrevVariable[exprs_] := Length[Cases[exprs, prev[_, _], {0, Infinity}]] > 0;

resolvePrev[consList_] :=
  Module[
    {vars=getVariablesWithDerivatives[consList], prevs=getPrevVariables[consList], relatedCons, relatedVars, newRelatedVars, newRelatedCons, remainCons=consList, ret={}},
    For[i=1, i<=Length[prevs], i++,
      If[remainCons === {}, Break[]];
      relatedCons = Select[remainCons, MemberQ[getPrevVariables[#], prevs[[i]] ]& ];
      remainCons = Complement[remainCons, relatedCons];
      relatedVars = {};
      newRelatedCons = relatedCons;
      While[True,
        newRelatedVars = getVariablesWithDerivatives[newRelatedCons];
        If[Length[newRelatedVars] === 0,
          Break[],
          newRelatedCons = Select[remainCons, MemberQ[getVariablesWithDerivatives[#], newRelatedVars]& ];
          remainCons = Complement[remainCons, newRelatedCons];
          relatedVars = Join[relatedVars, newRelatedVars];
          relatedCons = Join[relatedCons, newRelatedCons];
        ];
      ];
      simplePrint[relatedVars];
      simplePrint[relatedCons];
      If[Length[relatedVars] < Length[relatedCons],
        ret = Join[ret, relatedCons];
      ];
    ];
    {And@@ret, And@@Complement[consList, ret]}
  ]

resolveOrd[cons_, vars_] :=
  Module[
    {relatedConsMap, resolved = {}, restVars=vars, found},
    For[i=1, i<=Length[vars], i++,
      relatedConsMap[vars[[i]]] = Select[cons, MemberQ[getVariablesWithDerivatives[#], vars[[i]] ]&];
    ];
    While[True,
      found =
        Select[
          restVars,
          Function[var, Fold[(#1||Complement[getVariablesWithDerivatives[#2], Append[resolved, var]]==={})&, False, applyList[relatedConsMap[var]] ]]
        ];
      If[found == {},
         Break[],
         resolved = Join[resolved, found];
         restVars = Complement[restVars, found];
         found = {}
      ]
    ];
    Join[resolved, restVars]
  ]


(* 変数varsに関する制約ストアからjudgeFunctionでTrueとなる変数に関するMapを作成 *)
createMap[cons_, judgeFunction_, vars_, pars_] :=
  Module [
    {resolvedVars},
    resolvedVars = If[vars === {}, {}, resolveOrd[cons, vars]];
    simplePrint[Join[pars, resolvedVars]];
    Map[
      (Fold[
        (If[Head[#2]===Inequality&&judgeFunction[#2[[3]]],
            Union[#1, {getReverseRelop[#2[[2]] ]@@{#2[[3]],#2[[1]] }, #2[[4]]@@{#2[[3]],#2[[5]] } } ],
          If[MemberQ[{Equal,Unequal,Less,LessEqual,Greater,GreaterEqual}, Head[#2]]&&judgeFunction[#2[[1]]],
            Append[#1, #2],
            #1] ])&,
        {},
        #
      ])&,
      consToDoubleList[Quiet[Reduce[cons, Join[pars,resolvedVars]], Reduce::useq]]
    ]
  ]

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
  resultConstraint = Null;
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
  initCache,
  tmpCacheStack = Append[tmpCacheStack, True];
];

publicMethod[
  enableCheckFirst,
  checkFirstFlag = True;
];

publicMethod[
  popCache,
  Module[
    {prevCond, ret},
    prevCond = Last[tmpCacheStack];
    ret = cache;
    cache = True;
    tmpCacheStack = Most[tmpCacheStack];
    ret = ret /. Derivative[cnt_][var_][_] -> Derivative[cnt][var];
    prevCond = prevCond /. Derivative[cnt_][var_][_] -> Derivative[cnt][var];
    toReturnForm[{{LogicalExpand[prevCond]}, {LogicalExpand[ret]}} ]
  ]
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
    cons = Assuming[assumptions, timeConstrainedSimplify[cons]];
    constraint = constraint && cons;
  ]
];

addInitConstraint[co_] := Module[
  {},
  initConstraint = initConstraint && And@@co;
];

addPrevLessEqual[var_, expr_] := addPrevConstraint[var <= expr];
addPrevLess[var_, expr_] := addPrevConstraint[var < expr];
addPrevGreaterEqual[var_, expr_] := addPrevConstraint[var >= expr];
addPrevGreater[var_, expr_] := addPrevConstraint[var > expr];

addLessEqual[var_, expr_] := addConstraint[var <= expr];
addLess[var_, expr_] := addConstraint[var <= expr];
addGreaterEqual[var_, expr_] := addConstraint[var >= expr];
addGreater[var_, expr_] := addConstraint[var > expr];

makeRulesForVariable[cons_] := Map[(#[[1]] -> #[[2]])&, adjustExprs[applyList[cons], hasVariable]];

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
    {var, timeVar},
    var = If[diffCnt > 0, derivative[diffCnt, name], name];
    timeVar = If[diffCnt > 0, Derivative[diffCnt][name], name];
    Unprotect[variables, prevVariables, timeVariables];
    variables = Union[variables, {var}];
    prevVariables = Union[prevVariables,
      {makePrevVar[var]} ];
    timeVariables = Union[timeVariables, {timeVar[t]} ];
    Protect[variables, prevVariables, timeVariables];
  ]
];


setVariables[vars_] := (
  Unprotect[variables, prevVariables, timeVariables];
  variables = vars;
  prevVariables = Map[makePrevVar, vars];
  timeVariables = Map[(#[t])&, vars];
  Protect[variables, prevVariables, timeVariables];
);


publicMethod[
  resetVariables,
  Unprotect[variables, prevVariables, timeVariables];
  variables = {};
  prevVariables = {};
  timeVariables = {};
  Protect[variables, prevVariables, timeVariables];
];


publicMethod[
  resetConstraintForParameter,
  pCons,
  pConstraint = Reduce[pCons, getParameters[pCons], Reals];
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
  pConstraint = Reduce[pConstraint && pCons, Union[getParameters[pConstraint], getParameters[pCons]], Reals];
  debugPrint["pConstraint", pConstraint];
  debugPrint["pCons", pCons]
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


(* Piecewiseの第二要素（第一要素の各場合以外の場合）に対し，
othersと第一要素中の全条件の否定の論理積を取って条件とし，第一要素の末尾に付加する*)

makeListFromPiecewise[minT_, others_] := Module[
  {tmpCondition = False, retMinT},
  If[Head[minT] =!= Piecewise, Return[{{minT, others}}] ];
  retMinT = minT[[1]];

  tmpCondition = Or @@ Map[(#[[2]] )&, minT[[1]]];
  tmpCondition = Reduce[And[others, Not[tmpCondition]], Reals];
  tmpCondition = removeUnnecessaryConstraints[tmpCondition, hasParameter];
  retMinT = Map[({#[[1]], removeUnnecessaryConstraints[Reduce[others && #[[2]], Reals ], hasParameter]})&, retMinT];
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

    consList = applyList[LogicalExpand[pCons]];
    parsInCons = Union[getRelativeVars[getParameters[tCons && maxCons], consList, getParameters]];
    debugPrint["parsInCons", parsInCons];
    necessaryPCons = Select[consList, (Length[Intersection[getParameters[#], parsInCons] ] > 0)&];
    restPCons = And@@Complement[consList, necessaryPCons];
    necessaryPCons = And@@necessaryPCons;
    simplePrint[necessaryPCons];
    simplePrint[restPCons];

    Quiet[Check[minT = TimeConstrained[Minimize[{t, t > startingTime && tCons && necessaryPCons && maxCons}, {t}], 3],
         onTime = False,
         Minimize::wksol
       ],
       {Minimize::wksol, Minimize::infeas}
    ];
    (* TODO: 解が分岐していた場合、onTimeは必ずしも一意に定まらないため分岐が必要 *)
    debugPrint["minT after Minimize:", minT];
    If[minT === $Aborted || Head[minT] === Minimize,
      Message[findMinTime::minimizeFailure, minT],
      minT = First[minT];
      If[minT === Infinity,
        {},
        ret = makeListFromPiecewise[minT, necessaryPCons];
        ret = Map[({#[[1]], #[[2]] && restPCons})&, ret];
        (* 時刻が0となる場合は弾く．*)
        ret = Select[ret, (#[[1]] =!= 0)&];
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
      andCond, caseEq, caseLe, caseGr, ret, usedPars, otherPCons = False, pRules, intervalLess, intervalGreater, interval
    },
    usedPars = Union[getParameters[time1], getParameters[time2] ];
    andCond = Reduce[pCons1 && pCons2];
    If[andCond === False,
      Return[{{False}, {False}, {False}}]
    ];
    (* First, compare 2 expressions using interval arithmetic *)
    If[time1 === Infinity,
      Return[{{False}, {toReturnForm[LogicalExpand[andCond] ]}, {False}}]
    ];
    pRules = createIntervalRules[andCond];
    If[pRules =!= failed,
      interval = Quiet[N[(time1 - time2) /. pRules], {N::meprec}];
      intervalLess = interval < 0;
      If[intervalLess === True,
        Return[{{toReturnForm[LogicalExpand[andCond] ]}, {False}, {False}}]
      ];
      intervalGreater = interval > 0;
      If[intervalGreater === True,
        Return[{{False}, {toReturnForm[LogicalExpand[andCond] ]}, {False}}]
      ];
    ];
    (* If it isn't determined, use Reduce *)
    caseEq = Quiet[Reduce[And[andCond, time1 == time2], Reals]];
    simplePrint[caseEq];
    caseLe = Quiet[Reduce[And[andCond, time1 < time2], Reals]];
    simplePrint[caseLe];
    caseGr = Reduce[andCond && !caseLe && !caseEq];
    simplePrint[caseGr];
    {{toReturnForm[LogicalExpand[caseLe] ]}, {toReturnForm[LogicalExpand[caseGr] ]}, {toReturnForm[LogicalExpand[caseEq] ]}}
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

exDSolve[expr_] :=
Module[
  {listExpr, reducedExpr, rules, tVars, resultCons, unsolvable = False, resultRule, searchResult, retCode, restCond},
  inputPrint["exDSolve", expr];
  listExpr = applyList[expr];
  sol = {};
  resultCons = Select[listExpr, (Head[#] =!= Equal)&];
  listExpr = Complement[listExpr, resultCons];
  (* add constraint "t > 0" to exclude past case *)
  resultCons = And@@resultCons && t > 0;
  simplePrint[listExpr];
  resultRule = Quiet[Check[solveByDSolve[listExpr, getVariables[listExpr] ], {}] ];
  If[resultRule =!= overconstrained && Head[resultRule] =!= DSolve && Length[resultRule] == 1, 
    resultRule = resultRule[[1]];
    listExpr = {},
    (* resultRule may equal overconstrained for the constraint such as x'[t] == 0 && x[t] == 0 && x[0] == 0,
       therefore we have to check furthermore. *)
    tVars = getTimeVariables[listExpr];
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
        simplePrint[listExpr];
        If[MemberQ[listExpr, ele_ /; (ele === False || (!hasVariable[ele] && MemberQ[ele, t, {0, Infinity}]))], Return[overConstrained] ];
        listExpr = Select[listExpr, (#=!=True)&];
        tVars = getTimeVariables[listExpr];
        simplePrint[listExpr, tVars];
        resultCons = applyDSolveResult[resultCons, resultRule];
        If[resultCons === False, Return[overConstrained] ];
      ]
    ]
  ];
  simplePrint[resultRule, resultCons];
  retCode = If[Length[listExpr] > 0 || unsolvable, underConstrained, solved];
  restCond = And@@listExpr && applyDSolveResult[resultCons, resultRule];
  restCond = LogicalExpand[Assuming[t > 0, Simplify[restCond] ] ];
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
  {ini = {}, sol, derivatives, i},
  tVars = Map[(#[t])&, vars];
  derivatives = getTimeVariablesWithDerivatives[expr];
  For[i = 1, i <= Length[derivatives], i++,
    ini = Append[ini, createPrevRules[derivatives[[i]] ] ]
  ];
  tmp = expr;
  sol = Quiet[
    Check[
      DSolve[Union[expr, ini], tVars, t],
          overConstrained,
      {DSolve::overdet, DSolve::bvimp}
    ],
  {DSolve::overdet, DSolve::bvimp, Solve::svars, PolynomialGCD::lrgexp}
  ];
  (* remove solutions with imaginary numbers *)
  For[i = 1, i <= Length[sol], i++,
    If[Count[Map[(timeConstrainedSimplify[Element[#[[2]], Reals]])&, sol[[i]] ], False] > 0,
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
      Quiet[Reduce[ForAll[pars, pCons, eq]] === True]
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
    sol = Quiet[Solve[borderCond && t > 0 && pCons, {t}], {PolynomialGCD::lrgexpr}];
    (*TODO: consider case branching*)
    timeList = Map[(#[[1,2]])&, sol];
    timeList = Map[(If[Head[#] === ConditionalExpression, #[[1]], #])&, timeList];
    timeList = Map[({toReturnForm[#], 1, {toReturnForm[LogicalExpand[pCons] ]}, -1})&, timeList];
    timeList
  ]
];

 (* TODO: consider case branching *)
sortWithParameters[timeList_, pCons_, pars_] := Sort[timeList, Reduce[#1 < #2 && pCons, Join[pars, t], Reals] =!= False];

trueAtInitialTime[guard_, initialTime_] := trueAtInitialTime[guard, initialTime, parameters, pConstraint];

publicMethod[
  trueAtInitialTime,
  guard, initialTime, pars, pCons, 
  (* Quiet[Reduce[(guard /. t -> initialTime) && pCons, pars, Reals] =!= False] *)
  Quiet[Reduce[((guard /. t -> initialTime) /. createIntervalRules[pCons]), pars, Reals] =!= False]
]

getMinimum[timeList_] := getMinimum[timeList, pConstraint, parameters]; 

(*
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
      debugPrint["BREAK1 reduce: ", Reduce[minimum <= timeList[[i, 1]] && pCons, pars, Reals]];
      If[Reduce[minimum <= timeList[[i, 1]] && pCons, pars, Reals] === False,
        minimum = timeList[[i, 1]];
        minimumIndex = i - 1;
      ];
    ];
    debugPrint["BREAK1 minimumIndex: ", minimumIndex];
    {{toReturnForm[minimum], 1, {toReturnForm[LogicalExpand[pCons] ]}, minimumIndex}}
  ]
];
*)
publicMethod[
  getMinimum,
  timeList,
  pCons, 
  pars,
  Module[
    {minimum, i, minimumIndex, pRules, interval, intervalLess, intervalGreater},
    minimum = timeList[[1, 1]];
    minimumIndex = 0;
    (* TODO: deal with case branching *)
    pRules = createIntervalRules[pCons];
	debugPrint["BREAK1 interval: ", N[(timeList[[1, 1]]) /. pRules]];
    For[i = 2, i <= Length[timeList], i++,
      interval = Quiet[N[(minimum - timeList[[i, 1]]) /. pRules], {N::meprec}];
      debugPrint["BREAK1 interval: ", N[(timeList[[i, 1]]) /. pRules]];
      intervalLess = interval < 0;
      intervalGreater = interval > 0;
      If[intervalLess =!= True && intervalGreater =!= True,
        minimum = timeList[[1, 1]];
        minimumIndex = 0;
        Break[]
      ];
      If[intervalGreater === True,
        minimum = timeList[[i, 1]];
        minimumIndex = i - 1
      ];
      If[i == Length[timeList],
        Return[{{toReturnForm[minimum], 1, {toReturnForm[LogicalExpand[pCons] ]}, minimumIndex}}]
      ];
    ];
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


publicMethod[
  removeRedundantParameters,
  start, end, vm, pm,
  Module[
    {parsInVM, parsInPM, redundantPars},
    parsInVM = Fold[Union[#1, getRelativeVars[Union[getParameters[start], getParameters[end], getParameters[vm] ], #2, getParameters]]&, {}, consToDoubleList[pm]];
    simplePrint[parsInVM];
    parsInPM = getParameters[pm];
    redundantPars = Complement[parsInPM, parsInVM];
    simplePrint[redundantPars];
    {toReturnForm[LogicalExpand[Reduce[Exists[Evaluate[redundantPars], pm], parsInVM, Reals] ] ]}
  ]
];
