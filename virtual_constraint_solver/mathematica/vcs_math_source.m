(* 再帰回数上限を上げてみる *)
$RecursionLimit = 1000;

(* 内部で用いる精度も上げてみる *)
$MaxExtraPrecision = 1000;


(*
 * プロファイリング用関数
 * timeFuncStart: startTimesに関数の開始時刻を積む
 * timeFuncEnd: startTimesから開始時刻を取り出し、profileListにプロファイル結果を格納
 * <使い方>
 *		プロファイリングしたい関数の定義の先頭にtimeFuncStart[];を
 *		末尾でtimeFuncEnd["関数名"];を追加する.
 *		ただしtimeFuncEndの後で値を返すようにしないと返値が変わってしまうので注意.
 * <プロファイリング結果の見方>
 *		(現在実行が終了した関数名) took (その関数実行に要した時間), elapsed time:(プログラム実行時間)
 *			function:(今までに呼び出された関数名)  calls:(呼び出された回数)  total time of this function:(その関数の合計実行時間)  average time:(その関数の平均実行時間)  max time:(その関数の最高実行時間)
 *		<例>
 *		calculateNextPointPhaseTime took 0.015635, elapsed time:1.006334
 *			function:checkConsistencyPoint  calls:1  total time of this function:0.000361  average time:0.000361  max time:0.000361
 *			function:createMap  calls:2  total time of this function:0.11461  average time:0.057304  max time:0.076988
 *			...
 *)
timeFuncStart[] := (
	If[Length[startTimes]>0,
		startTimes = Append[startTimes,SessionTime[]];
	,
		startTimes = {SessionTime[]};
	];
);

timeFuncEnd[funcname_] := (
Module[{endTime,startTime,funcidx,i},
	endTime = SessionTime[];
	startTime = Last[startTimes];
	startTimes = Drop[startTimes,-1];
	If[Length[profileList]===0,profileList={};];
	If[Position[profileList, funcname] =!= {},
		funcidx = Flatten[Position[profileList,funcname]][[1]];
		profileList[[funcidx,2]] = profileList[[funcidx,2]] + 1;
		profileList[[funcidx,3]] = profileList[[funcidx,3]] + (endTime-startTime);
		profileList[[funcidx,4]] = profileList[[funcidx,3]] / profileList[[funcidx,2]];
		profileList[[funcidx,5]] = If[profileList[[funcidx,5]]<(endTime-startTime), endTime-startTime, profileList[[funcidx,5]]];
	,
		profileList = Append[profileList, {funcname, 1, endTime-startTime, endTime-startTime, endTime-startTime}];
	];
	Print[funcname," took ",endTime-startTime,", elapsed time:",endTime];
	For[i=1,i<=Length[profileList],i=i+1,
		Print["    function:",profileList[[i,1]],"  calls:",profileList[[i,2]],"  total time of this function:",profileList[[i,3]],"  average time:",profileList[[i,4]],"  max time:",profileList[[i,5]]];
	];
];
);


(*
 * デバッグ用メッセージ出力関数
 * debugPrint：引数として与えられた要素要素を文字列にして出力する．（シンボルは評価してから表示）
 * simplePrint：引数として与えられた式を，「（評価前）:（評価後）」の形式で出力する．
 *)
 
SetAttributes[simplePrint, HoldAll];

symbolToString := (StringJoin[ToString[Unevaluated[#] ], ": ", ToString[InputForm[Evaluate[#] ] ] ])&;

SetAttributes[symbolToString, HoldAll];

If[optUseDebugPrint,
  debugPrint[arg___] := Print[InputForm[{arg}]];
  simplePrint[arg___] := Print[delimiterAddedString[", ",
    List@@Map[symbolToString, Map[Unevaluated, Hold[arg]] ]
     ] ],
  
  debugPrint[arg___] := Null;
  simplePrint[arg___] := Null
];


(*
 * 関数呼び出しを再現するための文字列出力を行う
 *)
 
inputPrint[name_, arg___] := Print[StringJoin[name, "[", delimiterAddedString[",", Map[(ToString[InputForm[#] ])&,{arg}] ], "]" ] ];


delimiterAddedString[del_, {h_}] := h;

delimiterAddedString[del_, {h_, t__}] := StringJoin[h, del, delimiterAddedString[del, {t}] ];


SetAttributes[publicMethod, HoldAll];

(* C++側から直接呼び出す関数の，本体部分の定義を行う関数．デバッグ出力とか，正常終了の判定とか，例外の扱いとかを統一する 
   少しでもメッセージを吐く可能性のある関数は，この関数で定義するようにする．
   defineにReturnが含まれていると正常に動作しなくなる（Returnの引数がそのまま返ることになる）ので使わないように！
*)

publicMethod[name_, args___, define_] := (
  name[Sequence@@Map[(Pattern[#, Blank[]])&, {args}]] := (
    inputPrint[ToString[name], args];
    Check[
      Block[{publicRet},
        publicRet = define;
        simplePrint[publicRet];
        {1, publicRet}
      ],
      debugPrint[$MessageList]; {0}
    ]
  )
);


(*
 * グローバル変数
 * constraint: 現在のフェーズでの制約
 * pConstraint: 定数についての制約
 * prevConstraint: 左極限値を設定する制約．
 * variables: 制約に出現する変数のリスト
 * parameters: 記号定数のリスト
 * isTemporary：制約の追加を一時的なものとするか．
 * tmpConstraint: 一時的に追加された制約
 * tmpVariables: 一時制約に出現する変数のリスト
 * guard:
 * guardVars:
 * startTimes: 呼び出された関数の開始時刻を積むプロファイリング用スタック
 * profileList: プロファイリング結果のリスト
 *)


(* （不）等式の右辺と左辺を入れ替える際に，関係演算子の向きも反転させる．Notとは違う *)

getReverseRelop[relop_] := Switch[relop,
                                  Equal, Equal,
                                  Less, Greater,
                                  Greater, Less,
                                  LessEqual, GreaterEqual,
                                  GreaterEqual, LessEqual];


(* ポイントフェーズにおける無矛盾性判定 *)

checkConsistencyPoint[] := (
  checkConsistencyPoint[constraint && tmpConstraint && guard, pConstraint, Union[variables, tmpVariables, guardVars]]
);

publicMethod[
  checkConsistencyPoint,
  cons, pcons, vars,
  Block[
    {trueMap, falseMap, cpTrue, cpFalse},
    Quiet[
      cpTrue = Reduce[Exists[vars, cons && pcons], Reals], {Reduce::useq}
    ];
    simplePrint[cpTrue];
    Quiet[
      cpFalse = Reduce[pcons && !cpTrue, Reals], {Reduce::useq}
    ];
    simplePrint[cpFalse];
    {trueMap, falseMap} = Map[(createMap[#, isParameter, hasParameter, {}])&, {cpTrue, cpFalse}];
    simplePrint[trueMap, falseMap];
    {trueMap, falseMap}
  ]
];

(* インターバルフェーズにおける無矛盾性判定 *)

checkConsistencyInterval[] :=  (
  checkConsistencyInterval[constraint && tmpConstraint, pConstraint, guard, guardVars, Union[variables, tmpVariables]]
);

appendZeroVars[vars_] := Join[vars, vars /. x_[t] -> x[0]];

(* TODO 微分方程式を解いたら，その結果を再利用する *)

publicMethod[
  checkConsistencyInterval,
  cons, pcons, gua, gVars, vars,

  Block[
    {tStore, sol, otherCons, originalOther, tCons, dVars, otherVars, i, cpTrue, cpFalse, trueMap, falseMap},
    sol = exDSolve[cons, vars];
    debugPrint["sol after exDSolve", sol];
    If[sol[[1]] === overConstraint,
      {False, pcons},
      If[sol[[1]] === underConstraint,
        (* 制約不足で微分方程式が解けない場合は，単純に各変数値およびその微分値が矛盾しないかを調べる *)
        tStore = {};
        otherVars = vars;
        otherCons = cons && gua,
        
        (* 微分方程式が解けた場合は，微分方程式を解くのに使われなかった式との整合性を調べる *)
        originalOther = And[And@@sol[[2]], gua];
        otherCons = False;
        dVars = sol[[3]];
        otherVars = sol[[4]];
        For[i = 1, i <= Length[sol[[1]] ], i++,
          tStore = Map[(# -> createIntegratedValue[#, sol[[1]] [[i]]])&, dVars];
          otherCons = Or[otherCons, originalOther /. tStore ]
        ]
      ];
      
      simplePrint[tStore, otherCons];
      
      (* Existsの第一引数はHold（HoldAll?）属性を持っているらしいので，Evaluateで評価する必要がある（気がする） *)
      tCons = Reduce[Exists[Evaluate[Union[appendZeroVars[otherVars], gVars]], otherCons && pcons], Reals];

      simplePrint[tCons];

      If[tCons === False,
        {False, pcons},
      
        cpTrue = Reduce[Quiet[Minimize[{t, tCons && t > 0}, t], {Minimize::wksol, Minimize::infeas}][[1]] == 0, Reals];        
        cpFalse = Reduce[pcons && !cpTrue, Reals];

        simplePrint[cpTrue, cpFalse];

        {trueMap, falseMap} = Map[(createMap[#, isParameter,hasParameter, {}])&, {cpTrue, cpFalse}];
        trueMap = Map[(Cases[#, Except[{{prev[_, _], _}, _, _}] ])&, trueMap];
        falseMap = Map[(Cases[#, Except[{{prev[_, _], _}, _, _}] ])&, falseMap];
        simplePrint[trueMap, falseMap];
        {trueMap, falseMap}
      ]
    ]
  ]
];



(* 変数もしくは記号定数とその値に関する式のリストを，表形式に変換 *)

createVariableMap[] := createVariableMap[constraint && pConstraint, variables];
 
publicMethod[
  createVariableMap,
  cons, vars,
  Block[
    {ret},
    ret = createMap[cons, isVariable, hasVariable, vars];
    debugPrint["ret after CreateMap", ret];
    ret = Map[(Cases[#, Except[{parameter[___], _, _}] ])&, ret];
    ret = ruleOutException[ret];
    simplePrint[ret];
    ret
  ]
];


createVariableMapInterval[] := createVariableMapInterval[constraint, variables, parameters];

publicMethod[
  createVariableMapInterval,
  cons, vars, pars,
  Block[
    {sol, originalOther, otherCons, dVars, tStore, cStore, i, ret},
    sol = exDSolve[cons, vars];
    debugPrint["sol after exDSolve", sol];
    If[sol[[1]] === underconstraint,
      underconstraint,
      
      (* 微分方程式が解けた場合は，微分方程式を解くのに使われなかった式との整合性を調べる *)
      originalOther = And@@sol[[2]];
      cStore = False;
      dVars = sol[[3]];
      otherVars = sol[[4]];
      For[i = 1, i <= Length[sol[[1]] ], i++,
        tStore = Map[(# -> createIntegratedValue[#, sol[[1]] [[i]]])&, dVars];
        cStore = Or[cStore, (originalOther /. tStore) && And@@Map[(Equal@@#)&, tStore] ]
      ];
      simplePrint[cStore];
      ret = createMap[cStore && t>0, isVariable, hasVariable, vars];
      debugPrint["ret after CreateMap", ret];
      ret = ruleOutException[ret];
      simplePrint[ret];
      ret
    ]
  ]
];


ruleOutException[list_] := Block[
  {ret},
  ret = Map[(Cases[#, {{_?isVariable, _}, _, _} ])&, list];
  ret = Map[(Cases[#, Except[{{t, 0}, _, _}] ])&, ret];
  ret = Map[(Cases[#, Except[{{prev[_, _], _}, _, _}] ])&, ret];
  ret
];


createParameterMap[] := createParameterMap[pConstraint];

publicMethod[
  createParameterMap,
  pcons,
  createMap[pcons, isParameter, hasParameter, {}];
]

createMap[cons_, judge_, hasJudge_, vars_] := Block[
  {map},
  If[cons === True || cons === False, 
    cons,
  
    map = Reduce[Exists[Evaluate[Cases[vars, prev[_,_]]], cons], vars, Reals];
    debugPrint["@createMap map after Reduce", map];
    map = map /. (expr_ /;((Head[expr] === Equal || Head[expr] === LessEqual || Head[expr] === Less|| Head[expr] === GreaterEqual || Head[expr] === Greater) && !hasJudge[expr]) -> True);
    map = LogicalExpand[map];
    map = applyListToOr[map];
    map = Map[(applyList[#])&, map];
    debugPrint["@createMap map after applyList", map];
    
    map = Map[(convertExprs[ adjustExprs[#, judge] ])&, map];
    map
  ]
];


(* 式中に変数名が出現するか否か *)

hasVariable[exprs_] := Length[StringCases[ToString[exprs], "usrVar" ~~ LetterCharacter]] > 0;

(* 式が変数もしくはその微分そのものか否か *)

isVariable[exprs_] := StringMatchQ[ToString[exprs], "usrVar" ~~ LetterCharacter__] || MatchQ[exprs, Derivative[_][_][_] ] || MatchQ[exprs, Derivative[_][_] ] ;

(* 式中に出現する変数を取得 *)

getVariables[exprs_] := ToExpression[StringCases[ToString[exprs], "usrVar" ~~ LetterCharacter..]];


(* 式中に定数名が出現するか否か *)

hasParameter[exprs_] := Length[StringCases[ToString[exprs], "parameter[" ~~ LetterCharacter]] > 0;

isParameter[exprs_] := Head[exprs] === parameter;

(* 式が定数そのものか否か *)

hasVariable[exprs_] := Length[StringCases[ToString[exprs], "usrVar" ~~ LetterCharacter]] > 0;



(* 必ず関係演算子の左側に変数名や定数名が入るようにする *)

adjustExprs[andExprs_, judgeFunction_] := 
  Fold[
    (
     If[#2 === True,
      #1,
      If[Not[judgeFunction[#2[[1]] ] ] && judgeFunction[#2[[2]] ],
       (* 逆になってるので、演算子を逆にして追加する *)
       Append[#1, getReverseRelop[Head[#2] ][#2[[2]], #2[[1]] ] ],
       Append[#1, #2]]
     ]) &,
    {},
    andExprs
  ];


resetConstraint[] := (
  constraint = True;
  pConstraint = True;
  prevConstraint = {};
  tmpConstraint = True;
  variables = tmpVariables = prevVariables = {};
  isTemporary = False;
  guard = True;
  guardVars = {};
  parameters = {};
);


addConstraint[co_, va_] := Block[
  {cons, vars},
  cons = co;
  cons = cons /. prevConstraint;
  vars = va;
  If[isTemporary,
    tmpVariables = Union[tmpVariables, vars];
    (* tmpConstraint = Reduce[Exists[Evaluate[prevVariables], prevConstraint && tmpConstraint && cons], tmpVariables, Reals], *)
    tmpConstraint = tmpConstraint && cons,
    variables = Union[variables, vars];
    (* constraint = Reduce[Exists[Evaluate[prevVariables], prevConstraint && constraint && cons], variables, Reals] *)
    constraint = constraint && cons
  ];
  simplePrint[cons, vars, constraint, variables, tmpConstraint, tmpVariables];
];


addPrevConstraint[co_, va_] := Block[
  {cons, vars},
  cons = co;
  vars = va;
  If[cons =!= True,
    prevConstraint = Union[prevConstraint, Map[(Rule@@#)&, applyList[cons]]]
  ];
  prevVariables = Union[prevVariables, vars];
  simplePrint[cons, vars, prevConstraint, prevVariables];
];

addVariables[vars_] := (
  If[isTemporary,
    tmpVariables = Union[tmpVariables, vars],
    variables = Union[variables, vars]
  ];
  simplePrint[vars, variables, tmpVariables];
);

setGuard[gu_, vars_] := (
  guard = gu;
  guardVars = vars;
  simplePrint[ gu, vars, guard, guardVars];
);

startTemporary[] := (
  isTemporary = True;
);

endTemporary[] := (
  isTemporary = False;
  resetTemporaryConstraint[];
);

resetTemporaryConstraint[] := (
  tmpConstraint = True;
  tmpVariables = {};
  guard = True;
  guardVars = {};
);


addParameterConstraint[pcons_, pars_] := (
  pConstraint = Reduce[pConstraint && pcons, Reals];
  parameters = Union[parameters, pars];
  simplePrint[pConstraint, pars];
);


(* 変数名からDerivativeやtを取り，微分回数とともに返す *)
removeDash[var_] := Block[
   {ret},
   If[Head[var] === parameter, Return[var]];
   ret = var /. x_[t] -> x;
   If[MatchQ[Head[ret], Derivative[_]],
     ret /. Derivative[d_][x_] -> {x, d},
     {ret, 0}
   ]
];

(* AndではなくListでくくる *)

applyList[reduceSol_] :=
  If[Head[reduceSol] === And, List @@ reduceSol, List[reduceSol]];


(* OrではなくListでくくる *)

applyListToOr[reduceSol_] :=
  If[Head[reduceSol] === Or, List @@ reduceSol, List[reduceSol]];


(* Piecewiseの第二要素を，その条件とともに第一要素に付加してリストにする．条件がFalseなら削除 *)

makeListFromPiecewise[minT_, others_] := Block[
  {tmpCondition = False},
  tmpCondition = Or @@ Map[(#[[2]])&, minT[[1]]];
  tmpCondition = Reduce[And[others, Not[tmpCondition]], Reals];
  If[ tmpCondition === False,
    minT[[1]],
    Append[minT[[1]], {minT[[2]], tmpCondition}]
  ]
];


(*
 * 次のポイントフェーズに移行する時刻を求める
 *)
 
calculateNextPointPhaseTime[maxTime_, discCause_] := 
  calculateNextPointPhaseTime[maxTime, discCause, constraint, pConstraint, variables];


(* 時刻と条件の組で，条件が論理和でつながっている場合それぞれに分解する *)

divideDisjunction[timeCond_] := Map[({timeCond[[1]], #})&, List@@timeCond[[2]]];

(* 変数とその値に関する式のリストを、変数表的形式に変換 *)
getExprCode[expr_] := Switch[Head[expr],
  Equal, 0,
  Less, 1,
  Greater, 2,
  LessEqual, 3,
  GreaterEqual, 4
];


replaceIntegerToString[num_] := (If[num < 0, minus[IntegerString[num]], IntegerString[num] ]);
integerString[expr_] := (
  expr /. (x_ :> ToString[InputForm[x]] /; Head[x] === Root )
       /. (x_Rational :> Rational[replaceIntegerToString[Numerator[x] ], replaceIntegerToString[Denominator[x] ] ] )
       /. (x_Integer :> replaceIntegerToString[x])
);


(* リストを整形する *)
(* FullSimplifyを使うと，Root&Functionが出てきたときにも結構簡約できる．というか簡約できないとエラーになるのでTODOと言えばTODO *)
(* TODO:複素数の要素に対しても，任意精度への対応（文字列への変換とか）を行う *)

convertExprs[list_] := Map[({removeDash[ #[[1]] ], getExprCode[#], integerString[FullSimplify[#[[2]] ] ] } )&, list];

(*
calculateNextPointPhaseTime[maxTime_, discCause_, cons_, pCons_, vars_] := Check[
  Block[
    {
      dSol,
      timeAppliedCauses,
      resultList,
      originalOther
    },
    
    (* まず微分方程式を解く．うまくやればcheckConsistencyIntervalで出した結果(tStore)をそのまま引き継ぐこともできるはず *)
    dSol = exDSolve[cons, vars];
    
    debugPrint["dSol after exDSolve", dSol];
    
    (* 次にそれらをdiscCauseに適用する *)
    dVars = dSol[[3]];
    timeAppliedCauses = False;
    For[i = 1, i <= Length[dSol[[1]] ], i++,
      tStore = Map[(# -> createIntegratedValue[#, dSol[[1]] [[i]]])&, dVars];
      timeAppliedCauses = Or[timeAppliedCauses, Or@@discCause /. tStore ]
    ];
    
    timeAppliedCauses = Reduce[timeAppliedCauses, {t}, Reals];
    
    simplePrint[timeAppliedCauses];
    
    
    
    (* 最後に，あらかじめ求められているはずのotherConsとpconsを付加してMinimize *)  
    
    resultList = Quiet[Minimize[{t, (timeAppliedCauses || t == maxTime) && pCons && t>0}, {t}], 
                           {Minimize::wksol, Minimize::infeas}];
    debugPrint["resultList after Minimize", resultList];
    resultList = First[resultList];
    If[Head[resultList] === Piecewise, resultList = makeListFromPiecewise[resultList, pCons], resultList = {{resultList, pCons}}];
    
    (* 整形して結果を返す *)
    resultList = Map[({#[[1]],LogicalExpand[#[[2]] ]})&, resultList];
    resultList = Fold[(Join[#1, If[Head[#2[[2]]]===Or, divideDisjunction[#2], {#2}]])&,{}, resultList];
    resultList = Map[({#[[1]], Cases[applyList[#[[2]] ], Except[True]] })&, resultList];
    
    debugPrint["resultList after Format", resultList];
    
    resultList = Map[({integerString[FullSimplify[#[[1]] ] ], convertExprs[adjustExprs[#[[2]], isParameter ] ], If[Quiet[Reduce[ForAll[Evaluate[parameters], And@@#[[2]], #[[1]] >= maxTime]], {Reduce::useq}] =!= False, 1, 0]})&, resultList];
    simplePrint[resultList];
    {1, resultList}
  ],
  debugPrint[$MessageList]; {0}
];
*)


publicMethod[
  calculateNextPointPhaseTime,
  maxTime, discCause, cons, pCons, vars,
  Block[
    {
      addMinTime, selectCondTime,
      sol, minT, paramVars, compareResult, resultList, condTimeList,
      calculateMinTimeList, convertExpr, removeInequalityInList, findMinTime, compareMinTime,
      compareMinTimeList, divideDisjunction
    },
    (* 条件を満たす最小の時刻と，その条件の組を求める *)
    (* maxTは理想的には無くても可能だが，あった方が事故がおきにくいのと高速化が見込めるかもしれないため追加 *)
    findMinTime[ask_, condition_, maxT_] := (
    
      sol = Quiet[Check[Reduce[ask&&condition&&t>0&&maxT>t, t, Reals],
                        errorSol,
                        {Reduce::nsmet}],
                  {Reduce::nsmet}];
      If[sol=!=False && sol=!=errorSol, 
        (* true *)
        (* 成り立つtの最小値を求める *)
        minT = First[Quiet[Minimize[{t, sol}, {t}], 
                           Minimize::wksol]],
        (* false *)
        minT = error
      ];
      If[minT === error,
        {},
        (* Piecewiseなら分解*)
        If[Head[minT] === Piecewise, minT = makeListFromPiecewise[minT, condition], minT = {{minT, condition}}];
        (* 時刻が0となる場合を取り除く．安全のためにあった方が良いが，理想的には無くても動くはず？ *)
        minT = Select[minT, (#[[1]] =!= 0)&];
        minT
      ]
    );
    
    
    unifyCases[{}] := {};
    (* 時刻と条件の組のリストを見て，時刻が重複しているものは結合する *)
    unifyCases[{h_, t___}] := ( Block[
        {select, result, next},
        select = Select[{t}, (#[[1]] === h[[1]])&];
        next = Complement[{t}, select];
        select = Or@@Map[(#[[2]])&, select];
        result = {h[[1]], Reduce[Or[h[[2]], select]]};
        Append[unifyCases[next], result]
      ]
    );
    
    (* ２つの時刻と条件の組を比較し，最小時刻とその条件の組のリストを返す *)
    compareMinTime[timeCond1_, timeCond2_] := ( Block[
        {
          case1, case2,
          andCond
        },
        andCond = Reduce[timeCond1[[2]]&&timeCond2[[2]], Reals];
        If[andCond === False, Return[{}] ];
        case1 = Quiet[Reduce[And[andCond,timeCond1[[1]] < timeCond2[[1]]], Reals]];
        If[ case1 === False, Return[{{timeCond2[[1]], andCond}} ] ];
        case2 = Reduce[andCond&&!case1];
        If[ case2 === False, Return[{{timeCond1[[1]], andCond}} ] ];
        Return[ {{timeCond2[[1]],  case2}, {timeCond1[[1]], case1}} ];
      ]
    );
    
    (* ２つの時刻と条件の組のリストを比較し，各条件組み合わせにおいて，最小となる時刻と条件の組のリストを返す *)
    compareMinTimeList[list1_, list2_] := ( Block[
        {resultList, i, j},
        If[list2 === {}, Return[list1] ];
        resultList = {};
        For[i = 1, i <= Length[list1], i++,
          For[j = 1, j <= Length[list2], j++,
            resultList = Join[resultList, compareMinTime[list1[[i]], list2[[j]] ] ]
          ]
        ];
        resultList
      ]
    );

    
    (* 最小時刻と条件の組をリストアップする関数 *)
    calculateMinTimeList[guardList_, condition_, maxT_] := (
      Block[
        {findResult, i},
        timeCaseList = {{maxT, condition}};
        For[i = 1, i <= Length[guardList], i++,
          findResult = findMinTime[guardList[[i]], (condition), maxT];
          timeCaseList = compareMinTimeList[timeCaseList, findResult];
          timeCaseList = unifyCases[timeCaseList]
        ];
        timeCaseList
      ]
    );
    
    (*  リストからInequalityを除く *)
    removeInequalityInList[{}] := {};
    removeInequalityInList[{h_,t___}] := ( Block[
        {
          resultList
        },
        If[Head[h] === Inequality,
          resultList = Join[{Reduce[h[[2]][h[[3]], h[[1]]], h[[3]]]},
               {h[[4]][h[[3]], h[[5]]]}
          ],
          If[h === True,(* ついでにTrueも除く *)
            resultList = {},
            resultList = {h}
          ];
        ];
        Join[resultList, removeInequalityInList[{t}]]
      ]
    );
    
    
    
    (* 時刻と条件の組で，条件が論理和でつながっている場合それぞれに分解する *)
    divideDisjunction[timeCond_] := Map[({timeCond[[1]], #})&, List@@timeCond[[2]]];
    
    
    (* まず微分方程式を解く．うまくやればcheckConsistencyIntervalで出した結果(tStore)をそのまま引き継ぐこともできるはず *)
    dSol = exDSolve[cons, vars];
    
    debugPrint["dSol after exDSolve", dSol];
    
    (* 次にそれらをdiscCauseに適用する *)
    dVars = dSol[[3]];
    timeAppliedCauses = False;
    For[i = 1, i <= Length[dSol[[1]] ], i++,
      tStore = Map[(# -> createIntegratedValue[#, dSol[[1]] [[i]]])&, dVars];
      timeAppliedCauses = Or[timeAppliedCauses, Or@@discCause /. tStore ]
    ];
    
    timeAppliedCauses = Reduce[timeAppliedCauses, {t}, Reals];
    timeAppliedCauses = applyListToOr[LogicalExpand[timeAppliedCauses]];
    
    simplePrint[timeAppliedCauses];
    
    
    (* 最小時刻と条件の組のリストを求める *)
    resultList = calculateMinTimeList[timeAppliedCauses, pCons, maxTime];
    

    (* 整形して結果を返す *)
    resultList = Map[({#[[1]],LogicalExpand[#[[2]]]})&, resultList];
    resultList = Fold[(Join[#1, If[Head[#2[[2]]]===Or, divideDisjunction[#2], {#2}]])&,{}, resultList];
    resultList = Map[({#[[1]], applyList[#[[2]]]})&, resultList];
    debugPrint["resultList after Format", resultList];
    resultList = Map[({integerString[FullSimplify[#[[1]] ] ],
      convertExprs[adjustExprs[#[[2]], isParameter ] ],
      If[Quiet[Reduce[ForAll[Evaluate[parameters], 
      And@@#[[2]], #[[1]] >= maxTime]], {Reduce::useq}] =!= False, 1, 0]})&, resultList];
      
    simplePrint[resultList];
    resultList
  ]
];

getDerivativeCount[variable_[_]] := 0;

getDerivativeCount[Derivative[n_][f_][_]] := n;


createIntegratedValue[variable_, integRule_] := (
  Simplify[
    variable /. Map[(Rule[#[[1]] /. x_[t]-> x, #[[2]]])&, integRule]
             /. Derivative[n_][f_] :> D[f, {t, n}] 
             /. x_[t] -> x]
);


(* 変数についての制約のうち，微分方程式を解ければ解く． 
   微分方程式を解くにあたって邪魔になる式は解かずに，返り値の要素の第2要素とする
   制約が多すぎて微分方程式が解けない場合は，Falseを
   制約が少なすぎて値が決定できない場合は，Trueを返すものとする
   デフォルト連続性を信頼して，dvnulの可能性は考えないことにする
   @return 考えられる解と，無視した式の組のリスト
    *)

exDSolve[expr_, vars_] := 
Quiet[
  Block[
    {sol, dExpr, dVars, otherExpr, otherVars, tmpSol, lenSol},
    
    sol = expr;
    
    sol = sol /. (e_ /;((Head[e] === Equal || Head[e] === LessEqual || Head[e] === Less|| Head[e] === GreaterEqual || Head[e] === Greater) && !hasJudge[hasVariable]) -> True);
    sol = LogicalExpand[sol];
    debugPrint["@exDSolve sol before separete", sol]; 
    If[Head[sol]===Or, 
       lenSol = Length[sol];

       For[i=1,i<=lenSol,i+=1,
	   tmpSol = applyList[sol[[i]]];

	   debugPrint["@exDSolve i,sol[[i]] before splitExprs",i, sol[[i]]];

	   {dExpr, dVars, otherExpr, otherVars} = splitExprs[tmpSol];

	   debugPrint["@exDSolve i,sol[[i]] after splitExprs",i, dExpr, dVars, otherExpr, otherVars];

	   If[dExpr === {},
	      (* 微分方程式が存在しない *)
	      Return[{underConstraint, otherExpr}]
	   ];
	   Check[
	       Check[
	          tmpSol = DSolve[dExpr, dVars, t];
		  If[Reduce[(sol[[i]]/.tmpSol)[[1]]&&t>=0,Append[vars,t],Reals] =!= False,
		     Return[{tmpSol,otherExpr,dVars,otherVars}]
		  ],
		  If[i === lenSol,
		     Return[{underConstraint, otherExpr}]
		  ],
	          {DSolve::underdet, Solve::svars}
	       ],
	       If[i === lenSol,
		   Return[{overConstraint, otherExpr}]
	       ],
               {DSolve::overdet}
	   ];
	   debugPrint["@exDSolve i,tmpSol in Or loop",i,tmpSol]
       ];
       
	  (*sol = First[sol]*)
    ];
    
    sol = applyList[sol];
    
    debugPrint["@exDSolve before splitExprs", sol];
    
    {dExpr, dVars, otherExpr, otherVars} = splitExprs[sol];
    
    debugPrint["@exDSolve after splitExprs", dExpr, dVars, otherExpr, otherVars];
    
    If[dExpr === {},
      (* 微分方程式が存在しない *)
      Return[{underConstraint, otherExpr}]
    ];
    Check[
      Check[
        sol = DSolve[dExpr, dVars, t];
        {sol, otherExpr, dVars, otherVars},
        {underConstraint, otherExpr},
        {DSolve::underdet, Solve::svars}
      ],
      {overConstraint, otherExpr},
      {DSolve::overdet}
    ]
  ]
];


(* 微分方程式を解くにあたり，邪魔になる式（下記）を除く
   ・不等式
   ・同じ変数についての，初期値で無い言及のうち，重複するもの（例：x[t] == 1 && x'[t] == 0のx'[t] == 0）
     ※ただし，他に新規変数が出現している場合は省かない（例：x[t] == 1 && x'[t] == y'[t]）
   除いた式は第3要素として返す．
   第2要素と第4要素はそれぞれ，必要な式と邪魔な式に含まれる変数のリスト *)


(*
splitExprs[expr_] := Block[
  {dExprs, appendedTimeVars, dVars, iter, otherExprs, otherVars, getTimeVars, getNoInitialTimeVars, timeVars, exprStack},
  
  getTimeVars[list1_, list2_] := Union[list1, Cases[list2, _[t] | _[0], Infinity] /. x_[0] -> x[t]];
  getNoInitialTimeVars[list_] := Union[Cases[list, _[t], Infinity] /. Derivative[_][f_][_] -> f[t]];
  
  (* TODO: iterとか使っちゃダメ *)
  otherExprs = dExprs = appendedTimeVars = {};
  exprStack = List@@expr;
  iter = 0;
  While[Length[exprStack] > 0,
    iter++;
    timeVars = getNoInitialTimeVars[exprStack[[1]] ];
    (* Print["exprStack:", exprStack, ", otherExprs:", otherExprs, ", dExprs:", dExprs, ", timeVars:", timeVars, ", appendedTimeVars:", appendedTimeVars]; *)
    If[Length[Select[timeVars, (FreeQ[appendedTimeVars, #])& ] ] > 1 && iter<Length[exprStack] ,
      exprStack = Append[exprStack, exprStack[[1]] ],
      iter = 0;
      If[Head[exprStack[[1]] ] =!= Equal || (Length[timeVars] > 0 && Length[Select[timeVars, (FreeQ[appendedTimeVars, #])& ] ] == 0),
        otherExprs = Append[otherExprs, exprStack[[1]] ],
        dExprs = Append[dExprs, exprStack[[1]] ];
        appendedTimeVars = Union[appendedTimeVars, timeVars]
      ];
    ];
    exprStack = Delete[exprStack, 1]
  ];
  
  (* Print["dExprs:", dExprs, ", otherExprs:", otherExprs]; *)
  dVars = Fold[(getTimeVars[#1,#2])&, {}, dExprs];
  otherVars = Fold[(getTimeVars[#1,#2])&, {}, otherExprs];
  {dExprs, dVars, otherExprs, otherVars}
];
*)


(* DSolveで扱える式 とそれ以外 （otherExpr）に分ける *)
(* 微分値を含まず/////変数が2種類以上出るや等式以外はDSolveで扱えない *)
splitExprs[expr_] := Block[
  {dExprs, dVars, otherExprs, otherVars},
  
  getTimeVars[list1_, list2_] := Union[list1, Cases[list2, _[t] | _[0], Infinity] /. x_[0] -> x[t]];

  otherExprs = Select[expr, 
                  (Head[#] =!= Equal || MemberQ[#, Derivative[n_][x_][t], Infinity] =!= True && Length[Union[Cases[#, _[t], Infinity]]] > 1) &];      
  otherVars = Fold[(getTimeVars[#1,#2])&, {}, otherExprs];

  dExprs = Complement[expr, otherExprs];
  dVars = Union[Fold[(Join[#1, Cases[#2, _[t] | _[0], Infinity] /. x_[0] -> x[t]]) &, 
                         {}, dExprs]];
  {dExprs, dVars, otherExprs, otherVars}
];



(*
 * 式に対して与えられた時間を適用する
 *)

publicMethod[
  applyTime2Expr,
  expr, time,
  Block[
    {appliedExpr},
    appliedExpr = FullSimplify[(expr /. t -> time)];
    If[Element[appliedExpr, Reals] =!= False,
      integerString[appliedExpr],
      Message[applyTime2Expr::nrls, appliedExpr]
    ]
  ]
];

applyTime2Expr::nrls = "`1` is not a real expression.";

(*
 * 与えられた式を近似する
 *)
publicMethod[
  approxExpr,
  precision, expr,
  integerString[
    Rationalize[
      N[Simplify[expr], precision + 3],
      Divide[1, Power[10, precision]]
    ]
  ]
];
              


(* 
 * 与えられたtの式をタイムシフト
 *)

publicMethod[
  exprTimeShift,
  expr, time,
  integerString[Simplify[expr /. t -> t - time ]]
];