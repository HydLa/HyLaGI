$RecursionLimit = 1000;
$MaxExtraPrecision = 1000;

(*
 * global variables
 * constraint: 現在のフェーズでの制約
 * pConstraint: 定数についての制約
 * prevConstraint: 左極限値を設定する制約
 * initConstraint: 初期値制約
 * variables: 制約に出現する変数のリスト
 * parameters: 記号定数のリスト
 * isTemporary：制約の追加を一時的なものとするか
 * tmpConstraint: 一時的に追加された制約
 * initTmpConstraint: 一時的に追加された初期値制約
 * tmpVariables: 一時制約に出現する変数のリスト
 * guard:
 * guardVars:
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

dList = {};
profileList = {};
createMapList = {};

(* 想定外のメッセージが出ていないかチェック．出ていたらそこで終了．
 想定外の形式の結果になって変な計算を始めてエラーメッセージが爆発することが無いようにするため．
 あまり良い形の実装ではなく，publicMethodにだけ書いておく形で実装できるなら多分それが設計的に一番良いはず．
 現状では，危ないと思った個所に逐一挟んでおくことになる． *)
If[optIgnoreWarnings,
  checkMessage := (If[Length[Cases[$MessageList, Except[HoldForm[Minimize::ztest1], Except[HoldForm[Reduce::ztest1] ] ] ] ] > 0, Print[FullForm[$MessageList]];Abort[]]),
  checkMessage := (If[Length[$MessageList] > 0, Abort[] ])
];

publicMethod::timeout = "Calculation has reached to timeout";

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
 
inputPrint[name_, arg___] := Print[StringJoin[name, "[", delimiterAddedString[",", Map[(ToString[InputForm[#] ])&,{arg}] ], "]" ] ];

delimiterAddedString[del_, {h_}] := h;

delimiterAddedString[del_, {h_, t__}] := StringJoin[h, del, delimiterAddedString[del, {t}] ];

SetAttributes[publicMethod, HoldAll];

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
      debugPrint[$MessageList]; {0}
    ]
  )
);
