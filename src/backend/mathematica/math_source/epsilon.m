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

publicMethod[
             cutHighOrderVariable,
             expr, var, dcount,
             Module[
                    {sTmp,sCond,dTimes,dExpr,ret},
                    dTimes = 0;
                    dExpr = expr;
                    sTmp = Quiet[Check[dExpr /. var -> 0, False, {Power::infy, Power::indet}]];
                    sCond = sTmp =!= False;
                    If[sCond, ret = sTmp, ret = expr]
                    While[sCond && dTimes++ < dcount,
                          dExpr = D[dExpr, var];
                          sTmp = Quiet[Check[dExpr /. var -> 0, False, {Power::infy, Power::indet}]];
                          sCond = sTmp =!= False;
                          If[sCond, ret = ret + sTmp*var^dTimes, ret = expr]
                          ];
                    toReturnForm[ret]
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
