pointToPointsAcc = <||>;
pointToPoints = <||>;

clearPointToPoint[] :=
  pointToPointsAcc = <||>;
  pointToPoints = <||>;

getEdges::NotHandledRelop = "予期せぬオペレータ`1`です"
getEdges[cons_, vars_] :=
  Module[
    {i,consList, edge, edges={}, endPoints=<||>, var},
    consList = applyList[cons];
    For[i=1,i<=Length[vars],i++,
      endPoints[vars[[i]]] = {};
    ];
    For[i=1,i<=Length[consList],i++,
      var = getVariablesWithDerivatives[consList[[i]]][[1]];
      (* TODO 一つの変数に付き、Equalならば1つ、lt,gt,le,geなら2つ制約があることが前提になっている*)
      (* lt,gt,le,geが一つなのも考えられる*)
      endPoints[var] =
        Switch[Head[consList[[i]]],
          Equal, Append[endPoints[var], {Cases[consList[[i]], _?NumericQ][[1]], True}],
          Less, Append[endPoints[var], {Cases[consList[[i]], _?NumericQ][[1]], False}],
          Greater, Append[endPoints[var], {Cases[consList[[i]], _?NumericQ][[1]], False}],
          LessEqual, Append[endPoints[var], {Cases[consList[[i]], _?NumericQ][[1]], True}],
          GreaterEqual, Append[endPoints[var], {Cases[consList[[i]], _?NumericQ][[1]], True}],
          _, Message[getEdges::NotHandledRelop, Head[consList[[i]]]]; endPoints[var]
        ];
      (*xとyの端点を直積*)
    ];
    If[Length[endPoints[vars[[1]]]]==1 && Length[endPoints[vars[[2]]]]==1,
      edges = Union[edges, {{{{endPoints[vars[[1]]][[1]][[1]],endPoints[vars[[2]]][[1]][[1]]}, True},{{endPoints[vars[[1]]][[1]][[1]],endPoints[vars[[2]]][[1]][[1]]}, True}}}],
      edge = Flatten[Fold[Function[{a, b}, Append[a, Fold[Append[#1, {{b[[1]], #2[[1]]}, b[[2]]&&#2[[2]]}]&, {}, endPoints[vars[[1]]] ]]], {}, endPoints[vars[[2]]] ], 1];
      edges = Union[edges, If[Length[edge] > 1, edge, {}]];
      edge = Flatten[Fold[Function[{a, b}, Append[a, Fold[Append[#1, {{#2[[1]], b[[1]]}, #2[[2]]&&b[[2]]}]&, {}, endPoints[vars[[2]]] ]]], {}, endPoints[vars[[1]]] ], 1];
      edges = Union[edges, If[Length[edge] > 1, edge, {}]];
      edges
    ]
  ];

(* point0 -> point1 -> point2 が時計回りならば1, 半時計回りなら-1, 直線状にあるなら0を返す*)
isClockWise[point0_, point1_, point2_] :=
  Module[
    {s},
    s = (point1[[1]]-point0[[1]]) * (point2[[2]]-point0[[2]]) - (point1[[2]]-point0[[2]]) * (point2[[1]]-point0[[1]]);
    Which[s>0, -1, s<0, 1, True, 0]
  ]

(* 与えられた点と凸法の接線を特定し、凸法を更新する*)
(* 上方と下方で分けたほうが効率が良かったので使わない *)
addPointToConvex[convexStack_, point_] :=
  Module[
    (*convexStackElem :: {point, left, right, redundantFlag}*)
    {retStack, remainingElems, checkingElem, newElem, pointLeft, pointCenter, pointRight1, pointRight2, isClockWise1, isClockWise2, aboveUpdated, belowUpdated},
    pointLeft = point;
    retStack = convexStack;
    aboveUpdated = False;
    belowUpdated = False;
    remainingElems = {};
    newElem = {point, _, _, False};
    While[!aboveUpdated || !belowUpdated,
      checkingElem = Last[retStack];
      retStack = Most[retStack];
      pointCenter = checkingElem[[1]];
      pointRight1 = checkingElem[[2]];
      pointRight2 = checkingElem[[3]];
      isClockWise1 = isClockWise[pointLeft, pointCenter, pointRight1];
      isClockWise2 = isClockWise[pointLeft, pointCenter, pointRight2];
      If[!aboveUpdated && isClockWise1 == 1 || !belowUpdated && isClockWise2 == -1,
        Continue[];
      ];
      (* 上方の接線。接点の右手と結合 *)
      If[!aboveUpdated && (isClockWise1 == 0 || isClockWise1 == -1) && (isClockWise1 == 0 || isClockWise2 == -1),
        aboveUpdated = True;
        checkingElem[[3]] = pointLeft;
        (* 上方の接線だが、接点が凸法の表面になる。接点の右手と結合し、接点を冗長な点とする *)
        checkingElem[[4]] = (isClockWise1 == 0);
        newElem[[2]] = checkingElem[[1]];
      ];
      (* 下方の接線。接点の左手と結合 *)
      If[!belowUpdated && (isClockWise1 == 0 || isClockWise1 == 1) && (isClockWise2 == 0 || isClockWise2 == 1),
        belowUpdated = True;
        checkingElem[[2]] = pointLeft;
        (* 下方の接線だが、接点が凸法の表面になる。接点の左手と結合し、接点を冗長な点とする *)
        checkingElem[[4]] = (isClockWise2 == 0);
        newElem[[3]] = checkingElem[[1]];
      ];
      remainingElems = Append[remainingElems, checkingElem];
    ];
    Join[retStack, Reverse[remainingElems], {newElem}]
  ];

(* 凸包の上方への接線を特定し、凸包の上方へ点を追加する*)
addPointToUpperConvex[convexStack_, point_] :=
  Module[
    {pointRight, pointCenter, pointLeft, checkingElem, retStack, s},
    pointRight = point;
    retStack = convexStack;
    While[True,
      checkingElem = Last[retStack];
      If[checkingElem[[4]] == True, retStack = Most[retStack]; Continue[]];
      pointCenter = checkingElem[[1]];
      pointLeft = checkingElem[[2]];
      If[pointLeft == _, Break[]];
      (*simplePrint[pointLeft, pointCenter, pointRight];*)
      s = isClockWise[pointLeft, pointCenter, pointRight];
      (*simplePrint[s];*)
      Switch[s,
        1, Break[],
        0, retStack[[Length[retStack]]][[4]] = True; Break[],
        -1, retStack = Most[retStack]
      ];
    ];
    retStack[[Length[retStack]]][[3]] = point;
    Append[retStack, {point, checkingElem[[1]], _, False}]
  ];

addPointToLowerConvex[convexStack_, point_] :=
  Module[
    {pointRight, pointCenter, pointLeft, checkingElem, retStack, s},
    pointRight = point;
    retStack = convexStack;
    While[True,
      checkingElem = First[retStack];
      If[checkingElem[[4]] == True, retStack = Rest[retStack]; Continue[]];
      pointCenter = checkingElem[[1]];
      pointLeft = checkingElem[[3]];
      If[pointLeft == _, Break[]];
      (*simplePrint[pointLeft, pointCenter, pointRight];*)
      s = isClockWise[pointLeft, pointCenter, pointRight];
      (*simplePrint[s];*)
      Switch[s,
        -1, Break[],
        0, retStack[[Length[retStack]]][[4]] = True; Break[],
        1, retStack = Rest[retStack]
      ];
    ];
    retStack[[1]][[2]] = point;
    Prepend[retStack, {point, _, checkingElem[[1]], False}]
  ];


(* p1, p2が{p3, p4}で分離されているか*)
isSameSide[p1_, p2_, p3_, p4_] :=
    isClockWise[p4, p3, p1] * isClockWise[p4, p3, p2]

(* 線分の交差判定 *)
isIntersectingSegs[p1_, p2_, p3_, p4_, permitTouch_] :=
  Module[
    {s1, s2},
    s1 = isSameSide[p1, p2, p3, p4];
    s2 = isSameSide[p3, p4, p1, p2];
    If[permitTouch,
      s1 <  0 && s2 <  0,
      s1 <= 0 && s2 <= 0
    ]
  ]
  

(* 三角形と線分の交差判定(外積計算16回). 分離軸全探索よりももっといい方法がある？*)
isIntersectingTriAndSeg[p1_, p2_, t1_, t2_, t3_, permitTouch_] :=
  Module[
    {s1, s2, s3, s4, s5, s6, s7, s8},
    (* t2, t3が分離軸 *)
    s1 = isSameSide[p1, t1, t2, t3];
    s2 = isSameSide[p2, t1, t2, t3];
    (* t3, t1が分離軸 *)
    s2 = isSameSide[p1, t2, t3, t1];
    s4 = isSameSide[p2, t2, t3, t1];
    (* t1, t2が分離軸 *)
    s5 = isSameSide[p1, t3, t1, t2];
    s6 = isSameSide[p2, t3, t1, t2];
    (* 直線が分離軸 *)
    s7 = isSameSide[t1, t2, p1, p2];
    s8 = isSameSide[t2, t3, p1, p2];
    If[permitTouch,
      s1<0&&s2<0 || s3<0&&s4<0 || s5<0&&s6<0 || s7<0&&s8<0,
      s1<=0&&s2<=0 || s3<=0&&s4<=0 || s5<=0&&s6<=0 || s7<=0&&s8<=0
    ]
  ]

isTriContainsPoint[p_, t1_, t2_, t3_, permitTouch_] :=
  Module[
    {s1, s2, s3},
    s1 = isClockWise[p, t1, t2];
    s2 = isClockWise[p, t2, t3];
    s3 = isClockWise[p, t3, t1];
    If[permitTouch,
      s1 == s2 == s3 || (s1==0 || s2==0 || s3==0),
      s1 == s2 == s3
    ]
  ]

(* startPointsが新しい凸包に含まれないなら、凸包の右側(時計回りの方向)にpointを追加*)
addPointToConvexWithChecking[convexStack_, point_, startPoints_] :=
  Module[
    {tmp, tmp2, upperYConvexStack, lowerYConvexStack, upperYEnd, lowerYEnd},
    upperYConvexStack = convexStack[[1]];
    lowerYConvexStack = convexStack[[2]];
    (*simplePrint["start"];*)
    (*simplePrint[point];*)
    (*simplePrint[upperYConvexStack];*)
    (*simplePrint[lowerYConvexStack];*)
    tmp = {};
    (* 最後尾が更新されるので、上/下包を更新 *)
    While[First[upperYConvexStack][[4]] == True || isClockWise[First[upperYConvexStack][[1]], Last[upperYConvexStack][[1]], point] === -1,
      tmp = Append[tmp, Last[upperYConvexStack]];
      upperYConvexStack = Most[upperYConvexStack];
    ];
    upperYConvexStack[[Length[upperYConvexStack]]][[3]] = _;
    tmp2 = Last[upperYConvexStack];
    If[tmp =!= {},
      tmp2[[2]] = _;
      tmp2[[3]] = Last[tmp][[1]];
      tmp[[Length[tmp]]][[2]] = tmp2[[1]];
      tmp[[1]][[3]] = lowerYConvexStack[[2]][[1]];
    ];
    lowerYConvexStack = Join[{tmp2}, Reverse[tmp], Rest[lowerYConvexStack]];
    (*simplePrint[upperYConvexStack];*)
    (*simplePrint[lowerYConvexStack];*)
    upperYConvexStack = addPointToUpperConvex[upperYConvexStack, point];
    lowerYConvexStack = addPointToLowerConvex[lowerYConvexStack, point];
    (*simplePrint[upperYConvexStack];*)
    (*simplePrint[lowerYConvexStack];*)
    upperYEnd = Last[Most[upperYConvexStack]];
    lowerYEnd = First[Rest[lowerYConvexStack]];
    (*simplePrint["end"];*)
    If[Length[startPoints] == 1,
      If[isTriContainsPoint[startPoints[[1]], point, upperYEnd[[1]], lowerYEnd[[1]], True],
        Return[{convexStack, False}]
      ],
      For[i=1,i<=Length[startPoints],i++,
        (* TODO startPointsはOrdered? *)
        If[isIntersectingTriAndSeg[startPoint[[i]], startPoint[[Mod[i+1, Length[startPoints]]]], point, upperYEnd[[1]], lowerYEnd[[1]], True],
          Return[{convexStack, False}]
        ]
      ]
    ];
    {{upperYConvexStack, lowerYConvexStack}, True}
  ]


(* 与えられた点集合から、それらの凸法に対応するconvexStack(最小のx座標の点から右回り)を返す*)
(* 上方と下方で分割した逐次添加法で計算 *)
convexFull[points_] :=
  Module[
    {i, tmpList, lowerX, upperX, lowerYPoints, upperYPoints, lowerYConvexStack, upperYConvexStack, convexStack},
    lowerX = First[points];
    upperX = Last[points];
    tmpList = Rest[Most[points]];
    (*上方, 下方凸法に分解*)
    tmpList = Map[{#, isClockWise[lowerX, upperX, #]}&, tmpList];
    upperYPoints = Map[#[[1]]&, Select[tmpList, (#[[2]]==-1)&]];
    lowerYPoints = Map[#[[1]]&, Select[tmpList, (#[[2]]==1)&]];
    (* x座標の最大/最小がつながるような凸包(上方/下方のみ)の場合、線上の点を空の方に詰める*)
    If[upperYPoints == {},
      upperYPoints = Map[#[[1]]&, Select[tmpList, (#[[2]]==0)&]],
      If[lowerPoints == {},
        lowerYPoints = Map[#[[1]]&, Select[tmpList, (#[[2]]==0)&]];
      ];
    ];
    upperYPoints = Join[{lowerX}, upperYPoints, {upperX}];
    (* 下方凸包は、右から左に方向に作る点に注意 *)
    lowerYPoints = Reverse[Join[{lowerX}, lowerYPoints, {upperX}]];
    (*(*simplePrint[lowerYPoints];*)*)
    (*(*simplePrint[upperYPoints];*)*)

    (* 上方凸包 *)
    upperYConvexStack = {{upperYPoints[[1]], _, upperYPoints[[2]], False}, {upperYPoints[[2]], upperYPoints[[1]], _, False}};
    For[i=3,i<=Length[upperYPoints], i++,
      upperYConvexStack = addPointToUpperConvex[upperYConvexStack, upperYPoints[[i]]];
    ];
    (*(*simplePrint[upperYConvexStack];*)*)
    (* 下方凸包 *)
    lowerYConvexStack = {{lowerYPoints[[1]], _, lowerYPoints[[2]], False}, {lowerYPoints[[2]], lowerYPoints[[1]], _, False}};
    For[i=3,i<=Length[lowerYPoints], i++,
      lowerYConvexStack = addPointToUpperConvex[lowerYConvexStack, lowerYPoints[[i]]];
    ];
    (*(*simplePrint[lowerYConvexStack];*)*)
    (*upperYConvexStack[[1]][[3]] = Last[lowerYConvexStack][[1]];
    upperYConvexStack[[Length[upperYConvexStack]]][[2]] = lowerYConvexStack[[1]][[1]];
    lowerYConvexStack[[1]][[3]] = Last[upperYConvexStack][[1]];
    lowerYConvexStack[[Length[lowerYConvexStack]]][[2]] = upperYConvexStack[[1]][[1]];*)
    {upperYConvexStack, lowerYConvexStack}
    (*{upperYConvexStack, Rest[Most[lowerYConvexStack]]}*)
  ]

(*上下の凸包を表すスタックをマージ*)
mergeConvexStack[convexStack] :=
  Select[Flatten[{convexStack[[1]], Rest[Most[convexStack[[2]]]]}, 1], (#[[4]] == False)&]
 
endPointToConstrant[point0_, point1_, reop_] :=
  Module[
    {x0, x1, y0, y1, a},
    (*simplePrint[point0, point1];*)
    x0 = point0[[1]];
    y0 = point0[[2]];
    x1 = point1[[1]];
    y1 = point1[[2]];
    Quiet[Check[
      a = (y1-y0) / (x1-x0);
      Switch[reop,
        0, uy[t]-y0 = a*(ux[t]-x0) && x0 <= ux[t] <= x1,
        1, uy[t]-y0 >= a*(ux[t]-x0),
        -1, uy[t]-y0 <= a*(ux[t]-x0)
      ],
      Switch[reop,
        0, x0 <= ux[t] <= x1,
        1, ux[t] <= x0,
        -1, ux[t] >= x0
      ]
    ]]
  ]

(*凸包が内包する領域を制約として返す*)
translateConvexPointsIntoConstraint[convexStack_] :=
  Module[
    {i, retCons, lowerYConvexStack, upperYConvexStack},
    (*simplePrint[convexStack];*)
    upperYConvexStack = Select[convexStack[[1]], (#[[4]]==False)&];
    lowerYConvexStack = Select[convexStack[[2]], (#[[4]]==False)&];
    retCons = True;
    For[i=1,i<Length[upperYConvexStack],i++,
      retCons = retCons && endPointToConstrant[upperYConvexStack[[i]][[1]], upperYConvexStack[[i+1]][[1]], -1];
    ];
    For[i=1,i<Length[lowerYConvexStack],i++,
      retCons = retCons && endPointToConstrant[lowerYConvexStack[[i]][[1]], lowerYConvexStack[[i+1]][[1]], 1];
    ];
    retCons = LogicalExpand[retCons];
    (*simplePrint[retCons];*)
    retCons
  ];

(*
getSurfaceEdges[convexStack_, pointToPoints_] :=
  Module[
    {surfaces, i, j, points, ret},
    ret = {}
    surfaces = Map[(#[[1]])&, convexStack];
    For[i=1,i<Length[convexStack],i++,
      points = pointToPoints[convexStack[[i]][[1]]];
      For[j=1,j<Length[points],j++,
        If[MemberQ[surfaces, points[[i]],
          
        ]
      ]
    ]
  ]
  *)

(*pointToPointsを初期化, gListをpointsに変換*)
calculateRelaxedGuardsInit[gList_] :=
  Module[
    {i, edges, edgeFrom, edgeTo, points, vars},
    (*simplePrint[vars];*)
    vars = getVariablesWithDerivatives[gList];
    If[Head[pointToPointsAcc[gList]] === Missing,
      edges = Fold[Append[#1, getEdges[#2, vars]]&, {}, gList];
      (*(*simplePrint[edges];*)*)
      (* TODO 一旦、boundaryを考慮しない*)
      points = Fold[Union[#1, {#2[[1]][[1]], #2[[2]][[1]]}]&, {}, edges];
      points = Sort[points];
      For[i=1,i<=Length[edges],i++,
        edgeFrom = edges[[i]][[1]][[1]];
        edgeTo = edges[[i]][[2]][[1]];
        If[edgeFrom == edgeTo, Continue[]];
        If[Head[pointToPoints[edgeFrom]] === Missing,
          pointToPoints[edgeFrom] = {edgeTo},
          pointToPoints[edgeFrom] = Union[pointToPoints[edgeFrom], {edgeTo}]
        ];
        If[Head[pointToPoints[edgeTo]] === Missing,
          pointToPoints[edgeTo] = {edgeFrom},
          pointToPoints[edgeTo] = Union[pointToPoints[edgeTo], {edgeFrom}]
        ];
      ];
      pointToPointsAcc[gList] = {pointToPoints, points},
      pointToPoints = pointToPointsAcc[gList][[1]];
      points = pointToPointsAcc[gList][[2]]
    ];
    points
  ]

(* ret :: {{constraint, convexStack}}*)
calculateRelaxedGuards[points_, shiftedStartCons_] := 
  Module[
    (*
      point :: {x座標, y座標}
      points :: {point}
      edge :: {{point, 端点を含むか否か}, {point, 端点を含むか否か}}
      edges :: {edge}
      pointToPoints :: <|point -> {point, includeLeftEnd, includeLine, includeRightEnd}|>
      convexStackElem :: {point, left, right, redundantFlag}
      上方と下方のスタックのペア
      convexStack :: {{convexStackElem}, {convexStackElem}}
    *)
    {i, tmp, edges, vars, startPoints, lowerXPoints, upperXPoints, floatingPoints, lowerXConvexStack, upperXConvexStack, convexConstraints},
    vars = Map[(#1[[1]])&, shiftedStartCons];
    edges = getEdges[shiftedStartCons, vars];
    (*simplePrint[edges];*)
    startPoints = Fold[Union[#1, {#2[[1]][[1]], #2[[2]][[1]]}]&, {}, edges];
    startPoints = Sort[startPoints];
    (*二次元のみ対応*)
    If[Length[vars] > 2
    , Return[{}]
    ];
    (*(*simplePrint[startPoints];*)*)
    (*(*simplePrint[pointToPoints];*)*)
    (*(*simplePrint[points];*)*)
    lowerXPoints = Select[points, (#[[1]] < startPoints[[1]][[1]])&];
    upperXPoints = Select[points, (#[[1]] > startPoints[[Length[startPoints]]][[1]])&];
    floatingPoints = Complement[points, lowerXPoints, upperXPoints];
    (*(*simplePrint[lowerXPoints];*)*)
    (*(*simplePrint[upperXPoints];*)*)
    lowerXConvexStack = convexFull[lowerXPoints];
    upperXConvexStack = convexFull[upperXPoints];
    (*右側の凸包は、上下を逆転させたほうが処理が一貫する*)
    upperXConvexStack = {upperXConvexStack[[2]], upperXConvexStack[[1]]};
    (*
      組み込み関数を用いた凸包導出
    SetOptions[ConvexHull, AllPoints -> False];
    lowerXConvexIndexes = ConvexHull[lowerXPoints];
    upperXConvexIndexes = ConvexHull[upperXPoints];
    (*(*simplePrint[lowerXConvexIndexes];*)*)
    (*(*simplePrint[upperXConvexIndexes];*)*)
    *)
    (*simplePrint[lowerXConvexStack];*)
    (*simplePrint[upperXConvexStack];*)
    (*simplePrint[Map[#[[1]]&, Select[Flatten[lowerXConvexStack, 1], (#[[4]] == False)&]]];*)
    (*simplePrint[Map[#[[1]]&, Select[Flatten[upperXConvexStack, 1], (#[[4]] == False)&]]];*)
    remainingEdges = {};

    (* 左側の凸包から、外側に伸びている線分の端点を列挙 *)
    For[i=1,i<=Length[lowerXPoints],i++,
      If[Head[pointToPoints[lowerXPoints[[i]]]] =!= Missing,
        remainingEdges = Union[remainingEdges, Map[({lowerXPoints[[i]], #})&, Select[pointToPoints[lowerXPoints[[i]]], (!MemberQ[lowerXPoints, #])&]]];
      ];
    ];
    remainingEdges = Sort[remainingEdges, (#1[[2]] < #2[[2]])&];
    (*simplePrint[remainingEdges];*)
    (* 残りの点を、左の凸法にマージを試みる*)
    tmp = {};
    For[i=1,i<=Length[remainingEdges],i++,
      (*simplePrint[remainingEdges];*)
      {lowerXConvexStack, succeeded} = addPointToConvexWithChecking[lowerXConvexStack, remainingEdges[[i]][[2]], startPoints];
      (*simplePrint[lowerXConvexStack];*)
      (*simplePrint[succeeded];*)
      If[!succeeded,
        tmp = Union[tmp, {remainingEdges[[i]]}],
        lowerXPoints = Append[lowerXPoints, remainingEdges[[i]][[2]]];
      ];
    ];
    (*simplePrint[tmp];*)
    remainingEdges = {};
    (* 右側の凸包から、外側に伸びている線分の端点を列挙。左右に橋ができている場合、左にマージ失敗したのもマージを試みる *)
    For[i=1,i<=Length[upperXPoints],i++,
      If[Head[pointToPoints[upperXPoints[[i]]]] =!= Missing,
        remainingEdges = Union[remainingEdges, Map[({#, upperXPoints[[i]]})&, Select[pointToPoints[upperXPoints[[i]]], (!MemberQ[upperXPoints, #] || MemberQ[remainingEdges, {#, upperXPoints[[i]]}])&]]];
      ];
    ];
    remainingEdges = Sort[remainingEdges, (#1[[2]] > #2[[2]])&];
    (*simplePrint[remainingEdges];*)
    For[i=1,i<=Length[remainingEdges],i++,
      {upperXConvexStack, succeeded} = addPointToConvexWithChecking[upperXConvexStack, remainingEdges[[i]][[1]], startPoints];
      (*simplePrint[upperXConvexStack];*)
      (*simplePrint[succeeded];*)
      If[!succeeded,
        tmp = Union[tmp, {remainingEdges[[i]]}],
        tmp = Complement[tmp, {remainingEdges[[i]]}];
        upperXPoints = Prepend[upperXPoints, remainingEdges[[i]][[1]]]
      ];
    ];
    (*simplePrint[upperXConvexStack];*)
    (*simplePrint[lowerXConvexStack];*)
    (*simplePrint[tmp];*)
    convexConstraints = {};
    For[i=1,i<=Length[tmp],i++,
      convexConstraints = Append[convexConstraints, {endPointToConstrant[tmp[[i]][[1]], tmp[[i]][[2]], 0], {}, {}}];
    ];
    (*simplePrint[convexConstraints];*)
    convexConstraints = Fold[(Append[#1, {translateConvexPointsIntoConstraint[#2[[1]]], #2[[1]], #2[[2]]}])&, convexConstraints, {{{upperXConvexStack[[2]], upperXConvexStack[[1]]}, upperXPoints}, {lowerXConvexStack, lowerXPoints}}];
    (*simplePrint[Map[#[[1]]&, Select[lowerXConvexStack[[1]], (#[[4]] == False)&]]];*)
    (*simplePrint[Map[#[[1]]&, Select[lowerXConvexStack[[2]], (#[[4]] == False)&]]];*)
    (*simplePrint[Map[#[[1]]&, Select[upperXConvexStack[[1]], (#[[4]] == False)&]]];*)
    (*simplePrint[Map[#[[1]]&, Select[upperXConvexStack[[2]], (#[[4]] == False)&]]];*)
    (*simplePrint[convexConstraints]*)
    convexConstraints
  ];

(* 2ステップ以降は、初期位置が凸包の表面にいるので、わざわざ包含判定をせずに簡単にできる *)
(* TODO 凸包の面積を2等分するように分けるとかして、探索木をバランスさせたい *)
calculateRelaxedGuardsFollowingStep[points_, splitX_] := 
  Module[
    {i, tmp, lowerXPoints, upperXPoints, lowerXConvexStack, toRightPoints, upperXConvexStack, convexConstraints},
    lowerXPoints = Select[points, (#[[1]] < splitX)&];
    upperXPoints = Complement[points, lowerXPoints];
    toLeftPoints = {};
    For[i=1,i<=Length[upperXPoints],i++,
      tmp = pointToPoints[upperXPoints[[i]]];
      If[Head[tmp] =!= Missing,
        toLeftPoints = Union[toLeftPoints, Select[tmp, (MemberQ[lowerXPoints, #])&]];
      ];
    ];
    toLeftPoints = Sort[toLeftPoints, (#1[[1]] < #2[[1]])&];
    upperXPoints = Join[toLeftPoints, upperXPoints];
    simplePrint[lowerXPoints];
    simplePrint[upperXPoints];
    lowerXConvexStack = convexFull[lowerXPoints];
    upperXConvexStack = convexFull[upperXPoints];
    simplePrint[lowerXConvexStack];
    simplePrint[upperXConvexStack];
    convexConstraints = Fold[(Append[#1, {translateConvexPointsIntoConstraint[#2[[1]]], #2[[1]], #2[[2]]}])&, {}, {{upperXConvexStack, upperXPoints}, {lowerXConvexStack, lowerXPoints}}];
    convexConstraints
  ];

checkGuardSatisfiability::nonContinuousStartPosition := "初期位置が線分ではありません: `1`";
(* findMinTimeの結果が、凸包の表面上のガードを満たすかの検証(緩和問題の解 = 元の問題の解かを検証) *)
checkGuardSatisfiability[shiftedStartCons_, pCons_, convexStack_] :=
  Module[
    {i, j, vars, pars, tmp, startOnePoint, checkingConvex, checkingConvexLine, checkingEdges, checkingPoints, ret},
    (* 上下左右の凸法から独立した凸包は、必ず真の解 *)
    If[convexStack =={},
      Return[{}]
    ];
    (* FindInstanceは、不等式を満たす適当な点を求める *)
    vars = getVariables[shiftedStartCons];
    pars = getParameters[pCons];
    If[pars =!= {},
      startOnePoint = shiftedStartCons /. Map[(FindInstance[#, getParameters[#]])&, ApplyList[pCons]];
      startOnePoint = {startOnePoint[[1]][[2]], startOnePoint[[2]][[2]]},
      startOnePoint = {shiftedStartCons[[1]][[2]], shiftedStartCons[[1]][[2]]}
    ];
    (* 凸包との衝突点(線)が、上方か下方かを特定 *)
    (* TODO 端点*)
    checkingConvex = If[isClockWise[Last[convexStack[[1]]][[1]], First[convexStack[[1]]][[1]], startOnePoint] == 1, convexStack[[1]], Reverse[convexStack[[2]]]];
    For[i=1,i<Length[checkingConvex],i++,
      j = i+1;
      (* 凸包上の衝突した辺を特定. 上下は特定しているので、x座標の比較で十分 *)
      If[checkingConvex[[i]][[1]][[1]] <= startOnePoint[[1]] <= checkingConvex[[j]][[1]][[1]],
        checkingConvexLine = Map[(#1[[1]])&, checkingConvex[[i;;j]]];
      ]
    ];
    simplePrint[checkingConvexLine];
    If[checkingConvexLine == {},
      Return[{}];
    ];
    checkingPoints = checkingEdges = {};
    (* 不等式で表されているガードを取得 *)
    For[i=1,i<=Length[checkingConvexLine],i++,
      Module[
        {points, edges},
        points = pointToPoints[checkingConvexLine[[i]]];
        simplePrint[points];
        If[Head[points] =!= Missing,
          edges = Map[({checkingConvexLine[[i]], #})&, Select[points, (MemberQ[checkingConvexLine, #] && checkingConvexLine[[i]][[1]]<#[[1]])&]];
          If[edges == {},
            checkingPoints = Append[checkingPoints, checkingConvexLine[[i]]],
            checkingEdges = Union[checkingEdges, edges]
          ],
          checkingPoints = Append[checkingPoints, checkingConvexLine[[i]]]
        ]
      ]
    ];
    ret = {};
    (* TODO 2分探索したい *)
    For[i=1,i<=Length[checkingPoints],i++,
      tmp = Reduce[shiftedStartCons[[1]] && pCons && ux == checkingPoints[[i]][[1]], Union[vars, pars]];
      simplePrint[tmp];
      If[tmp =!= False,
        ret = Append[ret, {checkingPoints[[i]], And@@Select[applyList[tmp], (hasParameter[#])&]}];
      ]
    ];
    For[i=1,i<=Length[checkingEdges],i++,
      tmp = Reduce[shiftedStartCons[[1]] && pCons && checkingEdges[[i]][[1]][[1]] <= ux && ux <= checkingEdges[[i]][[2]][[1]], Union[vars, pars]];
      simplePrint[tmp];
      If[tmp =!= False,
        ret = Append[ret, {checkingEdges[[i]], And@@Select[applyList[tmp], (hasParameter[#])&]}];
      ]
    ];
    simplePrint[checkingPoints];
    simplePrint[checkingEdges];
    ret
  ]
