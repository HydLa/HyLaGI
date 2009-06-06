Print[]

testEqual[a_, b_] :=
  If[a === b, 
      Print["OK : ", a],
      Print["*Error* : ", a, b]]

createModuleSet[code_] := 
  Map[(#[[2;;-1]])&, 
        Split[Sort[createModuleSet[code, {}, False], 
                (First[#1] > First[#2])&], 
          (First[#1] === First[#2])&],
        {2}]

createModuleSet[order[], mset_, slave_] := mset
createModuleSet[order[h_, t___], mset_, slave_] :=
  createModuleSet[order[t], createModuleSet[h, mset, slave], True]

createModuleSet[group[], mset_, slave_] := mset
createModuleSet[group[h_, t___], mset_, slave_] :=
  createModuleSet[group[t], createModuleSet[h, mset, slave], slave]

createModuleSet[unit[a__], mset_, True] :=
  Join[mset, Map[(Join[unit[First[#1]+1], #1[[2;;-1]], unit[a]])&, mset]]
createModuleSet[unit[a__], mset_, False] :=
  Join[mset, Map[(Join[unit[First[#1]+1], #1[[2;;-1]], unit[a]])&, mset], {unit[1, a]}]
  
Print[Flatten[createModuleSet[group[unit[a], unit[b]]]]]
(* Print[createModuleSet[order[unit[a], unit[b]]]] *)
Print[createModuleSet[order[group[unit[a], unit[b]], unit[c]]]]
Print[createModuleSet[order[order[unit[a], unit[b]], unit[c]]]]
(* Print[createModuleSet[order[unit[a], group[unit[b], unit[c]]]]] *)

collectTell[{elem___}]              := Fold[collectTell, {}, unit[elem]]
collectTell[terms_, tell[elem_]]    := Append[terms, elem]
collectTell[terms_, ask[__]]        := terms
collectTell[terms_, always[elem__]] := Fold[collectTell, terms, {elem}]

(* 
 * 現在の制約ストアからask制約がエンテール出来るかどうか 
 *)
askTestQ[cons_, asks_, vars_] := (
  gblSol = Reduce[Append[cons, asks], vars, Reals];
  If[gblSol===False, Return[False]];
  If[Reduce[Append[cons, Not[gblSol]], vars, Reals]===False, True, False]	
)

applyAsk[cons_, consStore_, posAsk_, vars_] :=
  applyAsk[cons, {}, consStore, False, posAsk, {}, vars]
applyAsk[{}, consTable_, consStore_, askSuc_, posAsk_, negAsk_, vars_] :=
  {consTable, askSuc, posAsk, negAsk} 
applyAsk[{tell[elem__], t___}, consTable_, consStore_, askSuc_, posAsk_, negAsk_, vars_] := 
  applyAsk[{t}, Append[consTable, tell[elem]], consStore, askSuc, posAsk, negAsk, vars]
applyAsk[{always[elem__], t___}, consTable_, consStore_, askSuc_, posAsk_, negAsk_, vars_] := (
  {gblConsTable, gblAskSuc, gblPosAsk, gblNegAsk} = 
    applyAsk[{elem}, {}, consStore, askSuc, posAsk, negAsk, vars];
  applyAsk[{t}, Append[consTable, always @@ gblConsTable], 
           consStore, gblAskSuc, gblPosAsk, gblNegAsk, vars]
)
applyAsk[{ask[guard_, elem__], t___}, consTable_, consStore_, askSuc_, posAsk_, negAsk_, vars_] := (
  Print[consStore, " ", guard, " ", askTestQ[consStore, guard, vars]];
  If[askTestQ[consStore, guard, vars]===True,
      applyAsk[{t}, Join[consTable, {elem}], consStore, 
               True, Append[posAsk, guard], negAsk, vars],
      applyAsk[{t}, Append[consTable, ask[guard, elem]], consStore,
               askSuc, posAsk, Append[negAsk, guard], vars]]
)

chSolve[cons_, posAsk_] := (
  gblConsStore = {Reduce[collectTell[cons], vars]};
  If[gblConsStore === {False},
      False,
      {gblConsTable, gblAskSuc, gblPosAsk, gblNegAsk} =
        applyAsk[cons, gblConsStore, posAsk, vars];
      If[gblAskSuc === True,
          chSolve[gblConsTable, gblPosAsk],
          {gblConsTable, gblPosAsk, gblNegAsk}]]      
)

pointPhase[{h_, t___}, vars_] := (
  gblSol = chSolve[List @@ h, {}];
  If[gblSol === False,
      pointPhase[{t}, vars],
      gblSol]
)

HydLaSolve[cons_, vars_, maxTime_] := Module[{
  moduleSet
},
  moduleSet = Flatten[createModuleSet[cons]]; (* 全解探索をおこなわないために展開 *)
  pointPhase[moduleSet, vars]

]

(* HydLaSolve[ *)
(* group[unit[tell[a==1], ask[a==1, tell[b==1]]], *)
(*       unit[ask[b==1, tell[d==1]]], *)
(*       unit[always[tell[c==2]]]], *)
(* {a,b,c},1 *)
(* ] *)

HydLaSolve[
order[unit[tell[a==1], ask[a==1, tell[b==1]]],
      unit[tell[b==2]]],
{a,b,c},1
]

HydLaSolve[
order[unit[ask[a==1, tell[a==2]]],
      unit[tell[a==1]]],
{a,b,c},1
]


(* HydLaSolve[group[unit[tell[Equal[ht, 10]], tell[Equal[v, 0]]], order[unit[always[ask[Equal[prev[ht], 0], tell[Equal[v, Times[Minus[Divide[4, 5]], prev[v]]]]]]], group[unit[always[tell[Equal[ht', *)
(* v]]]], unit[always[tell[Equal[v', Minus[10]]]]]]]], {ht, v}, 1] *)
