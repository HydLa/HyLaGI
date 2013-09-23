makeIntervalRulesList[pcons_] := makeIntervalRulesList[pcons] = 
Module[
  {rules, iter, plist, pc, appearedp = {}, ret = {} var},
  plist = applyList[LogicalExpand[pcons] ];
  plist = adjustExprs[plist, isParameter];
  simplePrint[plist];
  For[iter = 1, iter <= Length[plist], iter++,
    pc = plist[[iter]];
    var = pc[[1]];
    
    If[Head[rules[var] ] =!= Rule,
      appearedp = Append[appearedp, var]
    ];
    
    If[ MemberQ[{Less, LessEqual}, Head[ pc ] ],
      If[Head[rules[var] ] === Rule,
        rules[var] = var -> Interval[{rules[var][[2]][[1]][[1]], pc[[2]] } ],
        rules[var] = var -> Interval[{-Infinity, pc[[2]]}]
      ]
    ];
    If[ MemberQ[{Greater, GreaterEqual}, Head[ pc ] ],
      If[Head[rules[var] ] === Rule,
        rules[var] = var -> Interval[{pc[[2]], rules[var][[2]][[1]][[2]] }],
        rules[var] = var -> Interval[{pc[[2]], Infinity}]
      ]
    ];
    If[ Head[ pc ]=== Equal,
      rules[var] = var -> Interval[pc[[2]] ]
    ];
  ];
  For[iter = 1, iter <= Length[appearedp], iter++,
    ret = Append[ret, rules[appearedp[[iter]] ] ]
  ];
  ret
];

approxValue[val_] := approxValue[val, pConstraint, approxMode, approxPrecision, approxThreshold];


approxValue[val_, mode_] := approxValue[val, pConstraint, mode, approxPrecision, 0];

(*
 * approx given value
 * approxMode === none: do nothing
 * approxMode === numeric: numeric->numeric，interval->interval (invalid for expressions with parameters)
 * approxMode === interval: numeric->interval，interval->interval
 *)
publicMethod[
  approxValue,
  val, pcons, mode, precision, threshold,
  Module[
    {lb, ub, itv},
    If[mode === none || LeafCount[val] <= 2*threshold || hasVariable[val],
      {0},
      If[mode === numeric,
        If[Length[val] == 1,
          {1, integerString[approxExpr[precision, val[[1]] ] ] },
          {1, integerString[approxExpr[precision, val[[1]] ] ], integerString[approxExpr[precision, val[[2]] ] ] }
        ],
        (* if approxMode === interval *)
        If[Length[val] == 1,
          (* make interval from exact value *)
          itv = getInterval[val[[1]], pcons, precision];
          simplePrint[itv];
          itv = integerString[{itv[[1]][[1]], itv[[1]][[2]]}];
          simplePrint[itv];
          Join[{1}, itv],
          (* make interval from interval *)
          lb = getInterval[val[[1]], pcons, precision];
          ub = getInterval[val[[2]], pcons, precision];
          simplePrint[lb, ub];
          Join[{1}, integerString[{Min[lb[[1]][[1]], ub[[1]][[1]] ], Max[lb[[1]][[2]], ub[[1]][[2]] ]}] ]
        ]
      ]
    ]
  ]
];

getInterval[expr_, pcons_, precision_] := Module[
  {tmp},
  tmp = If[pcons =!= True, expr /. makeIntervalRulesList[pcons], expr ];
  If[Head[tmp] =!= Interval, tmp = Interval[{tmp, tmp}] ];
  tmp = N[tmp, 10];
  tmp = Rationalize[tmp, 0];
  tmp
];


approxExpr[precision_, expr_] := (
  Rationalize[
    N[Simplify[expr], precision + 3],
    Divide[1, Power[10, precision] ]
  ]
);


linearApprox[val_, precision_] := linearApprox[val, pConstraint, precision];

(*
 * linear approximation
 *)

publicMethod[
  linearApprox,
  val, pcons, precision,
  Module[
    {res}, 
    res = primaryTaylorExpansion[val, pcons, precision];
    res = If[res[[2]] == 0, {integerString[res[[1]] ]},  {integerString[res[[1]] ], integerString[res[[2]] ], integerString[res[[3]] ]} ];
    res 
  ]
];

(* @return {linear approximated value, lb of interval, ub of interval} *)
primaryTaylorExpansion[expr_, pcons_, precision_] := Module[
  {i, tmp, pars, par, pRules, zeroRules, linear, coef, itv = 0},
  pRules = If[pcons =!= True, makeIntervalRulesList[pcons], {} ];
  pars = Union[getParameters[pRules]];
  simplePrint[pRules];
  (* calculate f(0) *)
  zeroRules = Map[(#[[1]] -> 0)&, pRules];
  simplePrint[zeroRules];
  linear = expr /. zeroRules;
  simplePrint[linear];
  For[i = 1, i <= Length[pars], i++,
    par = pars[[i]];
    coef = D[expr, par ];
    simplePrint[coef];
    coef = coef /. zeroRules;
    coef = Rationalize[N[coef, precision], 0];
    itv = itv + (Max[coef] - Min[coef])/2;
    coef = (Max[coef] + Min[coef])/2;
    simplePrint[itv];
    linear = linear + coef * par
  ];
  linear = linear + coef * par;
  simplePrint[linear, itv];
  {linear, -itv, itv}
];
