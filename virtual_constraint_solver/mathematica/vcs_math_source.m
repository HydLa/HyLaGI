(*
 * デバッグ用メッセージ出力V数
 *)
If[optUseDebugPrint,
  debugPrint[arg___] := Print[InputForm[{arg}]],
  debugPrint[arg___] := Null];

(*
 * グローバル変数
 * constraints: 現在扱っている制約集合（IPでは変数についてのみ）
 * pconstraints: IPで現在扱っている定数についての条件の集合
 * variables: 制約集合に出現する変数のリスト（PPでは定数も含む）
 * parameters: 制約集合に出現する定数のリスト
 *)

(* ルールのリストを受けて，tについてのルールを除いたものを返す関数 *)
removeRuleForTime[ruleList_] := DeleteCases[ruleList, t -> _];

getInverseRelop[relop_] := Switch[relop,
                                  Equal, Equal,
                                  Less, Greater,
                                  Greater, Less,
                                  LessEqual, GreaterEqual,
                                  GreaterEqual, LessEqual];


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
    ret = Map[LogicalExpand, candidates[[1]]];
    (* tの最小値候補を調べるため，tの下限とt以外の変数についての制約と出現する変数のリストを抽出する． *)
    {underBounds, constraintsForC, listOfC, otherConstraints} = getValidExpression[ret, pars];
    otherConstraints = applyList[Reduce[And[And@@otherConstraints, constraints], Reals]];
    otherConstraints = Map[LogicalExpand, otherConstraints];

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

checkConsistencyInterval[] :=  (
  checkConsistencyIntervalMain[constraints, variables, pconstraints, parameters]
);


checkConsistencyTemporaryInterval[expr_, vars_, pexpr_, pars_] :=  (
  checkConsistencyIntervalMain[constraints && expr, Union[vars, variables], pconstraints && pexpr, Union[pars, parameters] ]
);


checkConsistencyTemporaryInterval[expr_, vars_] :=  (
  checkConsistencyIntervalMain[constraints && expr, Union[vars, variables], pconstraints, parameters]
);



checkConsistencyIntervalMain[expr_, vars_, pexpr_, pars_] :=  
Quiet[
  Check[
    Block[
      {tStore, sol, integGuard, otherExpr, condition},
      debugPrint["expr:", expr, "vars:", vars, "pexpr", pexpr, "pars:", pars, "all", expr, vars, pexpr, pars];
      sol = exDSolve[expr, vars];
      If[sol === overconstraint,
        {2},
        If[sol === underconstraint || sol[[1]] === {} ,
          (* 警告出したりした方が良いかも？ *)
          {1},
          tStore = sol[[1]];
          tStore = Map[(# -> createIntegratedValue[#, tStore])&, vars];
          otherExpr = Fold[(If[Head[#2] === And, Join[#1, List@@#2], Append[#1, #2]])&, {}, Simplify[expr]];
          otherExpr = Select[otherExpr, (MemberQ[{Or, Less, LessEqual, Greater, GreaterEqual, Inequality, Unequal}, Head[#] ] || # === False)&];
          
          (* otherExprにtStoreを適用する *)
          otherExpr = otherExpr /. tStore;
          (* まず，t>0で条件を満たす可能性があるかを調べる *)
          sol = LogicalExpand[Quiet[Check[Reduce[{And@@otherExpr && t > 0 && pexpr}, t, Reals],
                        False, {Reduce::nsmet}], {Reduce::nsmet}]];
          If[sol === False,
            {2},
            (* リストのリストにする *)
            sol = applyListToOr[sol];
            sol = Map[applyList, sol];
            (* tの最大下界を0とできる可能性を調べる． *)
            sol = checkInf[sol, pexpr, pars];
            {sol[[1]]}
          ]
        ]
      ]
    ],
    {0, $MessageList}
  ]
];



removeP[par_] := First[StringCases[ToString[par], StartOfString ~~ "p" ~~ x__ -> x]];

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
  {formatExprs, applyParameter, resultExprs},
  applyEqualityRule[expr_, undeterminedVariables_, rules_] := Block[
    {tmp, complement},
    tmp = expr;
    complement = Complement[getVariables[tmp[[2]] ], undeterminedVariables];
    If[Head[tmp] === Equal,
      While[ tmp =!= True && Length[ complement ] > 0,
        tmp[[2]] = tmp[[2]] /. rules;
        complement = Complement[getVariables[tmp[[2]] ], undeterminedVariables];
      ]
    ];
    tmp
  ];
  
  formatExprs[exprs_] := Block[
    {determinedVariables, undeterminedVariables, undeterminedExprs, rhsVariables, retExprs, rules, tmp},
    If[Cases[exprs, Except[True]]==={},
      {},
      
      retExprs = adjustExprs[exprs];
      
      (* 値が定まらないもののリストを作る *)
      (*
      rules = Fold[(If[Head[#2] === Equal, Append[#1, Rule@@#2], #1])&, {}, retExprs];
      determinedVariables = Fold[(Union[#1, {#2[[1]]} ])&, {}, rules];
      undeterminedVariables = Complement[Join[variables, parameters], determinedVariables];
      
      undeterminedExprs = Select[retExprs, (MemberQ[undeterminedVariables, #[[1]] ])& ];
      retExprs = Join[undeterminedExprs, Complement[retExprs, undeterminedExprs] ];
      
      retExprs = Map[(applyEqualityRule[#, undeterminedVariables, rules])&, retExprs];*)
      retExprs = reducePrevVariable[retExprs];
      
      (* 式を{（変数名）, （関係演算子コード）, (値のフル文字列)｝の形式に変換する *)
      retExprs = DeleteCases[Map[({renameVar[#[[1]]], 
                        getExprCode[#], 
                        ToString[FullForm[#[[2]]]] }) &, 
                        retExprs],
                  {invalidVar, _, _}];
      retExprs
    ]
  ];
  
  debugPrint["@convertCSToVM: constraints", constraints, "variables", variables, "parameters", parameters];
  constraints = LogicalExpand[constraints];
  If[Head[constraints] === Or,
    resultExprs = Map[(applyList[#])&, List@@constraints],
    resultExprs = Map[(applyList[#])&, {constraints}]
  ];
  resultExprs = Map[formatExprs, resultExprs];
  debugPrint["@convertCSToVM resultExprs:", resultExprs];
  resultExprs
];

(* 式中に変数名が出現するか否か *)
hasVariable[exprs_] := Length[StringCases[ToString[exprs], "usrVar" ~~ LetterCharacter]] > 0;

(* 式中に出現する変数を取得 *)
getVariables[exprs_] := ToExpression[StringCases[ToString[exprs], "usrVar" ~~ LetterCharacter..]];

(* 式が変数そのものか否か *)
isVariable[exprs_] := StringMatchQ[ToString[exprs], "usrVar" ~~ __];

(* 式が定数そのものか否か *)
isParameter[exprs_] := StringMatchQ[ToString[exprs], "p" ~~ __];


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
            If[isParameter[#2[[2]]],
              (* true *)
              (* 逆になってるので、演算子を逆にして追加する *)
              Append[#1, getInverseRelop[Head[#2]][#2[[2]], #2[[1]]]],
              (* false *)
              Append[#1, #2]]],
          (* false *)
          Append[#1, #2]]) &,
       {}, andExprs];


resetConstraint[] := (
  constraints = True;
  pconstraints = True;
  variables = {};
  parameters = {};
);

addConstraint[cons_, vars_] := (
  constraints = constraints && cons;
  variables = Union[variables, vars];
  debugPrint["constraints:", constraints, "variables:", variables];
);

addConstraint[cons_, vars_, pcons_, pars_] := (
  pconstraints = pconstraints && pcons;
  parameters = Union[parameters, pars];
  addConstraint[cons, vars];
  debugPrint["pconstraints:", pconstraints, "parameters:", parameters];
);


(* 制約がすべてインクリメンタルに増えていく実装ならこっちだけ使いたい．毎回解いた結果に更新していくからたぶん高速 *)
checkConsistency[] := Block[
  {sol},
  sol = checkConsistencyByReduce[constraints, pconstraints, variables, parameters];
  If[sol[[1]] == 1, constraints = sol[[2]]; sol = {1}  ];
  sol
];

(* 一時的に制約を追加して充足可能性判定 *)
checkConsistencyTemporary[expr_, pexpr_, vars_, pars_] := (
  {checkConsistencyByReduce[constraints && expr, pconstraints && pexpr, Union[variables, vars], Union[parameters, pars][[1]] ]}
);

checkConsistencyTemporary[expr_, vars_] := (
 {checkConsistencyByReduce[constraints && expr, pconstraints, Union[variables, vars], parameters ][[1]] }
);

checkConsistencyByReduce[expr_, pexpr_, vars_, pars_] := 
Quiet[
  Check[
    Block[
      {sol},
      debugPrint["@checkConsistencyReduce", "expr:", expr, "pexpr", pexpr, "vars", vars, "pars", pars, "all:", expr, pexpr, vars, pars];
      sol = Reduce[expr&&pexpr, vars, Reals];
      If[sol === False,
        {2},
        {1, sol}
        (*
        If[pars === {},
          {1, sol},
          If[ Reduce[ForAll[pars, pexpr, Exists[vars, sol]], Reals] === False,
            {3},
            {1, sol}
          ]
        ]*)
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
  
hasPrevVariableAtLeft[expr_] := MemberQ[{expr[[1]]}, prev[x_, y_], Infinity];


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
calculateNextPointPhaseTime[includeZero_, maxTime_, discCause_, otherExpr_] := Block[
{
  addMinTime, selectCondTime,
  sol, minT, paramVars, compareResult, resultList, condTimeList,
  calculateMinTimeList, convertExpr, removeInequalityInList, findMinTime, compareMinTime,
  compareMinTimeList, divideDisjunction
},
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
      (* 時刻が0となる場合を取り除く．安全のためにあった方が良いが，理想的には無くても動くはず？ *)
      minT = Select[minT, (#[[1]] =!= 0)&];
      minT
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
      timeCaseList = {{maxT, And@@condition}};
      For[i = 1, i <= Length[guardList], i++,
        findResult = findMinTime[guardList[[i]], (And @@ condition), maxT];
        timeCaseList = compareMinTimeList[timeCaseList, findResult]
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
  
  (* 最小時刻と条件の組のリストを求める *)
  resultList = calculateMinTimeList[discCause, otherExpr, maxTime];

  (* 整形して結果を返す *)
  resultList = Map[({#[[1]],LogicalExpand[#[[2]]]})&, resultList];
  resultList = Fold[(Join[#1, If[Head[#2[[2]]]===Or, divideDisjunction[#2], {#2}]])&,{}, resultList];
  resultList = Map[({#[[1]], applyList[#[[2]]]})&, resultList];
  resultList = Map[({ToString[FullForm[#[[1]]]], convertExpr[#[[2]]], If[Simplify[#[[1]]] === Simplify[maxTime], 1, 0]})&, resultList];
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

(* prev変数に関するルールのリストを作り，その後prev変数がなくなるまでルール適用を繰り返す *)
reducePrevVariable[{}, {r___}] := {r};
reducePrevVariable[{h_, t___}, {r___}] :=
  Block[
    {lhs, rhgs},
    If[hasPrevVariableAtLeft[h],
      lhs = Map[( # /. h[[1]] -> h[[2]] )&, {t}];
      rhs = Map[( # /. h[[1]] -> h[[2]] )&, {r}];
      reducePrevVariable[lhs, rhs],
      reducePrevVariable[{t}, Append[{r}, h] ]
    ]
  ];
reducePrevVariable[{h_, t___}] := reducePrevVariable[{h, t}, {}];

(* パラメータ制約を得る *)
getParamCons[cons_] := Cases[cons, x_ /; Not[hasVariable[x]], {1}];
(* 式中のパラメータ変数を得る *)
getParamVar[paramCons_] := Cases[paramCons, x_ /; Not[NumericQ[x]], Infinity];



exDSolve[expr_, vars_] := Block[
{sol, DExpr, DExprVars, NDExpr, otherExpr, paramCons},
  sol = And@@reducePrevVariable[applyList[expr]];
  
  Print["sol:", sol];
  
  paramCons = getParamCons[sol];
  sol = LogicalExpand[Reduce[Cases[Complement[applyList[sol], paramCons],Except[True]], vars, Reals]];
  If[sol===False,
    overconstraint,
    If[sol===True,
      {{}, {}},
      (* 1つだけ採用 *)
      (* TODO: 複数解ある場合も考える *)
      If[Head[sol]===Or, sol = First[sol]];
      sol = applyList[sol];
      
      {DExpr, DExprVars, NDExpr, otherExpr} = splitExprs[sol];
      (* 定数関数の場合に過剰決定系の原因となる微分制約を取り除く *)
      DExpr = removeTrivialCons[DExpr, DExprVars];

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

            (*improve by takeguchi*)
            sol = If[Length[NDExpr] == 0, {sol},
                 FullSimplify[ExpToTrig[Quiet[
                     Solve[Join[Map[(Equal @@ #) &, sol], TrigToExp[NDExpr]], getNDVars[vars]],
                 {Solve::incnst, Solve::ifun, Solve::svars}]]]
            ];
            
            If[sol =!= {},
              {sol[[1]], Join[otherExpr, paramCons]},
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




(* 変数[0]を変数[t]に変える*)
changeZeroTot[var_] := (
   var /. x_[0] -> x[t]
);


isPrevVariable[expr_] := MemberQ[{expr}, prev[x_], Infinity];
isTimeVariable[expr_] := MemberQ[{expr}, x_[t], Infinity];

(*
 * askの導出状態が変化するまで積分をおこなう
 *)
integrateCalc[expr_, 
              discCause_,
              vars_, 
              maxTime_] := Quiet[Check[Block[
{
  cons,
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
  paramCons
},
  cons = expr&&constraints&&pconstraints;
  cons = And@@reducePrevVariable[List@@cons];
  returnVars = Union[vars, variables];
  returnVars = Map[changeZeroTot, returnVars];
  returnVars = Select[returnVars, isTimeVariable];
  debugPrint["@Integrate cons:", cons, 
             "discCause:", discCause,
             "returnvars:", returnVars, 
             "maxTime:", maxTime,
             "all", cons, discCause, returnVars, maxTime];
  If[cons =!= True,
    paramCons = getParamCons[applyList[cons]];
    tmpIntegSol = exDSolve[cons, returnVars][[1]];
    If[tmpIntegSol === underconstraint, Return[{0, "under_constraint"}] ];
    debugPrint["@Integrate tmpIntegSol", tmpIntegSol];
      
    (* DSolveの結果には，y'[t]など微分値についてのルールが含まれていないのでreturnVars全てに対してルールを作る *)
    tmpIntegSol = Map[(# -> createIntegratedValue[#, tmpIntegSol])&, returnVars];
    debugPrint["@Integrate tmpIntegSol", tmpIntegSol];

    tmpDiscCause = Map[(# /. tmpIntegSol) &, discCause];
    paramCons = {Reduce[paramCons]};
    
    debugPrint["@Integrate nextpointphase arg:", {False, maxTime, tmpDiscCause, paramCons}];
    tmpMinT = calculateNextPointPhaseTime[False, maxTime, tmpDiscCause, paramCons];

    tmpVarMap = 
      Map[({getVariableName[#], 
            getDerivativeCount[#], 
            createIntegratedValue[#, tmpIntegSol] // FullForm // ToString})&, 
          returnVars],

    (* cons === True *)
    debugPrint["@Integrate tmpIntegSol:"];
    debugPrint["@Integrate nextpointphase arg: no next point phase"];
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
