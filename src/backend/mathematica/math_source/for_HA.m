(*
 * v1(now) in v2(past) => true
 *)

publicMethod[checkIncludeBound, v1, v2, pConstraintPast, pConstraintNow,
  Module[{minPast, maxPast, minNow, maxNow, tmp, reduceExprPast, 
    reduceExprNow},
   minPast = 
    Quiet[Minimize[{v2, And@@pConstraintPast && t > 0}, 
      If[Union[getParameters[v2 && pConstraintPast]] =!= {}, 
       Union[getParameters[v2 && pConstraintPast]], {tmp}], 
      Reals], {Minimize::wksol, Minimize::infeas}];
   simplePrint[minPast];
   maxPast = 
    Quiet[Maximize[{v2, And@@pConstraintPast && t > 0}, 
      If[Union[getParameters[v2 && pConstraintPast]] =!= {}, 
       Union[getParameters[v2 && pConstraintPast]], {tmp}], 
      Reals], {Maximize::wksol, Maximize::infeas}];
   simplePrint[maxPast];
   minNow = 
    Quiet[Minimize[{v1, And@@pConstraintNow && t > 0}, 
      If[Union[getParameters[v1 && pConstraintNow]] =!= {}, 
       Union[getParameters[v1 && pConstraintNow]], {tmp}], 
      Reals], {Minimize::wksol, Minimize::infeas}];
   simplePrint[minNow];
   maxNow = 
    Quiet[Maximize[{v1, And@@pConstraintNow && t > 0}, 
      If[Union[getParameters[v1 && pConstraintNow]] =!= {}, 
       Union[getParameters[v1 && pConstraintNow]], {tmp}], 
      Reals], {Maximize::wksol, Maximize::infeas}];
   simplePrint[maxNow];
   (* not (minPast < tmp < maxPast) && minNow < tmp < 
   maxNow (=はcheckIncludeの戻り値で判断) *)
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
    False]]
];

publicMethod[checkInclude, includeBound, 
 Module[{flg, flgInclude, res, tmpT0}, flg = True; flgInclude = True;
   If[Cases[includeBound[[1]], {_, _}] === {}, res = {includeBound[[1]], 1};,
     For[i = 1, i <= Length[includeBound[[1, 1]]], i = i + 1,
       If[Reduce[Not[includeBound[[1, 1, i, 2]]] && t == 0] === False,
         (* t==0 の場合があったら開区間、なかったら閉区間 （あやしい） *)
         flgInclude = False;
       ];
       If[Reduce[Not[includeBound[[1, 1, i, 2]]] && t > 0] === False,
         flg = False;
         res = {includeBound[[1, 1, i, 1]], 0};
       ];
     ](* For *);
     If[flg === True,
       If[flgInclude === True,
         res = {includeBound[[1, 2]], 1};,
         res = {includeBound[[1, 2]], 0};
       ];
     ];
   ](* If *);
   Limit[res, t -> 0, Direction -> -1]
 ](* Module *)
];


publicMethod[
  substituteParameterCondition,
  expr, pCond,
  Module[
    {substitutedVal, pRules},
    pRules = Map[(#[[1]] -> #[[2]])&, pCond];
    substitutedVal = Simplify[expr /. pRules];
    If[Element[substitutedVal, Reals] =!= False,
      toReturnForm[substitutedVal],
      Message[substitute_value::nrls, substitutedVal]
    ]
  ]
];



publicMethod[
  SubstituteValue,
  expr, value,
  Module[
    {appliedExpr},
    appliedExpr = Reduce[{expr, value}, expr[[1]], Reals];
    simplePrint[appliedExpr];
    appliedExpr = appliedExpr[[Length[appliedExpr],2]];    
    If[Element[appliedExpr, Reals] =!= False,
      toReturnForm[appliedExpr],
      Message[substitute_value::nrls, appliedExpr]
    ]
  ]
];

publicMethod[
  SubstituteTime,
  expr, value,
  Module[
    {appliedExpr},
    appliedExpr = Reduce[{t == expr, t>=0, value}, t, Reals];
    simplePrint[appliedExpr];
    appliedExpr = appliedExpr[[Length[appliedExpr],2]];    
    If[Element[appliedExpr, Reals] =!= False,
      integerString[appliedExpr],
      Message[substitute_value::nrls, appliedExpr]
    ]
  ]
];

publicMethod[
  timeAdd,
  expr, time,
  integerString[expr + time]
];

(* パラメタの抽象化を行う関数 *)
publicMethod[
  abstractCP,
  cons,
  Module[
    {ret=True, param, tmpCons, listCons},
    If[
      ToString[Head[cons] ] == "List",
      listCons = cons,
      listCons = {cons}
    ];
    For[i = 1, i <= Length[listCons[[1]]], i++,
      tmpCons = listCons[[1]][[i]];
      param = getParameters[tmpCons][[1]];
      debugPrint["param, ", param];
      debugPrint["tmpCons[[1]]", tmpCons[[1]]];
      If[ToString[Head[tmpCons] ] == "Greater" || ToString[Head[tmpCons] ] == "GreaterEqual",
        If[
          ToString[param] == ToString[tmpCons[[1]]],
          tmpCons[[2]] = tmpCons[[2]] - 1,
          tmpCons[[1]] = tmpCons[[1]] + 1
        ],
        (* case [Head[cons[[i]]]  == Less || Head[cons[[i]]] == LessEqual *)
        If[
          ToString[param] == ToString[tmpCons[[1]]],
          tmpCons[[2]] = tmpCons[[2]] + 1,
          tmpCons[[1]] = tmpCons[[1]] - 1
        ]
      ];
      ret = ret && tmpCons
    ];
    ret
  ]
]

(* C++ から呼び出される, 領域同士の重なりのスコアを算出する関数 *)
publicMethod[
  calculateInclusionScore, largeTime, largeVm, largePm, smallTime, smallVm, smallPm,
  Module[
    {ret = True,score = 0.0, i,tmp,tmpLargeTime,tmpLargeVm,tmpSmallPar,tmpLargePar,allExpr,listLarge,listSmall},

    tmpLargeVm = largeVm /. p -> pL;
    tmpLargeTime = largeTime /. p -> pL;
    (* compare variable num *)
    allExpr = True;
    For[i = 1, i <= Length[tmpLargeVm], i++,
        (* compare variable name *)
        If[tmpLargeVm[[i]][[1]] != smallVm[[i]][[1]], Return[score];];
        
        (* correct expr : large.2 == small.2 *)
        tmp = Simplify[tmpLargeVm[[i]][[2]] /. t -> tmpLargeTime] == Simplify[smallVm[[i]][[2]] /. t -> smallTime];
        allExpr = And[allExpr,Simplify[tmpLargeVm[[i]][[2]] /. t -> tmpLargeTime] == Simplify[smallVm[[i]][[2]] /. t -> smallTime] ];
    ];
    allExpr = Simplify[allExpr];
    If[allExpr === False, Return[score];]; (* ここは絶対値の大きい負の値を返して, 抽象化する意味もないことを伝える *)
    tmpLargePm = largePm /. p -> pL;
    tmpSmallPm = smallPm;
    listLarge = Union[getParameters[largePm], getParameters[largeVm] ] /. p -> pL;
    listSmall = Union[getParameters[smallPm], getParameters[smallVm] ];
    
    gravPlots = 900;
    largeParamMin = 
      If[
        Length[listLarge] == 2,
        {MinValue[{listLarge[[1]], tmpLargePm}, listLarge], MinValue[{listLarge[[2]], tmpLargePm}, listLarge]},
        {MinValue[{listLarge[[1]], tmpLargePm}, listLarge]}
      ];
    largeParamMax = 
      If[
        Length[listLarge] == 2,
        {MaxValue[{listLarge[[1]], tmpLargePm}, listLarge], MaxValue[{listLarge[[2]], tmpLargePm}, listLarge]},
        {MaxValue[{listLarge[[1]], tmpLargePm}, listLarge]}
      ];
    smallParamMin = 
      If[
        Length[listSmall] == 2,
        {MinValue[{listSmall[[1]], tmpSmallPm}, listSmall], MinValue[{listSmall[[2]], tmpSmallPm}, listSmall]},
        {MinValue[{listSmall[[1]], tmpSmallPm}, listSmall]}
      ];
    smallParamMax = 
      If[
        Length[listSmall] == 2,
        {MaxValue[{listSmall[[1]], tmpSmallPm}, listSmall], MaxValue[{listSmall[[2]], tmpSmallPm}, listSmall]},
        {MaxValue[{listSmall[[1]], tmpSmallPm}, listSmall]}
      ];
    {gTime, garbage} = AbsoluteTiming[
        {gLarge, rLarge} = calcCenterOfGravity[gravPlots, tmpLargeVm, listLarge, largeParamMin, largeParamMax, largeTime];
    ];
    debugPrint["calculate gS and rS time with ", gravPlots, " plots: ", gTime, "[s]"];

    (* {gLarge, rLarge} = calculateGfromNPoints[900]; 重心と半径は 500 個のプロットから求めたものとする *)

    samplePlots = 900;
    {scoreTime2, garbage} = AbsoluteTiming[
        ans = calcNumOfNearPlots[samplePlots, smallVm, listSmall, smallParamMin, smallParamMax, gLarge, rLarge, smallTime];
    ];
    debugPrint["T's ", ans, "/", samplePlots, " (", N[100*ans/samplePlots], "[%]) plot is near to S, time: ", scoreTime2];


    ret = Reduce[ForAll[Evaluate[listSmall],tmpSmallPm,Exists[Evaluate[listLarge],tmpLargePm,allExpr] ],Reals];
    If[ret == True, score = 1.0];
    simplePrint[score];
    score
  ]
];


dist[p1_][p2_] := Module[
    {i, acc = 0},
    For[
        i = 1, i <= Length[p1], i++,
        acc += (p1[[i]] - p2[[i]])^2
    ];
    Sqrt[N[acc] ]
]

(* 領域 S の重心と半径を指定したサンプル数から求める *)
calcCenterOfGravity[num_, vm_, paramNameList_, paramMinValList_, paramMaxValList_, phaseStartTime_] := Module[
    {
        grav = {}, rad = 0, rMax = 0, rMin = Infinity, plot, plotList = {}, valP1, valP2,
        i, varIdx, plotIdx
    },
    For[
        i = 1, i <= Length[vm], i++,
        grav = Append[grav, 0];
    ];
    debugPrint["vm: ", vm];
    debugPrint["paramNameList: ", paramNameList];
    debugPrint["paramMinValList: ", paramMinValList];
    debugPrint["paramMaxValList: ", paramMaxValList];
    debugPrint["phaseStartTime: ", phaseStartTime];
    If[
        Length[paramNameList] == 2,
        (* パラメタが二個のケース *)
        For[
            valP1 = paramMinValList[[1]], valP1 <= paramMaxValList[[1]], valP1 += (paramMaxValList[[1]] - paramMinValList[[1]])/(Sqrt[num] - 1),
            For[
                valP2 = paramMinValList[[2]], valP2 <= paramMaxValList[[2]], valP2 += (paramMaxValList[[2]] - paramMinValList[[2]])/(Sqrt[num] - 1),
                plot = {};
                For[
                    varIdx = 1, varIdx <= Length[vm], varIdx++,
                    plot = Append[plot, N[vm[[varIdx]][[2]] /. {paramNameList[[1]] -> valP1, paramNameList[[2]] -> valP2, t -> phaseStartTime}] ];
                ];
                grav += N[plot/num];
                plotList = Append[plotList, plot];
            ];
        ];
        For[
            plotIdx = 1, plotIdx <= Length[plotList], plotIdx++,
            rMax = Max[rMax, dist[plotList[[plotIdx]]][grav] ];
            rMin = Min[rMin, dist[plotList[[plotIdx]]][grav] ];
        ],
        (* パラメタが一個のケース *)
        For[
            valP1 = paramMinValList[[1]], valP1 <= paramMaxValList[[1]], valP1 += (paramMaxValList[[1]] - paramMinValList[[1]])/(num - 1),
            plot = {};
            For[
                varIdx = 1, varIdx <= Length[vm], varIdx++,
                plot = Append[plot, N[vm[[varIdx]][[2]] /. {paramNameList[[1]] -> valP1, t -> phaseStartTime}] ];
            ];
            grav += N[plot/num];
            plotList = Append[plotList, plot];
        ];
        For[
            i = 1, i <= Length[plotList], i++,
            rMax = Max[rMax, dist[grav][plotList[[i]]] ];
            rMin = Min[rMin, dist[grav][plotList[[i]]] ];
        ];
    ];
    debugPrint["rMax: ", rMax, " rMin: ", rMin];
    rad = rMax;
    (* rad = (9*rMax + rMin)/10; *)
    (* rad = (7.5*rMax + 2*rMin)/10; *)
    (* rad = (6*rMax + 4*rMin)/10; *)
    (* rad = (2.5*rMax + 7.5*rMin)/10; *)
    (* rad = rMin; *)
    {grav, rad}
];

(* 領域 T からサンプリングした点で, 領域 S の重心からその半径以上離れているものはいくつあるか *)
calcNumOfNearPlots[num_, vm_, paramNameList_, paramMinValList_, paramMaxValList_, grav_, rad_, phaseStartTime_] := Module[
    {
        ans = 0, valP1, valP2, plot,
        i, varIdx
    },
    If[
        Length[paramNameList] == 2,
        (* パラメタが二個のケース *)
        For[
            valP1 = paramMinValList[[1]], valP1 <= paramMaxValList[[1]], valP1 += (paramMaxValList[[1]] - paramMinValList[[1]])/(Sqrt[num] - 1),
            For[
                valP2 = paramMinValList[[2]], valP2 <= paramMaxValList[[2]], valP2 += (paramMaxValList[[2]] - paramMinValList[[2]])/(Sqrt[num] - 1),
                plot = {};
                For[
                    varIdx = 1, varIdx <= Length[vm], varIdx++,
                    plot = Append[plot, N[vm[[varIdx]][[2]] /. {paramNameList[[1]] -> valP1, paramNameList[[2]] -> valP2, t -> phaseStartTime}] ];
                ];
                If[
                    dist[plot][grav] <= rad,
                    ans += 1
                ];
            ];
        ],
        (* パラメタが一個のケース *)
        For[
            valP1 = paramMinValList[[1]], valP1 <= paramMaxValList[[1]], valP1 += (paramMaxValList[[1]] - paramMinValList[[1]])/(num - 1),
            plot = {};
            For[
                varIdx = 1, varIdx <= Length[vm], varIdx++,
                plot = Append[plot, N[vm[[varIdx]][[2]] /. {paramNameList[[1]] -> valP1, t -> phaseStartTime}] ];
                If[
                    dist[plot][grav] <= rad,
                    ans += 1
                ];
            ];
        ]
    ];
    ans
];