publicMethod[checkInclude, largeTime, largeVm, largePm, smallTime, smallVm, smallPm,
             Module[{ret,i,tmpLargeTime,tmpSmallTime,tmpSmallVm,tmpLargeVm,tmpSmallPar,tmpLargePar,allExpr,listLarge,listSmall},
                    ret = True;
                    tmpLargeVm = largeVm //. p -> p1;
                    tmpSmallVm = smallVm //. p -> p2;
                    tmpLargeTime = largeTime //. p -> p1;
                    tmpSmallTime = smallTime //. p -> p2;
                    (* debugPrint["# tmpLargeVm", tmpLargeVm];
                     debugPrint["# tmpSmallVm", tmpSmallVm]; *)
                    (* compare variable num *)
                    If[Length[tmpLargeVm] == Length[tmpSmallVm],
                       ret = And[ret,True],
                       ret = And[ret,False]
                       ];
                    For[i=1,And[ret, Length[tmpLargeVm] >= i],i++,
                        (* debugPrint["# largeVm[i]",tmpLargeVm[[i]]];
                         debugPrint["# largeVm[i][0]",tmpLargeVm[[i]][[0]]];
                         debugPrint["# largeVm[i][1]",tmpLargeVm[[i]][[1]]];
                         debugPrint["# largeVm[i][2]",tmpLargeVm[[i]][[2]]];
                         debugPrint["# smallVm[i]",tmpSmallVm[[i]]];
                         debugPrint["# smallVm[i][0]",tmpSmallVm[[i]][[0]]];
                         debugPrint["# smallVm[i][1]",tmpSmallVm[[i]][[1]]];
                         debugPrint["# smallVm[i][2]",tmpSmallVm[[i]][[2]]]; *)
                        (* compare variable name *)
                        If[tmpLargeVm[[i]][[1]] == tmpSmallVm[[i]][[1]],
                           ret = And[ret,True],
                           ret = And[ret,False]
                           ];
                        (* correct expr : large.2 == small.2 *)
                        (* debugPrint["# adding expr : ",Simplify[tmpLargeVm[[i]][[2]] /. t -> tmpLargeTime] == Simplify[tmpSmallVm[[i]][[2]] /. t -> tmpSmallTime]]; *)
                        If[i == 1,
                           allExpr = Simplify[tmpLargeVm[[i]][[2]] /. t -> tmpLargeTime] == Simplify[tmpSmallVm[[i]][[2]] /. t -> tmpSmallTime],
                           allExpr = And[allExpr,Simplify[tmpLargeVm[[i]][[2]] /. t -> tmpLargeTime] == Simplify[tmpSmallVm[[i]][[2]] /. t -> tmpSmallTime]]
                           ];
                        ];
                    allExpr = Simplify[allExpr];
                    debugPrint["# all Expr from vm", allExpr];
                    tmpLargePm = largePm //. p -> p1;
                    tmpSmallPm = smallPm //. p -> p2;
                    (* debugPrint["# tmpLargePm", tmpLargePm];
                     debugPrint["# tmpSmallPm", tmpSmallPm]; *)
                    listLarge = {};
                    listSmall = {};
                    For[i=1,And[ret,Length[tmpLargePm] >= i],i++,
                        (* debugPrint["# largePm[i]",tmpLargePm[[i]]];
                         debugPrint["# largePm[i][0]",tmpLargePm[[i]][[0]]];
                         debugPrint["# largePm[i][1]",tmpLargePm[[i]][[1]]];
                         debugPrint["# largePm[i][2]",tmpLargePm[[i]][[2]]]; *)
                        listLarge = Append[listLarge,tmpLargePm[[i]][[1]]];
                        ];
                    listLarge = DeleteDuplicates[listLarge];
                    (* debugPrint["# large list",listLarge]; *)
                    For[i=1,And[ret,Length[tmpSmallPm] >= i],i++,
                        (* debugPrint["# smallPm[i]",tmpSmallPm[[i]]];
                         debugPrint["# smallPm[i][0]",tmpSmallPm[[i]][[0]]];
                         debugPrint["# smallPm[i][1]",tmpSmallPm[[i]][[1]]];
                         debugPrint["# smallPm[i][2]",tmpSmallPm[[i]][[2]]]; *)
                        listSmall = Append[listSmall,tmpSmallPm[[i]][[1]]];
                        ];
                    listSmall = DeleteDuplicates[listSmall];
                    (* debugPrint["# small list",listSmall]; *)
                    (* tmpLargePm = And[tmpLargePm, t>0];
                     tmpSmallPm = And[tmpSmallPm, t>0]; *)
                    If[ret,
                       (* debugPrint["#listLarge",listLarge];
                        debugPrint["#listSmall",listSmall];
                        debugPrint["#tmpLargePm",tmpLargePm];
                        debugPrint["#tmpSmallPm",tmpSmallPm];
                        debugPrint["#allExpr",allExpr];
                        debugPrint["#forall exists",forAll[evaluate[listSmall],tmpSmallPm,exists[evaluate[listLarge],tmpLargePm,allExpr]]]; *)
                       ret = Reduce[ForAll[Evaluate[listSmall],tmpSmallPm,Exists[Evaluate[listLarge],tmpLargePm,allExpr]]],
                       ret = False
                       ];
                    If[ret =!= True,ret = False,ret = True];
                    (* debugPrint["# final return", ret]; *)
                    ret
                    ]
             ];

publicMethod[checkInclude, past, now, pConstraintPast, pConstraintNow,
             Module[{minPast, maxPast, minNow, maxNow, tmp, reduceExprPast, reduceExprNow, tmpresult},
                    minPast = Quiet[Minimize[{past, And@@pConstraintPast && t > 0},
                                             If[Union[getParameters[past && pConstraintPast]] =!= {},
                                                Union[getParameters[past && pConstraintPast]], {tmp}],
                                             Reals], {Minimize::wksol, Minimize::infeas}];
                    simplePrint[minPast];
                    maxPast = Quiet[Maximize[{past, And@@pConstraintPast && t > 0},
                                             If[Union[getParameters[past && pConstraintPast]] =!= {},
                                                Union[getParameters[past && pConstraintPast]], {tmp}],
                                             Reals], {Maximize::wksol, Maximize::infeas}];
                    simplePrint[maxPast];
                    minNow = Quiet[Minimize[{now, And@@pConstraintNow && t > 0},
                                            If[Union[getParameters[now && pConstraintNow]] =!= {},
                                               Union[getParameters[now && pConstraintNow]], {tmp}],
                                            Reals], {Minimize::wksol, Minimize::infeas}];
                    simplePrint[minNow];
                    maxNow = Quiet[Maximize[{now, And@@pConstraintNow && t > 0},
                                            If[Union[getParameters[now && pConstraintNow]] =!= {},
                                               Union[getParameters[now && pConstraintNow]], {tmp}],
                                            Reals], {Maximize::wksol, Maximize::infeas}];
                    simplePrint[maxNow];
                                   (* not (minPast < tmp < maxPast) && minNow < tmp <
                                                    maxNow (=ã¯checkIncludeã®æ»ãå¤ã§å¤æ­) *)
                    debugPrint["# minPast # : ",Refine[minPast,t>0][[1]]];
                    minPast = Refine[minPast,t>0][[1]];
                    debugPrint["# maxPast # : ",Refine[maxPast,t>0][[1]]];
                    maxPast = Refine[maxPast,t>0][[1]];
                    debugPrint["# minNow # : ",Refine[minNow,t>0][[1]]];
                    minNow = Refine[minNow,t>0][[1]];
                    debugPrint["# maxNow # : ",Refine[maxNow,t>0][[1]]];
                    maxNow = Refine[maxNow,t>0][[1]];
                    tmpresult = Reduce[minPast <= minNow && maxPast >= maxNow];
                    debugPrint["# result # : ",tmpresult];
                    tmpresult
                    (* If[Reduce[minPast <= minNow && maxPast >= maxNow], True , False]*)
                    (*
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
                                              Not[reduceExprPast] && reduceExprNow] === False, True,
                                                         False]
                     *)
                                        ]
             ];

(* ポイントフェーズにおける無矛盾性判定 *)
checkConsistencyPointEpsilon[] := (
  checkConsistencyPointEpsilon[constraint && tmpConstraint && initConstraint && initTmpConstraint && prevIneqs, pConstraint, Union[variables, prevVariables], parameters ]
                            );

publicMethod[
             checkConsistencyPointEpsilon,
             cons, pcons, vars, pars,
             Module[
                    {cpTrue, cpFalse},
                    Quiet[
                          cpTrue = Reduce[Exists[vars, cons && pcons], Reals], {Reduce::useq}
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
                    debugPrint["# checkConsistencyP # : cpTrue", cpTrue];
                    debugPrint["# checkConsistencyP # : cpFalse", cpFalse ];
                    toReturnForm[{{LogicalExpand[cpTrue]}, {LogicalExpand[cpFalse]}}]
                    ]
             ];

(* インターバルフェーズにおける無矛盾性判定 *)
checkConsistencyIntervalEpsilon[] :=  (
  checkConsistencyIntervalEpsilon[constraint && tmpConstraint, initConstraint && initTmpConstraint, prevIneqs, pConstraint, timeVariables, initVariables, prevVariables, parameters]
                                );


moveTermsToLeft[expr_] := Head[expr][expr[[1]] - expr[[2]], 0];

ccIntervalForEachEpsilon[cond_, initRules_, pCons_] :=
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
             checkConsistencyIntervalEpsilon,
             cons, initCons, prevCons, pCons, timeVars, initVars, prevVars, pars,
             Module[
                    {sol, otherCons, tCons, i, j, conj, cpTrue, eachCpTrue, cpFalse},
                    If[cons === True,
                        {{LogicalExpand[pCons]}, {False}},
                       sol = exDSolve[cons, initCons];
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


(* 次のポイントフェーズに移行する時刻を求める *)
calculateNextPointPhaseTimeEpsilon[maxTime_, discCauses_] :=
  calculateNextPointPhaseTimeEpsilon[maxTime, discCauses, constraint, initConstraint, pConstraint, timeVariables];

publicMethod[
             calculateNextPointPhaseTimeEpsilon,
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
                    debugPrint["# In EPSILON MODE # : CNPPTT constraint ", cons];
                    debugPrint["# In EPSILON MODE # : CNPPTT cauzeAndIDs ", causeAndIDs];

                    tStore = Map[(Rule@@#)&, createDifferentiatedEquations[vars, applyList[cons] ] ];
                    timeAppliedCauses = causeAndIDs /. tStore;
                    simplePrint[timeAppliedCauses];

                    parameterList = getParameters[timeAppliedCauses];

                    (* 必要なpConsだけを選ぶ．不要なものが入っているとMinimzeの動作がおかしくなる？ *)

                    necessaryPCons = LogicalExpand[pCons] /. (expr_ /; (( Head[expr] === Equal || Head[expr] === LessEqual || Head[expr] === Less|| Head[expr] === GreaterEqual || Head[expr] === Greater) && (!hasSymbol[expr, parameterList])) -> True);

                    debugPrint["# In EPSILON MODE # : necessaryPCons is ",necessaryPCons];

                    resultList = calculateMinTimeListEpsilon[timeAppliedCauses, necessaryPCons, maxTime];

                    (* 整形して結果を返す *)
                    resultList = Map[({#[[1]], #[[2]], LogicalExpand[#[[3]] ]})&, resultList];
                    resultList = Fold[(Join[#1, If[Head[#2[[3]]]===Or, divideDisjunction[#2], {#2}]])&,{}, resultList];
                    resultList = Map[({#[[1]], #[[2]], Cases[applyList[#[[3]] ], Except[True]]})&, resultList];
                    resultList = Map[
                                     ({timeAndIDsToReturn[#[[1]] ], Map[(timeAndIDsToReturn[#])&, #[[2]] ], convertExprs[adjustExprs[#[[3]], isParameter ] ] })&, resultList];

                    debugPrint["# In EPSILON MODE # :CNPPTT resultList ", resultList];
                    resultList
                    ]
             ];

(* 最小時刻と条件の組をリストアップする関数 *)
calculateMinTimeListEpsilon[causeAndIDList_, condition_, maxT_] := (
Block[
  {findResult, i},
  (* -1 is regarded as the id of time limit *)
  timeCaseList = {{timeAndIDs[maxT, ids[-1]], {}, condition}};
  For[i = 1, i <= Length[causeAndIDList], i++,
      findResult = findMinTimeEpsilon[causeAndIDList[[i]], condition];
      timeCaseList = compareMinTimeListEpsilon[timeCaseList, findResult]
      ];
  debugPrint["# In EPSILON MODE # : timeCaseList is ",timeCaseList];
  timeCaseList
      ]
);

(* 条件を満たす最小の時刻と，その条件の組を求める *)
findMinTimeEpsilon[causeAndID_, condition_] :=
  Module[
         {
           id,
           tmp,
           cause,
           minT,
           ret
         },
         id = causeAndID[[2]];
         cause = causeAndID[[1]];
         debugPrint["# In EPSILON MODE # : causeAndID[[1]] ", causeAndID[[1]]];
         debugPrint["# In EPSILON MODE # : causeAndID[[2]] ", causeAndID[[2]]];
         tmp = cause && condition && t > 0;
         debugPrint["# In EPSILON MODE # : sol before Reduce", tmp];
         sol = Reduce[cause && condition && t > 0, t, Reals];
         checkMessage[];
         If[sol === False, Return[{}] ];
         (* 成り立つtの最小値を求める *)
         debugPrint["# In EPSILON MODE # : get Min t of sol", sol];
         minT = First[Quiet[Minimize[{t, sol}, {t}], Minimize::wksol]];
         debugPrint["# In EPSILON MODE # : minT ", minT];
         ret = makeListFromPiecewise[minT, condition];
         (* 時刻が0となる場合を取り除く．*)
         ret = Select[ret, (#[[1]] =!= 0)&];
         If[False,
            ret = nextPPTimeShift[ret];
            ];
         (* append id for each time *)
         ret = Map[({timeAndIDs[#[[1]], ids[id] ], #[[2]]})&, ret];
         debugPrint["# In EPSILON MODE # : FindMinTime result ", ret];
         ret
         ];

nextPPTimeShift[ret_] :=
  Module[
         {head,isNotZero,shift,result}
         isNotZero = Limit[#[[1]], p[peps, 0, 1] -> 0] =!= 0;
         ];

(* ２つの時刻と条件の組のリストを比較し，各条件組み合わせにおいて，最小となる時刻と条件の組のリストを返す *)
compareMinTimeListEpsilon[list1_, list2_] := ( Block[
  {resultList, i, j},
  If[list2 === {}, Return[list1] ];
  resultList = {};
  For[i = 1, i <= Length[list1], i++,
      For[j = 1, j <= Length[list2], j++,
          resultList = Join[resultList, compareMinTimeEpsilon[list1[[i]], list2[[j]] ] ]
          ]
      ];
  resultList
  ]
);

(* TODO: 場合分けをしていくだけで、併合はしないので最終的に冗長な場合分けが発生する可能性がある。 *)
(* ２つの時刻と条件の組を比較し，最小時刻とその条件の組のリストを返す *)
compareMinTimeEpsilon[timeCond1_, timeCond2_] := ( Block[
  {
    minTime1, minTime2,
    timeAndID1, timeAndID2,
    nonMinimum,
    caseEq,caseLe, caseGr,
    ret,
    andCond
  },
  (* assume that only timeCond1 only has nonMinimum *)
  andCond = Reduce[timeCond1[[3]] && timeCond2[[2]], Reals];
  If[andCond === False, Return[{}] ];
  timeAndID1 = timeCond1[[1]];
  timeAndID2 = timeCond2[[1]];
  minTime1 = timeAndID1[[1]];
  minTime2 = timeAndID2[[1]];
  nonMinimum = timeCond1[[2]];
  caseEq = Quiet[Reduce[And[andCond, minTime1 == minTime2], Reals]];
  caseLe = Quiet[Reduce[And[andCond, minTime1 < minTime2], Reals]];
  caseGr = Reduce[andCond && !caseLe && !caseEq];
  ret = {};
  If[ caseEq =!= False,
      ret = Append[ret,
                   {timeAndIDs[minTime1, Join[timeAndID1[[2]], timeAndID2[[2]] ] ], nonMinimum, caseEq}]
      ];
  If[ caseLe =!= False,
      ret = Append[ret,
                   {timeAndID1, Append[nonMinimum, timeCond2[[1]]], caseLe}]
      ];
  If[ caseGr =!= False,
      ret = Append[ret,
                   {timeAndID2, Append[nonMinimum, timeCond1[[1]]], caseGr}]
      ];
  Return[ ret ];
  ]
);


publicMethod[
             exprTimeShift,
             expr, time,
             toReturnForm[Simplify[expr /. t -> t - time]]
             ];

publicMethod[
             exprTimeShiftInverse,
             expr, time,
             toReturnForm[expr /. t -> t + time]
             ];

(* expr > 0かどうか *)
isOverZero[expr_] := isOverZero[expr, pConstraint];
publicMethod[
             isOverZero,
             expr, pCons,
             Module[
                    {isOverZeroTrue,ret},
                    debugPrint["is > 0 ?",expr];
                    isOverZeroTrue = Refine[expr > 0, Assumptions -> pCons];
                    simplePrint[isOverZeroTrue];
                    ret = isOverZeroTrue === True;
                    simplePrint[ret];
                    ret
                    ]
             ];


(* expr < 0 *)
lessThanZero[expr_] := lessThanZero[expr, pConstraint];
publicMethod[
             lessThanZero,
             expr, pCons,
             Module[
                    {less,ret},
                    less = Refine[expr < 0, Assumptions -> pCons];
                    ret = less === True;
                    ret
                    ]
             ];

(* expr < 0 *)
unequalZero[expr_] := unequalZero[expr, pConstraint];
publicMethod[
             unequalZero,
             expr, pCons,
             Module[
                    {unequal,ret},
                    unequal = Refine[expr != 0, Assumptions -> pCons];
                    ret = unequal === True;
                    ret
                    ]
             ];


(* n(dcount)次近似をする *)
publicMethod[
             cutHighOrderVariable,
             expr, var, dcount,
             Module[
                    {sTmp,sCond,dTimes,factorial,dExpr,ret},
                    dTimes = 0;
                    factorial = 1;
                    dExpr = expr;
                    sTmp = Quiet[Check[dExpr /. var -> 0, False, {Power::infy, Power::indet}]];
                    sCond = sTmp =!= False;
                    If[sCond, ret = sTmp, ret = expr];
                    While[sCond && dTimes < dcount,
                          dExpr = D[dExpr, var];
                          Clear[sTmp,sCond];
                          sTmp = Quiet[Check[dExpr /. var -> 0, False, {Power::infy, Power::indet}]];
                          sCond = sTmp =!= False;
                          dTimes++;
                          factorial = factorial * dTimes;
                          If[sCond, ret = ret + sTmp * var ^ dTimes / factorial, ret = expr];
                          ];
                    toReturnForm[ret]
                    ]
             ];

publicMethod[
             isZero,
             expr,
             Module[
                    {ret},
                    debugPrint["is not zero expr = ",expr];
                    ret = Limit[expr, p[peps, 0, 1] -> 0] === 0;
                    simplePrint[ret];
                    ret
                    ]
             ];


publicMethod[
             limitEpsilon,
             arg,
             debugPrint["limitEpsilon arg",arg];
             toReturnForm[Limit[arg, p[peps, 0, 1] -> 0]]
             ];

publicMethod[
             reduceEpsilon,
             arg,
             Module[
                    {one,two,ret},
                    one = Quiet[Check[arg /. p[eps, 0, 1] -> 0, False, {Power::infy, Power::indet}]];
                    two = Quiet[Check[D[arg, p[eps, 0, 1]] /. p[eps, 0, 1] -> 0, False, {Power::infy, Power::indet}]];
                    If[one =!= False && two =!= False, ret = one + two * p[eps, 0, 1], ret = arg];
                    toReturnForm[ret]
                    ]
             ];

publicMethod[
             checkEpsilon,
             arg,
             Module[
                    {direplus,direminus,flag,ret},
                    debugPrint["checkEpsilon arg",arg];
                    ret = 0;
                    If[arg =!= Infinity,
                       direplus = Limit[arg, p[eps, 0, 1] -> 0,Direction->-1];
                       direminus = Limit[arg, p[eps, 0, 1] -> 0,Direction->1];
                       flag = FullSimplify[direplus - direminus];
                       If[flag === 0,
                          ret = 1,
                          ret = 0
                          ];
                       ];
                    ret
                    ]
             ];

publicMethod[
             limitEpsilonP,
             arg,
             debugPrint["limitEpsilonP arg",arg];
             toReturnForm[Limit[arg, p[eps, 0, 1] -> 0,Direction->-1]]
             ];

publicMethod[
             limitEpsilonM,
             arg,
             debugPrint["limitEpsilonM arg",arg];
             toReturnForm[Limit[arg, p[eps, 0, 1] -> 0,Direction->1]]
             ];

publicMethod[
             diffEpsilon,
             arg,
             Module[
                    {tmp,ret},
                    debugPrint["json diffEpsilonP arg",arg];
                    tmp = arg /. p[peps, 0, 1] -> 0;
                    ret = Simplify[arg - tmp];
                    toReturnForm[ret]
                    ]
             ];
