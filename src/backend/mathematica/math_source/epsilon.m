(* 次のポイントフェーズに移行する時刻を求める *)
calculateNextPointPhaseTimeTest[maxTime_, discCauses_] :=
  calculateNextPointPhaseTimeTest[maxTime, discCauses, constraint, initConstraint, pConstraint, timeVariables];

publicMethod[
             calculateNextPointPhaseTimeTest,
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
                    debugPrint["# In EPSILON MODE # : constraint ", cons];
                    debugPrint["# In EPSILON MODE # : cauzeAndIDs ", causeAndIDs];

                    tStore = Map[(Rule@@#)&, createDifferentiatedEquations[vars, applyList[cons] ] ];
                    timeAppliedCauses = causeAndIDs /. tStore;
                    simplePrint[timeAppliedCauses];

                    parameterList = getParameters[timeAppliedCauses];

                    (* 必要なpConsだけを選ぶ．不要なものが入っているとMinimzeの動作がおかしくなる？ *)

                    necessaryPCons = LogicalExpand[pCons] /. (expr_ /; (( Head[expr] === Equal || Head[expr] === LessEqual || Head[expr] === Less|| Head[expr] === GreaterEqual || Head[expr] === Greater) && (!hasSymbol[expr, parameterList])) -> True);

                    debugPrint["# In EPSILON MODE # : necessaryPCons is ",necessaryPCons];

                    resultList = calculateMinTimeListTest[timeAppliedCauses, necessaryPCons, maxTime];

                    (* 整形して結果を返す *)
                    resultList = Map[({#[[1]], #[[2]], LogicalExpand[#[[3]] ]})&, resultList];
                    resultList = Fold[(Join[#1, If[Head[#2[[3]]]===Or, divideDisjunction[#2], {#2}]])&,{}, resultList];
                    resultList = Map[({#[[1]], #[[2]], Cases[applyList[#[[3]] ], Except[True]]})&, resultList];
                    resultList = Map[
                                     ({timeAndIDsToReturn[#[[1]] ], Map[(timeAndIDsToReturn[#])&, #[[2]] ], convertExprs[adjustExprs[#[[3]], isParameter ] ] })&, resultList];

                    debugPrint["# In EPSILON MODE # : resultList ", resultList];
                    resultList
                    ]
             ];

(* 最小時刻と条件の組をリストアップする関数 *)
calculateMinTimeListTest[causeAndIDList_, condition_, maxT_] := (
Block[
  {findResult, i},
  (* -1 is regarded as the id of time limit *)
  timeCaseList = {{timeAndIDs[maxT, ids[-1]], {}, condition}};
  For[i = 1, i <= Length[causeAndIDList], i++,
      findResult = findMinTimeTest[causeAndIDList[[i]], condition];
      timeCaseList = compareMinTimeListTest[timeCaseList, findResult]
      ];
  debugPrint["# In EPSILON MODE # : timeCaseList is ",timeCaseList];
  timeCaseList
      ]
);

(* 条件を満たす最小の時刻と，その条件の組を求める *)
findMinTimeTest[causeAndID_, condition_] :=
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
         debugPrint["# In EPSILON MODE # : Reals", Reals];
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
compareMinTimeListTest[list1_, list2_] := ( Block[
  {resultList, i, j},
  If[list2 === {}, Return[list1] ];
  resultList = {};
  For[i = 1, i <= Length[list1], i++,
      For[j = 1, j <= Length[list2], j++,
          resultList = Join[resultList, compareMinTimeTest[list1[[i]], list2[[j]] ] ]
          ]
      ];
  resultList
  ]
);

(* TODO: 場合分けをしていくだけで、併合はしないので最終的に冗長な場合分けが発生する可能性がある。 *)
(* ２つの時刻と条件の組を比較し，最小時刻とその条件の組のリストを返す *)
compareMinTimeTest[timeCond1_, timeCond2_] := ( Block[
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


(* 式が0以上かどうか *)
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
             toReturnForm[Limit[arg, p[eps, 0, 1] -> 0]]
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
                    direplus = Limit[arg, p[eps, 0, 1] -> 0,Direction->-1];
                    direminus = Limit[arg, p[eps, 0, 1] -> 0,Direction->1];
                    flag = FullSimplify[direplus - direminus];
                    If[flag === 0,
                       ret = 1,
                       ret = 0
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
                    debugPrint["diffEpsilonP arg",arg];
                    tmp = arg /. p[eps, 0, 1] -> 0;
                    ret = Reduce[arg - tmp];
                    toReturnForm[ret]
                    ]
             ];
