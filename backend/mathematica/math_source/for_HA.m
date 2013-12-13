(*
 * v1(now) in v2(past) => true
 *)

publicMethod[checkIncludeBound, v1, v2, 
  Module[{minPast, maxPast, minNow, maxNow, tmp, reduceExprPast, 
    reduceExprNow},
   minPast = 
    Quiet[Minimize[{v2, pConstraintPast}, 
      If[Union[getParameters[v2 && pConstraintPast]] =!= {}, 
       Union[getParameters[v2 && pConstraintPast]], {tmp}], 
      Reals], {Minimize::wksol, Minimize::infeas}];
   simplePrint[minPast];
   maxPast = 
    Quiet[Maximize[{v2, pConstraintPast}, 
      If[Union[getParameters[v2 && pConstraintPast]] =!= {}, 
       Union[getParameters[v2 && pConstraintPast]], {tmp}], 
      Reals], {Maximize::wksol, Maximize::infeas}];
   simplePrint[maxPast];
   minNow = 
    Quiet[Minimize[{v1, pConstraintNow}, 
      If[Union[getParameters[v1 && pConstraintNow]] =!= {}, 
       Union[getParameters[v1 && pConstraintNow]], {tmp}], 
      Reals], {Minimize::wksol, Minimize::infeas}];
   simplePrint[minNow];
   maxNow = 
    Quiet[Maximize[{v1, pConstraintNow}, 
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
      Not[reduceExprPast] && reduceExprNow] === False, toReturnForm[1], 
    toReturnForm[0]]]
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

addParameterConstraintNow[pcons_, pars_] := (
     pConstraintNow = True;
     parametersNow = {};
     pConstraintNow = Reduce[pConstraintNow && And@@pcons, Reals];
     parametersNow = Union[parametersNow, pars];
     simplePrint[pConstraintNow];
     );

addParameterConstraintPast[pcons_, pars_] := (
     pConstraintPast = True;
     parametersPast = {};
     pConstraintPast = Reduce[pConstraintPast && And@@pcons, Reals];
     parametersPast = Union[parametersPast, pars];
     simplePrint[pConstraintPast];
     );
