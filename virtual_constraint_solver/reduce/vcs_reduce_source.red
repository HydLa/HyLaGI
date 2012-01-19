load_package sets;

% グローバル変数
% constraintStore_: 現在扱っている制約集合（リスト形式、PPの定数未対応）
% csVariables_: 制約ストア内に出現する変数の一覧（リスト形式、PPの定数未対応）
% parameterStore_: 現在扱っている、定数制約の集合（リスト形式、IPのみ使用）
% psParameters_: 定数制約の集合に出現する定数の一覧（リスト形式、IPのみ使用）
%
% optUseDebugPrint_: デバッグ出力をするかどうか
% approxPrecision_: checkOrderingFormula内で、数式を数値に近似する際の精度
%

% グローバル変数初期化
% TODO:要検討
approxPrecision_:= 30;

% デバッグ用メッセージ出力関数
% TODO:任意長の引数に対応したい
procedure debugWrite(arg1_, arg2_)$
  if(optUseDebugPrint_) then <<
    write(arg1_, arg2_);
  >> 
  else <<
    1$
  >>;

%MathematicaでいうHead関数
procedure myHead(expr_)$
  if(arglength(expr_)=-1) then nil
  else part(expr_, 0)$

%MathematicaでいうFold関数
procedure myFoldLeft(func_, init_, list_)$
  if(list_ = {}) then init_
  else myFoldLeft(func_, func_(init_, first(list_)), rest(list_))$

procedure getArgsList(expr_)$
  if(arglength(expr_)=-1) then {}
  else for i:=1 : arglength(expr_) collect part(expr_, i)$

%MathematicaでいうApply関数
procedure myApply(func_, expr_)$
  part(expr_, 0):= func_$

procedure getReverseRelop(relop_)$
  if(relop_=equal) then equal
  else if(relop_=neq) then neq
  else if(relop_=geq) then leq
  else if(relop_=greaterp) then lessp
  else if(relop_=leq) then geq
  else if(relop_=lessp) then greaterp
  else nil$

procedure getInverseRelop(relop_)$
  if(relop_=equal) then neq
  else if(relop_=neq) then equal
  else if(relop_=geq) then lessp
  else if(relop_=greaterp) then leq
  else if(relop_=leq) then greaterp
  else if(relop_=lessp) then geq
  else nil$


procedure checkOrderingFormula(orderingFormula_)$
%入力: 論理式(特にsqrt(2), greaterp_, sin(2)などを含むようなもの), 精度
%出力: t or nil or -1
%      (xとyがほぼ等しい時 -1)
%geq_= >=, geq; greaterp_= >, greaterp; leq_= <=, leq; lessp_= <, lessp;
begin;
  scalar head_, x, op, y, bak_precision, ans, margin;

  debugWrite("in checkOrderingFormula", " ");
  debugWrite("orderingFormula_: ", orderingFormula_);

  head_:= myHead(orderingFormula_);
  % 大小に関する論理式以外が入力されたらエラー
  if(hasLogicalOp(head_)) then return ERROR;

  x:= part(orderingFormula_, 1);
  op:= head_;
  y:= part(orderingFormula_, 2);

  debugWrite("-----checkOrderingFormula-----", " ");
  debugWrite("x: ", x);
  debugWrite(" op: ", op);
  debugWrite(" y: ", y);

  if(x=y) then <<
    debugWrite("x=y= ", x);
    if((op = geq) or (op = leq)) then return t
    else return nil
  >>;

  if(not freeof({x,y}, INFINITY)) then <<
    ans:= myInfinityIf(x, op, y);
    debugWrite("ans after myInfinityIf: ", ans);
    return ans;
  >>;
  

  bak_precision := precision 0;
  on rounded$ precision approxPrecision_$

  % xおよびyが有理数である時
  % 10^(3 + yかxの指数部の値 - 有効桁数)
  if(min(x,y)=0) then
    margin:=10 ^ (3 + floor log10 max(x, y) - approxPrecision_)
  else if(min(x,y)>0) then 
    margin:=10 ^ (3 + floor log10 min(x, y) - approxPrecision_)
  else
    margin:=10 ^ (3 - floor log10 abs min(x, y) - approxPrecision_);

  debugWrite("margin:= ", margin);

  debugWrite("x:= ", x);
  debugWrite("y:= ", y);
  debugWrite("abs(x-y):= ", abs(x-y));
  %xとyがほぼ等しい時
  if(abs(x-y)<margin) then <<off rounded$ precision bak_precision$ write(-1); return -1>>;

  if (op = geq) then
    (if (x >= y) then ans:=t else ans:=nil)
  else if (op = greaterp) then
    (if (x > y) then ans:=t else ans:=nil)
  else if (op = leq) then
    (if (x <= y) then ans:=t else ans:=nil)
  else if (op = lessp) then
    (if (x < y) then ans:=t else ans:=nil);

  off rounded$ precision bak_precision$


  debugWrite("ans in checkOrderingFormula: ", ans);
  return ans;
end;


procedure myInfinityIf(x, op, y)$
begin;
  scalar ans, tmpAns_, tupleDNF_, retTuple_;

  debugWrite("in myInfinityIf", " ");
  % INFINITY > -INFINITYとかの対応
  if(x=INFINITY or y=-INFINITY) then 
    if((op = geq) or (op = greaterp)) then ans:=t else ans:=nil
  else if(x=-INFINITY or y=INFINITY) then
    if((op = leq) or (op = lessp)) then ans:=t else ans:=nil
  else <<
    % 係数等への対応として、まずinfinity relop valueの形にしてから解きなおす
    tupleDNF_:= exIneqSolve(op(x, y));
    debugWrite("tupleDNF_: ", tupleDNF_);
    % 1次式であるときは、解は1つなはず
    % TODO：INFINITY^2などへの対応
    retTuple_:= first(first(tupleDNF_));
    ans:= myInfinityIf(part(retTuple_, 1), part(retTuple_, 2), part(retTuple_, 3));
  >>;

  return ans;
end;


procedure getf(x,lst)$
if(lst={}) then nil
	else if(x=lhs(first(lst))) then rhs(first(lst))
		else getf(x,rest(lst))$
procedure lgetf(x,llst)$
%入力: 変数名, 等式のリストのリスト(ex. {{x=1,y=2},{x=3,y=4},...})
%出力: 変数に対応する値のリスト
if(llst={}) then {}
	else if(rest(llst)={}) then getf(x,first(llst))
		else getf(x,first(llst)) . {lgetf(x,rest(llst))}$

procedure mymin(x,y)$
%入力: 数値という前提
if(checkOrderingFormula(x<y)) then x else y$

procedure mymax(x,y)$
%入力: 数値という前提
if(checkOrderingFormula(x>y)) then x else y$

procedure myFindMinimumValue(x,lst)$
%入力: 現段階での最小値x, 最小値を見つけたい対象のリスト
%出力: リスト中の最小値
if(lst={}) then x
else if(mymin(x, first(lst)) = x) then myFindMinimumValue(x,rest(lst))
else myFindMinimumValue(first(lst),rest(lst))$

procedure myFindMaximumValue(x,lst)$
%入力: 現段階での最大値x, 最大値を見つけたい対象のリスト
%出力: リスト中の最大値
if(lst={}) then x
else if(mymax(x, first(lst)) = x) then myFindMaximumValue(x,rest(lst))
else myFindMaximumValue(first(lst),rest(lst))$

procedure compareValueAndParameter(val_, paramExpr_, condDNF_)$
%入力: 比較する対象の値, パラメータを含む式, パラメータに関する条件
%出力: {「値」側が小さくなるためのパラメータの条件, 「パラメータを含む式」が小さくなるためのパラメータの条件}
begin;
  scalar valueLeqParamCondSol_, valueGreaterParamCondSol_, 
         ret_;

  debugWrite("in compareValueAndParameter", " ");
  debugWrite("val_: ", val_);
  debugWrite("paramExpr_: ", paramExpr_);
  debugWrite("condDNF_: ", condDNF_);


  % minValue_≦paramの場合の条件式
  valueLeqParamCondSol_:= addCondTupleToCondDNF({val_, leq, paramExpr_}, condDNF_);
  debugWrite("valueLeqParamCondSol_: ", valueLeqParamCondSol_);

  % minValue_＞paramの場合の条件式
  valueGreaterParamCondSol_:= addCondTupleToCondDNF({val_, greaterp, paramExpr_}, condDNF_);
  debugWrite("valueGreaterParamCondSol_: ", valueGreaterParamCondSol_);

  return {valueLeqParamCondSol_, valueGreaterParamCondSol_};
end;

% 入力：デフォルトの最小値に関する「tの式と条件の組」defaultTC_, 候補となる「tの式と条件の組」のリストTCList_
% 出力：(最小値, それを与える定数の条件)の組のリスト
% 返す値は必ず0より大きいものになるようにしている点に注意（0以下はエラー扱い）
procedure myFindMinimumValueCond(defaultTC_, TCList_)$
begin;
  scalar ineqTCList_, ineqList_, valueTCList_, paramTCList_, numberTCList_,
         minValueTCList_, minValue_, splitIneqsResult_, ubList_, lbList_, maxUb_, minLb_,
         retTCList_, defaultCond_;

  debugWrite("in myFindMinimumValueCond", " ");
  debugWrite("defaultTC_: ", defaultTC_);
  debugWrite("TCList_: ", TCList_);

  defaultCond_:= part(defaultTC_, 2);

  ineqTCList_:= for each x in TCList_ join 
    if(hasInequality(part(x, 1)) or hasLogicalOp(part(x, 1))) then {x} else {};
  valueTCList_:= TCList_ \ ineqTCList_;
  paramTCList_:= union(for each x in valueTCList_ join 
    if(hasParameter(part(x, 1))) then {x} else {});
  numberTCList_:= valueTCList_ \ paramTCList_;
  debugWrite("ineqTCList_: ", ineqTCList_);
  debugWrite("valueTCList_: ", valueTCList_);
  debugWrite("paramTCList_: ", paramTCList_);
  debugWrite("numberTCList_: ", numberTCList_);


  % 不等式の集合から、最小値を求める
  ineqList_:= for each x in ineqTCList_ collect part(x, 1);
  debugWrite("ineqList_: ", ineqList_);
  splitIneqsResult_:= getIneqBoundLists(ineqList_);

  % すべての上限は0より小さくなくてはならない
  ubList_:= part(splitIneqsResult_, 2);
  maxUb_:= myfindMaximumValue(0, ubList_);
  debugWrite("maxUb_: ", maxUb_);
  if(maxUb_ neq 0) then return {{ERROR, defaultCond_}};

  % 最も小さい下限は最小値となりうる
  lbList_:= part(splitIneqsResult_, 1);
  minLb_:= myfindMinimumValue(INFINITY, lbList_);
  debugWrite("minLb_: ", minLb_);

  % valueとdefault_と不等式の中での最小値を求める
  minValue_:= mymin(part(defaultTC_, 1), minLb_);
  minValueTCList_:= for each x in numberTCList_ join 
    if(mymin(minValue_, part(x, 1)) = part(x, 1)) then {x} else {};
  debugWrite("minValueTCList_: ", minValueTCList_);

  % paramがない場合はこの後の処理は不要
  if(paramTCList_={}) then return minValueTCList_;


  % paramTCとminValueTCとの比較（condition_によって結果が変わることがある）
  % 前提：condition_内の定数の種類は1つまで？
  % TODO：なんとかする
  if(minValueTCList_ neq {}) then
    retTCList_:= for each x in paramTCList_ join     
      for each y in minValueTCList_ join compareMinTime(x, y)
  else retTCList_:= myFoldLeft(compareMinTime, {INFINITY, defaultCond_}, paramTCList_); %？？？

  debugWrite("retTCList_: ", retTCList_);
  return retTCList_;
end;

%待ち行列I関係
procedure enq(state,queue);
  begin;
    return append(queue,{state});
  end;


procedure deq queue;
  begin;
    if(queue={}) then return nil;
    elem:= first(queue);
    return queue:= rest(queue);
  end;

load_package redlog; rlset R;

%数式のリストをandで繋いだ論理式に変換する
procedure mymkand(lst)$
for i:=1:length(lst) mkand part(lst,i);

procedure mymkor(lst)$
for i:=1:length(lst) mkor part(lst, i);

procedure myex(lst,var)$
rlqe ex(var, mymkand(lst));

procedure myall(lst,var)$
rlqe all(var, mymkand(lst));

procedure exRlqe(formula_)$
begin;
  scalar appliedFormula_, header_, argsCount_, appliedArgsList_;
  debugWrite("formula_: ", formula_);

  % 引数を持たない場合（trueなど）
  if(arglength(formula_)=-1) then <<
    appliedFormula_:= rlqe(formula_);
    return appliedFormula_;
  >>;

  header_:= part(formula_, 0);
  debugWrite("header_: ", header_);
  argsCount_:= arglength(formula_);

  if((header_=and) or (header_=or)) then <<
    argsList_:= for i:=1 : argsCount_ collect part(formula_, i);
    appliedArgsList_:= for each x in argsList_ collect exRlqe(x);
    if(header_= and) then appliedFormula_:= rlqe(mymkand(appliedArgsList_));
    if(header_= or) then appliedFormula_:= rlqe(mymkor(appliedArgsList_));
  >> else if(not freeof(formula_, sqrt)) then <<
    % 数式内にsqrtが入っている時のみ、厳密な大小比較が有効となる
    % TODO:当該の数式内に変数が入った際にも正しく処理ができるようにする
    if(checkOrderingFormula(myApply(getInverseRelop(header_), {part(formula_, 1), part(formula_, 2)}))) then 
      appliedFormula_:= false 
    else appliedFormula_:= rlqe(formula_)
  >> else <<
    appliedFormula_:= rlqe(formula_)
  >>;

  debugWrite("appliedFormula_: ", appliedFormula_);
  return appliedFormula_;

end;


procedure getFrontTwoElemList(lst_)$
  if(length(lst_)<2) then {}
  else for i:=1 : 2 collect part(lst_, i)$

% TODO:3乗根以上への対応
procedure rationalisation(expr_)$
begin;
  scalar head_, denominator_, numerator_, denominatorHead_, argsList_,
         frontTwoElemList_, restElemList_, timesRhs_, conjugate_, 
         rationalisedArgsList_, rationalisedExpr_, flag_;

  debugWrite("expr_: ", expr_);
  if(getArgsList(expr_)={}) then return expr_;

  % 想定する対象：分母の項数が4まで
  % TODO:より一般的な形への対応→5項以上？
  % TODO:3乗根以上への対応

  head_:= myHead(expr_);
  debugWrite("head_: ", head_);

  if(head_=quotient) then <<
    numerator_:= part(expr_, 1);
    denominator_:= part(expr_, 2);
    % 分母に無理数がなければ有理化必要なし
    if(numberp(denominator_)) then return expr_;

    denominatorHead_:= myHead(denominator_);
    debugWrite("denominatorHead_: ", denominatorHead_);
    if((denominatorHead_=plus) or (denominatorHead_=times)) then <<
      if(denominatorHead_=plus) then argsList_:=
    getArgsList(denominator_)
      else argsList_:= getArgsList(part(denominator_, 2));
      debugWrite("argsList_: ", argsList_);

      % 項数が3以上の場合、確実に無理数が減るように工夫して共役数を求める
      if(length(argsList_)>2) then <<
        frontTwoElemList_:= getFrontTwoElemList(argsList_);
        debugWrite("frontTwoElemList_: ", frontTwoElemList_);
        restElemList_:= argsList_ \ frontTwoElemList_;
        debugWrite("restElemList_: ", restElemList_);
        if(denominatorHead_=plus) then <<
          conjugate_:= plus(myApply(plus, frontTwoElemList_), -1*(myApply(plus, restElemList_)));
        >> else <<
          % 前提：積の右辺はすべてplusでつながっている形式(-5は+(-5)のように)
          % TODO：そうでない場合でも平気なように？
          timesRhs_:= plus(myApply(plus, frontTwoElemList_), -1*(myApply(plus, restElemList_)));
          conjugate_:= part(denominator_, 1) * timesRhs_;
        >>;
      >> else <<
        if(denominatorHead_=plus) then <<
          conjugate_:= plus(part(argsList_, 1), -1*part(argsList_, 2));
        >> else <<
          timesRhs_:= plus(part(argsList_, 1), -1*part(argsList_, 2));
          conjugate_:= part(denominator_, 1) * timesRhs_;
        >>;
      >>;
    >> else if(denominatorHead_=difference) then <<
      conjugate_:= difference(part(denominator_, 1), -1*part(denominator_, 2));
    >> else <<
      conjugate_:= -1*denominator_;
    >>;
    debugWrite("conjugate_: ", conjugate_);
    % 共役数を分母子にかける
    numerator_:= numerator_ * conjugate_;
    denominator_:= denominator_ * conjugate_;
    rationalisedExpr_:= numerator_ / denominator_;
    flag_:= true;
  >> else if(length(expr_)>1) then <<
    rationalisedArgsList_:= map(rationalisation, getArgsList(expr_));
    debugWrite("rationalisedArgsList_: ", rationalisedArgsList_);
    rationalisedExpr_:= myApply(head_, rationalisedArgsList_);
  >> else <<
    rationalisedExpr_:= expr_;
  >>;

  debugWrite("rationalisedExpr_: ", rationalisedExpr_);
  debugWrite("flag_: ", flag_);
  if(flag_=true) then rationalisedExpr_:= rationalisation(rationalisedExpr_);
  return rationalisedExpr_;
end;


procedure bball_out()$
% gnuplot用出力, 未完成
% 正規表現 {|}|\n
<<
off nat; 
out "out";
write(t=lt);
write(y=ly);
write(v=lv);
write(fy=lfy);
write(fv=lfv);
write(";end;");
shut "out";
on nat;
>>$

%procegure myout(x,t)$

%---------------------------------------------------------------
% HydLa向け関数
%---------------------------------------------------------------

operator prev;

%rettrue___ := "RETTRUE___";
%retfalse___ := "RETFALSE___";
rettrue___ := 1;
retfalse___ := 2;

% 関数呼び出しはredevalを経由させる
% <redeval> end:の次が最終行

symbolic procedure redeval(foo_)$
begin scalar ans_;

  debugWrite("<redeval> reval :", (car foo_));
  ans_ :=(reval foo_);
  write("<redeval> end:");

  return ans_;
end;



% 制約ストアのリセット

procedure resetConstraintStore()$
begin;
  putLineFeed();

  constraintStore_ := {};
  csVariables_ := {};
  parameterStore_:= {};
  psParameters_:= {};
  debugWrite("constraintStore_: ", constraintStore_);
  debugWrite("csVariables_: ", csVariables_);
  debugWrite("parameterStore_: ", parameterStore_);
  debugWrite("psParameters_: ", psParameters_);

end;

procedure getConstraintStore()$
begin;
  scalar ret_;
  putLineFeed();

  ret_:= {constraintStore_, parameterStore_};
  debugWrite("ret_: ", ret_);
  return ret_;
end;

% 制約ストアへの制約の追加

procedure addConstraint(cons_, vars_)$
begin;
  putLineFeed();

  debugWrite("in addConstraint", " ");
  debugWrite("constraintStore_: ", constraintStore_);
  debugWrite("csVariables_: ", csVariables_);
  debugWrite("cons_: ", cons_);
  debugWrite("vars_:", vars_);

  constraintStore_ := union(constraintStore_, cons_);
  csVariables_ := union(csVariables_, vars_);
  debugWrite("new constraintStore_: ", constraintStore_);
  debugWrite("new csVariables_: ", csVariables_);

end;

% PP/IPで共通のreset時に行う、制約ストアへの制約の追加

procedure addConstraintReset(cons_, vars_, pcons_, pars_)$
  begin;
  putLineFeed();

  debugWrite("in addConstraintReset", " ");
  debugWrite("parameterStore_: ", parameterStore_);
  debugWrite("psParameters_: ", psParameters_);
  debugWrite("pcons_: ", pcons_);
  debugWrite("pars_:", pars_);

  parameterStore_ := union(parameterStore_, pcons_);
  psParameters_ := union(psParameters_, pars_);
  debugWrite("new parameterStore_: ", parameterStore_);
  debugWrite("new psParameters_: ", psParameters_);

  return addConstraint(cons_, vars_);

end;

% (制限 andを受け付けない) TODO 制限への対応
% (制限 trueを受け付けない) TODO 制限への対応

procedure checkConsistencyWithTmpCons(expr_, lcont_, vars_)$
begin;
  scalar ans_;
  putLineFeed();

  ans_:= {part(checkConsistencyBySolveOrRlqe(expr_, lcont_, vars_), 1)};
  debugWrite("ans_ in checkConsistencyWithTmpCons: ", ans_);

  return ans_;
end;

procedure removeTrueList(patternList_)$
  for each x in patternList_ join if(rlqe(x)=true) then {} else {x}$

% 論理式がtrueであるとき、現在はそのままtrueを返している
% TODO：なんとかする
procedure removeTrueFormula(formula_)$
  if(formula_=true) then true
  else myApply(and, for each x in getArgsList(formula_) join 
    if(rlqe(x)=true) then {} else {x})$

% expr_中に等式以外や論理演算子が含まれる場合に用いる置換関数
procedure exSub(patternList_, expr_)$
begin;
  scalar subAppliedExpr_, head_, subAppliedLeft_, subAppliedRight_, 
         argCount_, subAppliedExprList_, test_;

  debugWrite("in exSub", " ");
  debugWrite("patternList_: ", patternList_);
  debugWrite("expr_: ", expr_);
  
  % expr_が引数を持たない場合
  if(arglength(expr_)=-1) then <<
    subAppliedExpr_:= sub(patternList_, expr_);
    return subAppliedExpr_;
  >>;
  % patternList_からTrueを意味する制約を除く
  patternList_:= removeTrueList(patternList_);
  debugWrite("patternList_: ", patternList_);

  head_:= myHead(expr_);
  debugWrite("head_: ", head_);

  % orで結合されるもの同士を括弧でくくらないと、neqとかが違う結合のしかたをする可能性あり
  if((head_=neq) or (head_=geq) or (head_=greaterp) or (head_=leq) or (head_=lessp)) then <<
    % 等式以外の関係演算子の場合
    subAppliedLeft_:= exSub(patternList_, part(expr_, 1));
    debugWrite("subAppliedLeft_:", subAppliedLeft_);
    subAppliedRight_:= exSub(patternList_, part(expr_, 2));
    debugWrite("subAppliedRight_:", subAppliedRight_);
    % なぜかエラーが起きるようになった？？
    % subAppliedExpr_:= head_(subAppliedLeft_, subAppliedRight_);
    subAppliedExpr_:= if(head_=neq) then neq(subAppliedLeft_, subAppliedRight_)
                      else if(head_=geq) then geq(subAppliedLeft_, subAppliedRight_)
                      else if(head_=greaterp) then greaterp(subAppliedLeft_, subAppliedRight_)
                      else if(head_=leq) then leq(subAppliedLeft_, subAppliedRight_)
                      else if(head_=lessp) then lessp(subAppliedLeft_, subAppliedRight_);
  >> else if((head_=and) or (head_=or)) then <<
    % 論理演算子の場合
    argCount_:= arglength(expr_);
    debugWrite("argCount_: ", argCount_);
    subAppliedExprList_:= for i:=1 : argCount_ collect exSub(patternList_, part(expr_, i));
    debugWrite("subAppliedExprList_:", subAppliedExprList_);
    subAppliedExpr_:= myApply(head_, subAppliedExprList_);

  >> else <<
    % 等式や、変数名などのfactorの場合
    % TODO:expr_を見て、制約ストア（あるいはcsvars）内にあるようなら、それと対をなす値（等式の右辺）を適用
    subAppliedExpr_:= sub(patternList_, expr_);
  >>;

  debugWrite("subAppliedExpr_:", subAppliedExpr_);
  return subAppliedExpr_;
end;


procedure exSolve(exprs_, vars_)$
begin;
  % 実数解のみを返すようにする


end;

% PPにおける無矛盾性の判定
% 返り値は{ans, {{変数名 = 値},...}} の形式
% 仕様 QE未使用 % (使用するなら, 変数は基本命題的に置き換え)

procedure checkConsistencyBySolveOrRlqe(exprs_, lcont_, vars_)$
begin;
  scalar exprList_, eqExprs_, otherExprs_, modeFlagList_, mode_, tmpSol_,
         solvedExprs_, solvedExprsQE_, ans_;

  debugWrite("checkConsistencyBySolveOrRlqe: ", " ");
  debugWrite("exprs_: ", exprs_);
  debugWrite("lcont_: ", lcont_);
  debugWrite("vars_: ", vars_);

  exprList_:= union(constraintStore_, exprs_);
  debugWrite("exprList_: ", exprList_);
  otherExprs_:= getOtherExpr(exprList_);
  debugWrite("otherExprs_: ", otherExprs_);
  eqExprs_:= exprList_ \ otherExprs_;
  debugWrite("eqExprs_: ", eqExprs_);
  debugWrite("union(eqExprs_, lcont_):",  union(eqExprs_, lcont_));
  debugWrite("union(csVariables_, vars_):", union(csVariables_, vars_));

  % 未知変数を追加しないようにする
  off arbvars;
  tmpSol_:= solve(union(eqExprs_, lcont_),  union(csVariables_, vars_));  on arbvars;
  debugWrite("tmpSol_: ", tmpSol_);

  if(tmpSol_={}) then return {retfalse___};
  % 2重リストの時のみfirstで得る
  % TODO:複数解得られた場合への対応
  if(myHead(first(tmpSol_))=list) then tmpSol_:= first(tmpSol_);


  % exprs_および制約ストアに等式以外が入っているかどうかにより、解くのに使用する関数を決定
  mode_:= if(otherExprs_={}) then SOLVE else RLQE;
  debugWrite("mode_:", mode_);

  if(mode_=SOLVE) then
  <<
    ans_:=tmpSol_;
    debugWrite("ans_ in checkConsistencyBySolveOrRlqe: ", ans_);
    if(ans_ <> {}) then return {rettrue___, ans_} else return {retfalse___};
  >> else
  <<
    % subの拡張版を用いる手法
    solvedExprs_:= union(for each x in otherExprs_ join {exSub(tmpSol_, x)});
    debugWrite("solvedExprs_:", solvedExprs_);
    solvedExprsQE_:= exRlqe(mymkand(solvedExprs_));
    debugWrite("solvedExprsQE_:", solvedExprsQE_);
    debugWrite("union(tmpSol_, solvedExprsQE_):", union(tmpSol_, {solvedExprsQE_}));
%    ans_:= exRlqe(mymkand(union(tmpSol_, {solvedExprs_})));
    ans_:= rlqe(mymkand(union(tmpSol_, {solvedExprsQE_})));
    debugWrite("ans_ in checkConsistencyBySolveOrRlqe: ", ans_);
%    if(ans_ <> false) then return {rettrue___, ans_}
    if(ans_ <> false) then return {rettrue___, union(tmpSol_, solvedExprs_)} 
    else return {retfalse___};
  >>;
end;


procedure checkConsistency()$
begin;
  scalar sol_;
  putLineFeed();

  sol_:= checkConsistencyBySolveOrRlqe({}, {}, {});
  debugWrite("sol_ in checkConsistency: ", sol_);
  % ret_codeがrettrue___、つまり1であるかどうかをチェック
  if(part(sol_, 1) = 1) then constraintStore_:= part(sol_, 2);
  debugWrite("constraintStore_: ", constraintStore_);

end;

procedure getReverseCons(cons_)$
begin;
  scalar reverseRelop_, lhs_, rhs_;

  reverseRelop_:= getReverseRelop(myHead(cons_));
  lhs_:= part(cons_, 1);
  rhs_:= part(cons_, 2);
  return reverseRelop_(rhs_, lhs_);
end;

procedure applyPrevCons(csList_, retList_)$
begin;
  scalar firstCons_, newCsList_, ret_;
  debugWrite("in applyPrevCons", " ");
  if(csList_={}) then return retList_;

  firstCons_:= first(csList_);
  debugWrite("firstCons_: ", firstCons_);
  if(not freeof(part(firstCons_, 1), prev)) then <<
    newCsList_:= union(for each x in rest(csList_) join {exSub({firstCons_}, x)});
    ret_:= applyPrevCons(rest(csList_), retList_);
  >> else if(not freeof(part(firstCons_, 2), prev)) then <<
    ret_:= applyPrevCons(cons(getReverseCons(firstCons_), rest(csList_)), retList_);
  >> else <<
    ret_:= applyPrevCons(rest(csList_), cons(firstCons_, retList_));
  >>;
  return ret_;
end;

procedure getExprCode(cons_)$
begin;
  scalar head_;

  % relopが引数として直接渡された場合へも対応
  if(arglength(cons_)=-1) then head_:= cons_
  else head_:= myHead(cons_);

  if(head_=equal) then return 0
  else if(head_=lessp) then return 1
  else if(head_=greaterp) then return 2
  else if(head_=leq) then return 3
  else if(head_=geq) then return 4
  else return nil;
end;

% convertCSToVM内で使う、整形用関数
procedure makeConsTuple(cons_)$
begin;
  scalar varName_, relopCode_, value_, tupleDNF_, retTuple_, adjustedCons_, sol_;

  debugWrite("in makeConsTuple", " ");
  debugWrite("cons_: ", cons_);
  
  % 左辺に変数名のみがある形式にする
  % 前提：等式はすでにこの形式になっている
  if(not hasInequality(cons_)) then <<
    varName_:= part(cons_, 1);
    relopCode_:= getExprCode(cons_);
    value_:= part(cons_, 2);
  >> else <<
    tupleDNF_:= exIneqSolve(cons_);
    debugWrite("tupleDNF_: ", tupleDNF_);
    % 1次式になってるはずなので、解は1つなはず
    retTuple_:= first(first(tupleDNF_));
    
    varName_:= part(retTuple_, 1);
    relopCode_:= getExprCode(part(retTuple_, 2));
    value_:= part(retTuple_, 3);
  >>;
  debugWrite("varName_: ", varName_);
  debugWrite("relopCode_: ", relopCode_);
  debugWrite("value_: ", value_);
  return {varName_, relopCode_, value_};
end;

% 前提：Orでつながってはいない
% TODO：なんとかする
procedure convertCSToVM()$
begin;
  scalar tmpRet_, ret_;
  putLineFeed();

  debugWrite("constraintStore_:", constraintStore_);
  if(constraintStore_={}) then return {};

  tmpRet_:= applyPrevCons(constraintStore_, {});    
  debugWrite("tmpRet_: ", tmpRet_);

  % 式を{(変数名), (関係演算子コード), (値のフル文字列)}の形式に変換する
  ret_:= map(makeConsTuple, tmpRet_);
  debugWrite("ret_: ", ret_);
  return ret_;
end;


procedure returnCS()$
begin;
  putLineFeed();

  debugWrite("constraintStore_:", constraintStore_);
  if(constraintStore_={}) then return {};

  % 解を1つだけ得る
  % TODO: Orでつながった複数解への対応
  % 2重リスト状態なら1レベル内側を返す。1重リストならそのまま返す
  if(myHead(part(constraintStore_, 1))=list) then return part(constraintStore_, 1)
  else return constraintStore_;
end;


procedure hasInequality(expr_)$
  if(freeof(expr_, neq) and freeof(expr_, not) and
    freeof(expr_, geq) and freeof(expr_, greaterp) and
    freeof(expr_, leq) and freeof(expr_, lessp)) then nil else t$

procedure hasLogicalOp(expr_)$
  if(freeof(expr_, and) and freeof(expr_, or)) then nil else t$

% 制約リストから、等式以外を含む制約を抽出する
procedure getOtherExpr(exprs_)$
  for each x in exprs_ join if(hasInequality(x) or hasLogicalOp(x)) then {x} else {}$

% 式中にパラメタが含まれているかどうかを、psParameters_内の変数が含まれるかどうかで判定
procedure hasParameter(expr_)$
  if(collectParameters(expr_) neq {}) then t else nil$

% 式構造中のパラメタを、集める
procedure collectParameters(expr_)$
begin;
  scalar collectedParameters_;

  debugWrite("in collectParameters", " ");
  debugWrite("expr_: ", expr_);

  debugWrite("psParameters_: ", psParameters_);
  collectedParameters_:= union({}, for each x in psParameters_ join if(not freeof(expr_, x)) then {x} else {});

  debugWrite("collectedParameters_: ", collectedParameters_);
  return collectedParameters_;
end;


%% 式構造中の定数（無理数・パラメタ）を集める
%procedure collectParameters(expr_, lst_)$
%begin;
%  scalar retList_, lhsRet_, rhsRet_, newLst_;
%
%  if((hasInequality(expr_)) or (myHead(expr_)=equal)) then <<
%    lhsRet_:= collectParameters(part(expr_, 1), lst_);
%    rhsRet_:= collectParameters(part(expr_, 2), lst_);
%    retList_:= union(lhsRet_, rhsRet_);
%  >> else if(hasParameter(expr_)) then <<
%    % expr_をlst_に新しく登録する
%    % すでに登録してある中に含まれる場合はそっちを消した上で次を見る
%    newLst_:= union(for each x in lst_ join 
%      if(not freeof(x, expr_)) then {} else {x});
%    newLst_:= cons(expr_, newLst_);
%    lhsRet_:= collectParamaeters(part(expr_, 1), newLst_);
%    rhsRet_:= collectParamaeters(part(expr_, 2), newLst_);
%    retList_:= union(lhsRet_, rhsRet_);
%  >> else <<
%    retList_:= lst_;
%  >>;
%
%  debugWrite("retList_: ", retList_);
%  return retList_;
%end;


load_package "laplace";
% 逆ラプラス変換後の値をsin, cosで表示するスイッチ
on ltrig;

% {{v, v(t), lapv(s)},...}の対応表、グローバル変数
% exDSolveのexceptdfvars_に対応させるため、exDSolve最後で空集合を代入し初期化している
table_:={};

% operator宣言されたargs_を記憶するグローバル変数
loadedOperator_:={};

% 初期条件init○_○lhsを作成
procedure makeInitId(f,i)$
if(i=0) then
  mkid(mkid(INIT,f),lhs)
else
  mkid(mkid(mkid(mkid(INIT,f),_),i),lhs);

%laprule_用、mkidしたｆを演算子として返す
procedure setMkidOperator(f,x)$
  f(x);

%laprule_用の自由演算子
operator !~f$

% 微分に関する変換規則laprule_, letは一度で
let {
  laplace(df(~f(~x),x),x) => il!&*laplace(f(x),x) - makeInitId(f,0),
  laplace(df(~f(~x),x,~n),x) => il!&**n*laplace(f(x),x) -
    for i:=n-1 step -1 until 0 sum
      makeInitId(f,n-1-i) * il!&**i,
  laplace(~f(~x),x) => setMkidOperator(mkid(lap,f),il!&)
}$

% ラプラス変換対の作成, オペレータ宣言
% {{v, v(t), lapv(s)},...}の対応表table_の作成
procedure LaplaceLetUnit(args_)$
begin;
  scalar arg_, LAParg_;

  arg_:= first args_;
  LAParg_:= second args_;

  % arg_が重複してないか判定
  if(freeof(loadedOperator_,arg_)) then 
    << 
     operator arg_, LAParg_;
     loadedOperator_:= arg_ . loadedOperator_;
    >>;

  % {{v, v(t), lapv(s)},...}の対応表
  table_:= {arg_, arg_(t), LAParg_(s)} . table_;
  debugWrite("table_: ", table_);
end;

% vars_からdfを除いたものを返す
procedure removedf(vars_)$
begin;
  exceptdfvars_:={};
  for each x in vars_ collect
    if(freeof(x,df)) then exceptdfvars_:=x . exceptdfvars_;
  return exceptdfvars_;
end;

retsolvererror___ := 0;
retoverconstraint___ := 2;
retunderconstraint___ := 3;

procedure exDSolve(expr_, init_, vars_)$
begin;
  scalar flag_, ans_, tmp_;
  scalar exceptdfvars_, diffexpr_, LAPexpr_, solveexpr_, solvevars_, solveans_, ans_;
 
  debugWrite("in exDSolve", " ");
  debugWrite("expr_: ", expr_);
  debugWrite("init_: ", init_);
  debugWrite("vars_: ", vars_);

  exceptdfvars_:= removedf(vars_);
  tmp_:= for each x in exceptdfvars_ collect {x,mkid(lap,x)};
  debugWrite("tmp_ before LaplaceLetUnit: ", tmp_);
  % ラプラス変換規則の作成
  map(LaplaceLetUnit, tmp_);

  %ht => ht(t)置換
  tmp_:=map(first(~w)=second(~w), table_);
  debugWrite("MAP: ", tmp_);

  tmp_:= sub(tmp_, expr_);
  debugWrite("SUB: ", tmp_);

  % expr_を等式から差式形式に
  diffexpr_:={};
  for each x in tmp_ do 
    if(not freeof(x, equal))
      then diffexpr_:= append(diffexpr_, {lhs(x)-rhs(x)})
    else diffexpr_:= append(diffexpr_, {lhs(x)-rhs(x)});
      
  % laplace演算子でエラー時、laplace演算子込みの式が返ると想定
  if(not freeof(LAPexpr_, laplace)) then return retsolvererror___;

  % ラプラス変換
  LAPexpr_:=map(laplace(~w,t,s), diffexpr_);
  debugWrite("LAPexpr_: ", LAPexpr_);

  % init_制約をLAPexpr_に適応
  solveexpr_:= append(LAPexpr_, init_);
  debugWrite("solveexpr_:", solveexpr_);

  % 逆ラプラス変換の対象
  solvevars_:= append(append(map(third, table_), map(lhs, init_)), {s});
  debugWrite("solvevars_:", solvevars_);

  % 変換対と初期条件を連立して解く
  solveans_ := solve(solveexpr_, solvevars_);
  debugWrite("solveans_: ", solveans_);

  % solveが解無しの時 overconstraintと想定
  if(solveans_={}) then return retoverconstraint___;
  % sがarbcomplexでない値を持つ時 overconstraintと想定
  if(freeof(lgetf(s, solveans_), arbcomplex)) then  return retoverconstraint___;
  % solveans_にsolvevars_の解が一つでも含まれない時 underconstraintと想定
  for each x in table_ do 
    if(freeof(solveans_, third(x))) then tmp_:=true;
  if(tmp_=true) then return retunderconstraint___;
  
  % solveans_の逆ラプラス変換
  ans_:= for each table in table_ collect
      (first table) = invlap(lgetf((third table), solveans_),s,t);
  debugWrite("ans expr?: ", ans_);

  table_:={};
  return ans_;
end;

%depend {ht,v}, t;
%expr_:={df(ht,t) = v,
%        df(v,t) = -10
%       };
%init_:={inithtlhs = 10,
%        initvlhs = 0
%       };
%vars_:={ht,v,df(ht,t),df(v,t)};
%exDSolve(expr_, init_, vars_);


% NDExpr（exDSolveで扱えないような制約式）であるかどうかを調べる
% 式の中にsinもcosも入っていなければfalse
procedure isNDExpr(expr_)$
  if(freeof(expr_, sin) and freeof(expr_, cos)) then nil else t$


procedure splitExprs(exprs_, vars_)$
begin;
  scalar otherExprs_, NDExprs_, DExprs_, DExprVars_;

  otherExprs_:= union(for each x in exprs_ join 
                  if(hasInequality(x) or hasLogicalOp(x)) then {x} else {});
  NDExprs_ := union(for each x in setdiff(exprs_, otherExprs_) join 
                if(isNDExpr(x)) then {x} else {});
  DExprs_ := setdiff(setdiff(exprs_, otherExprs_), NDExprs_);
  DExprVars_:= union(for each x in vars_ join if(not freeof(DExprs_, x)) then {x} else {});
  return {NDExprs_, DExprs_, DExprVars_, otherExprs_};
end;


procedure getNoDifferentialVars(vars_)$
  union(for each x in vars_ join if(freeof(x, df)) then {x} else {})$

procedure removePrev(var_)$
  if(myHead(var_)=prev) then part(var_, 1) else var_$

procedure removePrevVars(varsList_)$
  select(freeof(~x, prev), varsList_)$

procedure removePrevCons(consList_)$
  select(freeof(~x, prev), consList_)$

% 前提：initxlhs=prev(x)の形
% TODO：なんとかする
procedure getInitVars(rcont_)$
  part(rcont_, 1)$

% 20110705 overconstraint___無し
ICI_SOLVER_ERROR___:= 0;
ICI_CONSISTENT___:= 1;
ICI_INCONSISTENT___:= 2;
ICI_UNKNOWN___:= 3; % 不要？

procedure checkConsistencyInterval(tmpCons_, rconts_, vars_)$
begin;
  scalar tmpSol_, splitExprsResult_, NDExprs_, DExprs_, DExprVars_, otherExprs_,
         initCons_, initVars_,
         integTmp_, integTmpQE_, integTmpQEList_, integTmpSolList_, infList_, ans_;
  putLineFeed();

  debugWrite("constraintStore_: ", constraintStore_);
  debugWrite("csVariables_: ", csVariables_);
  debugWrite("tmpCons_: ", tmpCons_);
  debugWrite("rconts_: ", rconts_);
  debugWrite("vars_: ", vars_);

  % SinやCosが含まれる場合はラプラス変換不可能なのでNDExpr扱いする
  % TODO:なんとかしたいところ？
  splitExprsResult_ := splitExprs(removePrevCons(constraintStore_), csVariables_);
  NDExprs_ := part(splitExprsResult_, 1);
  debugWrite("NDExprs_: ", NDExprs_);
  DExprs_ := part(splitExprsResult_, 2);
  debugWrite("DExprs_: ", DExprs_);
  DExprVars_ := part(splitExprsResult_, 3);
  debugWrite("DExprVars_: ", DExprVars_);
  otherExprs_:= part(splitExprsResult_, 4);
  debugWrite("otherExprs_: ", otherExprs_);

  initCons_:= union(for each x in rconts_ join {exSub(constraintStore_, x)});
  debugWrite("initCons_: ", initCons_);
  initVars_:= map(getInitVars, rconts_);
  debugWrite("initVars_: ", initVars_);

  tmpSol_:= exDSolve(DExprs_, initCons_, union(DExprVars_, (vars_ \ initVars_)));
  debugWrite("tmpSol_: ", tmpSol_);
  
  if(tmpSol_ = retsolvererror___) then return {ICI_SOLVER_ERROR___}
  else if(tmpSol_ = retoverconstraint___) then return {ICI_INCONSISTENT___};

  % NDExpr_を連立
  debugWrite("getNoDifferentialVars(union(DExprVars_, (vars_ \ initVars_))): ", 
             getNoDifferentialVars(union(DExprVars_, (vars_ \ initVars_))));
  tmpSol_:= solve(union(tmpSol_, NDExprs_), 
                  getNoDifferentialVars(union(DExprVars_, (vars_ \ initVars_))));
  debugWrite("tmpSol_ after solve: ", tmpSol_);

  % tmpCons_がない場合は無矛盾と判定して良い
  if(tmpCons_ = {}) then return {ICI_CONSISTENT___};

  integTmp_:= sub(tmpSol_, tmpCons_);
  debugWrite("integTmp_: ", integTmp_);

  integTmpQE_:= rlqe(mymkand(integTmp_));
  debugWrite("integTmpQE_: ", integTmpQE_);

  % ただのtrueやfalseはそのまま判定結果となる
  if(integTmpQE_ = true) then return {ICI_CONSISTENT___}
  else if(integTmpQE_ = false) then return {ICI_INCONSISTENT___};


  % まず、andでつながったtmp制約をリストに変換
  integTmpQEList_:= if(myHead(integTmpQE_)=and) then getArgsList(integTmpQE_)
                    else {integTmpQE_};
  debugWrite("integTmpQEList_:", integTmpQEList_);

  % それぞれについて、等式ならばsolveしてintegTmpSolList_とする。不等式ならば後回し。
  integTmpSolList_:= union(for each x in integTmpQEList_ join 
                         if(not hasInequality(x) and not hasLogicalOp(x)) then solve(x, t) else {x});
  debugWrite("integTmpSolList_:", integTmpSolList_);

  % integTmpSolList_の各要素について、checkInfUnitして、infList_を得る
  % TODO:integTmpQEList_の要素内にorが入っている場合を考える
  infList_:= union(for each x in integTmpSolList_ join checkInfUnit(x));
  debugWrite("infList_: ", infList_);

  ans_:= rlqe(mymkand(infList_));
  debugWrite("ans_: ", ans_);
  if(ans_=true) then return {ICI_CONSISTENT___}
  else if(ans_=false) then return {ICI_INCONSISTENT___}
  else 
  <<
    debugWrite("rlqe ans: ", ans_);
    return {ICI_UNKNOWN___};
  >>;

end;

%depend {ht,v}, t;
%expr_:={df(ht,t) = v,
%        df(v,t) = -10
%       };
%init_:={inithtlhs = 10,
%        initvlhs = 0
%       };
%vars_:={ht,v,df(ht,t),df(v,t)};
%symbolic redeval '(checkConsistencyInterval expr_ init_ vars_);




procedure checkInfUnit(tExpr_)$
begin;
  scalar head_, infCheckAns_, orArgsAnsList_, andArgsAnsList_;

  debugWrite("tExpr_: ", tExpr_);

  head_:= myHead(tExpr_);
  debugWrite("head_: ", head_);

  if(head_=or) then <<
    orArgsAnsList_:= for i:=1 : arglength(tExpr_) join
      checkInfUnit(part(tExpr_, i));
    debugWrite("orArgsAnsList_: ", orArgsAnsList_);
    infCheckAns_:= {rlqe(mymkor(orArgsAnsList_))};
  >> else if(head_=and) then <<
    andArgsAnsList_:= for i:=1 : arglength(tExpr_) join
      checkInfUnit(part(tExpr_, i));
    debugWrite("andArgsAnsList_: ", andArgsAnsList_);
    infCheckAns_:= {rlqe(mymkand(andArgsAnsList_))};
  >> else if(head_=equal) then <<
    % ガード条件判定においては等式の場合はfalse
    infCheckAns_:= {false}
  >> else <<
    if(rlqe(ex(t, tExpr_ and t>0)) = false) then infCheckAns_:= {false} 
    else infCheckAns_:= {true}
  >>;
  debugWrite("infCheckAns_: ", infCheckAns_);

  return infCheckAns_;
end;

procedure getIneqBoundLists(ineqList_)$
begin;
  scalar lbList_, ubList_, exprLhs_, solveAns_;

  lbList_:= {};
  ubList_:= {};
  for each x in ineqList_ do <<
    % (t - value) op 0 の形を想定
    exprLhs_:= part(x, 1);
    solveAns_:= part(solve(exprLhs_=0, t), 1);

    if((not freeof(x, geq)) or (not freeof(x, greaterp))) then
      lbList_:= cons(part(solveAns_, 2), lbList_)
    else ubList_:= cons(part(solveAns_, 2), ubList_);
  >>;
  debugWrite("lbList_: ", lbList_);
  debugWrite("ubList_: ", ubList_);
  return {lbList_, ubList_};
end;

% 出力：時刻と条件の組（TC）のリスト
% 前提：tExpr_は等式も不等式もtの1次式になっている
% TODO：ERROR処理
procedure checkInfMinTime(tExpr_, condDNF_)$
begin;
  scalar head_, tExprSol_, argsAnsTCList_, ineqTCList_, ineqList_, eqTCList_,
         minTCList_, andEqTCArgsCount_,
         lbList_, ubList_, maxLb_, minUb_, minTValue_, compareRet_;

  debugWrite("in checkInfMinTime", " ");
  debugWrite("tExpr_: ", tExpr_);
  debugWrite("condDNF_: ", condDNF_);

  % 引数を持たない場合
  if(arglength(tExpr_)=-1) then return {{tExpr_, condDNF_}};

  head_:= myHead(tExpr_);
  debugWrite("head_: ", head_);
  ineqTCList_:={};
  eqTCList_:={};

  if(hasLogicalOp(head_)) then <<
    argsAnsTCList_:= union(for i:=1 : arglength(tExpr_) join
      checkInfMinTime(part(tExpr_, i), condDNF_));
    debugWrite("argsAnsTCList_: ", argsAnsTCList_);
    % 長さ1のリストならそのまま返す
    if(length(argsAnsTCList_)=1) then return argsAnsTCList_;

    for each x in argsAnsTCList_ do
      if(hasInequality(x)) then ineqTCList_:= cons(x, ineqTCList_);
    debugWrite("ineqTCList_: ", ineqTCList_);
    eqTCList_:= argsAnsTCList_ \ ineqTCList_;
    debugWrite("eqTCList_: ", eqTCList_);
    % INFINITY消去
    eqTCList_:= for each x in eqTCList_ join 
      if(freeof(part(x, 1), INFINITY)) then {x} else {};

    if(head_=or) then <<
      minTCList_:= myFindMinimumValueCond({INFINITY, condDNF_}, eqTCList_);
    >> else if(head_=and) then <<
      andEqTCArgsCount_:= length(eqTCList_);
      debugWrite("andEqTCArgsCount_:", andEqTCArgsCount_);
      % 2つ以上の等式が論理積でつながっていたらエラー
      if(andEqTCArgsCount_ > 1) then return {{ERROR}};

      % lbとubとで分ける
      ineqList_:= for each x in ineqTCList_ collect part(x, 1);
      splitIneqsResult_:= getIneqBoundLists(ineqList_);
      lbList_:= part(splitIneqsResult_, 1);
      ubList_:= part(splitIneqsResult_, 2);
      debugWrite("lbList_: ", lbList_);
      debugWrite("ubList_: ", ubList_);
      % lbの最大値とubの最小値を求める
      maxLb_:= myfindMaximumValue(0, lbList_); 
      minUb_:= myfindMinimumValue(INFINITY, ubList_);
      debugWrite("maxLb_: ", maxLb_);
      debugWrite("minUb_: ", minUb_);

      if(andEqTCArgsCount_ = 1) then <<
        % minTValue_が存在するので、lb<ptかつpt<ubであることを確かめる
        minTValue_:= part(first(eqTCList_), 1);
        debugWrite("minTValue_: ", minTValue_);
        if((mymin(maxLb_, minTValue_) neq maxLb_) or (mymin(minTValue_, minUb_) neq minTValue_)) then
          minTCList_:= {{INFINITY, condDNF_}}
        else minTCList_:= {{minTValue_, part(first(eqTCList_), 2)}};
      >> else <<
        % 不等式だけなので、lb<ubかつlb>0を確かめる
        if((mymin(maxLb_, minUb_) = maxLb_) and (mymin(0, maxLb_) = 0)) then 
          minTCList_:= {{maxLb_, condDNF_}}
        else minTCList_:= {{INFINITY, condDNF_}};
      >>;
    >>;
  >> else if(head_=equal) then <<
    tExprSol_:= first(solve(tExpr_, t));
    debugWrite("tExprSol_:", tExprSol_);
    % t>0でなければ（連立してfalseなら）、INFINITYを返す
    if(not hasParameter(tExprSol_)) then <<
      if(mymin(part(tExprSol_,2),0) neq part(tExprSol_,2)) then 
        minTCList_:= {{part(tExprSol_, 2), condDNF_}}
      else minTCList_:= {{INFINITY, condDNF_}};
    >> else <<
      compareRet_:= compareValueAndParameter(0, part(tExprSol_, 2), condDNF_);
      if(isFalseDNF(part(compareRet_, 1))) then minTCList_:= {{INFINITY, condDNF_}}
      else minTCList_:= {{part(tExprSol_, 2), part(compareRet_, 1)}};
    >>;
  >> else <<
    % 不等式の場合はそのまま返す
    minTCList_:= {{tExpr_, condDNF_}};
  >>;

  debugWrite("minTCList_: ", minTCList_);
  return minTCList_;
end;



IC_SOLVER_ERROR___:= 0;
IC_NORMAL_END___:= 1;


procedure integrateCalc(cons_, rconts_, discCause_, vars_, maxTime_)$
begin;
  scalar tmpSol_, splitExprsResult_, NDExprs_, DExprs_, DExprVars_, otherExprs_,
         tmpDiscCause_, retCode_, tmpVarMap_, tmpMinTList_, integAns_;
  putLineFeed();

  debugWrite("cons_: ", cons_);
  debugWrite("rconts_: ", rconts_);
  debugWrite("discCause_: ", discCause_);
  debugWrite("vars_: ", vars_);
  debugWrite("maxTime_: ", maxTime_);

  % SinやCosが含まれる場合はラプラス変換不可能なのでNDExpr扱いする
  % TODO:なんとかしたいところ？
  splitExprsResult_ := splitExprs(removePrevCons(constraintStore_), csVariables_);
  NDExprs_ := part(splitExprsResult_, 1);
  debugWrite("NDExprs_: ", NDExprs_);
  DExprs_ := part(splitExprsResult_, 2);
  debugWrite("DExprs_: ", DExprs_);
  DExprVars_ := part(splitExprsResult_, 3);
  debugWrite("DExprVars_: ", DExprVars_);
  otherExprs_:= union(part(splitExprsResult_, 4), parameterStore_);
  % 空集合なら、{{true}}として扱う（trueを表すDNF）
  if(otherExprs_={}) then otherExprs_:= {{true}};
  debugWrite("otherExprs_: ", otherExprs_);

  initCons_:= union(for each x in rconts_ join {exSub(constraintStore_, x)});
  debugWrite("initCons_: ", initCons_);
  initVars_:= map(getInitVars, rconts_);
  debugWrite("initVars_: ", initVars_);

  tmpSol_:= exDSolve(DExprs_, initCons_, union(DExprVars_, (vars_ \ initVars_)));
  debugWrite("tmpSol_: ", tmpSol_);

  % NDExprs_を連立
  tmpSol_:= solve(union(tmpSol_, NDExprs_), getNoDifferentialVars(vars_ \ initVars_));
  debugWrite("tmpSol_ after solve: ", tmpSol_);

  % TODO:Solver error処理

  tmpDiscCause_:= sub(tmpSol_, discCause_);
  debugWrite("tmpDiscCause_:", tmpDiscCause_);

  tmpVarMap_:= first(myFoldLeft(createIntegratedValue, {{},tmpSol_}, union(DExprVars_, (vars_ \ initVars_)))); 
  debugWrite("tmpVarMap_:", tmpVarMap_);

  tmpMinTList_:= calcNextPointPhaseTime(maxTime_, tmpDiscCause_, otherExprs_);
  debugWrite("tmpMinTList_:", tmpMinTList_);
  if(tmpMinTList_ = {error}) then retCode_:= IC_SOLVER_ERROR___
  else retCode_:= IC_NORMAL_END___;

  integAns_:= {retCode_, tmpVarMap_, tmpMinTList_};
  debugWrite("integAns_", integAns_);
  
  return integAns_;
end;



procedure createIntegratedValue(pairInfo_, variable_)$
begin;
  scalar retList_, integRule_, integExpr_, newRetList_;

  retList_:= first(pairInfo_);
  integRule_:= second(pairInfo_);

  integExpr_:= {variable_, sub(integRule_, variable_)};
  debugWrite("integExpr_: ", integExpr_);

  newRetList_:= cons(integExpr_, retList_);
  debugWrite("newRetList_: ", newRetList_);

  return {newRetList_, integRule_};
end;



procedure calcNextPointPhaseTime(maxTime_, discCauseList_, condDNF_)$
begin;
  scalar minTCondListList_, comparedMinTCondList_, 
         minTTime_, minTCond_, ans_;

  debugWrite("in calcNextPointPhaseTime", " ");
  debugWrite("condDNF_: ", condDNF_);

  % 離散変化が起きえない場合は、maxTime_まで実行して終わり
  if(discCauseList_ = {}) then return {{maxTime_, condDNF_, 1}};

  minTCondListList_:= union(for each x in discCauseList_ collect findMinTime(x, condDNF_));
  debugWrite("minTCondListList_ in calcNextPointPhaseTime: ", minTCondListList_);

  if(not freeof(minTCondListList_, error)) then return {error};

  comparedMinTCondList_:= myFoldLeft(compareMinTimeList, {{maxTime_, condDNF_}}, minTCondListList_);
  debugWrite("comparedMinTCondList_ in calcNextPointPhaseTime: ", comparedMinTCondList_);

  ans_:= union(for each x in comparedMinTCondList_ collect <<
    minTTime_:= part(x, 1);
    minTCond_:= part(x, 2);
    % Condがtrueの場合は空集合として扱う
    if(isTrueDNF(minTCond_)) then minTCond_:= {};
    % 演算子部分をコードに置き換える
    minTCond_:= for each conj in minTCond_ collect
      for each term in conj collect {part(term, 1), getExprCode(part(term, 2)), part(term, 3)};
    if(mymin(minTTime_, maxTime_) neq maxTime_) then {minTTime_, minTCond_, 0}
    else {minTTime_, minTCond_, 1}
  >>);
  debugWrite("ans_ in calcNextPointPhaseTime: ", ans_);
  return ans_;
end;


load_package "ineq";
load_package "numeric";

% 不等式を、左辺に正の変数名のみがある形式に整形し、それを表すタプルを作る
% (不等式の場合、必ず右辺が0になってしまう（自動的に移項してしまう）ことや、
% 不等式の向きが限定されてしまったりすることがあるため)
% 前提：入力される不等式も変数も1つ
% TODO：複数の不等式が渡された場合への対応？
% 2次不等式までは解ける
% TODO：3次以上への対応？
% 入力：1変数の2次以下の不等式1つ
% 出力：{左辺, relop, 右辺}の形式によるDNFリスト
procedure exIneqSolve(ineqExpr_)$
begin;
  scalar lhs_, relop_, rhs_, adjustedLhs_, adjustedRelop_, 
         reverseRelop_, adjustedIneqExpr_, sol_, adjustedEqExpr_,
         retTuple_, exprVarList_, exprVar_, retTupleDNF_,
         boundList_, ub_, lb_;

  debugWrite("in exIneqSolve", " ");

  lhs_:= part(ineqExpr_, 1);
  relop_:= myHead(ineqExpr_);
  rhs_:= part(ineqExpr_, 2);
  % 右辺を左辺に移項する
  adjustedIneqExpr_:= myApply(relop_, {lhs_+(-1)*rhs_, 0});
  adjustedLhs_:= lhs(adjustedIneqExpr_);
  debugWrite("adjustedIneqExpr_: ", adjustedIneqExpr_);
  debugWrite("adjustedLhs_: ", adjustedLhs_);
  adjustedRelop_:= myHead(adjustedIneqExpr_);


  % この時点でx+value relop 0または-x+value relop 0の形式になっているので、
  % -x+value≦0の場合はx-value≧0の形にする必要がある（relopとして逆のものを得る必要がある）
  % サーバーモードだと≧を使った式表現は保持できないようなので、reverseするかどうかだけ分かれば良い
  % minusがなぜか使えないので、式に出現する変数名を調べて、「-x」の形がないかどうか調べる
  exprVarList_:= union(for each x in union(union(csVariables_, psParameters_), {INFINITY}) join
    if(not freeof(adjustedLhs_, x)) then {x} else {});
  debugWrite("exprVarList_: ", exprVarList_);
  % TODO：複数の変数が入っている場合への対応？
  exprVar_:= first(exprVarList_);
  debugWrite("exprVar_: ", exprVar_);
  if(not freeof(adjustedLhs_, -1*exprVar_)) then <<
    adjustedRelop_:= getReverseRelop(myHead(adjustedIneqExpr_));
  >>;

  % 変数にsqrtがついてる場合は2乗しておく
  if(not freeof(adjustedLhs_, sqrt(exprVar_))) then <<
    adjustedLhs_:= adjustedLhs_*adjustedLhs_;
    debugWrite("adjustedLhs_ after squaring: ", adjustedLhs_);
  >>;

  
  % ubまたはlbを求める
  off arbvars;
  % INFINITYのときは変数としてinfinityを指定し、それ以外は制約ストアの変数を指定する
  % TODO：elseのときの変数部分をちゃんと指定する
  if(not freeof(adjustedLhs_, INFINITY)) then 
    sol_:= solve(equal(adjustedLhs_, 0), INFINITY)
  else sol_:= solve(equal(adjustedLhs_, 0), exprVar_);
  debugWrite("sol_: ", sol_);
  on arbvars;

  % TODO：複数変数への対応？
  % 複数変数が入ると2重リストになるはずだが、1変数なら不要か？
%  if(length(sol_)>1 and myHead(first(sol_))=list) then <<
%    % orでつながった中にandが入っている関係を表している
%      
%    adjustedEqExpr_:= first(first(sol_))
%
%  >> 
  if(length(sol_)>1) then <<
    % 2次方程式を解いた場合。厳密には長さ2のはず
    % TODO：3次以上への一般化？

    boundList_:= {rhs(part(sol_, 1)), rhs(part(sol_, 2))};
    lb_:= myFindMinimumValue(INFINITY, boundList_);
    ub_:= first(boundList_ \ {lb_});
    
    % relopによって、tupleDNFの構成を変える
    if((adjustedRelop_ = geq) or (adjustedRelop_ = greaterp)) then <<
      % ≧および＞の場合はorの関係になる
      retTupleDNF_:= {{ {lb_, adjustedRelop_, exprVar_} }, { {exprVar_, adjustedRelop_, ub_} }};
    >> else <<
      % ≦および＜の場合はandの関係になる
      retTupleDNF_:= {{ {lb_, adjustedRelop_, exprVar_},     {exprVar_, adjustedRelop_, ub_} }};
    >>;
  >> else <<
    adjustedEqExpr_:= first(sol_);
    debugWrite("adjustedEqExpr_: ", adjustedEqExpr_);
    retTuple_:= {lhs(adjustedEqExpr_), adjustedRelop_, rhs(adjustedEqExpr_)};
    debugWrite("retTuple_: ", retTuple_);
    retTupleDNF_:= {{retTuple_}};
  >>;

  debugWrite("retTupleDNF_: ", retTupleDNF_);
  return retTupleDNF_;
end;

% ineq_solveで得られたリスト形式の出力や区間形式の出力を整形し、orやandでつながった形式にする
% TODO：orやandで良いのか？？→良くない。なので使わない
procedure adjustIneqSol(ineqSolExpr_, relop_)$
begin;
  scalar head_, rhs_, adjustedIneq_, lb_, ub_;

  debugWrite("in adjustIneqSol", " ");
  debugWrite("ineqSolExpr_: ", ineqSolExpr_);
  debugWrite("relop_: ", relop_);

  head_:= myHead(ineqSolExpr_);
  debugWrite("head_: ", head_);
  rhs_:= part(ineqSolExpr_, 2);

  if(head_=list) then <<
    adjustedIneq_:= mymkor(union(for each x in ineqSolExpr_ join {adjustIneqSol(x)}));
  >> else if(not freeof(rhs_, ..)) then <<
    % x=(lb_ .. ub_)の形
    lb_:= part(rhs_, 1);
    ub_:= part(rhs_, 2);
    adjustedIneq_:= mymkand({myApply(relop_, {lb_, t}), myApply(relop_, {t, ub_})});
  >> else <<
    adjustedIneq_:= ineqSolExpr_;
  >>;

  debugWrite("adjustedIneq_: ", adjustedIneq_);
  return adjustedIneq_;
end;


procedure findMinTime(integAsk_, condDNF_)$
begin;
  scalar integAskList_, integAskSolList_, integAskSolFormula_,
         minTCList_, tmpSol_, ineqSolDNF_;

  debugWrite("in findMinTime", " ");
  debugWrite("integAsk_: ", integAsk_);
  debugWrite("condDNF_: ", condDNF_);

  % t>0と連立してfalseになるような場合はMinTimeを考える必要がない
  if(rlqe(integAsk_ and t>0) = false) then return {{INFINITY, condDNF_}};

  %%%%%%%%%%%% TODO:この辺から、%%%%%%%%%%%%%%
  % まず、andでつながったtmp制約をリストに変換
  if(myHead(integAsk_)=and) then integAskList_:= getArgsList(integAsk_)
  else integAskList_:= {integAsk_};
  debugWrite("integAskList_:", integAskList_);

  % それぞれについて、等式ならばsolve・不等式ならば1次にしてintegAskSolList_とする。
  integAskSolList_:= union(for each x in integAskList_ join
                       if(not hasInequality(x)) then <<
                         tmpSol_:= solve(x, t);
                         if(length(tmpSol_)>1) then {map(rationalisation, tmpSol_)} 
                         else map(rationalisation, tmpSol_)
                       >> else <<
                         ineqSolDNF_:= exIneqSolve(x);
                         debugWrite("ineqSolDNF_: ", ineqSolDNF_);
                         % DNF形式からただのor形式に直す
                         for each x in ineqSolDNF_ collect first(x)
                       >>
                     );
  debugWrite("integAskSolList_:", integAskSolList_);

  % 論理式形式に変換
  integAskSolFormula_:= rlqe(mymkand(for each x in integAskSolList_ collect
                          if(myHead(x)=list) then rlqe(mymkor(x)) else x
                        ));
  debugWrite("integAskSolFormula_: ", integAskSolFormula_);
  %%%%%%%%%%%% TODO:この辺までを1つの処理にまとめたい%%%%%%%%%%%%

  minTCList_:= checkInfMinTime(integAskSolFormula_, condDNF_);
  debugWrite("minTCList_ in findMinTime: ", minTCList_);

  % ERRORが返っていたらerror
  if(not freeof(minTCList_, ERROR)) then return {error};
  return minTCList_;
end;


procedure compareMinTimeList(candidateTCList_, newTCList_)$
begin;
  scalar tmpRet_, arg2_, arg3_, ret_;

  debugWrite("in compareMinTimeList", " ");
  debugWrite("candidateTCList_: ", candidateTCList_);
  debugWrite("newTCList_: ", newTCList_);

  %ret_:= myFoldLeft((makeMapAndUnion(candidateTCList_, #1, #2))&, {}, newTCList_);を実現
  if(newTCList_={}) then return {};
  tmpRet_:= {};
  for i:=1 : length(newTCList_) do <<
    debugWrite("in loop", " ");
    debugWrite("tmpRet_: ", tmpRet_);
    arg2_:= tmpRet_;
    arg3_:= part(newTCList_, i);
    tmpRet_:= makeMapAndUnion(candidateTCList_, arg2_, arg3_);
    debugWrite("i: ", i);
    debugWrite("arg2_: ", arg2_);
    debugWrite("arg3_: ", arg3_);
    debugWrite("tmpRet_ after makeMapAndUnion: ", tmpRet_);    
  >>;
  ret_:= tmpRet_;

  debugWrite("ret_ in compareMinTimeList: ", ret_);
  return ret_;
end;

procedure makeMapAndUnion(candidateTCList_, retTCList_, newTC_)$
begin;
  scalar comparedList_, ret_;

  debugWrite("in makeMapAndUnion", " ");
  debugWrite("candidateTCList_: ", candidateTCList_);
  debugWrite("retTCList_: ", retTCList_);
  debugWrite("newTC_: ", newTC_);

  % Mapではなく、Joinを使う方が正しいか？
%  comparedList_:= for each x in candidateTCList_ collect compareMinTime(newTC_, x);
  comparedList_:= for each x in candidateTCList_ join compareMinTime(newTC_, x);
  debugWrite("comparedList_ in makeMapAndUnion: ", comparedList_);
  ret_:= union(comparedList_, retTCList_);

  debugWrite("ret_ in makeMapAndUnion: ", ret_);
  return ret_;
end;

procedure compareMinTime(TC1_, TC2_)$
begin;
  scalar TC1Time_, TC1Cond_, TC2Time_, TC2Cond_,
         intersectionCondDNF_, TC1LeqTC2CondDNF_, TC1GreaterTC2CondDNF_,
         retTCList_;

  debugWrite("in compareMinTime", " ");
  debugWrite("TC1_: ", TC1_);
  debugWrite("TC2_: ", TC2_);

  retTCList_:= {};

  TC1Time_:= part(TC1_, 1);
  TC1Cond_:= part(TC1_, 2);
  TC2Time_:= part(TC2_, 1);
  TC2Cond_:= part(TC2_, 2);
  % それぞれの条件部分について論理積を取り、falseなら空集合
  intersectionCondDNF_:= addCondDNFToCondDNF(TC1Cond_, TC2Cond_);
  debugWrite("intersectionCondDNF_: ", intersectionCondDNF_);
  if(isFalseDNF(intersectionCondDNF_)) then return retTCList_;

  % 条件の共通部分と時間に関する条件との論理積を取る
  % TC1Time_≦TC2Time_という条件
  TC1LeqTC2CondDNF_:= addCondTupleToCondDNF({TC1Time_, leq, TC2Time_}, intersectionCondDNF_);
  debugWrite("TC1LeqTC2CondDNF_: ", TC1LeqTC2CondDNF_);
  % TC1Time_＞TC2Time_という条件
  TC1GreaterTC2CondDNF_:= addCondTupleToCondDNF({TC1Time_, greaterp, TC2Time_}, intersectionCondDNF_);
  debugWrite("TC1GreaterTC2CondDNF_: ", TC1GreaterTC2CondDNF_);


  % それぞれ、falseでなければretTCList_に追加
  if(not isFalseDNF(TC1LeqTC2CondDNF_)) then retTCList_:= cons({TC1Time_, TC1LeqTC2CondDNF_}, retTCList_);
  if(not isFalseDNF(TC1GreaterTC2CondDNF_)) then retTCList_:= cons({TC2Time_, TC1GreaterTC2CondDNF_}, retTCList_);

  debugWrite("retTCList_ in compareMinTime: ", retTCList_);
  return retTCList_;
end;

procedure isFalseDNF(DNF_)$
  if(DNF_={}) then t else nil;

procedure isTrueDNF(DNF_)$
  if(DNF_={{true}}) then t else nil;

procedure addCondDNFToCondDNF(newCondDNF_, condDNF_)$
begin;
  scalar addedCondDNF_;

  debugWrite("in addCondDNFToCondDNF", " ");
  debugWrite("newCondDNF_: ", newCondDNF_);
  debugWrite("condDNF_: ", condDNF_);

  addedCondDNF_:= for each x in newCondDNF_ join
    for each y in x join addCondTupleToCondDNF(y, condDNF_);

  debugWrite("addedCondDNF_ in addCondDNFToCondDNF: ", addedCondDNF_);
  return addedCondDNF_;
end;

procedure addCondTupleToCondDNF(newCondTuple_, condDNF_)$
%入力：追加する（パラメタの）条件タプルnewCondTuple_, （パラメタの）条件を表す論理式のリストcondDNF_
%出力：（パラメタの）条件を表す論理式のリスト
%注意：リストの要素1つ1つは論理和でつながり、要素内は論理積でつながっていることを表す。
begin;
  scalar addedCondDNF_, addedCondConj_;

  %追加した結果、要素（andでつながった式）がfalseになったら、その要素は消す
  addedCondDNF_:= for each x in condDNF_ join <<
    addedCondConj_:= addCondTupleToCondConj(newCondTuple_, x);
    debugWrite("addedCondConj_ in loop in addCondTupleToCondDNF: ", addedCondConj_);
    if(addedCondConj_=false) then {} else {addedCondConj_}
  >>;

  %%空集合になってしまったらfalseを返す
  %if(addedCondDNF_={}) then addedCondDNF_:= false;

  debugWrite("addedCondDNF_ in addCondTupleToCondDNF: ", addedCondDNF_);
  return addedCondDNF_;
end;

% {左辺, relop, 右辺}のタプルから、式を作って返す
procedure makeFormulaFromTuple(tuple_)$
  myApply(part(tuple_, 2), {part(tuple_, 1), part(tuple_, 3)});

procedure addCondTupleToCondConj(newCondTuple_, condConj_)$
begin;
  scalar addedCondConj_, varName_, relop_, value_,
         varTerms_, ubTuple_, lbTuple_, ub_, lb_;

  debugWrite("in addCondTupleToCondConj", " ");
  debugWrite("newCondTuple_: ", newCondTuple_);
  debugWrite("condConj_: ", condConj_);

  % trueを追加しようとする場合、追加しないのと同じ
  if(newCondTuple_=true) then return condConj_;
  % パラメタが入らない場合、単に大小判定をした結果が残る
  if(not hasParameter(newCondTuple_)) then 
    if(checkOrderingFormula(makeFormulaFromTuple(newCondTuple_))) then return condConj_
    else return false;
  % trueに追加しようとする場合
  if(condConj_={true}) then return {newCondTuple_};


  varName_:= part(newCondTuple_, 1);
  relop_:= part(newCondTuple_, 2);
  value_:= part(newCondTuple_, 3);

  % 論理積の中から、追加する変数と同じ変数の項を探す
  varTerms_:= union(for each x in condConj_ join
    if(not freeof(x, varName_)) then {x} else {});

  % 下限・上限を得る
  % length(varTerms_)<=2を想定
  ubTuple_:= first(for each x in varTerms_ join
    if((not freeof(x, leq)) or (not freeof(x, lessp))) then {x} else {});
  lbTuple_:= first(varTerms_ \ {ubTuple_});
  ub_:= part(ubTuple_, 3);
  lb_:= part(lbTuple_, 3);

  % 追加する不等式と上下限とを比較し、更新する
  if((relop_=leq) or (relop_=lessp)) then <<
    if(mymin(value_, ub_)=value_) then ub_:= value_
  >> else <<
    if(mymin(value_, lb_)=lb_) then lb_:= value_
  >>;

  % lb<ubを確かめる
  if((mymin(lb_, ub_) = lb_)) then addedCondConj_:= {}
  else addedCondConj_:= false;
				  
  debugWrite("addedCondConj_: ", addedCondConj_);
  return addedCondConj_;
end;

% パラメタに関する不等式を、タプル形式に変換する
% exIneqSolveで代用できるので、使わない
procedure makeParamTuple(paramCons_)$
begin;
  scalar parName_, relopCode_, parValue_, lhs_, adjustedCons_,
         reverseRelop_, sol_;

  debugWrite("in makeParamTuple", " ");
  debugWrite("paramCons_: ", paramCons_);

  parName_:= part(paramCons_, 1);
  relopCode_:= getExprCode(paramCons_);
  parValue_:= part(paramCons_, 2);


  debugWrite("parName_: ", parName_);
  debugWrite("relopCode_: ", relopCode_);
  debugWrite("parValue_: ", parValue_);
  return {parName_, relopCode_, parValue_};
end;

% 不等式内にパラメタ変数が含まれる場合に使う関数
% exIneqSolveの仕様変更（変数指定が不要となった）により、使わない
procedure solveParameterIneq(ineqList_)$
begin;
  scalar paramNameList_, paramName_, ret_;

  debugWrite("in solveParameterIneq", " ");
  debugWrite("ineqList_: ", ineqList_);

  paramNameList_:= collectParameters(ineqList_);
  debugWrite("paramNameList_: ", paramNameList_);
  % 2種類以上のパラメタが含まれていると扱えない
  % TODO：なんとかする？
  if(length(paramNameList_)>1) then return ERROR;
  paramName_:= first(paramNameList_);

  % exIneqSolveでは1つずつしか不等式を扱えないので、別々に解く
  ret_:= exIneqSolve(ineqList_);
  return ret_;
end;


procedure getRealVal(value_, prec_)$
begin;
  scalar tmp_, defaultPrec_;
  putLineFeed();

  defaultPrec:= precision(0)$
  on rounded$
  precision(prec_);
  tmp_:= value_;
  debugWrite("tmp_:", tmp_);
  precision(defaultPrec_)$
  off rounded$

  return tmp_;
end;



%TODO エラー検出（適用した結果実数以外になった場合等）

procedure applyTime2Expr(expr_, time_)$
begin;
  scalar appliedExpr_;
  putLineFeed();

  appliedExpr_:= sub(t=time_, expr_);
  debugWrite("appliedExpr_:", appliedExpr_);

  return {1, appliedExpr_};
end;



procedure exprTimeShift(expr_, time_)$
begin;
  scalar shiftedExpr_;
  putLineFeed();

  shiftedExpr_:= sub(t=t-time_, expr_);
  debugWrite("shiftedExpr_:", shiftedExpr_);

  return shiftedExpr_;
end;



%load_package "assist";

procedure simplifyExpr(expr_)$
begin;
  scalar simplifiedExpr_;
  putLineFeed();

  % TODO:simplify関数を使う
%  simplifiedExpr_:= simplify(expr_);
  simplifiedExpr_:= expr_;
  debugWrite("simplifiedExpr_:", simplifiedExpr_);

  return simplifiedExpr_;
end;



procedure checkLessThan(lhs_, rhs_)$
begin;
  scalar ret_;
  putLineFeed();

  ret_:= if(mymin(lhs_, rhs_) = lhs_) then rettrue___ else retfalse___;
  debugWrite("ret_:", ret_);

  return ret_;
end;



procedure getSExpFromString(str_)$
begin;
  scalar retSExp_;
  putLineFeed();

  retSExp_:= str_;
  debugWrite("retSExp_:", retSExp_);

  return retSExp_;
end;


procedure putLineFeed()$
begin;
  write("");
end;




%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%

symbolic redeval '(putLineFeed);

;end;
