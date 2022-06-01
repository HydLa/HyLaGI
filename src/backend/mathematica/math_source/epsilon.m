(* ガード条件を満たしているか判定する *)
publicMethod[checkEdgeGuard, guard, relatedVm, relatedPm,
  Module[{ret,i,tmp,newCondition},
    tmp = guard;
    newCondition = relatedPm;
    For[i=1, i<=Length[relatedVm], i++,
      debugPrint["#check edge guard : relatedVm ", i ," : ", relatedVm[[i]]];
      If[relatedVm[[i]][[0]] === Equal,
        tmp = tmp /. relatedVm[[i]][[1]] -> relatedVm[[i]][[2]];,
        newCondition = And[newCondition, relatedVm[[i]]];
      ];
    ];
    debugPrint["#check edge guard : checking guard : ", tmp];
    debugPrint["#check edge guard : conditions : ", newCondition];
    ret = Quiet[timeConstrainedSimplify[tmp,newCondition] ];
    debugPrint["#check edge guard : ret : ",ret];
    TrueQ[ret]
  ]
];

publicMethod[checkEdgeGuardWt, guard, relatedVm, relatedPm, startTime, endTime,
  Module[{ret,i,tmp,newCondition},
    tmp = guard;
    For[i=1, i<=Length[relatedVm], i++,
      debugPrint["#check edge guard : relatedVm ", i ," : ", relatedVm[[i]]];
      tmp = tmp /. relatedVm[[i]][[1]] -> relatedVm[[i]][[2]];
    ];
    debugPrint["#check edge guard : checking guard : ", tmp];
    newCondition = And[relatedPm, t > startTime && t <= endTime];
    debugPrint["#check edge guard : conditions : ", newCondition];
    ret = Quiet[timeConstrainedSimplify[tmp,newCondition] ];
    debugPrint["#check edge guard : ret : ",ret];
    TrueQ[ret]
  ]
];

(* phaseの包含を判定する *)
publicMethod[checkInclude, largeTime, largeVm, largePm, smallTime, smallVm, smallPm,
  Module[{ret,i,tmp,tmpLargeTime,tmpLargeVm,tmpSmallPar,tmpLargePar,allExpr,listLarge,listSmall},
    ret = True;
    tmpLargeVm = largeVm /. p -> pL;
    tmpLargeTime = largeTime /. p -> pL;
    (* compare variable num *)
    allExpr = True;
    For[i = 1, i <= Length[tmpLargeVm], i++,
      (* compare variable name *)
      If[tmpLargeVm[[i]][[1]] != smallVm[[i]][[1]], Return[False];];
      (* correct expr : large.2 == small.2 *)
      tmp = Simplify[tmpLargeVm[[i]][[2]] /. t -> tmpLargeTime] == Simplify[smallVm[[i]][[2]] /. t -> smallTime];
      simplePrint[tmp];
      (* IP では開始時刻のみ見るので, 時刻 t は tmpLargeTime や smallTime で置き換える *)
      allExpr = And[allExpr,Simplify[tmpLargeVm[[i]][[2]] /. t -> tmpLargeTime] == Simplify[smallVm[[i]][[2]] /. t -> smallTime] ];
    ];
    allExpr = Simplify[allExpr];
    simplePrint[allExpr];
    If[allExpr === False, Return[False];];
    tmpLargePm = largePm /. p -> pL;
    tmpSmallPm = smallPm;
    
    listLarge = Union[getParameters[largePm], getParameters[largeVm] ] /. p -> pL;
    listSmall = Union[getParameters[smallPm], getParameters[smallVm] ];
    simplePrint[listSmall, tmpSmallPm, listLarge, tmpLargePm, allExpr];
    ret = Reduce[
            ForAll[
              Evaluate[listSmall], tmpSmallPm, 
              Exists[Evaluate[listLarge],tmpLargePm,allExpr] 
            ],
            Reals
          ];
    simplePrint[ret];
    If[ret =!= True,ret = False];
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
             toReturnForm[Limit[arg, {p[peps, 0, 1], ueps} -> {0, 0}]]
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
                       direplus = Limit[arg, {p[peps, 0, 1], ueps} -> {0, 0}, Direction->-1];
                       direminus = Limit[arg, {p[peps, 0, 1], ueps} -> {0, 0}, Direction->1];
                       flag = timeConstrainedSimplify[direplus - direminus];
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
