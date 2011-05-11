(*
 * �ǥХå��ѥ�å��������ϴؿ�
 *)
If[optUseDebugPrint,
  debugPrint[arg___] := Print[InputForm[{arg}]],
  debugPrint[arg___] := Null];

(*
 * checkEntailment��IP�С������
 * 0 : Solver Error
 * 1 : Ƴ�в�ǽ
 * 2 : Ƴ���Բ�ǽ
 *)
checkEntailmentInterval[guard_, store_, vars_, pars_] := Quiet[Check[Block[
  {tStore, sol, integGuard, otherExpr, minT},
  debugPrint["guard:", guard, "store:", store, "vars:", vars, "pars:", pars];
  sol = exDSolve[store, vars];
  (*debugPrint["sol:", sol];*)
  If[sol =!= overconstraint && sol =!= underconstraint,
    tStore = sol[[1]];
    otherExpr = sol[[2]];
    (* guard��otherExpr��tStore��Ŭ�Ѥ��� *)
    integGuard = guard /. tStore;
    otherExpr = otherExpr /. tStore;
    If[integGuard =!= False,
      (* ���η�̤�t>0�Ȥ�ϢΩ *)
      (*debugPrint["integGuard:", integGuard];*)
      sol = Quiet[Check[Reduce[{integGuard && t > 0 && (And@@otherExpr)}, t],
                        False, {Reduce::nsmet}], {Reduce::nsmet}];
      (* Inf���ä�0�ˤʤ��Entailed *)
      If[sol =!= False,
        minT = Quiet[First[Minimize[{t, sol}, Append[pars,t]]]];
        If[minT === 0,
          (* ��ʬ��狼�ɤ���Ƚ�ꤷ���� *)
          (* MinValue�ǤϤʤ�Minimize����Ѥ��� *)
          (* If[Reduce[ForAll[pars, And@@otherExpr, MinValue[{t, sol}, t] == 0]] =!= False, *)
          If[Quiet[Reduce[ForAll[pars, And@@otherExpr, First[Minimize[{t, sol}, t]] == 0]]] =!= False,
            {1},
            {3}
          ],
          {2}
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
 * ����1�Ĥ�ask����˴ؤ��ơ����Υ����ɤ����󥹥ȥ�����entail�Ǥ��뤫�ɤ���������å�
 *  0 : Solver Error
 *  1 : Ƴ�в�ǽ
 *  2 : Ƴ���Բ�ǽ
 *  3 : ���� ����ʬ����
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

renameVar[varName_] := Block[
  {renamedVarName, derivativeCount = 0, prevFlag = 0,
   getDerivativeCountPoint, removeUsrVar
  },  

  getDerivativeCountPoint[Derivative[n_][var_]] := n;
  removeUsrVar[var_] := First[StringCases[ToString[var], "usrVar" ~~ x__ -> x]];
  removeP[par_] := First[StringCases[ToString[par], "p" ~~ x__ -> x]];

  (* �ѿ�̾��'���Ĥ����ν��� *)
  If[MemberQ[{varName}, Derivative[n_][x_], Infinity],
    derivativeCount = getDerivativeCountPoint[varName];
    renamedVarName = removeDash[varName],
    renamedVarName = varName
  ];
  (* �ѿ�̾��prev���Ĥ����ν��� *)
  If[Head[renamedVarName] === prev,
    renamedVarName = First[renamedVarName];
    prevFlag = 1
  ];

  (*�ѿ�̾��Ƭ�ˤĤ��Ƥ��� "usrVar"������� ��̾����Ƭ��usrVar����ʤ��Τ�invalidVar�����̾�� *)
  If[StringMatchQ[ToString[renamedVarName], "usrVar" ~~ x__],
    (* true *)
    renamedVarName = removeUsrVar[renamedVarName];
    (* ���λ�����ñ�Τ��ѿ�̾�ΤϤ��� *)
    If[Length[renamedVarName] === 0,
      {renamedVarName, derivativeCount, prevFlag},
      (* �������Τ�Τϥϥͤ� *)
      invalidVar],
    (* false *)
    If[StringMatchQ[ToString[renamedVarName], "p" ~~ x__],
     (*���̾��Ƭ�ˤĤ��Ƥ��� "p"������� ̾����Ƭ��p����ʤ��Τ�invalidVar�� *)
      renamedVarName = removeP[renamedVarName];
      (* prev��-1�Τ�Τ�����Ȥ��ư������Ȥˤ��� *)
      {renamedVarName, derivativeCount, -1},
      invalidVar
    ]
  ]
];

(* �ѿ��Ȥ����ͤ˴ؤ��뼰�Υꥹ�Ȥ��ѿ�ɽŪ�������Ѵ�
 * 0:Equal
 * 1:Less
 * 2:Greater
 * 3:LessEqual
 * 4:GreaterEqual
*)
convertCSToVM[orExprs_] := Block[
  {andExprs, inequalityVariable,
   getExprCode, removeInequality},
      
  getExprCode[expr_] := Switch[Head[expr],
    Equal, 0,
    Less, 1,
    Greater, 2,
    LessEqual, 3,
    GreaterEqual, 4
  ];
   
  (* Inequality[a, relop, x, relop, b]�η����ѷ� *)
  removeInequality[ret_, expr_] := (
    If[Head[expr] === Inequality,
      Join[ret,
           {Reduce[expr[[2]][expr[[1]], expr[[3]]], expr[[3]]]},
           {expr[[4]][expr[[3]], expr[[5]]]}
      ],
      Append[ret, expr]
    ]
  );


  debugPrint["orExprs:", orExprs];
  If[Head[First[orExprs]] === Or,
    (* Or���ޤޤ�����1�Ĥ������� *)
    (* TODO: ʣ���򤢤����ͤ��� *)
    andExprs = First[First[orExprs]],
    (* Or���ޤޤ�ʤ���� *)
    andExprs = First[orExprs]
  ];
  andExprs = applyList[andExprs];
  If[Cases[andExprs, Except[True]]==={},
    (* ���ȥ������ξ��϶�������֤� *)
    {},
    andExprs = Fold[(removeInequality[#1, #2]) &, {}, andExprs];
    DeleteCases[Map[({renameVar[#[[1]]], 
                      getExprCode[#], 
                      ToString[FullForm[#[[2]]]]}) &, 
                    andExprs],
                {invalidVar, _, _}]]

];

(*
 * isConsistent���Reduce�η������줿��� {�ѿ�̾, ��}���Υꥹ�ȷ����ˤ���
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

(* Or�ǤĤʤ��äƤ��� ��ʣ���򤬤���˾��ϡ����Τ�����1�ĤΤߤ��֤��� *)
createVariableList[Or[expr_, others__], vars_, result_] := (
  createVariableList[expr, vars, result]
);

createVariableList[Equal[varName_, varValue_], vars_, result_] := Block[{
  sol
},
  sol = (Solve[Equal[varName, varValue], vars])[[1]];
  createVariableList[sol, result]
];

(* Reduce[{}, {}]�ΤȤ� *)
createVariableList[True, vars_, result_] := (
  Return[result]
);

(* ������ѿ�̾���и����뤫�ݤ� *)
hasVariable[exprs_] := Length[StringCases[ToString[exprs], "usrVar" ~~ LetterCharacter]] > 0;

(*
 * ����̵̷��Ǥ��뤫������å�����
 * Reduce�ǲ򤤤���̡����������̵̷��
 * ����줿����Ѥ��ơ����ѿ��˴ؤ��� {�ѿ�̾, ��}���Ȥ���������ɽ�����ꥹ�Ȥ��֤�
 *
 * ����ͤΥꥹ�Ȥ���Ƭ��
 *  0 : Solver Error
 *  1 : ��­
 *  2 : ���󥨥顼
 *)
isConsistent[pexpr_, expr_, vars_] := Quiet[Check[Block[
{
  sol,
  getInverseRelOp, adjustExprs
},

  getInverseRelop[relop_] := Switch[relop,
                                    Equal, Equal,
                                    Less, Greater,
                                    Greater, Less,
                                    LessEqual, GreaterEqual,
                                    GreaterEqual, LessEqual];

  (* ɬ���ط��黻�Ҥκ�¦���ѿ�̾������褦�ˤ��� *)
  adjustExprs[andExprs_] := 
    Fold[(If[Not[hasVariable[#2[[1]]]],
            (* true *)
            If[hasVariable[#2[[2]]],
              (* true *)
              (* �դˤʤäƤ�Τǡ��黻�Ҥ�դˤ����ɲä��� *)
              Append[#1, getInverseRelop[Head[#2]][#2[[2]], #2[[1]]]],
              (* false *)
              (* �ѥ�᡼������ξ��ˤ��������� *)
              If[NumericQ[#2[[1]]],
                (* true *)
                (* �դˤʤäƤ�Τǡ��黻�Ҥ�դˤ����ɲä��� *)
                Append[#1, getInverseRelop[Head[#2]][#2[[2]], #2[[1]]]],
                (* false *)
                Append[#1, #2]]],
            (* false *)
            Append[#1, #2]]) &,
         {}, andExprs];


  debugPrint["expr:", expr, "pexpr", pexpr, "vars:", vars];
  sol = Reduce[expr, vars, Reals];
  (*  debugPrint["sol after Reduce:", sol];*)
  If[sol =!= False,
    sol = Reduce[Append[pexpr,sol],vars, Reals];
    If[sol =!= False,
      (* true *)
      (* ��¦��Or[]�Τ������List���֤������롣�ʤ�����List�ǰϤ� *)
      sol = LogicalExpand[sol];
      (* debugPrint["sol after LogicalExpand:", sol];*)
      sol = If[Head[sol] === Or, Apply[List, sol], {sol}];
      (* debugPrint["sol after Apply Or List:", sol];*)
      (* ����줿�ꥹ�Ȥ����ǤΥإåɤ�And�Ǥ������List���֤������롣�ʤ�����List�ǰϤ� *)
      sol = removeNotEqual[Map[(If[Head[#] === And, Apply[List, #], {#}]) &, sol]];
      (* debugPrint["sol after Apply And List:", sol];*)
      (* ������¦������ �ʥ�٥�2�ˤ�ʸ����ˤ��� *)
      {1, Map[(ToString[FullForm[#]]) &, 
              Map[(adjustExprs[#])&, sol], {2}]},
      {2}
    ],
    (* false *)
    {2}]
],
  {0, $MessageList}
]];

(* Print[isConsistent[s[x==2, 7==x*x], {x,y}]] *)

(* �ѿ�̾���� ��\[CloseCurlyQuote]�פ��� *)
removeDash[var_] := (
   var /. Derivative[_][x_] -> x
);

(* 
 * tellVars�򸵤ˡ����ν��������ɬ�פ��ɤ�����Ĵ�٤�
 * �����������˽и������ѿ���vars��ˤʤ��������
 *)
isRequiredConstraint[cons_, tellVars_] := Block[{
   consVar
},
   consVar = cons /. x_[0] == y_ -> x;
   removeDash[consVar];
   If[MemberQ[tellVars, consVar], True, False]
];

(* tellVars�򸵤ˡ������ѿ���ɬ�פ��ɤ�����Ĵ�٤� *)
isRequiredVariable[var_, tellVars_] := (
   If[MemberQ[tellVars, var], True, False]
);

removeNotEqual[sol_] := 
   DeleteCases[sol, Unequal[lhs_, rhs_], Infinity];

getNDVars[vars_] := Union[Map[(removeDash[#])&, vars]];

(* �����ѿ�x�ˤĤ��ơ� *)
(* x[t]==a �� x[0]==a �����ä����ϡ�x'[t]==0��õ� �ʤ���С� *)
removeTrivialCons[cons_, consVars_] := Block[
  {exprRule, varSol, removedExpr,
   removeTrivialConsUnit, getRequiredVars},

  removeTrivialConsUnit[expr_, var_]:=(
    (* ��������롼�벽���� *)
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

(* Reduce������줿��̤�ꥹ�ȷ����ˤ��� *)
(* And�ǤϤʤ�List�Ǥ����� *)
applyList[reduceSol_] :=
  If[Head[reduceSol] === And, List @@ reduceSol, List[reduceSol]];

(* �ѥ�᡼������������� *)
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

      (* 1�Ĥ������� *)
      (* TODO: ʣ���򤢤����ͤ��� *)
      If[Head[sol]===Or, sol = First[sol]];

      sol = applyList[sol];

      (* ����ؿ��ξ��˲�����Ϥθ����Ȥʤ���ʬ���������� *)
      sol = removeTrivialCons[sol, vars];

      {DExpr, DExprVars, NDExpr, otherExpr} = splitExprs[sol];

      Quiet[Check[Check[If[Cases[DExpr, Except[True]] === {},
                          (* ���ȥ������ξ���DSolve���򤱤ʤ��ΤǶ�������֤� *)
                          sol = {},
                          sol = DSolve[DExpr, DExprVars, t];
                          (* 1�Ĥ������� *)
                          (* TODO: ʣ���򤢤����ͤ��� *)
                          sol = First[sol]];

                        sol = Solve[Join[Map[(Equal @@ #) &, sol], NDExpr], getNDVars[vars]];
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

(* DSolve�ǰ����뼰 (DExpr)�Ȥ����Ǥʤ��� (NDExpr)�Ȥ���ʳ� ��otherExpr�ˤ�ʬ���� *)
(* ��ʬ�ͤ�ޤޤ��������������ѿ���2����ʾ�Ф뼰 (NDExpr)�������ʳ� ��otherExpr�ˤ�DSolve�ǰ����ʤ� *)
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
(*      (\* ���󥹥ȥ������Ǥʤ���硢���פʽ��������������ɬ�פ����� *\) *)
(*      newTellVars = Map[removeDash, Map[(# /. x_[t] -> x) &, tellsVars]]; *)
(*      (\* store��List[And[]]�η��ˤʤäƤ�����ϡ���ö�����And����Ф� *\) *)
(*      newStore = If[Head[First[store]] === And, Apply[List, First[store]], store]; *)
(*      removedStore = {Apply[And, Select[newStore, (isRequiredConstraint[#, newTellVars]) &]]}; *)
(*      newStoreVars = Map[removeDash, Map[(# /. x_[t] -> x) &, storeVars]]; *)
(*      removedStoreVars = Map[(#[t])&, Select[newStoreVars, (isRequiredVariable[#, newTellVars])&]], *)

(*      (\* ���󥹥ȥ������ξ�� *\) *)
(*      removedStore = store; *)
(*      removedStoreVars = storeVars]; *)

(*   If[sol =!= overconstraint, *)
(*      cons = Map[(ToString[FullForm[#]]) &, Join[tells, removedStore]]; *)
(*      vars = Join[tellsVars, removedStoreVars]; *)
(*      If[sol =!= underconstraint, {1, cons, vars}, {2, cons, vars}], *)
(*      0] *)
(* ); *)

(*
 * ����ͤΥꥹ�Ȥ���Ƭ��
 *  0 : Solver Error
 *  1 : ��­
 *  2 : ���󥨥顼
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

(*
 * ���Υݥ���ȥե������˰ܹԤ����������
 *)
calcNextPointPhaseTime[includeZero_, maxTime_, posAsk_, negAsk_, NACons_, otherExpr_] := Block[
{
  calcMinTime,
  sol, minT, paramVars, compareResult,
  timeMinCons = If[includeZero===True, (t>=0), (t>0)]
},
  calcMinTime[{currentMinT_, currentMinAsk_}, {type_, integAsk_, askID_}] := (
    If[integAsk=!=False,
      (* true *)
      (* ��ʤ��ȶ����ͤβ����̤��� *)  
      sol = Quiet[Check[Reduce[{timeMinCons && (maxTime>=t) && (integAsk) && (And @@ otherExpr)}, t],
                        errorSol,
                        {Reduce::nsmet}],
                  {Reduce::nsmet}];
      (*  debugPrint["calcMinTime#sol: ", sol]; *)
      If[sol=!=False && sol=!=errorSol, 
        (* true *)
        (* ����Ω��t�κǾ��ͤ���� *)
        minT = First[Quiet[Minimize[{t, timeMinCons && (sol)}, {t}], 
                           Minimize::wksol]],

        (* false *)
        minT = error],
      
      (* false *)
      minT=0];

    debugPrint["calcMinTime#minT: ", minT];
    (* Piecewise�ؤ��б� *)
    If[Head[minT] === Piecewise, minT = First[First[First[minT]]]];
    (* 0�ø�Τ�ޤ�ǤϤ����ʤ� *)
    If[includeZero===False && minT===0, minT=error];
    (* 0�ø��Υ���Ѳ����Ԥ��뤫�Υ����å��ʤΤ�0�Ǥʤ���Х��顼 *)
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
        (* ��ʬ�� *)
        (* TODO: Ŭ�ڤʽ����򤹤� *)
        1,
        Switch[First[First[Position[compareResult, x_ /; x =!= False, {1}, Heads -> False]]],
          1, {minT, {{type, askID}}},
          2, {minT, Append[currentMinAsk, {type, askID}]},
          3, {currentMinT, currentMinAsk}]
      ]
    ]
  );


  (* �Ǿ��ͤȾ����Ȥ�ꥹ�ȥ��åפ�����˻Ȥ��ؿ����� *)
  addMinTime[currentList_, {type_, integAsk_, askID_}] := (
    If[integAsk=!=False,
      (* true *)
      (* ��ʤ��ȶ����ͤβ����̤��� *)  
      sol = Quiet[Check[Reduce[{timeMinCons && (maxTime>=t) && (integAsk) && (And @@ otherExpr)}, t],
                        errorSol,
                        {Reduce::nsmet}],
                  {Reduce::nsmet}];
      (*  debugPrint["calcMinTime#sol: ", sol]; *)
      If[sol=!=False && sol=!=errorSol, 
        (* true *)
        (* ����Ω��t�κǾ��ͤ���� *)
        minT = First[Quiet[Minimize[{t, timeMinCons && (sol)}, {t}], 
                           Minimize::wksol]],

        (* false *)
        minT = error
      ],
      (* false *)
      minT=0
    ];

    debugPrint["calcMinTime#minT: ", minT];
    (* 0�ø�Τ�ޤ�ǤϤ����ʤ� *)
    If[includeZero===False && minT===0, minT=error];
    (* 0�ø��Υ���Ѳ����Ԥ��뤫�Υ����å��ʤΤ�0�Ǥʤ���Х��顼 *)
    If[includeZero===True && minT=!=0,
      (* Piecewise�ؤ��б� *)
      If[Head[minT] === Piecewise, minT = First[First[First[minT]]]];
    minT=error];

    If[minT === error,
      (* true *)
      currentList,

      (* false *)
      Append[currentList,{minT, type, askID}]
    ]
  );



  Fold[calcMinTime,
       {maxTime, {}},
       Join[Map[({pos2neg, Not[#[[1]]], #[[2]]})&, posAsk],
            Map[({neg2pos,     #[[1]],  #[[2]]})&, negAsk],
            Fold[(Join[#1, Map[({neg2pos, #[[1]], #[[2]]})&, #2]])&, {}, NACons]]]
  (* �ޤ��ꥹ�ȥ��å� *)
  (*Fold[addMinTime,
       {},
       Join[Map[({pos2neg, Not[#[[1]]], #[[2]]})&, posAsk],
            Map[({neg2pos,     #[[1]],  #[[2]]})&, negAsk],
            Fold[(Join[#1, Map[({neg2pos, #[[1]], #[[2]]})&, #2]])&, {}, NACons]]]*)
   
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

(* �ѥ�᡼����������� *)
getParamCons[cons_] := Cases[cons, x_ /; Not[hasVariable[x]], {1}];
(* ����Υѥ�᡼���ѿ������� *)
getParamVar[paramCons_] := Cases[paramCons, x_ /; Not[NumericQ[x]], Infinity];

(*
 * ask��Ƴ�о��֤��Ѳ�����ޤ���ʬ�򤪤��ʤ�
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
  tmpMinAskIDs,
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
    tmpIntegSol = LogicalExpand[removeNotEqual[Reduce[Complement[cons, paramCons], Join[vars, paramVars], Reals]]];
    (* 1�Ĥ������� *)
    (* TODO: ʣ���򤢤����ͤ��� *)
    If[Head[tmpIntegSol]===Or, tmpIntegSol = First[tmpIntegSol]];

    tmpIntegSol = removeTrivialCons[applyList[tmpIntegSol], Join[vars, paramVars]];

    {DExpr, DExprVars, NDExpr, otherExpr} = splitExprs[tmpIntegSol];

    If[Cases[DExpr, Except[True]] === {},
      tmpIntegSol = {},
      tmpIntegSol = Quiet[DSolve[DExpr, DExprVars, t],
                        {Solve::incnst}];
      (* 1�Ĥ������� *)
      (* TODO:ʣ���򤢤����ͤ��� *)
      tmpIntegSol = First[tmpIntegSol]];

    (* debugPrint["tmpIntegSol: ", tmpIntegSol, "paramVars", paramVars "paramCons", paramCons]; *)
    (* tmpIntegSol�����NDExpr��˽и�����ND�ѿ��ΰ��������� *)
    solVars = getNDVars[Union[Cases[Join[tmpIntegSol, NDExpr], _[t], Infinity]]];
    (* integrateCalc�η׻���̤Ȥ���ɬ�פ��ѿ��ΰ��� *)
    returnVars = Select[vars, (MemberQ[solVars, removeDash[#]]) &];

    tmpIntegSol = First[Solve[Join[Map[(Equal @@ #) &, tmpIntegSol], NDExpr], 
                              getNDVars[returnVars]]];

    tmpPosAsk = Map[(# /. tmpIntegSol ) &, posAsk];
    tmpNegAsk = Map[(# /. tmpIntegSol) &, negAsk];
    tmpNACons = Map[(# /. tmpIntegSol) &, NACons];
    {tmpMinT, tmpMinAskIDs} = 
      calcNextPointPhaseTime[False, maxTime, tmpPosAsk, tmpNegAsk, tmpNACons, Join[otherExpr, paramCons]];

    tmpVarMap = 
      Map[({getVariableName[#], 
            getDerivativeCount[#], 
            createIntegratedValue[#, tmpIntegSol] // FullForm // ToString})&, 
          returnVars];
    endTimeFlag = If[Reduce[tmpMinT >= maxTime && (And @@ Join[otherExpr, paramCons]), paramVars] =!= False, 1, 0],

    (* false *)
    tmpMinT = maxTime;
    tmpVarMap = {};
    tmpMinAskIDs = {};
    endTimeFlag = 1
  ];
  tmpRet = {1,
            ToString[FullForm[tmpMinT]],
            tmpVarMap,
            tmpMinAskIDs, 
            endTimeFlag};
  debugPrint["tmpRet:", tmpRet];
  tmpRet
],
  (* debugPrint["MessageList:", $MessageList]; *)
  {0, $MessageList}
]];

(*
 * �����Ф���Ϳ����줿���֤�Ŭ�Ѥ���
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
 * form����to�ޤǤΥꥹ�Ȥ�interval�ֳ֤Ǻ�������
 * �Ǹ��ɬ��to����뤿��ˡ�
 * �Ǹ�δֳ֤Τ�interval����û���ʤ��ǽ��������
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
 * Ϳ����줿������ʬ�����֤�
 *
 * 0: Solver Error
 * 1: Solved Successfully
 * 2: Under Constraint
 * 3: Over Constraint
 * 
 *)
integrateExpr[cons_, vars_] := Quiet[Check[Block[
{ sol },

(*  debugPrint["cons:", cons, "vars:", vars]; *)
 
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
             ToString[createIntegratedValue[#, sol], InputForm]})&, 
           vars]}]
],
  {0, $MessageList}
]];

(* Print["integ:", integrateExpr[{ht'[t]==v[t], v'[t]==-10, ht[0]==a, v[0]==b}, {ht[t], ht'[t], v[t], v'[t]}]]; *)
(* Print["integ:", integrateExpr[{ht'[t]==v[t], v'[t]==-10, v'[t]==-20, ht[0]==a, v[0]==b}, {ht[t], ht'[t], v[t], v'[t]}]]; *)
(* Print["integ:", integrateExpr[{ht'[t]==x[t], v'[t]==-10, ht[0]==a, v[0]==b}, {x[t], ht[t], ht'[t], v[t], v'[t]}]]; *)

(*
 * Ϳ����줿����������
 *)
approxExpr[precision_, expr_] :=
  Rationalize[N[Simplify[expr], precision + 3], 
              Divide[1, Power[10, precision]]];

(* 
 * Ϳ����줿t�μ��򥿥��ॷ�ե�
 *)
exprTimeShift[expr_, time_] := ToString[FullForm[Simplify[expr /. t -> t - time ]]];
