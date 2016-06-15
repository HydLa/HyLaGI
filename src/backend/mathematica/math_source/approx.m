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
  If[parameterCondition === True || prameterCondition === False, Return[{}]];
  parameters = getParameters[parameterCondition];
  adjustedCond = adjustExprs[LogicalExpand[parameterCondition], isParameter];
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

mid[itv_Interval] := (itv[[1, 1]] + itv[[1, 2]])/2;
wid[itv_Interval] := (itv[[1, 2]] - itv[[1, 1]]);
inf[itv_Interval] := itv[[1,1]];
sup[itv_Interval] := itv[[1,2]];


Linearize[f_, pm_, tMin_, tMax_] :=
Module[
  {dtf, dxf, pRules, pars, result, midRules, tMid = (tMin + tMax)/2, i, additionalCons, itv, itvPar, resItv},
  pRules = Append[createIntervalRules[pm], t -> Interval[{tMin, tMax}] ];
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
    resItv = N[result /. pRules];
    simplePrint[resItv];
    simplePrint[wid[resItv] ];
  ];
  {result, additionalCons, pRules}
];


calculateTLinear[f_, pm_, tMin_Real, tMax_Real] := calculateTLinear[f, pm, toRational[tMin], toRational[tMax]];

publicMethod[
  calculateTLinear,
  f, pm, tMin, tMax,
  Module[
    {pRules, pars, result = 0, midRules, i, itv, iRem = 0, iMid = (tMin + tMax) / 2, frt, par, remMid, remWid, remItv},
    pRules = Append[createIntervalRules[pm], t -> Interval[{tMin, tMax}]];
    pars = getParameters[pm];
    midRules = Map[(#[[1]] -> mid[#[[2]]])&, pRules];
    frt = D[f, t ] /. pRules;
    For[i = 1, i <= Length[pars], i++,
      par = pars[[i]];
      itv = toRational[N[D[f, pars[[i]] ] / frt /. pRules] ]; (* f∂xi/f∂t *)
      iRem += wid[itv];
      result += -mid[itv] * par;
    ];
    iRem = iMid + iRem/2 * Interval[{-1, 1}] - (f /. midRules)/frt;
    remMid = toRational[N[Interval[mid[iRem] ] ] ];
    remItv = N[iRem - remMid];
    remWid = toRational[Max[Abs[inf[remItv]], Abs[sup[remItv] ] ] ];
    remMid = mid[remMid];

    {toReturnForm[result], toReturnForm[midpointRadius[remMid, remWid] ] }
  ]
];
