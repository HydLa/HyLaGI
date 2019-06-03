pointToPointsAcc = <||>;
pointToPoints = <||>;
edgeToAskPtr = <||>;

Needs["Parallel`Queue`Priority`"];
Unprotect@Priority; Priority[i_List] := -i[[1]];

(*pointToPointsを初期化, gListをpointsに変換*)
calculateRelaxedGuardsInit[g_] := (
  calculateRelaxedGuardsInit[g, variables]
)
publicMethod[
  calculateRelaxedGuardsInit,
  gList, vars
  Module[
    {i, edges, edgeFrom, edgeTo, points},
    (*simplePrint[vars];*)
    vars = {ux, uy};
    If[Head[pointToPointsAcc[{}]] === Missing,
      edges = Fold[Append[#1, getEdges[#2, vars]]&, {}, gList];
      (*(*simplePrint[edges];*)*)
      (* TODO 一旦、boundaryを考慮しない*)
      points = Fold[Union[#1, {#2[[1]][[1]], #2[[2]][[1]]}]&, {}, edges];
      points = Sort[points];
      simplePrint[edges];
      For[i=1,i<=Length[edges],i++,
        edgeFrom = edges[[i]][[1]][[1]];
        edgeTo = edges[[i]][[2]][[1]];
        If[edgeFrom == edgeTo, Continue[]];
        If[Head[pointToPoints[edgeFrom]] === Missing,
          pointToPoints[edgeFrom] = {edgeTo};
          edgeToAskPtr[{edgeFrom, edgeTo}] = i,
          pointToPoints[edgeFrom] = Union[pointToPoints[edgeFrom], {edgeTo}];
          edgeToAskPtr[{edgeFrom, edgeTo}] = i;
        ];
        If[Head[pointToPoints[edgeTo]] === Missing,
          pointToPoints[edgeTo] = {edgeFrom},
          pointToPoints[edgeTo] = Union[pointToPoints[edgeTo], {edgeFrom}];
        ];
      ];
      pointToPointsAcc[{}] = {pointToPoints, points},
      pointToPoints = pointToPointsAcc[{}][[1]];
      points = pointToPointsAcc[{}][[2]]
    ];
    simplePrint[pointToPointsAcc];
    simplePrint[pointToPoints];
    simplePrint[edgeToAskPtr];
    points;
  ];
];

findMinTimeWithRelaxation[vm_, maxT_] := (
  findMinTimeWithRelaxation[vm, pConstraint, maxT, pointToPointsAcc, pointToPoints]
)

publicMethod[
  findMinTimeWithRelaxation,
  variableMap, pCons, maxT, ppa, pp,
  Module[
    {i, j, k, pq, newpq, tmp, sol, elem, vm, points, shiftedStartCons, tRemovedRules, tCons, crgResult, minResult, cgsResult, unsatPCons,
      crgTm, minTm, cgsTm, tm,
      ret
      (*
        elem :: {time, points, startCons, pCons, guards}
      *)
    },
    pointToPointsAcc = ppa;
    pointToPoints = pp;
    simplePrint[variableMap];
    (*Return[{{toReturnForm[1], 1, {toReturnForm[True]}, 1}}];*)
    crgTm = minTm = cgsTm = 0;
    vm = Select[variableMap, (!isVariable[#[[1]]])&];
    simplePrint[vm];
    tRemovedRules = Map[(Rule[#[[1]] /. x_[t] -> x, #[[2]]])&, vm];
    pq = priorityQueue[];
    sol = {}; 
    shiftedStartCons = And@@Map[((#1 /. x_[t] -> x) /. t -> 0)&, vm];
    shiftedStartCons = Resolve[Exists[{p[px,0,1]}, shiftedStartCons && pCons], {ux, uy}, Reals];
    simplePrint[shiftedStartCons];
    points = ppa[{}][[2]];
    {tm, crgResult} = Timing[calculateRelaxedGuards[points, shiftedStartCons]];
    crgTm = crgTm + tm;
    simplePrint[crgResult];
    For[i=1,i<=Length[crgResult],i++,
      simplePrint["============="];
      simplePrint[crgResult[[i]]];
      tCons = ((crgResult[[i]][[1]] /. x_[t] -> x) //. tRemovedRules) && 0<t;
      simplePrint[tCons];
      {tm, minResult} = Timing[minimizeTimePrivate[tCons, pCons, 0, Infinity]];
      simplePrint[minResult];
      minTm = minTm + tm;
      If[minResult === {}, Continue[]];
      If[minResult[[1]] === Infinity, Continue[]];
      For[j=1,j<=Length[minResult],j++,
        simplePrint["~~~~~~~~~~~~~"];
        shiftedStartCons = And@@Map[(# /. x_[t] -> x //. t -> minResult[[j]][[1]])&, vm];
        (*TODO minResultの戻り値のパラメタが複数(OR)だったら、それもForする必要あり*)
        {tm, cgsResult} = Timing[checkGuardSatisfiability[shiftedStartCons, minResult[[j]][[3]][[1]], crgResult[[i]][[2]], crgResult[[i]][[3]]]];
        simplePrint[cgsResult];
        cgsTm = cgsTm + tm;
        For[k=1,k<=Length[cgsResult],k++,
          EnQueue[pq, {minResult[[j]][[1]], crgResult[[i]][[3]], shiftedStartCons, cgsResult[[k]][[2]], cgsResult[[k]][[1]], cgsResult[[k]][[3]]}]
        ];
      ]
    ];
    While[Size[pq]>0,
      simplePrint["+++++++++++++++"];
      elem = DeQueue[pq];
      simplePrint[elem];
      If[elem[[4]] === False,
        Continue[];
      ];
      If[elem[[6]],
        sol = Append[sol, {elem[[1]], elem[[4]], elem[[5]]}];
        newpq = priorityQueue[];
        While[Size[pq]>0,
          tmp = DeQueue[pq];
          tmp[[4]] = tmp[[4]] && Not[elem[[4]]];
          EnQueue[newpq, tmp];
        ];
        pq = newpq;
        simplePrint[elem];
        Continue[];
      ];
      simplePrint[elem[[3]]];
      simplePrint[FindInstance[elem[[4]], getParameters[elem[[4]]]]];
      simplePrint[elem[[5]]];
      {tm, crgResult} = Timing[calculateRelaxedGuardsFollowingStep[elem[[2]], elem[[3]] /. FindInstance[elem[[4]], getParameters[elem[[4]]]], elem[[5]]]];
      crgTm = crgTm + tm;
      simplePrint[crgResult];
      For[i=1,i<=Length[crgResult],i++,
        simplePrint["============="];
        tCons = ((crgResult[[i]][[1]] /. x_[t] -> x) //. tRemovedRules) && elem[[1]]<t;
        {tm, minResult} = Timing[minimizeTimePrivate[tCons, elem[[4]], 0, Infinity]];
        simplePrint[minResult];
        minTm = minTm + tm;
        If[minResult === {}, Continue[]];
        If[minResult[[1]] === Infinity, Continue[]];
        For[j=1,j<=Length[minResult],j++,
          simplePrint["============="];
          shiftedStartCons = And@@Map[(# /. x_[t] -> x //. t -> minResult[[j]][[1]])&, vm];
          (*TODO minResultの戻り値のパラメタが複数(OR)だったら、それもForする必要あり*)
          {tm, cgsResult} = Timing[checkGuardSatisfiability[shiftedStartCons, minResult[[j]][[3]][[1]], crgResult[[i]][[2]], crgResult[[i]][[3]]]];
          simplePrint[cgsResult];
          cgsTm = cgsTm + tm;
          unsatPCons = minResult[[j]][[3]][[1]];
          For[k=1,k<=Length[cgsResult],k++,
            EnQueue[pq, {minResult[[j]][[1]], crgResult[[i]][[3]], shiftedStartCons, cgsResult[[k]][[2]], cgsResult[[k]][[1]], cgsResult[[k]][[3]]}]
          ];
        ];
      ];
    ];
    simplePrint["======"];
    simplePrint[initTm];
    simplePrint[crgTm];
    simplePrint[minTm];
    simplePrint[cgsTm];
    simplePrint["======"];
    simplePrint[sol];
    ret=Map[({toReturnForm[#[[1]] ], 1, {toReturnForm[LogicalExpand[#[[2]] ] ]}, edgeToAskPtr[#[[3]] ]})&, sol];
    ret
  ]
]
