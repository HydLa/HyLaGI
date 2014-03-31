(* This file must be loaded first! *)

$RecursionLimit = 1000;
$MaxExtraPrecision = 1000;

(*
 * global variables
 * constraint: 現在のフェーズでの制約
 * pConstraint: 定数についての制約
 * prevIneqs: constraints of inequalities of left-hand limits
 * prevRules:      rules converted from equalities of left-hand limits
 * initConstraint: 初期値制約
 * variables: プログラム内に出現する変数のリスト
 * prevVariables: variables内の変数をusrVarx=>prev[x, 0]のようにしたもの
 * timeVariables: variables内の変数を，usrVarx[t]のようにしたもの
 * initVariables: variables内の変数を，usrVarx[0]のようにしたもの
 * parameters: 使用する記号定数のリスト
 * isTemporary：制約の追加を一時的なものとするか
 * tmpConstraint: 一時的に追加された制約
 * initTmpConstraint: 一時的に追加された初期値制約
 * startTimes: 呼び出された関数の開始時刻を積むプロファイリング用スタック
 * profileList: プロファイリング結果のリスト
 * dList: 微分方程式とその一般解を保持するリスト {微分方程式のリスト, その一般解, 変数の置き換え規則}
 * createMapList: createMap関数への入力と出力の組のリスト
 * timeOutS: タイムアウトまでの時間．秒単位．
 * opt...: 各種オプションのON/OFF．
 * approxMode: 近似モード．
 * approxThreshold: 近似閾値．現在はLeafCountの値で判断している．
 * approxPrecision: 近似精度．
 *)


variables = {};
prevVariables = {};
timeVariables = {};
initVariables = {};
parameters = {};
dList = {};
profileList = {};
createMapList = {};

(* 想定外のメッセージが出ていないかチェック．出ていたらそこで終了．*)
If[optIgnoreWarnings,
  checkMessage := (If[Length[Select[$MessageList, (FreeQ[{HoldForm[Minimize::ztest1], HoldForm[Reduce::ztest1], HoldForm[DSolve::bvnul], HoldForm[General::stop]}, #])&] ] > 0, Abort[]]),
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

SetAttributes[parameter, Constant];

If[optUseDebugPrint || True,  (* エラーが起きた時の対応のため，常にdebugPrintを返すようにしておく．いずれにしろそんなにコストはかからない？ *)
  debugPrint[arg___] := Print[InputForm[{arg}]];
  simplePrint[arg___] := Print[delimiterAddedString[", ",
    List@@Map[symbolToString, Map[Unevaluated, Hold[arg]] ]
     ] ],
  
  debugPrint[arg___] := Null;
  simplePrint[arg___] := Null
];

profilePrint[arg___] := If[optUseProfilePrint, Print[InputForm[arg]], Null];

(*
 * 関数呼び出しを再現するための文字列出力を行う
 *)

inputPrint[name_] := Print[StringJoin[name, "[]"]];
 
inputPrint[name_, arg__] := Print[StringJoin[name, "[", delimiterAddedString[",", Map[(ToString[InputForm[#] ])&,{arg}] ], "]" ] ];

delimiterAddedString[del_, {h_}] := h;

delimiterAddedString[del_, {h_, t__}] := StringJoin[h, del, delimiterAddedString[del, {t}] ];

SetAttributes[publicMethod, HoldAll];

(* MathLinkでDerivativeの送信がややこしいのでこっちで変換する *)

derivative[cnt_, var_] := Derivative[cnt][var];
derivative[cnt_, var_, suc_] := Derivative[cnt][var][suc];


(* C++側から直接呼び出す関数の，本体部分の定義を行う関数．デバッグ出力とか，正常終了の判定とか，例外の扱いとかを統一する 
   少しでもメッセージを吐く可能性のある関数は，この関数で定義するようにする．
   defineにReturnが含まれていると正常に動作しなくなる （Returnの引数がそのまま返ることになる）ので使わないように！
*)

publicMethod[name_, args___, define_] := (
  name[Sequence@@Map[(Pattern[#, Blank[]])&, {args}]] := (
    inputPrint[ToString[name], args];
    CheckAbort[
      TimeConstrained[
        timeFuncStart[];
        Module[{publicRet},
          publicRet = define;
          simplePrint[publicRet];
          timeFuncEnd[name];
          checkMessage;
          {1, publicRet}
        ],
        Evaluate[timeOutS],
        {-1}
      ],
      simplePrint[$MessageList];{0}
    ]
  )
);

publicMethod[
  simplify,
  arg,
  toReturnForm[Simplify[arg]]
];



toReturnForm[expr_] := 
Module[
  {ret},
  If[expr === Infinity, Return[inf]];
  (* Derivative[cnt, var] is for return form (avoid collision with derivative[cnt, var] *)
  If[MatchQ[expr, Derivative[_][_]], Return[Derivative[expr[[0, 1]], expr[[1]] ] ] ];
  If[MatchQ[expr, Derivative[_][_][t_]], Return[Derivative[expr[[0, 0, 1]], expr[[0, 1]] ] ] ];
  If[MatchQ[expr, _[t]] && isVariable[Head[expr] ], Return[Head[expr] ] ];
  If[Head[expr] === p, Return[expr] ];

  ret = Map[toReturnForm, expr];
  ret = Replace[ret, (x_ :> ToString[InputForm[x]] /; Head[x] === Root )];
  ret = Replace[ret, (x_Rational :> Rational[replaceIntegerToString[Numerator[x] ], replaceIntegerToString[Denominator[x] ] ] )];
  ret = Replace[ret, (x_Integer :> replaceIntegerToString[x])];
  ret
];

toRational[float_] := Rationalize[float, 0];

replaceIntegerToString[num_] := (If[num < 0, minus[IntegerString[num]], IntegerString[num] ]);

(* リストを整形する *)
(* TODO:複素数の要素に対しても，任意精度への対応 （文字列への変換とか）を行う *)

convertExprs[list_] := Map[({removeDash[ #[[1]] ], getExprCode[#], toReturnForm[#[[2]] ] } )&, list];


(* 変数とその値に関する式のリストを、変数表的形式に変換 *)
getExprCode[expr_] := Switch[Head[expr],
  Equal, 0,
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


(* （不）等式の右辺と左辺を入れ替える際に，関係演算子の向きも反転させる．Notとは違う *)

getReverseRelop[relop_] := Switch[relop,
                                  Equal, Equal,
                                  Less, Greater,
                                  Greater, Less,
                                  LessEqual, GreaterEqual,
                                  GreaterEqual, LessEqual];
