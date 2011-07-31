(*
 * デバッグ用メッセージ出力V数
 *)
If[optUseDebugPrint,
  debugPrint[arg___] := Print[InputForm[{arg}]],
  debugPrint[arg___] := Null];

(*
 * グローバル変数
 * constraints: 現在扱っている制約集合
 * vars: 制約集合に出現する変数のリスト
 * pars: 制約集合に出現する定数のリスト
 *)

(* ルールのリストを受けて，tについてのルールを除いたものを返す関数 *)
removeRuleForTime[ruleList_] := DeleteCases[ruleList, t -> _];

getInverseRelop[relop_] := Switch[relop,
                                  Equal, Equal,
                                  Less, Greater,
                                  Greater, Less,
                                  LessEqual, GreaterEqual,
                                  GreaterEqual, LessEqual];

(* Inequality[a, relop, x, relop, b]の形を変形する関数 *)
removeInequality[ret_, expr_] := (
  If[Head[expr] === Inequality,
    Join[ret,
         {Reduce[expr[[2]][expr[[1]], expr[[3]]], expr[[3]]]},
         {expr[[4]][expr[[3]], expr[[5]]]}
    ],
    Append[ret, expr]
  ]
);

(* 条件1から条件2を引き，変数表形式にして返す *)
getComplementCondition[cond1_, cond2_] := (
  debugPrint["cond1:", cond1, "cond2:", cond2];
  convertCSToVM[Complement[applyList[cond1[[1]] ], applyList[cond2[[1]] ] ] ]
);

(* 与えられた式の中の，Cに関する制約と，tの下限となる式を抜き出す関数．
 * 全部Andでつながれてるものという前提．
 * Inequalityは無視．
 * IPなのでtに関する等式も無視する．もしCを連続的な値としてtとの等式にしてきた場合は駄目な気がするけど，多分それは無いはず．
 * あと，Cやtに関する制約は，それぞれについて解かれているという前提．こっちは結構怪しいか．
 *)

getValidExpression[expr_, pars_] := Block[
  {underBounds, constraintsForC, listOfC, tmpExpr, parameterConstraints},
  underBounds = {};
  constraintsForC = {};
  parameterConstraints = {};
  listOfC = {};
  For[i=1, i <= Length[expr], i++,
    tmpExpr = expr[[i]];
    (* まず，不等式は全部LessもしくはLessEqualにする． *)
    If[Head[tmpExpr] === Greater || Head[tmpExpr] === GreaterEqual,
      tmpExpr = getInverseRelop[Head[tmpExpr] ] [tmpExpr[[2]], tmpExpr[[1]] ]
    ];
    (* 右辺がtなら下限リストに追加．そうでないなら定数か，そうでないならCについての制約かをチェックする *)
    If[tmpExpr[[2]] === t,
      (* tについては不等式のみを受け付ける *)
      If[Head[tmpExpr] === Less || Head[tmpExpr] === LessEqual,
        underBounds = Append[underBounds, tmpExpr[[1]] ]
      ],
      If[MemberQ[pars, tmpExpr[[1]] ] || MemberQ[pars, tmpExpr[[2]] ], 
        parameterConstraints = Append[parameterConstraints, tmpExpr],
        If[Head[ tmpExpr[[1]] ] === C,
          constraintsForC = Append[constraintsForC, tmpExpr];
          listOfC = Union[listOfC, {tmpExpr[[1]] } ],
          If[Head[ tmpExpr[[2]] ] === C,
            constraintsForC = Append[constraintsForC, tmpExpr];
            listOfC = Union[listOfC, {tmpExpr[[2]] } ]
          ]
        ]
      ]
    ]
  ];
  {underBounds, constraintsForC, listOfC, parameterConstraints}
];

(*
 * 論理積でつながれた各tの下限リストを，再帰で1つずつ調べていく関数．
 * 1つでも定数値に関係なく0以下にできないものがあれば2を返す．
 * そうでなく，1つでも定数値によっては0以下にできないものがあれば3を返す．
 * いずれでもないなら1を返す．
 *)
checkInfForEach[underBounds_, constraintsForC_, otherConstraints_, pars_, listOfC_] := Block[
  {ret, restRet, tmp, expr},
  If[underBounds === {},
    {1},
    expr = underBounds[[1]];
    If[pars==={}&&listOfC==={},
      (* パラメータが無いなら単純に下限で決まる *)
      If[expr>0,
        {2},
        {1}
      ],
      (* 0以下にできるなら，導出できる可能性がある *)
      tmp = Quiet[Reduce[Join[constraintsForC, otherConstraints, {expr <= 0}] , pars ] ];
      If[tmp === False,
        {2},
        (* 必ず導出できるかどうかを判定 *)
        If[Quiet[Reduce[ForAll[pars, And@@otherConstraints, Reduce[Join[constraintsForC, {expr <= 0}] ] ] ] ] =!= False,
          checkInfForEach[Rest[underBounds], constraintsForC, otherConstraints, pars, listOfC],
          restRet = checkInfForEach[Rest[underBounds], constraintsForC, otherConstraints, pars, listOfC];
          Switch[restRet[[1]],
            1, {3, tmp},
            2, {2},
            3, {3, Reduce[restRet[[2]]&&tmp, Reals]}
          ]
        ]
      ]
    ]
  ]
];

(*
 * 論理和でつながれた各tの下限候補を，再帰で1つずつ調べていく関数．
 * 1つでも定数値に関係なく0以下にできるものがあれば{1}を返す．
 * そうでないなら，1つでも定数値によっては0以下にできるものがあれば{3}を返す．
 * いずれでもないなら{2}を返す．
 *)

checkInf[candidates_, constraints_, pars_] := Block[
  {ret, restRet, underBounds, constraintsForC, listOfC},
  If[Length[candidates] == 0,
    {2},
    ret = Fold[removeInequality, {}, candidates[[1]] ];
    (* tの最小値候補を調べるため，tの下限とt以外の変数についての制約と出現する変数のリストを抽出する． *)
    {underBounds, constraintsForC, listOfC, otherConstraints} = getValidExpression[ret, pars];
    otherConstraints = applyList[Reduce[Join[otherConstraints, constraints], Reals]];
    otherConstraints = Fold[removeInequality, {}, otherConstraints ];

    If[underBounds === {},
      (* 候補が存在しなければ導出不可能なので，次を試す *)
      checkInf[Rest[candidates], constraints, pars],
      (* 候補が存在するなら，それを試す．*)
      ret = checkInfForEach[underBounds, constraintsForC, otherConstraints, pars, listOfC ];
      If[ret[[1]] != 2 && Quiet[Reduce[ForAll[pars, And@@constraints, Reduce[otherConstraints] ] ] ] === False,
        If[ret[[1]] == 1,
          ret = {3, And@@otherConstraints},
          ret = {3, Reduce[ret[[2]] && And@@otherConstraints, Reals]}
        ]
      ];
      Switch[ret[[1]],
        1, {1},
          (* 無理なら，次を試す． *)
        2, checkInf[Rest[candidates], constraints, pars ],
        3,
          (* 定数値によっては可能な場合，次以降で{1}があれば{1}を．そうでなければ{3}を返す． *)
          restRet = checkInf[Rest[candidates], constraints, pars];
          Switch[restRet[[1]],
            1, {1},
            2, {3, ret[[2]] },
            3, {3, Reduce[ ret[[2]] || restRet[[2]], Reals ] }
          ]
      ]
    ]
  ]
];


(*
 * 戻り値のリストの先頭：
 *  0 : Solver Error
 *  1 : 充足
 *  2 : 矛盾
 *)

checkConsistencyInterval[expr_, pexpr_, vars_, pars_] :=  
Quiet[
  Check[
    Block[
      {tStore, sol, integGuard, otherExpr, condition},
      debugPrint["expr:", expr, "pexpr", pexpr, "vars:", vars, "pars:", pars, "all", expr, pexpr, vars, pars];
      sol = exDSolve[expr, vars];
      If[sol === overconstraint,
        {2},
        If[sol === underconstraint || sol[[1]] === {} ,
          (* 警告出したりした方が良いかも？ *)
          {1},
          tStore = sol[[1]];
          tStore = Map[(# -> createIntegratedValue[#, tStore])&, vars];
          otherExpr = Fold[(If[Head[#2] === And, Join[#1, List@@#2], Append[#1, #2]])&, {},Simplify[expr]];
          otherExpr = Select[otherExpr, (MemberQ[{Or, Less, LessEqual, Greater, GreaterEqual}, Head[#] ] || # === False)&];


          (* otherExprにtStoreを適用する *)
          otherExpr = otherExpr /. tStore;
          (* まず，t>0で条件を満たす可能性があるかを調べる *)
          sol = LogicalExpand[Quiet[Check[Reduce[{And@@otherExpr && t > 0 && And@@pexpr}, t, Reals],
                        False, {Reduce::nsmet}], {Reduce::nsmet}]];

          If[sol === False,
            {2},
            (* リストのリストにする *)
            sol = applyListToOr[sol];
            sol = Map[applyList, sol];
            (* tの最大下界を0とできる可能性を調べる． *)
            sol = checkInf[sol, pexpr, pars];
            If[sol[[1]] == 3, sol[[2]] = ToString[FullForm[sol[[2]] ] ] ];
            sol
          ]
        ]
      ]
    ],
    {0, $MessageList}
  ]
];

removeP[par_] := First[StringCases[ToString[par], "p" ~~ x__ -> x]];

renameVar[varName_] := Block[
  {renamedVarName, derivativeCount = 0, prevFlag = 0,
   getDerivativeCountPoint, removeUsrVar
  },  

  getDerivativeCountPoint[Derivative[n_][var_]] := n;
  removeUsrVar[var_] := First[StringCases[ToString[var], "usrVar" ~~ x__ -> x]];

  (* 変数名に'がつく場合の処理 *)
  If[MemberQ[{varName}, Derivative[n_][x_], Infinity],
    derivativeCount = getDerivativeCountPoint[varName];
    renamedVarName = removeDash[varName],
    renamedVarName = varName
  ];
  (* 変数名にprevがつく場合の処理 *)
  If[Head[renamedVarName] === prev,
    renamedVarName = First[renamedVarName];
    prevFlag = 1
  ];

  (*変数名の頭についている "usrVar"を取り除く （名前の頭がusrVarじゃないのはinvalidVarか定数名） *)
  If[StringMatchQ[ToString[renamedVarName], "usrVar" ~~ x__],
    (* true *)
    renamedVarName = removeUsrVar[renamedVarName];
    (* この時点で単体の変数名のはず． *)
    If[Length[renamedVarName] === 0,
      {renamedVarName, derivativeCount, prevFlag},
      (* 式形式のものはハネる *)
      invalidVar],
    (* false *)
    If[StringMatchQ[ToString[renamedVarName], "p" ~~ x__],
     (*定数名の頭についている "p"を取り除く 名前の頭がpじゃないのはinvalidVar． *)
      renamedVarName = removeP[renamedVarName];
      (* 定数はprev_flagを-1にすることで表す *)
      {renamedVarName, derivativeCount, -1},
      invalidVar
    ]
  ]
];

(* 変数とその値に関する式のリストを、変数表的形式に変換 *)
getExprCode[expr_] := Switch[Head[expr],
  Equal, 0,
  Less, 1,
  Greater, 2,
  LessEqual, 3,
  GreaterEqual, 4
];

convertCSToVM[] := Block[
  {resultExprs, inequalityVariable},
  formatExprs[exprs_] := (
    If[Cases[exprs, Except[True]]==={},
      (* 式を{（変数名）, （関係演算子コード）, (値のフル文字列)｝の形式に変換する *)
      {},
      
      DeleteCases[Map[({renameVar[#[[1]]], 
                        getExprCode[#], 
                        ToString[FullForm[#[[2]]]]}) &, 
                      Fold[(removeInequality[#1, #2]) &, {}, exprs]],
                  {invalidVar, _, _}]
    ]
  );

  debugPrint["constraints:", constraints];
  If[Head[First[constraints]] === Or,
    resultExprs = Map[(applyList[#])&, List@@First[constraints]],
    resultExprs = Map[(applyList[#])&, constraints]
  ];
  resultExprs = Map[formatExprs, resultExprs];
  debugPrint["resultExprs:", resultExprs];
  resultExprs
];

(* checkConsistency内のReduceの結果得られた解を{変数名, 値}のリスト形式にする *)
createVariableList[Rule[varName_, varValue_], result_] := Block[{
  name
},
  name = renameVar[varName];
  Append[result, pair[name, varValue]]
];

createVariableList[{expr_}, result_] := (
  createVariableList[expr, result]
);

createVariableList[{expr_, others__}, result_] := Block[{
  variableList
},
  variableList = createVariableList[expr, result];
  createVariableList[{others}, variableList]
];

createVariableList[And[expr__], vars_, result_] := Block[
  {sol},
  sol = (Solve[expr, vars])[[1]];
  createVariableList[sol, result]
];

(* Orでつながっている （複数解がある）場合は、そのうちの1つのみを返す？ *)
createVariableList[Or[expr_, others__], vars_, result_] := (
  createVariableList[expr, vars, result]
);

createVariableList[Equal[varName_, varValue_], vars_, result_] := Block[{
  sol
},
  sol = (Solve[Equal[varName, varValue], vars])[[1]];
  createVariableList[sol, result]
];

(* Reduce[{}, {}]のとき *)
createVariableList[True, vars_, result_] := (
  Return[result]
);

(* 式中に変数名が出現するか否か *)
hasVariable[exprs_] := Length[StringCases[ToString[exprs], "usrVar" ~~ LetterCharacter]] > 0;



(* 必ず関係演算子の左側に変数名が入るようにする *)
adjustExprs[andExprs_] := 
  Fold[(If[Not[hasVariable[#2[[1]]]],
          (* true *)
          If[hasVariable[#2[[2]]],
            (* true *)
            (* 逆になってるので、演算子を逆にして追加する *)
            Append[#1, getInverseRelop[Head[#2]][#2[[2]], #2[[1]]]],
            (* false *)
            (* パラメータ制約の場合にここに入る *)
            If[NumericQ[#2[[1]]],
              (* true *)
              (* 逆になってるので、演算子を逆にして追加する *)
              Append[#1, getInverseRelop[Head[#2]][#2[[2]], #2[[1]]]],
              (* false *)
              Append[#1, #2]]],
          (* false *)
          Append[#1, #2]]) &,
       {}, andExprs];


resetConstraint := (
  constraints = {True};
  variables = {};
);

addConstraint[cons_, vars_] := (
  debugPrint["cons:", cons, "vars:", vars];
  constraints = Union[constraints, cons];
  variables = Union[variables, vars];
  debugPrint["constraints:", constraints, "variables:", variables];
);

checkConsistency[] := Block[
  {sol},
  debugPrint["constraints:", constraints, "variables:", variables];
  sol = checkConsistencyByReduce[constraints, variables];
  If[sol[[1]] == 1, constraints = sol[[2]]; sol = {1}  ];
  sol
];

checkConsistencyWithTemporaryConstraint[expr_, vars_] := (
  debugPrint["constraints:", constraints, "variables:", variables, "expr:", expr, "vars", vars];
  { checkConsistencyByReduce[Union[constraints, expr], Union[variables, vars] ] [[1]] }
);

checkConsistencyByReduce[expr_, vars_] := 
Quiet[
  Check[
    Block[
      {sol},
      sol = Reduce[expr, vars, Reals];
    If[sol === False,
      {2},
      {1, {sol}}
      ]
    ],
    {0, $MessageList}
  ]
];

(* 変数名から 「\[CloseCurlyQuote]」を取る *)
removeDash[var_] := (
   var /. Derivative[_][x_] -> x
);

(* 
 * tellVarsを元に、その初期値制約が必要かどうかを調べる
 * 初期値制約中に出現する変数がvars内になければ不要
 *)
isRequiredConstraint[cons_, tellVars_] := Block[{
   consVar
},
   consVar = cons /. x_[0] == y_ -> x;
   removeDash[consVar];
   If[MemberQ[tellVars, consVar], True, False]
];

(* tellVarsを元に、その変数が必要かどうかを調べる *)
isRequiredVariable[var_, tellVars_] := (
   If[MemberQ[tellVars, var], True, False]
);

removeNotEqual[sol_] := 
   DeleteCases[sol, Unequal[lhs_, rhs_], Infinity];

getNDVars[vars_] := Union[Map[(removeDash[#])&, vars]];

(* ある変数xについて、x[t]==a と x[0]==a があった場合は、x'[t]==0を消去 （あれば） *)
removeTrivialCons[cons_, consVars_] := Block[
  {exprRule, varSol, removedExpr,
   removeTrivialConsUnit, getRequiredVars},

  removeTrivialConsUnit[expr_, var_]:=(
    (* 方程式をルール化する *)
    exprRule = Map[(Rule @@ #) &, expr];
    varSol = var[t] /. exprRule;
    If[MemberQ[expr, var[0] == varSol] && MemberQ[expr, var'[t] == 0],
      (* true *)
      removedExpr = DeleteCases[expr, var'[t] == 0],

      (* false *)
      removedExpr = expr
    ];
    removedExpr
  );

  getRequiredVars[vars_] := (
    Union[Map[(# /. Derivative[n_][x_][t] -> Derivative[n - 1][x]
                         /.  Derivative[0][x_][t] -> x) &, vars]]
  );

  Fold[(Intersection[#1, removeTrivialConsUnit[cons, #2]]) &,
       cons, getRequiredVars[consVars]]

];

(* Reduceで得られた結果をリスト形式にする *)
(* AndではなくListでくくる *)
applyList[reduceSol_] :=
  If[Head[reduceSol] === And, List @@ reduceSol, List[reduceSol]];
  
(* OrではなくListでくくる *)
applyListToOr[reduceSol_] :=
  If[Head[reduceSol] === Or, List @@ reduceSol, List[reduceSol]];

exDSolve[expr_, vars_] := Block[
{sol, DExpr, DExprVars, NDExpr, otherExpr, paramCons},
  paramCons = getParamCons[expr];
  sol = LogicalExpand[removeNotEqual[Reduce[Cases[Complement[expr, paramCons],Except[True]], vars, Reals]]];

  If[sol===False || Reduce[expr, vars, Reals] === False,
    overconstraint,
    If[sol===True,
      {{}, {}},
      (* 1つだけ採用 *)
      (* TODO: 複数解ある場合も考える *)
      If[Head[sol]===Or, sol = First[sol]];
      sol = applyList[sol];

      (* 定数関数の場合に過剰決定系の原因となる微分制約を取り除く *)
      sol = removeTrivialCons[sol, vars];

      {DExpr, DExprVars, NDExpr, otherExpr} = splitExprs[sol];

      Quiet[
        Check[
          Check[
            
            If[Cases[DExpr, Except[True]] === {},
              (* ストアが空の場合はDSolveが解けないので空集合を返す *)
              sol = {},
              sol = DSolve[DExpr, DExprVars, t];
              (* 1つだけ採用 *)
              (* TODO: 複数解ある場合も考える *)
              sol = First[sol]
            ];

            sol = Quiet[Solve[Join[Map[(Equal @@ #) &, sol], NDExpr], getNDVars[vars]]];
            If[sol =!= {},
              {First[sol], Join[otherExpr, paramCons]},
              overconstraint
            ],
            underconstraint,
            {DSolve::underdet, Solve::svars, DSolve::deqx, 
             DSolve::bvnr, DSolve::bvsing}],
                  overconstraint,
                  {DSolve::overdet, DSolve::bvnul, DSolve::dsmsm}],
            {DSolve::underdet, DSolve::overdet, DSolve::deqx, 
             Solve::svars, DSolve::bvnr, DSolve::bvsing, 
             DSolve::bvnul, DSolve::dsmsm, Solve::incnst}
          ]
        ]
      ]
];

(* DSolveで扱える式 (DExpr)とそうでない式 (NDExpr)とそれ以外 （otherExpr）に分ける *)
(* 微分値を含まず/////変数が2種類以上出る式 (NDExpr)や等式以外 （otherExpr）はDSolveで扱えない *)
splitExprs[expr_] := Block[
  {NDExpr, DExpr, DExprVars, otherExpr},
  otherExpr = Select[expr, (Head[#] =!= Equal) &];
  NDExpr = Select[Complement[expr, otherExpr], 
                  (MemberQ[#, Derivative[n_][x_][t], Infinity] =!= True && Length[Union[Cases[#, _[t], Infinity]]] > 1) &];  
  DExpr = Complement[expr, Join[otherExpr, NDExpr]];
  DExprVars = Union[Fold[(Join[#1, Cases[#2, _[t] | _[0], Infinity] /. x_[0] -> x[t]]) &, 
                         {}, DExpr]];
  {DExpr, DExprVars, NDExpr, otherExpr}
];


(* Piecewiseを分解してリストにする．条件がFalseなのは削除 *)
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
calcNextPointPhaseTime[includeZero_, maxTime_, discCause_, otherExpr_] := Block[
{
  calcMinTime, addMinTime, selectCondTime,
  sol, minT, paramVars, compareResult, resultList, condTimeList,
  calcMinTimeList, convertExpr, removeInequalityInList, findMinTime, compareMinTime,
  compareMinTimeList, divideDisjunction, 
  timeMinCons = If[includeZero===True, (t>=0), (t>0)]
},
  calcMinTime[{currentMinT_, currentMinAsk_}, {type_, integAsk_, askID_}] := (
    If[integAsk=!=False,
      (* true *)
      (* 解なしと境界値の解を区別する *)  
      sol = Quiet[Check[Reduce[{timeMinCons && (maxTime>=t) && (integAsk) && (And @@ otherExpr)}, t],
                        errorSol,
                        {Reduce::nsmet}],
                  {Reduce::nsmet}];
      (*  debugPrint["calcMinTime#sol: ", sol]; *)
      If[sol=!=False && sol=!=errorSol, 
        (* true *)
        (* 成り立つtの最小値を求める *)
        minT = First[Quiet[Minimize[{t, timeMinCons && (sol)}, {t}], 
                           Minimize::wksol]],

        (* false *)
        minT = error],
      
      (* false *)
      minT=0];

    debugPrint["calcMinTime#minT: ", minT];
    (* Piecewiseへの対応 *)
    If[Head[minT] === Piecewise, minT = First[First[First[minT]]]];
    (* 0秒後のを含んではいけない *)
    If[includeZero===False && minT===0, minT=error];
    (* 0秒後の離散変化が行われるかのチェックなので0でなければエラー *)
    If[includeZero===True && minT=!=0,
    minT=error];

    If[minT === error,
      (* true *)
      {currentMinT, currentMinAsk},

      (* false *)
      paramVars = Union[Fold[(Join[#1, getParamVar[#2]]) &, {}, otherExpr]];
      compareResult = Map[(Reduce[{#[minT, currentMinT] && (And @@ otherExpr)}, paramVars]) &, 
                          {Less, Equal, Greater}];


      If[Count[compareResult, Not[False]] > 1,
        (* 要分岐 *)
        (* TODO: 適切な処理をする *)
        1,
        Switch[First[First[Position[compareResult, x_ /; x =!= False, {1}, Heads -> False]]],
          1, {minT, {{type, askID}}},
          2, {minT, Append[currentMinAsk, {type, askID}]},
          3, {currentMinT, currentMinAsk}]
      ]
    ]
  );
  

  (* 条件を満たす最小の時刻と，その条件の組を求める *)
  (* maxTは理想的には無くても可能だが，あった方が事故がおきにくいのと高速化が見込めるかもしれないため追加 *)
  findMinTime[ask_, condition_, maxT_] := (
    sol = Quiet[Check[Reduce[ask&&condition&&t>0&&maxT>=t, t, Reals],
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
      minT
    ]
  );
  
  (* ２つの時刻と条件の組を比較し，最小時刻とその条件の組のリストを返す *)
  compareMinTime[timeCond1_, timeCond2_] := ( Block[
      {
        sol, notSol,
        andCond, 
        currentMinTime
      },
      andCond = Reduce[timeCond1[[2]]&&timeCond2[[2]], Reals];
      If[andCond === False,
        {},
        sol = Quiet[Reduce[And[andCond,timeCond1[[1]] > timeCond2[[1]]], Reals]];
        If[ sol === False,
          {{timeCond1[[1]], andCond}},
          notSol = Reduce[andCond&&!sol];(* Impliesだと完全に同型じゃないと無理っぽいので *)
          If[ notSol === False,
            {{timeCond2[[1]], andCond}},
            {{timeCond1[[1]],  notSol}, {timeCond2[[1]], sol}}
          ]
        ]
      ]
    ]
    
  );
  
  (* ２つの時刻と条件の組のリストを比較し，各条件組み合わせにおいて，最小となる時刻と条件の組のリストを返す *)
  compareMinTimeList[{}, list2_] := {};
  compareMinTimeList[list1_, {}] := list1;
  compareMinTimeList[{h1_, t1___}, list2_] := ( Block[
      {
        resultList
      },
      resultList = Fold[(Join[#1, compareMinTime[h1, #2]])&,{}, list2];
      resultList = Join[resultList, compareMinTimeList[{t1}, list2]];
      resultList
    ]
  );

  (* 最小時刻と条件の組をリストアップする関数 *)
  calcMinTimeList[askList_, timeConditionList_, conditionForAll_, maxT_] := ( Block[
      {
        tmpList
      },
      If[askList === {},
        timeConditionList,
        tmpList = findMinTime[First[askList], (And @@ conditionForAll), maxT];
        tmpList = compareMinTimeList[timeConditionList, tmpList];
        tmpList = calcMinTimeList[Rest[askList], tmpList, conditionForAll, maxT];
        tmpList
        (*timeConditionList*)
      ]
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
  
  (* リストを整形する*)
  convertExpr[list_] := ( Block[
    {
      tmpList
    },
      tmpList = removeInequalityInList[list];
      tmpList = adjustExprs[tmpList];
      tmpList = Map[({removeP[#[[1]]], getExprCode[#], ToString[FullForm[#[[2]]]]})&, tmpList];
      tmpList
    ]
  );
  
  
  (* 時刻と条件の組で，条件が論理和でつながっている場合それぞれに分解する *)
  divideDisjunction[timeCond_] := Map[({timeCond[[1]], #})&, List@@timeCond[[2]]];


  (* 従来の，最小時刻を１つだけ見つけるための処理*)
  (*resultList = Fold[calcMinTime,
       {maxTime, {}},
       Join[Map[({pos2neg, Not[#[[1]]], #[[2]]})&, posAsk],
            Map[({neg2pos,     #[[1]],  #[[2]]})&, negAsk],
            Fold[(Join[#1, Map[({neg2pos, #[[1]], #[[2]]})&, #2]])&, {}, NACons]]];*)
            

  (* 最小時刻と条件の組のリストを求める *)
  resultList = calcMinTimeList[discCause, {{maxTime, And@@otherExpr}}, otherExpr, maxTime];

  (* 整形して結果を返す *)
  
  resultList = Map[({#[[1]],LogicalExpand[#[[2]]]})&, resultList];
  resultList = Fold[(Join[#1, If[Head[#2[[2]]]===Or, divideDisjunction[#2], {#2}]])&,{}, resultList];
  resultList = Map[({#[[1]], applyList[#[[2]]]})&, resultList];
  
  resultList = Map[({ToString[FullForm[#[[1]]]], convertExpr[#[[2]]], If[#[[1]] === maxTime, 1, 0]})&, resultList];
  resultList
];


getVariableName[variable_[_]] := variable;
getVariableName[Derivative[n_][f_][_]] := f;

getDerivativeCount[variable_[_]] := 0;
getDerivativeCount[Derivative[n_][f_][_]] := n;

createIntegratedValue[variable_, integRule_] := (
  Simplify[
    variable /. Map[(Rule[#[[1]] /. x_[t]-> x, #[[2]]])&, integRule]
             /. Derivative[n_][f_] :> D[f, {t, n}] 
             /. x_[t] -> x]
);

(* Print[createIntegratedValue[ToExpression["{ht'[t]}"], ToExpression["{ht[t] -> 2t+Sin[t]}"]]] *)
(* Print[createIntegratedValue[ToExpression["{ht'[t]}"], ToExpression["{ht[t] -> 10}"]]] *)

(* パラメータ制約を得る *)
getParamCons[cons_] := Cases[cons, x_ /; Not[hasVariable[x]], {1}];
(* 式中のパラメータ変数を得る *)
getParamVar[paramCons_] := Cases[paramCons, x_ /; Not[NumericQ[x]], Infinity];

(*
 * askの導出状態が変化するまで積分をおこなう
 *)
integrateCalc[cons_, 
              discCause_,
              vars_, 
              maxTime_] := Quiet[Check[Block[
{
  tmpIntegSol,
  tmpDiscCause,
  tmpMinT, 
  tmpVarMap,
  endTimeFlag,
  tmpRet,
  NDExpr,
  DExpr,
  DExprVars,
  otherExpr,
  returnVars,
  solVars,
  paramCons,
  paramVars
},
  debugPrint["cons:", cons, 
             "discCause:", discCause,
             "vars:", vars, 
             "maxTime:", maxTime,
             "all", cons, discCause, vars, maxTime];

  If[Cases[cons, Except[True]]=!={},
    (* true *)
    paramCons = getParamCons[cons];
    paramVars = Union[Fold[(Join[#1, getParamVar[#2]]) &, {}, paramCons]];
    tmpIntegSol = LogicalExpand[removeNotEqual[Reduce[Complement[cons, paramCons], vars, Reals]]];
    (* 1つだけ採用 *)
    (* TODO: 複数解ある場合も考える *)
    If[Head[tmpIntegSol]===Or, tmpIntegSol = First[tmpIntegSol]];

    tmpIntegSol = removeTrivialCons[applyList[tmpIntegSol], Join[vars, paramVars]];

    {DExpr, DExprVars, NDExpr, otherExpr} = splitExprs[tmpIntegSol];


    If[Cases[DExpr, Except[True]] === {},
      tmpIntegSol = {},
      tmpIntegSol = Quiet[DSolve[DExpr, DExprVars, t],
                        {Solve::incnst}];
      (* 1つだけ採用 *)
      (* TODO:複数解ある場合も考える *)
      tmpIntegSol = First[tmpIntegSol]];
    (* tmpIntegSolおよびNDExpr内に出現するND変数の一覧を得る *)
    solVars = getNDVars[Union[Cases[Join[tmpIntegSol, NDExpr], _[t], Infinity]]];
    (* integrateCalcの計算結果として必要な変数の一覧 *)
    returnVars = Select[vars, (MemberQ[solVars, removeDash[#]]) &];

     debugPrint["tmpIntegSol before Solve: ", tmpIntegSol];
     debugPrint["NDExpr before Solve: ", NDExpr];
     debugPrint["returnVars before Solve: ", returnVars];

     (*improve by takeguchi*)
     tmpIntegSol = If[Length[NDExpr] == 0, tmpIntegSol,
          First[FullSimplify[ExpToTrig[Quiet[
              Solve[Join[Map[(Equal @@ #) &, tmpIntegSol], TrigToExp[NDExpr]], getNDVars[returnVars]],
          {Solve::incnst, Solve::ifun, Solve::svars}]]]]
     ];
    
     (*before improve :*)
     (*     
        tmpIntegSol=First[Quiet[Solve[Join[Map[(Equal@@#)&,tmpIntegSol],
                 NDExpr],getNDVars[returnVars]],{Solve::incnst,Solve::ifun}]];
     *)

     debugPrint["tmpIntegSol after Solve: ", tmpIntegSol];
    
    (* DSolveの結果には，y'[t]など微分値についてのルールが含まれていないのでreturnVars全てに対してルールを作る *)
    tmpIntegSol = Map[(# -> createIntegratedValue[#, tmpIntegSol])&, returnVars];
    debugPrint["tmpIntegSol", tmpIntegSol];

    tmpDiscCause = Map[(# /. tmpIntegSol) &, discCause];
    
    paramCons = {Reduce[paramCons]};
    
    debugPrint["nextpointphase arg:", {False, maxTime, tmpDiscCause, paramCons}];
    tmpMinT = calcNextPointPhaseTime[False, maxTime, tmpDiscCause, paramCons];

    tmpVarMap = 
      Map[({getVariableName[#], 
            getDerivativeCount[#], 
            createIntegratedValue[#, tmpIntegSol] // FullForm // ToString})&, 
          returnVars],

    (* false *)
    debugPrint["tmpIntegSol before Solve: false"];
    debugPrint["NDExpr before Solve: false"];
    debugPrint["returnVars before Solve: false"];
    debugPrint["tmpIntegSol after Solve: false"];
    debugPrint["nextpointphase arg: no next point phase"];
    debugPrint["tmpIntegSol", tmpIntegSol];
    tmpMinT = {{maxTime // FullForm // ToString, {}, 1}};
    tmpVarMap = {};
  ];
  tmpRet = {1,
            tmpVarMap,
            tmpMinT};
  debugPrint["tmpRet:", tmpRet];
  tmpRet
],
  {0, $MessageList}
]];

(*
 * 式に対して与えられた時間を適用する
 *)
applyTime2Expr[expr_, time_] := Block[
  {appliedExpr},
  debugPrint["expr:", expr,
             "time:", time];
  appliedExpr = Simplify[(expr /. t -> time)];
  If[Element[appliedExpr, Reals] =!= False,
    {1, appliedExpr  // FullForm // ToString},
    {0}]
];

(*
 * formからtoまでのリストをinterval間隔で作成する
 * 最後に必ずtoが来るために，
 * 最後の間隔のみintervalよりも短くなる可能性がある
 *)
createValueList[from_, to_, interval_] := Block[
  {sol},
  sol = NestWhileList[((#) + (interval))&, from, (# <= to)&, 1, Infinity, -1];
  If[Last[sol] =!= to, 
      Append[sol, to], 
      sol]
];

createOutputTimeList[from_, to_, interval_] :=
  Map[(ToString[#, InputForm])&, createValueList[from, to, interval]];

(* Print[createOutputTimeList[0, 5, 1/3]] *)

(* Print[integrateCalc[ *)
(* {True, Derivative[1][usrVarht][t] == usrVarv[t], *)
(* Derivative[1][usrVarv][t] == -10, usrVarht[0] == 10, usrVarv[0] == 0}, *)
(* {}, *)
(* {{usrVarht[t] == 0, 26}}, *)
(* {usrVarht[t], Derivative[1][usrVarht][t], usrVarv[t], Derivative[1][usrVarv][t]}, *)
(* 1]]; *)

(*  Print[integrateCalc[ *)
(* {True, True, True, Derivative[2][usrVarht][t] == 0, Derivative[2][usrVarv][t] == 0, usrVarht[0] == 10, usrVarv[0] == 0, Derivative[1][usrVarht][0] == 0, Derivative[1][usrVarv][0] == -10}, *)
(* {}, *)
(* {{usrVarht[t] == 0, 73}}, *)
(* {usrVarht[t], Derivative[1][usrVarht][t], Derivative[2][usrVarht][t], usrVarv[t], Derivative[1][usrVarv][t], Derivative[2][usrVarv][t]}, *)
(* 4]]; *)

(* Print[integrateCalc[{Equal[usrVarht[0], 10], Equal[usrVarv[0], 0], *)
(*     Equal[Derivative[1][usrVarht][t], usrVarv[t]], Equal[Derivative[1][usrVarv][t], -10]}, *)
(*   {}, *)
(*   {{usrVarht[t]==0, 10}}, *)
(*   {usrVarht[t], usrVarv[t], Derivative[1][usrVarht][t], Derivative[1][usrVarv][t]}, 10]]; *)

(* Print[integrateCalc[{Equal[usrVarht[0], 10], Equal[Derivative[1][usrVarht][0], 0], *)
(*     Equal[Derivative[2][usrVarht][t], -10]}, *)
(*   {}, *)
(*   {{usrVarht[t]==0, 10}}, *)
(*   {usrVarht[t], Derivative[1][usrVarht][t], Derivative[2][usrVarht][t]}, 10] // FullForm]; *)

(*
 * 与えられた式を積分し，返す
 *
 * 0: Solver Error
 * 1: Solved Successfully
 * 2: Under Constraint
 * 3: Over Constraint
 * 
 *)
integrateExpr[cons_, vars_] := Quiet[Check[Block[
{ sol },

  debugPrint["cons:", cons, "vars:", vars]; 
 
  sol = exDSolve[cons, vars];
  Switch[sol,
    underconstraint, 
      {2},
    overconstraint,
      {3},
    _,
      {1,         
       Map[({getVariableName[#], 
             getDerivativeCount[#], 
             ToString[createIntegratedValue[#, sol[[1]]], InputForm]})&, 
           vars]}]
],
  {0, $MessageList}
]];

(* Print["integ:", integrateExpr[{ht'[t]==v[t], v'[t]==-10, ht[0]==a, v[0]==b}, {ht[t], ht'[t], v[t], v'[t]}]]; *)
(* Print["integ:", integrateExpr[{ht'[t]==v[t], v'[t]==-10, v'[t]==-20, ht[0]==a, v[0]==b}, {ht[t], ht'[t], v[t], v'[t]}]]; *)
(* Print["integ:", integrateExpr[{ht'[t]==x[t], v'[t]==-10, ht[0]==a, v[0]==b}, {x[t], ht[t], ht'[t], v[t], v'[t]}]]; *)

(*
 * 与えられた式を近似する
 *)
approxExpr[precision_, expr_] :=
  Rationalize[N[Simplify[expr], precision + 3], 
              Divide[1, Power[10, precision]]];

(* 
 * 与えられたtの式をタイムシフト
 *)
exprTimeShift[expr_, time_] := ToString[FullForm[Simplify[expr /. t -> t - time ]]];
