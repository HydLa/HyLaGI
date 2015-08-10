(* phaseの包含を判定する *)
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
                        (* debugPrint["# adding expr left : ",Simplify[tmpLargeVm[[i]][[2]] /. t -> tmpLargeTime]];
                         debugPrint["# adding expr right : ",Simplify[tmpSmallVm[[i]][[2]] /. t -> tmpSmallTime]];
                         debugPrint["# adding expr : ",Simplify[tmpLargeVm[[i]][[2]] /. t -> tmpLargeTime] == Simplify[tmpSmallVm[[i]][[2]] /. t -> tmpSmallTime]]; *)
                        If[i == 1,
                           allExpr = Simplify[tmpLargeVm[[i]][[2]] /. t -> tmpLargeTime] == Simplify[tmpSmallVm[[i]][[2]] /. t -> tmpSmallTime],
                           allExpr = And[allExpr,Simplify[tmpLargeVm[[i]][[2]] /. t -> tmpLargeTime] == Simplify[tmpSmallVm[[i]][[2]] /. t -> tmpSmallTime]]
                           ];
                        ];
                    allExpr = Simplify[allExpr];
                    debugPrint["# all Expr from vm", allExpr];
                    If[allExpr === False,
                       ret = False];
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


(* check whether the pCons includes case where the p[peps, 0, 1] = 0 *)
publicMethod[
  unsuitableCase,
  pCons,
  Reduce[pCons /. {Less -> LessEqual, Greater->GreaterEqual} /. p[peps, 0, 1] -> 0, Reals] === False
]


(* n(dcount)次近似をする *)
publicMethod[
             cutHighOrderVariable,
             expr, var, dcount,
             Module[
                    {sTmp,sCond,dTimes,factorial,dExpr,ret},
                    (* debugPrint["#epsilon cut high order input",expr]; *)
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
                    (* debugPrint["#epsilon cut high order return",ret]; *)
                    toReturnForm[ret]
                    ]
             ];

publicMethod[
             limitIsZero,
             expr,
             Module[
                    {ret},
                    ret = Limit[expr, p[peps, 0, 1] -> 0] === 0;
                    debugPrint["#epsilon checking zero expr = ",expr, " : ",ret];
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
                    one = Quiet[Check[arg /. p[peps, 0, 1] -> 0, False, {Power::infy, Power::indet}]];
                    two = Quiet[Check[D[arg, p[peps, 0, 1]] /. p[peps, 0, 1] -> 0, False, {Power::infy, Power::indet}]];
                    If[one =!= False && two =!= False, ret = one + two * p[peps, 0, 1], ret = arg];
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
                       direplus = Limit[arg, p[peps, 0, 1] -> 0,Direction->-1];
                       direminus = Limit[arg, p[peps, 0, 1] -> 0,Direction->1];
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
             toReturnForm[Limit[arg, p[peps, 0, 1] -> 0,Direction->-1]]
             ];

publicMethod[
             limitEpsilonM,
             arg,
             debugPrint["limitEpsilonM arg",arg];
             toReturnForm[Limit[arg, p[peps, 0, 1] -> 0,Direction->1]]
             ];

publicMethod[
             diffEpsilon,
             arg,
             Module[
                    {tmp,ret},
                    debugPrint["json diffEpsilonP arg",arg];
                    tmp = Quiet[Check[arg /. p[peps, 0, 1] -> 0, False, {Power::infy, Power::indet}]];
                    (* tmp = arg /. p[peps, 0, 1] -> 0; *)
                    If[tmp =!= False, ret = Simplify[arg - tmp], ret = arg];
                    toReturnForm[ret]
                    ]
             ];
