
checkConditions[] := (
  checkConditions[prevIneqs && And@@Map[(Equal@@#)&, prevRules], falseConditions, pConstraint, prevVariables ]
);

publicMethod[
  checkConditions,
  pCons, fCond, paramCons, vars,
  Module[
  {prevCons, falseCond, trueMap, falseMap, cpTrue, cpFalse, cpTmp},
   debugPrint["fcond",fCond];
   If[fCond === 1, 
    {True,False},
    prevCons = pCons;
    prevCons = prevCons /. Rule->Equal;
    If[prevCons[[0]] == List, prevCons[[0]] = And;];
    Quiet[
      cpTrue = Reduce[Exists[vars, Simplify[prevCons&&fCond&&paramCons] ], Reals], {Reduce::useq}
    ];
    simplePrint[cpTrue];
    checkMessage;
    Quiet[
      cpFalse = Reduce[paramCons && !cpTrue, Reals], {Reduce::useq}
    ];
    checkMessage;
    simplePrint[cpFalse];
      (*
    Quiet[
      cpTmp = Reduce[Exists[vars, prevCons && fCond],Reals];
      cpTrue = Reduce[cpTmp && paramCons, Reals];
      cpFalse = Reduce[!cpTmp && paramCons, Reals];
    ];
       
    falseCond = applyListToOr[LogicalExpand[fCond]];
    Quiet[
      cpTmp = ParallelMap[Reduce[Exists[vars, prevCons && #], Reals]&, falseCond], {Reduce::useq}
    ];
    If[cpTmp[[0]] == List, cpTmp[[0]] = Or;];
    cpTmp = Reduce[cpTmp,Reals];
    simplePrint[cpTmp];
    checkMessage;
    Quiet[
      cpTrue = Reduce[cpTmp && paramCons, Reals], {Reduce::useq}
    ];
    checkMessage;
    simplePrint[cpTrue];
    Quiet[
      cpFalse = Reduce[!cpTmp && paramCons, Reals], {Reduce::useq}
    ];
    checkMessage;
    simplePrint[cpFalse];
       *)
    {trueMap, falseMap} = Map[(createMap[#, isParameter, hasParameter, {}])&, {cpTrue, cpFalse}];
    simplePrint[trueMap, falseMap];
    {trueMap, falseMap}
   ]
  ]
];

(* 制約モジュールが矛盾する条件をセットする *)
setConditions[co_, va_] := Module[
  {cons, vars},
  cons = co;
  falseConditions = cons;
  simplePrint[cons, falseConditions];
];


(* 矛盾する条件を整形して返す *)
createPrevMap[cons_, vars_] := Module[
  {map},
  If[cons === True || cons === False, 
    cons,

    map = cons /. (expr_ /; (( Head[expr] === Inequality || Head[expr] === Equal || Head[expr] === LessEqual || Head[expr] === Less|| Head[expr] === GreaterEqual || Head[expr] === Greater) && (hasVariable[expr] || hasParameter[expr] || !hasPrevVariable[expr])) -> False);
    map = Reduce[map, vars, Reals];
    map = cons /. (expr_ /; (( Head[expr] === Inequality || Head[expr] === Equal || Head[expr] === LessEqual || Head[expr] === Less|| Head[expr] === GreaterEqual || Head[expr] === Greater) && (hasVariable[expr] || hasParameter[expr] || !hasPrevVariable[expr])) -> False);

    simplePrint[map];
    If[map =!= False, 
      map = LogicalExpand[map];
      map = applyListToOr[map];
      map = Map[(applyList[#])&, map];
      debugPrint["map after applyList in createMap", map];
 
      map = Map[(convertExprs[ adjustExprs[#, isPrevVariable] ])&, map];
    ];
    map
  ]
];

(* 制約モジュールが矛盾する条件を見つけるための無矛盾性判定 *)
findConditions[] := (
  findConditions[constraint && tmpConstraint && initConstraint && initTmpConstraint, variables ]
);

publicMethod[
  findConditions,
  cons, vars,
  Module[
    {cp},
    Quiet[
      cp = Reduce[Exists[vars, cons],Reals], {Reduce::useq}
    ];
    simplePrint[cp];
    checkMessage;
    If[cp =!= False && cp =!= True,
      cp = LogicalExpand[Simplify[cp] ];
5A      cp = toReturnForm[cp];
    ];
    simplePrint[cp];
    cp
  ]
];
