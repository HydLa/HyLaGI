load_package sets;

% グローバル変数
% constraintStore_: 現在扱っている制約集合（リスト形式、PPの定数未対応）
% csVariables_: 制約ストア内に出現する変数の一覧（リスト形式、PPの定数未対応）
% parameterStore_: 現在扱っている、定数制約の集合（リスト形式、IPのみ使用）
% psParameters_: 定数制約の集合に出現する定数の一覧（リスト形式、IPのみ使用）
%
% optUseDebugPrint_: デバッグ出力をするかどうか
%

% デバッグ用メッセージ出力関数
% TODO:任意長の引数に対応したい
procedure debugWrite(arg1_, arg2_)$
  if(optUseDebugPrint_) then <<
    write(arg1_, arg2_);
  >> 
  else <<
    1$
  >>$

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

procedure myif(x,op,y,approx_precision)$
%入力: 論理式(ex. sqrt(2), greaterp_, sin(2)), 精度
%出力: t or nil or -1
%      (xとyがほぼ等しい時 -1)
%geq_= >=, geq; greaterp_= >, greaterp; leq_= <=, leq; lessp_= <, lessp;
begin;
  scalar bak_precision, ans, margin;

  debugWrite("-----myif-----", " ");
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
  on rounded$ precision approx_precision$

% 10^(3 + yかxの指数部の値 - 有効桁数)
  if(min(x,y)=0) then
    margin:=10 ^ (3 + floor log10 max(x, y) - approx_precision)
  else if(min(x,y)>0) then 
    margin:=10 ^ (3 + floor log10 min(x, y) - approx_precision)
  else
    margin:=10 ^ (3 - floor log10 abs min(x, y) - approx_precision);

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

  return ans;
end;


procedure myInfinityIf(x, op, y)$
begin;
  scalar ans;
  % INFINITY > -INFINITYとかの対応
  if(x=INFINITY or y=-INFINITY) then 
    if((op = geq) or (op = greaterp)) then ans:=t else ans:=nil
  else 
    if((op = leq) or (op = lessp)) then ans:=t else ans:=nil;
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
if(myif(x,lessp,y,30)) then x else y$

procedure mymax(x,y)$
%入力: 数値という前提
if(myif(x,greaterp,y,30)) then x else y$

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
    % 数式内にsqrtが入っている時のみ、myif関数による大小比較が有効となる
    % TODO:当該の数式内に変数が入った際にも正しく処理ができるようにする
    if(myif(part(formula_, 1), getInverseRelop(header_), part(formula_, 2), 30)) then appliedFormula_:= false 
    else appliedFormula_:= rlqe(formula_)
  >> else <<
    appliedFormula_:= rlqe(formula_)
  >>;

  debugWrite("appliedFormula_: ", appliedFormula_);
  return appliedFormula_;

end;

procedure rationalisation(expr_)$
begin;
  scalar head_, denominator_, numerator_, denominatorHead_, denominatorArgsList_,
         conjugate_, rationalisedArgsList_, rationalisedExpr_, flag_;

  debugWrite("expr_: ", expr_);
  if(getArgsList(expr_)={}) then return expr_;

  % 想定する対象：分母の項数が2まで
  % TODO:より一般的な形への対応→分母がsqrt(a)+sqrt(b)+cの形(a,b>0)とか
  % TODO:3乗根以上への対応

  head_:= myHead(expr_);
  debugWrite("head_: ", head_);

  if(head_=quotient) then <<
    numerator_:= part(expr_, 1);
    denominator_:= part(expr_, 2);
    % 分母に無理数がなければ有理化必要なし
    if(numberp(denominator_)) then return expr_;

    denominatorHead_:= myHead(denominator_);
    if(denominatorHead_=plus) then <<
      denominatorArgsList_:= getArgsList(denominator_);
      if(length(denominatorArgsList_)>2) then
        conjugate_:= plus(first(denominatorArgsList_), -1*(myApply(plus, rest(denominatorArgsList_))))
      else conjugate_:= plus(part(denominator_, 1), -1*part(denominator_, 2));
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



% PPにおける制約ストアのリセット

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

% PPにおける制約ストアへの制約の追加

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

procedure removeTrue(patternList_)$
  for each x in patternList_ join if(rlqe(x)=true) then {} else {x}$

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
  patternList_:= removeTrue(patternList_);
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

  head_:= myHead(cons_);
  if(head_=equal) then return 0
  else if(head_=lessp) then return 1
  else if(head_=greaterp) then return 2
  else if(head_=leq) then return 3
  else if(head_=geq) then return 4
  else return nil;
end;

procedure makeConsTuple(cons_)$
begin;
  scalar varName_, relopCode_, value_, lhs_, adjustedCons_, reverseRelop_,
         sol_;

  debugWrite("in makeConsTuple", " ");
  debugWrite("cons_: ", cons_);
  
  % 左辺に変数名のみがある形式にする
  % 前提：等式はすでにこの形式になっている
  % 前提：不等式は≦しか出てこず、x+value≦0または-x+value≦0の形式になっている
  % TODO：なんとかする
  if(not hasInequality(cons_)) then <<
    varName_:= varName_:= part(cons_, 1);
    relopCode_:= getExprCode(cons_);
    value_:= part(cons_, 2);
  >> else <<
    lhs_:= part(cons_, 1);

    % -x+value≦0の場合はx-value≧0の形にする
    if(not freeof(lhs_, minus)) then <<
      reverseRelop_:= getReverseRelop(myHead(cons_));
      % 簡潔に書きたい
      % adustedCons_:= reverseRelop_(-1*lhs_, 0);
      adustedCons_:= myApply(reverseRelop_, {-1*lhs_, 0});
%      adustedCons_:= if(reverseRelop_=geq) then geq(-1*lhs_, 0)
%                     else if(reverseRelop_=greaterp) then greaterp(-1*lhs_, 0)
    >> else <<
      adjustedCons_:= cons_
    >>;
    debugWrite("adjustedCons_: ", adjustedCons_);

    % ubまたはlbを求める
    off arbvars;
    % TODO：変数部分をちゃんと指定する
    sol_:= solve(equal(lhs_, 0), csVariables_);
    on arbvars;
    % 2重リストの時のみfirstで得る
    % TODO:複数解得られた場合への対応
    if(myHead(first(sol_))=list) then sol_:= first(sol_);
    
    varName_:= part(first(sol_), 1);
    relopCode_:= getExprCode(adjustedCons_);
    value_:= part(first(sol_), 2);
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


% 前提：tExpr_は等式も不等式もtの1次式になっている

procedure checkInfMinTime(tExpr_, condition_)$
begin;
  scalar head_, infCheckAns_, tExprSol_, argsAnsList_, ineqList_, eqList_,
         regulatedMinTList_, minTValue_, andEqArgsCount_,
         lbList_, ubList_, maxLb_, minUb_, exprLhs_, solveAns_;

  debugWrite("tExpr_: ", tExpr_);
  debugWrite("condition_: ", condition_);

  % 引数を持たない場合
  if(arglength(tExpr_)=-1) then return tExpr_;

  head_:= myHead(tExpr_);
  debugWrite("head_: ", head_);
  ineqList_:={};
  eqList_:={};

  if(hasLogicalOp(head_)) then <<
    argsAnsList_:= union(for i:=1 : arglength(tExpr_) collect
      checkInfMinTime(part(tExpr_, i), condition_));
    debugWrite("argsAnsList_: ", argsAnsList_);
    % 長さ1のリストならその要素を返す
    if(length(argsAnsList_)=1) then return first(argsAnsList_);

    for each x in argsAnsList_ do
      if(hasInequality(x)) then ineqList_:= cons(x, ineqList_);
    debugWrite("ineqList_: ", ineqList_);
    eqList_:= argsAnsList_ \ ineqList_;
    debugWrite("eqList_: ", eqList_);
    % INFINITY消去
    eqList_:= eqList_ \ {INFINITY};

    if(head_=or) then <<
      % TODO：ineqList_も考える

      minTValue_:= myFindMinimumValue(INFINITY, eqList_);
    >> else if(head_=and) then <<
      andEqArgsCount_:= length(eqList_);
      debugWrite("andEqArgsCount_:", andEqArgsCount_);
      if(andEqArgsCount_ > 1) then return ERROR;
      % lbとubとで分ける
      lbList_:= {};
      ubList_:= {};
      for each x in ineqList_ do <<
        % (t - value) op 0  の形を想定
        % TODO:パラメタ対応
        exprLhs_:= part(x, 1);
        solveAns_:= part(solve(exprLhs_=0, t), 1);

        if((not freeof(x, geq)) or (not freeof(x, greaterp))) then
          lbList_:= cons(part(solveAns_, 2), lbList_)
        else ubList_:= cons(part(solveAns_, 2), ubList_);
      >>;
      debugWrite("lbList_: ", lbList_);
      debugWrite("ubList_: ", ubList_);
      % lbの最大値とubの最小値を求める
      maxLb_:= myfindMaximumValue(0, lbList_);
      minUB_:= myfindMinimumValue(INFINITY, ubList_);
      debugWrite("maxLb_: ", maxLb_);
      debugWrite("minUb_: ", minUb_);

      if(andEqArgsCount_ = 1) then <<
        % minTValue_が存在するので、lb<ptかつpt<ubであることを確かめる
        %minTValue_:= first(regulatedMinTList_);
        minTValue_:= first(eqList_);
        debugWrite("minTValue_: ", minTValue_);
        if(mymin(maxLb_, minTValue_) neq maxLb_ or mymin(minTValue_, minUb_) neq minTValue_) then
          minTValue_:= INFINITY;
      >> else <<
        % 不等式だけなので、lb<ubかつlb>0を確かめる
        if(mymin(maxLb_, minUb_) = maxLb_ and mymin(0, maxLb_) = 0) then minTValue_:= maxLb_
        else minTValue_:= INFINITY;
      >>;
    >>;
  >> else if(head_=equal) then <<
    tExprSol_:= first(solve(tExpr_, t));
    debugWrite("tExprSol_:", tExprSol_);
    if(mymin(part(tExprSol_,2),0) neq part(tExprSol_,2)) then minTValue_:= part(tExprSol_, 2)
    else minTValue_:= INFINITY;
  >> else <<
    % 不等式の場合はそのまま返す
    minTValue_:= tExpr_;
  >>;

  debugWrite("minTValue_: ", minTValue_);
  return minTValue_;
end;



IC_SOLVER_ERROR___:= 0;
IC_NORMAL_END___:= 1;


procedure integrateCalc(cons_, rconts_, discCause_, vars_, maxTime_)$
begin;
  scalar tmpSol_, splitExprsResult_, NDExprs_, DExprs_, DExprVars_, otherExprs_,
         tmpDiscCause_, retCode_, tmpVarMap_, tmpMinT_, integAns_;
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
  otherExprs_:= part(splitExprsResult_, 4);
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

  tmpMinT_:= calcNextPointPhaseTime(maxTime_, tmpDiscCause_, otherExprs_);
  debugWrite("tmpMinT_:", tmpMinT_);
  if(tmpMinT_ = error) then retCode_:= IC_SOLVER_ERROR___
  else retCode_:= IC_NORMAL_END___;

  % TODO:tmpMinT_は複数時刻扱えるようにする
  integAns_:= {retCode_, tmpVarMap_, {tmpMinT_}};
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



procedure calcNextPointPhaseTime(maxTime_, discCause_, otherExprs_)$
begin;
  scalar minTList_, minT_, ans_;

  % 離散変化が起きえない場合は、maxTime_まで実行して終わり
  if(discCause_ = {}) then return {maxTime_, 1};

  minTList_:= union(for each x in discCause_ join findMinTime(x, otherExprs_));
  debugWrite("minTList_ in calcNextPointPhaseTime: ", minTList_);

  if(not freeof(minTList_, error)) then return error;

  minT_:= myFindMinimumValue(INFINITY, minTList_);
  debugWrite("minT_: ", minT_);

  if(mymin(minT_, maxTime_) neq maxTime_) then ans_:= {minT_, 0}
  else ans_:= {maxTime_, 1}; 
  debugWrite("ans_ in calcNextPointPhaseTime: ", ans_);

  return ans_;
end;


load_package "ineq";
load_package "numeric";

% 1つの不等式に関して、上下限を正しく扱った上で解ける
% ＜と≦を区別できるような区間を出力する
% TODO：複数の不等式が渡された場合への対応？
procedure exIneqSolve(ineqs_, vars_)$
begin;
  scalar relop_, ineqSol_, ret_;

  debugWrite("ineqs_: ", ineqs_);

  % 演算子を取得
  % TODO：なんとかする
  relop_:= part(ineqs_, 0);

  ineqSol_:= ineq_solve(ineqs_, vars_);
  ret_:= adjustIneqSol(ineqSol_, relop_);
  debugWrite("ret_ in exIneqSolve: ", ret_);
  return ret_;
end;


procedure adjustIneqSol(ineqSolExpr_, relop_)$
begin;
  scalar head_, adjustedIneq_, lb_, ub_;

  debugWrite("ineqSolExpr_: ", ineqSolExpr_);
  debugWrite("relop_: ", relop_);

  head_:= myHead(ineqSolExpr_);

  if(head_=list) then <<
    adjustedIneq_:= mymkor(union(for each x in ineqSolExpr_ join {adjustIneqSol(x)}));
  >> else if(not freeof(head_, ..)) then <<
    lb_:= part(ineqSolExpr_, 1);
    ub_:= part(ineqSolExpr_, 1);
    adjustedIneq_:= mymkand({myApply(relop_, {lb_, t}), myApply(relop_, {t, ub_})});
  >> else <<
    adjustedIneq_:= ineqSolExpr_;
  >>;

  debugWrite("adjustedIneq_: ", adjustedIneq_);
  return adjustedIneq_;
end;


procedure findMinTime(integAsk_, condition_)$
begin;
  scalar integAskList_, integAskSolList_, integAskSolFormula_,
         minTList_, singletonMinTList_, tmpSol_, ineqSol_;

  debugWrite("in findMinTime", " ");
  debugWrite("integAsk_: ", integAsk_);
  debugWrite("condition_: ", condition_);

  % t>0と連立してfalseになるような場合はMinTimeを考える必要がない
  if(rlqe(integAsk_ and t>0) = false) then return {INFINITY};
%  if(rlqe(integAsk_ and t>0) = false) then return {INFINITY, condition_};

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
                         ineqSol_:= exIneqSolve(x, t);
                         debugWrite("ineqSol_: ", ineqSol_);
                         {ineqSol_}
                       >>
                     );
  debugWrite("integAskSolList_:", integAskSolList_);

  % 論理式形式に変換
  integAskSolFormula_:= rlqe(mymkand(for each x in integAskSolList_ collect
                          if(myHead(x)=list) then rlqe(mymkor(x)) else x
                        ));
  debugWrite("integAskSolFormula_: ", integAskSolFormula_);
  %%%%%%%%%%%% TODO:この辺までを1つの処理にまとめたい%%%%%%%%%%%%

  singletonMinTList_:= {checkInfMinTime(integAskSolFormula_, condition_)};
  debugWrite("singletonMinTList_: ", singletonMinTList_);

  % ERRORが返っていたらerror
  if(singletonMinList_ = {ERROR}) then return {error};
  return singletonMinTList_;

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
