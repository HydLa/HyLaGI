(*
 * デバック用メッセージ出力V数
 *)
If[optUseDebugPrint,
  debugPrint[arg___] := Print[InputForm[{arg}]],
  debugPrint[arg___] := Null];


(* ルールのリストを受けて，tについてのルールを除いたものを返す関数 *)
removeRuleForTime[ruleList_] := DeleteCases[ruleList, t -> _];

(*
 * checkEntailmentのIPバージョン
 * 0 : Solver Error
 * 1 : 導出可能
 * 2 : 導出不可能
 * 3 : 要分岐
 *)
checkEntailmentInterval[guard_, store_, vars_, pars_] := Quiet[Check[Block[
  {tStore, sol, integGuard, otherExpr, minT},
  debugPrint["guard:", guard, "store:", store, "vars:", vars, "pars:", pars];
  sol = exDSolve[store, vars];
  (*debugPrint["sol:", sol];*)
  If[sol =!= overconstraint && sol =!= underconstraint,
    tStore = sol[[1]];
    otherExpr = sol[[2]];
    (* guardとotherExprにtStoreを適用する *)
    integGuard = guard /. tStore;
    otherExpr = otherExpr /. tStore;
    (* その結果とt>0とを連立 *)
    (*debugPrint["integGuard:", integGuard];*)
    sol = Quiet[Check[Reduce[{integGuard && t > 0 && (And@@otherExpr)}, t, Reals],
                      False, {Reduce::nsmet}], {Reduce::nsmet}];
    If[sol =!= False,
      (* Infを取って0になれば，導出できる可能性がある *)
      minT = Quiet[Minimize[{t, sol}, Append[pars,t]]];
      If[First[minT] === 0 && Quiet[Reduce[And@@Map[(Equal@@#)&, removeRuleForTime[minT[[2]]]] && And@@otherExpr]] =!= False,
        (* 上の2番目の条件は一見すると不要だが，Minimizeが時間の境界に触れている場合と定数の境界に触れている場合を区別できないらしいので必要． *)
        (* 時間の境界はＯＫだが，定数が境界に乗ってるのは本来ありえない場合のため *)
        
        (* 必ず導出できるかどうかを判定 *)
        If[Quiet[Reduce[ForAll[pars, And@@otherExpr, First[Minimize[{t, sol}, t]] == 0]]] =!= False,
          {1},
          {3}
        ],
        {2}
      ],
      {2}
    ],
    {2}
  ]
],
  {0, $MessageList}
]];

(*
 * ある1つのask制約に関して、そのガードが制約ストアからentailできるかどうかをチェック
 *  0 : Solver Error
 *  1 : 導出可能
 *  2 : 導出不可能
 *  3 : 不明 （要分岐）
 *)
checkEntailment[guard_, store_, vars_] := Quiet[Check[Block[
  {sol, nsol},
  debugPrint["guard:", guard, "store:", store, "vars:", vars];
  sol = Reduce[Append[store, guard], vars, Reals];
  If[sol=!=False, 
    nsol = Reduce[Append[store, Not[sol]], vars, Reals];
    If[nsol === False, 
      {1}, 
      {3}
    ],
    {2}
  ]
],
  {0, $MessageList}
]];

(* Print[checkEntailment[ht==0, {}, {ht}]] *)

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
      (* prevが-1のものは定数として扱うことにする *)
      {renamedVarName, derivativeCount, -1},
      invalidVar
    ]
  ]
];

(* 変数とその値に関する式のリストを、変数表的形式に変換
 * 0:Equal
 * 1:Less
 * 2:Greater
 * 3:LessEqual
 * 4:GreaterEqual
*)

getExprCode[expr_] := Switch[Head[expr],
  Equal, 0,
  Less, 1,
  Greater, 2,
  LessEqual, 3,
  GreaterEqual, 4
];

convertCSToVM[orExprs_] := Block[
  {resultExprs, inequalityVariable,
   removeInequality},
  
  
  (* Inequality[a, relop, x, relop, b]の形を変形 *)
  removeInequality[ret_, expr_] := (
    If[Head[expr] === Inequality,
      Join[ret,
           {Reduce[expr[[2]][expr[[1]], expr[[3]]], expr[[3]]]},
           {expr[[4]][expr[[3]], expr[[5]]]}
      ],
      Append[ret, expr]
    ]
  );
  
  
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

  debugPrint["orExprs:", orExprs];
  If[Head[First[orExprs]] === Or,
    resultExprs = Map[(applyList[#])&, List@@First[orExprs]],
    resultExprs = Map[(applyList[#])&, orExprs]
  ];
  resultExprs = Map[formatExprs, resultExprs];
  debugPrint["resultExprs:", resultExprs];
  resultExprs
];

(*
 * isConsistent内のReduceの結果得られた解を{変数名, 値}のリスト形式にする
 *)
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

getInverseRelop[relop_] := Switch[relop,
                                  Equal, Equal,
                                  Less, Greater,
                                  Greater, Less,
                                  LessEqual, GreaterEqual,
                                  GreaterEqual, LessEqual];

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

(*
 * 制約が無矛盾であるかをチェックする
 * Reduceで解いた結果、�ｃF得られれば無矛盾
 * 得られた結果を用いて、各変数に関して {変数名, 値}　という形式で表したリストを返す
 *
 * 戻り値のリストの先頭：
 *  0 : Solver Error
 *  1 : 充足
 *  2 : 制約エラー
 *)
isConsistent[pexpr_, expr_, vars_] := Quiet[Check[Block[
{
  sol, ret
},

  debugPrint["pexpr:", pexpr, "expr", expr, "vars:", vars];
  sol = Reduce[expr, vars, Reals];
  (*  debugPrint["sol after Reduce:", sol];*)
  If[sol =!= False,
    sol = Reduce[Append[pexpr,sol],vars, Reals];
    If[sol =!= False,
      (* true *)
      (* 外側にOr[]のある場合はListで置き換える。ない場合もListで囲む *)
      sol = LogicalExpand[sol];
      (* debugPrint["sol after LogicalExpand:", sol];*)
      sol = If[Head[sol] === Or, Apply[List, sol], {sol}];
      (* debugPrint["sol after Apply Or List:", sol];*)
      (* 得られたリストの要素のヘッドがAndである場合はListで置き換える。ない場合もListで囲む *)
      sol = removeNotEqual[Map[(If[Head[#] === And, Apply[List, #], {#}]) &, sol]];
      (* debugPrint["sol after Apply And List:", sol];*)
      (* 一番内側の要素 （レベル2）を文字列にする *)
      ret={1, Map[(ToString[FullForm[#]]) &, 
              Map[(adjustExprs[#])&, sol], {2}]},
      ret={2}
    ],
    (* false *)
    ret={2}
  ];
  debugPrint["ret:", ret];
  ret
],
  {0, $MessageList}
]];

(* Print[isConsistent[s[x==2, 7==x*x], {x,y}]] *)

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

(* ある変数xについて、 *)
(* x[t]==a と x[0]==a があった場合は、x'[t]==0を消去 （あれば） *)
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

(* パラメータの制約を得る *)
getParamCons[cons_] := 
  Fold[(If[Not[hasVariable[#2]], Append[#1, #2], #1]) &, {}, cons];

exDSolve[expr_, vars_] := Block[
{sol, DExpr, DExprVars, NDExpr, otherExpr, paramCons},
  paramCons = getParamCons[expr];
  sol = LogicalExpand[removeNotEqual[Reduce[Cases[Complement[expr, paramCons], Except[True] | Except[False]], vars, Reals]]];

  If[sol===False,
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

      Quiet[Check[Check[If[Cases[DExpr, Except[True]] === {},
                          (* ストアが空の場合はDSolveが解けないので空集合を返す *)
                          sol = {},
                          sol = DSolve[DExpr, DExprVars, t];
                          (* 1つだけ採用 *)
                          (* TODO: 複数解ある場合も考える *)
                          sol = First[sol]];

                        sol = Quiet[Solve[Join[Map[(Equal @@ #) &, sol], NDExpr], getNDVars[vars]]];
                        If[sol =!= {}, 
                          {First[sol], Join[otherExpr, paramCons]},
                          overconstraint],
                        underconstraint,
                        {DSolve::underdet, Solve::svars, DSolve::deqx, 
                         DSolve::bvnr, DSolve::bvsing}],
                  overconstraint,
                  {DSolve::overdet, DSolve::bvnul, DSolve::dsmsm}],
            {DSolve::underdet, DSolve::overdet, DSolve::deqx, 
             Solve::svars, DSolve::bvnr, DSolve::bvsing, 
             DSolve::bvnul, DSolve::dsmsm, Solve::incnst}]]]
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

(* isConsistentInterval[tells_, store_, tellsVars_, storeVars_] := ( *)
(*   If[store =!= {}, *)
(*      (\* 制約ストアが空でない場合、不要な初期値制約を取り除く必要がある *\) *)
(*      newTellVars = Map[removeDash, Map[(# /. x_[t] -> x) &, tellsVars]]; *)
(*      (\* storeがList[And[]]の形になっている場合は、一旦、中のAndを取り出す *\) *)
(*      newStore = If[Head[First[store]] === And, Apply[List, First[store]], store]; *)
(*      removedStore = {Apply[And, Select[newStore, (isRequiredConstraint[#, newTellVars]) &]]}; *)
(*      newStoreVars = Map[removeDash, Map[(# /. x_[t] -> x) &, storeVars]]; *)
(*      removedStoreVars = Map[(#[t])&, Select[newStoreVars, (isRequiredVariable[#, newTellVars])&]], *)

(*      (\* 制約ストアが空の場合 *\) *)
(*      removedStore = store; *)
(*      removedStoreVars = storeVars]; *)

(*   If[sol =!= overconstraint, *)
(*      cons = Map[(ToString[FullForm[#]]) &, Join[tells, removedStore]]; *)
(*      vars = Join[tellsVars, removedStoreVars]; *)
(*      If[sol =!= underconstraint, {1, cons, vars}, {2, cons, vars}], *)
(*      0] *)
(* ); *)

(*
 * 戻り値のリストの先頭：
 *  0 : Solver Error
 *  1 : 充足
 *  2 : 制約エラー
 *)
isConsistentInterval[expr_, vars_] :=  Block[
  {sol},
  Quiet[Check[
    debugPrint["expr:", expr, "vars:", vars];
    sol = exDSolve[expr, vars];
    If[sol=!=overconstraint,
      {1},
      {2}],
  {0, $MessageList}
]]];

(* Print[exDSolve[{True, True, Derivative[1][usrVarht][t] == usrVarv[t], Derivative[1][usrVarv][t] == -10, usrVarht[0] == 10, usrVarv[0] == 0},  *)
(* {usrVarht[t], Derivative[1][usrVarht][t], usrVarv[t], Derivative[1][usrVarv][t]}]]; *)

(* Print[isConsistentInterval[ *)
(* {Derivative[1][usrVarht][t] == usrVarv[t],  *)
(*   Derivative[1][usrVarv][t] == -10,  *)
(*   usrVarv[0] == Derivative[1][usrVarht]},  *)
(*   {Derivative[1][usrVarht][t], usrVarv[t], Derivative[1][usrVarv][t]}]]; *)



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
calcNextPointPhaseTime[includeZero_, maxTime_, posAsk_, negAsk_, NACons_, otherExpr_] := Block[
{
  calcMinTime, addMinTime, selectCondTime, removeInequality,
  sol, minT, paramVars, compareResult, resultList, condTimeList,
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
      If[Head[minT] === Piecewise, minT = makeListFromPiecewise[minT, condition], minT = {{minT, {condition}}}];
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


  (* 従来の，最小時刻を１つだけ見つけるための処理*)
  (*resultList = Fold[calcMinTime,
       {maxTime, {}},
       Join[Map[({pos2neg, Not[#[[1]]], #[[2]]})&, posAsk],
            Map[({neg2pos,     #[[1]],  #[[2]]})&, negAsk],
            Fold[(Join[#1, Map[({neg2pos, #[[1]], #[[2]]})&, #2]])&, {}, NACons]]];*)

  (* 最小時刻と条件の組のリストを求める *)
  resultList = calcMinTimeList[ Join[Map[(Not[#[[1]]])&, posAsk], Map[(#[[1]])&, negAsk],
                                Fold[(Join[#1, Map[(#[[1]])&, #2]])&, {}, NACons]],
                                {{maxTime, And@@otherExpr}}, otherExpr, maxTime];

  (* 時刻と条件の組で，条件が論理和でつながっている場合それぞれに分解する *)
  divideDisjunction[timeCond_] := Map[({timeCond[[1]], #})&, List@@timeCond[[2]]];

  (* 整形して結果を返す *)
  
  resultList = Map[({#[[1]],LogicalExpand[#[[2]]]})&, resultList];
  resultList = Fold[(Join[#1, If[Head[#2[[2]]]===Or, divideDisjunction[#2], {#2}]])&,{}, resultList];
  resultList = Map[({#[[1]], applyList[#[[2]]]})&, resultList];
  
  resultList = Map[({ToString[FullForm[#[[1]]]], convertExpr[#[[2]]], If[#[[1]] === maxTime, 1, 0]})&, resultList];
  resultList
];


(* Print[nextPointPhaseTime[False, 10, {}, {{t*t==2, c3}}]] *)

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
              posAsk_, negAsk_, NACons_,
              vars_, 
              maxTime_] := Quiet[Check[Block[
{
  tmpIntegSol,
  tmpPosAsk,
  tmpNegAsk,
  tmpNACons,
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
(*   applyTime2VarMap *)
},
(*   applyTime2VarMap[{name_, derivative_, expr_}, time_, extrafunc_] := *)
(*     {name,  *)
(*      derivative,  *)
(*      extrafunc[expr /. t->time]}; *)
(* Map[(applyTime2VarMap[#,  *)
(*                                   tmpMinT,  *)
(*                                   Function[{expr}, ToString[FullForm[Simplify[expr]]]]])&,  *)

  debugPrint["cons:", cons, 
             "posAsk:", posAsk, 
             "negAsk:", negAsk, 
             "NACons:", NACons,
             "vars:", vars, 
             "maxTime:", maxTime];

  If[Cases[cons, Except[True]]=!={},
    (* true *)
    paramCons = getParamCons[cons];
    paramVars = Union[Fold[(Join[#1, getParamVar[#2]]) &, {}, paramCons]];
    tmpIntegSol = LogicalExpand[removeNotEqual[Reduce[Complement[cons, paramCons], vars, Reals]]];
    (*debugPrint["tmpIntegSol: ", tmpIntegSol, "paramVars", paramVars "paramCons", paramCons]; *)
    (* 1つだけ採用 *)
    (* TODO: 複数解ある場合も考える *)
    If[Head[tmpIntegSol]===Or, tmpIntegSol = First[tmpIntegSol]];

    tmpIntegSol = removeTrivialCons[applyList[tmpIntegSol], Join[vars, paramVars]];

    {DExpr, DExprVars, NDExpr, otherExpr} = splitExprs[tmpIntegSol];
    (* debugPrint["DExpr ", DExpr, "DExprVars", DExprVars]; *)


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

    tmpIntegSol = First[Quiet[Solve[Join[Map[(Equal @@ #) &, tmpIntegSol], NDExpr], 
                              getNDVars[returnVars]], {Solve::incnst, Solve::ifun}]];
    
    (* DSolveの結果には，y'[t]など微分値についてのルールが含まれていないのでreturnVars全てに対してルールを作る *)
    tmpIntegSol = Map[(# -> createIntegratedValue[#, tmpIntegSol])&, returnVars];
    debugPrint["tmpIntegSol", tmpIntegSol];

    tmpPosAsk = Map[(# /. tmpIntegSol) &, posAsk];
    tmpNegAsk = Map[(# /. tmpIntegSol) &, negAsk];
    tmpNACons = Map[(# /. tmpIntegSol) &, NACons];
    debugPrint["nextpointphase arg:", {False, maxTime, tmpPosAsk, tmpNegAsk, tmpNACons, paramCons}];
    tmpMinT = calcNextPointPhaseTime[False, maxTime, tmpPosAsk, tmpNegAsk, tmpNACons, paramCons];

    tmpVarMap = 
      Map[({getVariableName[#], 
            getDerivativeCount[#], 
            createIntegratedValue[#, tmpIntegSol] // FullForm // ToString})&, 
          returnVars],

    (* false *)
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
  (* debugPrint["MessageList:", $MessageList]; *)
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
