(* calculate the value of given variable which satisfies given equation *)
publicMethod[
  calculateConsistentValue,
  equation, variable, variableMap, parameterMap,
  Module[
    {rules, sol, originalValue},
    (* ignore entry of variableMap for given variable *)
    rules = Select[variableMap, (#[[1]] =!= variable)&];
    rules = Map[(Rule@@#)&, rules];
    (* get entry of variableMap for given variable *)
    originalValue = Select[variableMap, (#[[1]] === variable)&];
    originalValue = originalValue[[1]][[2]];
    simplePrint[rules, originalValue];

    sol = Solve[equation //. rules, {variable}];
    simplePrint[sol];
    sol = Select[sol, (Reduce[variable == #[[1]][[2]] && variable == originalValue && And@@parameterMap] =!= False)&];    
    simplePrint[sol];
    toReturnForm[sol[[1]][[1]][[2]] ]
  ]
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