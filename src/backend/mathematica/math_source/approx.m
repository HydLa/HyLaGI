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
  {intervalExpr, lbs, ubs, parameters, i, condition, parameter, rules = {} },
  simplePrint[parameterCondition];
  parameters = Union[Map[(#[[1]])&, parameterCondition] ];
  simplePrint[parameters];
  For[i = 1, i <= Length[parameterCondition], i++,
    condition = parameterCondition[[i]];
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
  toReturnForm[midpointRadius[Simplify[(lb + ub)/2], Simplify[(ub - lb) / 2] ] ]
];

publicMethod[
  transformToRational,
  float,
  toReturnForm[toRational[float]]
];

publicMethod[
  numericalOutput,
  expr,
  toReturnForm[N[Expand[expr]]]
];
