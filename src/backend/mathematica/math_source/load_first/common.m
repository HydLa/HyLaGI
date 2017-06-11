(* This file must be loaded first! *)

$RecursionLimit = 1000;
$MaxExtraPrecision = 1000;

(*
 * global variables
 * constraint: 現在のフェーズでの制約
 * assumptions: assumptions for the current phase
 * pConstraint: 定数についての制約
 * prevConstraint: constraint for prevs
 * resultConstraint: constraint solvedBy checkConsystency
 * prevRules:      rules converted from equalities of left-hand limits
 * currentTime:    symbolic expression of current time 
 * initConstraint: 初期値制約
 * variables: プログラム内に出現する変数のリスト
 * prevVariables: variables内の変数をux=>prev[x, 0]のようにしたもの
 * timeVariables: variables内の変数を，ux[t]のようにしたもの
 * parameters: 使用する記号定数のリスト
 * isTemporary：制約の追加を一時的なものとするか
 * tmpConstraint: 一時的に追加された制約
 * initTmpConstraint: 一時的に追加された初期値制約
 * startTimes: 呼び出された関数の開始時刻を積むプロファイリング用スタック
 * profileList: プロファイリング結果のリスト
 * dList: 微分方程式とその一般解を保持するリスト {微分方程式のリスト, その一般解, 変数の置き換え規則}
 * createMapList: createMap関数への入力と出力の組のリスト
 * opt...: 各種オプションのON/OFF．
 *)


variables = {};
prevVariables = {};
timeVariables = {};
parameters = {};
dList = {};
profileList = {};
createMapList = {};
assumptions = True;
optTimeConstraint = 1;
optSimplifyLevel = 1;

(* 想定外のメッセージが出ていないかチェック．出ていたらそこで終了．*)
If[optIgnoreWarnings,
  checkMessage := (If[Length[Select[$MessageList, (FreeQ[{HoldForm[Solve::incnst], HoldForm[Solve::ifun], HoldForm[Minimize::ztest1], HoldForm[Reduce::ztest1], HoldForm[Reduce::ztest], HoldForm[Minimize::ztest], HoldForm[DSolve::bvnul], HoldForm[General::stop]}, #])&] ] > 0, Abort[]]),
  checkMessage := (If[Length[$MessageList] > 0, Abort[] ])
];

publicMethod::timeout = "Calculation has reached to timeout";

$MessagePrePrint = InputForm;

(*
 * プロファイリング用関数
 * timeFuncStart: startTimesに関数の開始時刻を積む
 * timeFuncEnd: startTimesから開始時刻を取り出し、profileListにプロファイル結果を格納
 * <使い方>
 *    プロファイリングしたい関数の定義の先頭にtimeFuncStart[];を
 *    末尾でtimeFuncEnd["関数名"];を追加する.
 *    ただしtimeFuncEndの後で値を返すようにしないと返値が変わってしまうので注意.
 * <プロファイリング結果の見方>
 *    (現在実行が終了した関数名) took (その関数実行に要した時間), elapsed time:(プログラム実行時間)
 *      function:(今までに呼び出された関数名)  calls:(呼び出された回数)  total time of this function:(その関数の合計実行時間)  average time:(その関数の平均実行時間)  max time:(その関数の最高実行時間)
 *    <例>
 *    calculateNextPointPhaseTime took 0.015635, elapsed time:1.006334
 *      function:checkConsistencyPoint  calls:1  total time of this function:0.000361  average time:0.000361  max time:0.000361
 *      function:createMap  calls:2  total time of this function:0.11461  average time:0.057304  max time:0.076988
 *      ...
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
  If[Position[profileList, funcname] =!= {},
    funcidx = Flatten[Position[profileList,funcname]][[1]];
    profileList[[funcidx,2]] = profileList[[funcidx,2]] + 1;
    profileList[[funcidx,3]] = profileList[[funcidx,3]] + (endTime-startTime);
    profileList[[funcidx,4]] = profileList[[funcidx,3]] / profileList[[funcidx,2]];
    profileList[[funcidx,5]] = If[profileList[[funcidx,5]]<(endTime-startTime), endTime-startTime, profileList[[funcidx,5]]];
  ,
    profileList = Append[profileList, {funcname, 1, endTime-startTime, endTime-startTime, endTime-startTime}];
  ];
  profilePrint[funcname," took ",endTime-startTime,", elapsed time:",endTime];
  For[i=1,i<=Length[profileList],i=i+1,
    profilePrint["    function:",profileList[[i,1]],"  calls:",profileList[[i,2]],"  total time of this function:",profileList[[i,3]],"  average time:",profileList[[i,4]],"  max time:",profileList[[i,5]]];
  ];
];
);

(*
 * デバッグ用メッセージ出力関数
 * debugPrint：引数として与えられた要素要素を文字列にして出力する． （シンボルは評価してから表示）
 * simplePrint：引数として与えられた式を， 「（評価前）:（評価後）」の形式で出力する．
 *)
 
SetAttributes[simplePrint, HoldAll];

symbolToString := (StringJoin[ToString[Unevaluated[#] ], ": ", ToString[InputForm[Evaluate[#] ] ] ])&;

SetAttributes[symbolToString, HoldAll];

SetAttributes[prev, Constant];
SetAttributes[p, Constant];

SetAttributes[prev, NHoldAll];
SetAttributes[p, NHoldAll];

If[True,  (* エラーが起きた時の対応のため，常にdebugPrintを返すようにしておく *)
  debugPrint[arg___] := Print[InputForm[{arg}]];
  simplePrint[arg___] := Print[delimiterAddedString[", ",
    List@@Map[symbolToString, Map[Unevaluated, Hold[arg]] ]
     ] ],
  
  debugPrint[arg___] := Null;
  simplePrint[arg___] := Null
];

profilePrint[arg___] := If[optUseProfilePrint, Print[InputForm[arg]], Null];

oin(*
 * 関数呼び出しを再現するための文字列出力を行う
 *)

inputPrint[name_] := Print[StringJoin[name, "[]"]];
 
inputPrint[name_, arg__] := Print[StringJoin[name, "[", delimiterAddedString[",\n\t", Map[(ToString[InputForm[#] ])&,{arg}] ], "]" ] ];

delimiterAddedString[del_, {h_}] := h;

delimiterAddedString[del_, {h_, t__}] := StringJoin[h, del, delimiterAddedString[del, {t}] ];

SetAttributes[publicMethod, HoldAll];

(* MathLinkでDerivativeの送信がややこしいのでこっちで変換する *)

derivativeInit[cnt_, var_] := Derivative[cnt][var][0];
derivativeTime[cnt_, var_] := Derivative[cnt][var][t];
derivative[0, var_] := var;
derivative[cnt_, var_] := Derivative[cnt][ToExpression[StringReplacePart[ToString[var], derivativePrefix, {1,1}] ]];


(* C++側から直接呼び出す関数の，本体部分の定義を行う関数．デバッグ出力とか，正常終了の判定とか，例外の扱いとかを統一する 
   少しでもメッセージを吐く可能性のある関数は，この関数で定義するようにする．
   返り値にはtoReturnFormを常にかけるようにしたいけど，現状だとリストの番号とかまで文字列にすると例外吐かれるので個別対応・・・
*)

publicMethod[name_, args___, definition_] := (
  name[Sequence@@Map[(Pattern[#, Blank[]])&, {args}]] := (
    inputPrint[ToString[name], args];
    CheckAbort[
      timeFuncStart[];
      Module[{publicRet, returnValue = Null},
        returnValue = Do[publicRet = definition, {i, 1, 1}]; (* use Do[] to catch result of Return[] *)
        If[returnValue =!= Null, publicRet = returnValue];
        simplePrint[publicRet];
        timeFuncEnd[name];
        checkMessage;
        {1, publicRet}
      ],
      simplePrint[$MessageList];{0}
    ]
  )
);

publicMethod[
  simplify,
  arg,
  Switch[optSimplifyLevel,
    0, toReturnForm[arg],
    1, toReturnForm[Simplify[arg]],
    _, toReturnForm[FullSimplify[arg]]
  ]
];


toReturnForm[expr_] := 
Module[
  {ret},
  If[expr === Infinity, Return[inf]];
  (* Derivative[cnt, var] is for return form (avoid collision with derivative[cnt, var] *)
  If[MatchQ[expr, Derivative[_][_]], Return[Derivative[expr[[0, 1]], ToExpression[StringDrop[ToString[expr[[1]] ], 1]  ] ] ] ];
  If[MatchQ[expr, Derivative[_][_][t_]], Return[Derivative[expr[[0, 0, 1]],  ToExpression[StringDrop[ToString[expr[[1]] ], 1] ] ] ] ];
  If[MatchQ[expr, _[t]] && isVariable[Head[expr] ], Return[Head[expr] ] ];
  If[Head[expr] === Real, Return[ToString[expr] ] ];
  If[Head[expr] === p, Return[expr] ];
   
  ret = ToRadicals[expr];

  (* return Root[] as string. because it's difficult to handle as formulas in C++*)
  If[Head[ret] === Root, Return[ToString[ret] ] ];

  ret = Map[toReturnForm, ret];
  ret = Replace[ret, (x_Rational :> Rational[replaceIntegerToString[Numerator[x] ], replaceIntegerToString[Denominator[x] ] ] )];
  ret = Replace[ret, (x_Integer :> replaceIntegerToString[x])];
  ret
];

toRational[float_] := SetPrecision[float, Infinity];

replaceIntegerToString[num_] := (If[num < 0, minus[IntegerString[num]], IntegerString[num] ]);

removeDash[var_] := Module[
   {ret},
   If[Head[var] === p || Head[var] === prev, Return[var]];
   ret = var /. x_[t] -> x;
   If[MatchQ[Head[ret], Derivative[_]],
     ret /. Derivative[d_][x_] -> {x, d},
     {ret, 0}
   ]
];


(* リストを整形する *)
(* TODO:複素数の要素に対しても，任意精度への対応 （文字列への変換とか）を行う *)

convertExprs[list_] := Map[({removeDash[ #[[1]] ], getExprCode[#], toReturnForm[#[[2]] ] } )&, list];


(* 変数とその値に関する式のリストを、変数表的形式に変換 *)
getExprCode[expr_] := Switch[Head[expr],
  Equal, 0,
  Rule, 0,
  Less, 1,
  Greater, 2,
  LessEqual, 3,
  GreaterEqual, 4
];


(* AndではなくListでくくる *)
applyList[reduceSol_] :=
  If[reduceSol === True, {}, If[Head[reduceSol] === And, List @@ reduceSol, List[reduceSol]]];

(* OrではなくListでくくる *)
applyListToOr[reduceSol_] :=
  If[Head[reduceSol] === Or, List @@ reduceSol, List[reduceSol]];



(* And ではなくandでくくる。条件式の数が１つの場合でも特別扱いしたくないため *)
And2and[reduceSol_] :=
  If[reduceSol === True, and[], If[Head[reduceSol] === And, and @@ reduceSol, and[reduceSol]] ];

(* Or ではなくorでくくる。条件式の数が１つの場合でも特別扱いしたくないため *)
Or2or[reduceSol_] :=
  If[Head[reduceSol] === Or, or @@ reduceSol, or[reduceSol]];

checkAndIgnore[expr_, failExpr_, messages_] := 
Quiet[Check[expr, failExpr, messages], messages];
SetAttributes[checkAndIgnore, HoldAll];

(* （不）等式の右辺と左辺を入れ替える際に，関係演算子の向きも反転させる．Notとは違う *)

getReverseRelop[relop_] := Switch[relop,
                                  Equal, Equal,
                                  Less, Greater,
                                  Greater, Less,
                                  LessEqual, GreaterEqual,
                                  GreaterEqual, LessEqual];

variablePrefix = "u";

(* optSimplifyLevel の値に応じた簡約化関数を呼び出す。optTimeConstraint で指定された秒数が経過した場合、簡約化前の値を返す *)
Switch[optSimplifyLevel,
    0, timeConstrainedSimplify[expr_] := expr;
    1, timeConstrainedSimplify[expr_] := TimeConstrained[Simplify[expr], optTimeConstraint, expr];
    _, timeConstrainedSimplify[expr_] := TimeConstrained[FullSimplify[expr], optTimeConstraint, expr];
];

Switch[optSimplifyLevel,
    0, timeConstrainedSimplify[expr_, assum_] := expr;
    1, timeConstrainedSimplify[expr_, assum_] := TimeConstrained[Simplify[expr, assum], optTimeConstraint, expr];
    _, timeConstrainedSimplify[expr_, assum_] := TimeConstrained[FullSimplify[expr, assum], optTimeConstraint, expr];
];

derivativePrefix = "d";
