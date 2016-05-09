(* calculate the value of given variable which satisfies given equation *)
publicMethod[
  calculateConsistentValue,
  equation, variable, variableMap, parameterCondition,
  Module[
    {rules, sol, originalValue, range, originalRange, intervalRules},
    (* ignore entry of variableMap for given variable *)
    rules = Select[variableMap, (#[[1]] =!= variable)&];
    rules = Map[(Rule@@#)&, rules];
    (* get entry of variableMap for given variable *)
    originalValue = Select[variableMap, (#[[1]] === variable)&];
    originalValue = originalValue[[1, 2]];
    simplePrint[rules, originalValue];

    intervalRules = createIntervalRules[parameterCondition];
  
    sol = Solve[equation //. rules, {variable}];
    simplePrint[sol];
    originalRange = N[originalValue /. intervalRules];
    simplePrint[originalRange];
    For[j = 1, j <= Length[sol], j++,
      Print["interval:", N[sol[[j, 1, 2]] /. intervalRules]];
    ];
    sol = Select[sol, (IntervalIntersection[N[#[[1, 2]] /. intervalRules], originalRange] =!= Interval[])&];
    simplePrint[sol];
    If[Length[sol] == 1,
      toReturnForm[sol[[1, 1, 2]] ],
      Exception
    ]
  ]
];

(* create rules to replace parameters with intervals of their ranges *)
createIntervalRules[parameterCondition_] := 
Module[
  {lbs, ubs, parameters, i, condition, parameter, rules = {}, adjustedCond},
  simplePrint[parameterCondition];
  parameters = getParameters[parameterCondition];
  adjustedCond = adjustExprs[parameterCondition, isParameter];
  For[i = 1, i <= Length[adjustedCond], i++,
    condition = adjustedCond[[i]];
    parameter = condition[[1]];
    If[Head[condition] === Less || Head[condition] === LessEqual, ubs[parameter] = condition[[2]] ];
    If[Head[condition] === Greater || Head[condition] === GreaterEqual, lbs[parameter] = condition[[2]] ];
    If[Head[condition] === Equal, ubs[parameter] = lbs[parameter] = condition[[2]] ]
  ];
  For[i = 1, i <= Length[parameters], i++,
    parameter = parameters[[i]];
    rules = Append[rules, parameter -> Interval[{lbs[parameter], ubs[parameter]}]]
  ];
  rules
];


publicMethod[
  intervalToMidpointRadius,
  lb, ub,
  toReturnForm[midpointRadius[toRational[Simplify[(lb + ub)/2]], toRational[Simplify[(ub - lb) / 2] ] ] ]
];

publicMethod[
  transformToRational,
  float,
  toReturnForm[toRational[float]]
];

publicMethod[
  numerize,
  expr,
  toReturnForm[N[expr]]
];

mid[itv_] := (itv[[1, 1]] + itv[[1, 2]])/2;

(* convert given f[X] to F[X] = f[mid[X]] + ∂f/∂x[*(X-mid[X]) *)
Linearize[f_, pm_, tMin_, tMax_] :=
Module[
  {dtf, dxf, pRules, pars, result, midRules, tMid = (tMin + tMax)/2, i, additionalCons, itv, itvPar},
  pRules = Append[createIntervalRules[LogicalExpand[pm]], t -> Interval[{tMin, tMax}] ];
  pars = Append[getParameters[pm], t];
  simplePrint[pRules, pars];
  midRules = Map[(#[[1]] -> mid[#[[2]]])&, pRules];
  simplePrint[midRules];
  itv = toRational[N[Interval[f /. midRules] ] ];
  result = midT;
  additionalCons = pm && tMin <= t <= tMax && itv[[1, 1]] <= midT <= itv[[1, 2]];
  pRules = Append[pRules, midT -> Interval[{itv[[1, 1]], itv[[1, 2]]}] ];
  simplePrint[result];
  For[i = 1, i <= Length[pars], i++,
    itvPar = If[pars[[i]] === t, itvPt, itvP[pars[[i]][[1]], pars[[i]][[2]], pars[[i]][[3]] ] ];
    itv = toRational[N[D[f, pars[[i]] ] /. pRules] ];
    additionalCons = additionalCons && (itv[[1, 1]] <= itvPar <= itv[[1, 2]]);
    pRules = Append[pRules, itvPar -> Interval[{itv[[1, 1]], itv[[1, 2]]}]];
    result = result + itvPar * (pars[[i]] - (pars[[i]] /. midRules));
    simplePrint[result];
  ];
  {result, additionalCons, pRules}
];
  