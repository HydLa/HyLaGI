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
                    {sTmp,sCond,dTime,dExpr,ret},
                    dTime = 0;
                    dExpr = expr;
                    sTmp = Quiet[Check[dExpr /. var -> 0, False, {Power::infy, Power::indet}]];
                    sCond = sTmp =!= False;
                    If[sCond, ret = sTmp, ret = expr]
                    While[sCond && dTime++ < dcount,
                          dExpr = D[dExpr, var];
                          sTmp = Quiet[Check[dExpr /. var -> 0, False, {Power::infy, Power::indet}]];
                          sCond = sTmp =!= False;
                          If[sCond, ret = ret + sTmp, ret = expr]
                          ];
                    ret
                    ]
             ];

publicMethod[
             limitEpsilon,
             arg,
             debugPrint["limitEpsilon arg",arg];
             toReturnForm[Limit[arg, p[epsilon, 0, 0] -> 0]]
             ];

publicMethod[
             reduceEpsilon,
             arg,
             Module[
                    {one,two,ret},
                    one = Quiet[Check[arg /. p[epsilon, 0, 0] -> 0, False, {Power::infy, Power::indet}]];
                    two = Quiet[Check[D[arg, p[epsilon, 0, 0]] /. p[epsilon, 0, 0] -> 0, False, {Power::infy, Power::indet}]];
                    If[one =!= False && two =!= False, ret = one + two * p[epsilon, 0, 0], ret = arg];
                    ret
                    ]
             ];

publicMethod[
             checkEpsilon,
             arg,
             Module[
                    {direplus,direminus,flag,ret},
                    debugPrint["checkEpsilon arg",arg];
                    direplus = Limit[arg, p[epsilon, 0, 0] -> 0,Direction->-1];
                    direminus = Limit[arg, p[epsilon, 0, 0] -> 0,Direction->1];
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
             toReturnForm[Limit[arg, p[epsilon, 0, 0] -> 0,Direction->-1]]
             ];

publicMethod[
             limitEpsilonM,
             arg,
             debugPrint["limitEpsilonM arg",arg];
             toReturnForm[Limit[arg, p[epsilon, 0, 0] -> 0,Direction->1]]
             ];
