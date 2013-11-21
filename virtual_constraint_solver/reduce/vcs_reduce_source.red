load_package sets;
load_package redlog; rlset R;
load_package "laplace";
load_package "ineq";
load_package "numeric";
%load_package "assist";


operator interval;

LINELENGTH(100000000);

% グローバル変数
% constraintStore__: 現在扱っている制約集合（リスト形式、PPの定数未対応）
% csVariables__: 制約ストア内に出現する変数の一覧（リスト形式、PPの定数未対応）
% parameterStore__: 現在扱っている、定数制約の集合（リスト形式、IPのみ使用）
% psParameters__: 定数制約の集合に出現する定数の一覧（リスト形式、IPのみ使用）
%
% RCS復旧に向けて新しく追加するグローバル変数
% isTemporary__：制約の追加を一時的なものとするか
% tmpConstraint__: 一時的に追加された変数
% prevConstraint__: 左極限値を設定する制約
% initConstraint__: 初期値制約
% initTmpConstraint__: 一時的に追加された初期値制約
% (variables__: 制約に出現する変数のリスト)
% tmpVariables__: 一時制約に出現する変数のリスト
% guard__: ガード制約
% guardVars__: ガード制約に含まれる変数のリスト
% 
% initVariables__: init変数(init◯◯)のリスト 
%
% irrationalNumberIntervalList__: 無理数と、区間値による近似表現の組のリスト
%
% optUseDebugPrint__: デバッグ出力をするかどうか
% optUseApproximateCompare__: 近似値を用いた大小判定を行うかどうか
% approxPrecision__: checkOrderingFormula内で、数式を数値に近似する際の精度
% intervalPrecision__: 区間値への変換を用いた大小比較における、区間の精度
%
%
% piInterval__: 円周率Piを表す区間
% eInterval__: ネイピア数Eを表す区間
%
% 将来使うかもしれないが未実装のグローバル変数
% prevVariables__: HydLaプログラムの静的解析向け

% 出力設定初期化
off nat;
linelength(10^5);

% グローバル変数初期化
irrationalNumberIntervalList__:= {};
optUseApproximateCompare__:= nil;
approxPrecision__:= 30; % TODO:要検討
intervalPrecision__:= 2; % TODO:要検討
piInterval__:= interval(3141592/1000000, 3141593/1000000);
eInterval__:= interval(2718281/1000000, 2718282/1000000);


%---------------------------------------------------------------
% 基本的な数式操作関数
%---------------------------------------------------------------

%MathematicaでいうHead関数
procedure myHead(expr_)$
  if(arglength(expr_)=-1) then nil
  else part(expr_, 0);

%MathematicaでいうFold関数
procedure myFoldLeft(func_, init_, list_)$
  if(list_ = {}) then init_
  else myFoldLeft(func_, func_(init_, first(list_)), rest(list_));

procedure getArgsList(expr_)$
  if(arglength(expr_)=-1) then {}
  else for i:=1 : arglength(expr_) collect part(expr_, i);

%MathematicaでいうApply関数
procedure myApply(func_, expr_)$
  part(expr_, 0):= func_;

% 式の頭部がマイナスかどうか
% ただしexprが整数の場合は負であってもマイナス扱いしない
% TODO：不都合があれば、対応する
procedure hasMinusHead(expr_)$
  if(arglength(expr_)=-1) then nil
  else if(part(expr_, 1) = -1*expr_) then t
  else nil;

% TODO:3乗根以上への対応
procedure rationalisation(expr_)$
begin;
  scalar head_, denominator_, numerator_, denominatorHead_, denomPlusArgsList_,
         frontTwoElemList_, restElemList_, timesRhs_, conjugate_, 
         rationalisedArgsList_, rationalisedExpr_, flag_;

%  debugWrite("in rationalisation", " ");
%  debugWrite("expr_: ", expr_);
  if(getArgsList(expr_)={}) then return expr_;

  % 想定する対象：分母の項数が4まで
  % TODO:より一般的な形への対応→5項以上？
  % TODO:3乗根以上への対応

  head_:= myHead(expr_);
%  debugWrite("head_: ", head_);

  if(head_=quotient) then <<
    numerator_:= part(expr_, 1);
    denominator_:= part(expr_, 2);
    % 分母に無理数がなければ有理化必要なし
    if(numberp(denominator_)) then return expr_;

    denominatorHead_:= myHead(denominator_);
%    debugWrite("denominatorHead_: ", denominatorHead_);
    if((denominatorHead_=plus) or (denominatorHead_=times)) then <<
      denomPlusArgsList_:= if(denominatorHead_=plus) then getArgsList(denominator_)
      else << 
        % denominatorHead_=timesのとき
        if(myHead(part(denominator_, 2))=plus) then getArgsList(part(denominator_, 2))
        else {part(denominator_, 2)}
      >>;
%      debugWrite("denomPlusArgsList_: ", denomPlusArgsList_);

      % 項数が3以上の場合、確実に無理数が減るように工夫して共役数を求める
      if(length(denomPlusArgsList_)>2) then <<
        frontTwoElemList_:= getFrontTwoElemList(denomPlusArgsList_);
%        debugWrite("frontTwoElemList_: ", frontTwoElemList_);
        restElemList_:= denomPlusArgsList_ \ frontTwoElemList_;
%        debugWrite("restElemList_: ", restElemList_);
        if(denominatorHead_=plus) then <<
          conjugate_:= plus(myApply(plus, frontTwoElemList_), -1*(myApply(plus, restElemList_)));
        >> else <<
          % 前提：積の右辺はすべてplusで(-5は+(-5)のように)つながっている形式
          % TODO：そうでない場合でも平気なように？
          timesRhs_:= plus(myApply(plus, frontTwoElemList_), -1*(myApply(plus, restElemList_)));
          conjugate_:= part(denominator_, 1) * timesRhs_;
        >>;
      >> else if(length(denomPlusArgsList_)=2) then <<
        if(denominatorHead_=plus) then <<
          conjugate_:= plus(part(denomPlusArgsList_, 1), -1*part(denomPlusArgsList_, 2));
        >> else <<
          % denominatorHead_=timesのとき
          timesRhs_:= plus(part(denomPlusArgsList_, 1), -1*part(denomPlusArgsList_, 2));
          conjugate_:= part(denominator_, 1) * timesRhs_;
        >>;
      >> else <<
        % denomPlusArgsList_の長さは1のはず、このときdenominatorHead_はtimesであるはず
        conjugate_:= -1*first(denomPlusArgsList_);
      >>;
    >> else if(denominatorHead_=difference) then <<
      conjugate_:= difference(part(denominator_, 1), -1*part(denominator_, 2));
    >> else <<
      conjugate_:= -1*denominator_;
    >>;
%    debugWrite("conjugate_: ", conjugate_);
    % 共役数を分母子にかける
    numerator_:= numerator_ * conjugate_;
    denominator_:= denominator_ * conjugate_;
    rationalisedExpr_:= numerator_ / denominator_;
    flag_:= true;
  >> else if(length(expr_)>1) then <<
    rationalisedArgsList_:= map(rationalisation, getArgsList(expr_));
%    debugWrite("rationalisedArgsList_: ", rationalisedArgsList_);
    rationalisedExpr_:= myApply(head_, rationalisedArgsList_);
  >> else <<
    rationalisedExpr_:= expr_;
  >>;

%  debugWrite("rationalisedExpr_: ", rationalisedExpr_);
%  debugWrite("flag_: ", flag_);
  if(flag_=true) then rationalisedExpr_:= rationalisation(rationalisedExpr_);
  return rationalisedExpr_;
end;


% exSubの第二引数をリスト形式に対応させた置換関数
procedure myExSub(patternList_, exprs_)$
  if(myHead(exprs_) = list) then 
    for each x in exprs_ collect myExSub(patternList_, x)
  else exSub(patternList_, exprs_);

% expr_中に等式以外や論理演算子が含まれる場合にも対応できる置換関数
procedure exSub(patternList_, expr_)$
begin;
  scalar subAppliedExpr_, head_, subAppliedLeft_, subAppliedRight_, 
         argCount_, subAppliedExprList_, test_;

%  debugWrite("in exSub", " ");
%  debugWrite("patternList_: ", patternList_);
%  debugWrite("expr_: ", expr_);
  
  % expr_が引数を持たない場合
  if(arglength(expr_)=-1) then <<
    subAppliedExpr_:= sub(patternList_, expr_);
    return subAppliedExpr_;
  >>;
  % patternList_からTrueを意味する制約を除く
  patternList_:= removeTrueList(patternList_);
%  debugWrite("patternList_: ", patternList_);

  head_:= myHead(expr_);
%  debugWrite("head_: ", head_);

  % orで結合されるもの同士を括弧でくくらないと、neqとかが違う結合のしかたをする可能性あり
  if(isIneqRelop(head_)) then <<
    % 等式以外の関係演算子の場合
    subAppliedLeft_:= exSub(patternList_, lhs(expr_));
%    debugWrite("subAppliedLeft_:", subAppliedLeft_);
    subAppliedRight_:= exSub(patternList_, rhs(expr_));
%    debugWrite("subAppliedRight_:", subAppliedRight_);
    subAppliedExpr_:= myApply(head_, {subAppliedLeft_, subAppliedRight_});
  >> else if(isLogicalOp(head_)) then <<
    % 論理演算子の場合
    argCount_:= arglength(expr_);
%    debugWrite("argCount_: ", argCount_);
    subAppliedExprList_:= for i:=1 : argCount_ collect exSub(patternList_, part(expr_, i));
%    debugWrite("subAppliedExprList_:", subAppliedExprList_);
    subAppliedExpr_:= myApply(head_, subAppliedExprList_);

  >> else <<
    % 等式や、変数名などのfactorの場合
    % TODO:expr_を見て、制約ストア（あるいはcsvars）内にあるようなら、それと対をなす値（等式の右辺）を適用
    subAppliedExpr_:= sub(patternList_, expr_);
  >>;

%  debugWrite("subAppliedExpr_:", subAppliedExpr_);
  return subAppliedExpr_;
end;

% expr_中に出現する、sqrt(var_)に関する項の係数を得る
% 複数項存在する場合も考慮し、リスト形式で返す
procedure getSqrtList(expr_, var_, mode_)$
begin;
  scalar head_, argsAnsList_, coefficientList_, exprList_, insideSqrt_;

%  debugWrite("in getSqrtList", " ");
%  debugWrite("expr_: ", expr_);
%  debugWrite("var_: ", var_);
%  debugWrite("mode_: ", mode_);


  % 変数やsqrtが含まれなければ考える必要なし
  if(freeof(expr_, var_) or freeof(expr_, sqrt)) then return {};

  head_:= myHead(expr_);
%  debugWrite("head_: ", head_);
  if(hasMinusHead(expr_)) then <<
    % TODO：負号の中にvar_が複数個ある場合への対応？
    coefficientList_:= {-1*first(getSqrtList(part(expr_, 1), var_, mode_))};
    exprList_:= getSqrtList(part(expr_, 1), var_, mode_);
  >> else if(head_=plus) then <<
    % 多項式の場合
    argsAnsList_:= for each x in getArgsList(expr_) join getSqrtList(x, var_, mode_);
    coefficientList_:= argsAnsList_;
    exprList_:= argsAnsList_;
  >> else if(head_=times) then <<
    argsAnsList_:= for each x in getArgsList(expr_) join getSqrtList(x, var_, mode_);
%    debugWrite("argsAnsList_: ", argsAnsList_);
    coefficientList_:= {myFoldLeft(times, 1, argsAnsList_)};
    exprList_:= argsAnsList_;
  >> else if(head_=sqrt) then <<
    insideSqrt_:= part(expr_, 1);
    exprList_:= {insideSqrt_};

    if(freeof(insideSqrt_, sqrt)) then <<
      % lcofで係数を求める
      coefficientList_:= {lcof(insideSqrt_, var_)};
    >> else <<
      % 根号の中がさらにsqrtを含む多項式になっている場合
      argsAnsList_:= for each x in getArgsList(insideSqrt_) join getSqrtList(x, var_, mode_);
%      debugWrite("argsAnsList_: ", argsAnsList_);
      % TODO：複数個ある場合への対応？
      coefficientList_:= {first(argsAnsList_)};
      exprList_:= union(exprList_, argsAnsList_);
    >>;
  >> else <<
    coefficientList_:= hogehoge;
    exprList_:= fugafuga;
  >>;

%  debugWrite("coefficientList_: ", coefficientList_);
%  debugWrite("exprList_: ", exprList_);
  if(mode_=COEFF) then return coefficientList_
  else if(mode_=INSIDE) then return exprList_;
end;

%---------------------------------------------------------------
% 関係演算子関連の関数
%---------------------------------------------------------------

procedure isIneqRelop(expr_)$
  if((expr_=neq) or (expr_=not) or
    (expr_=geq) or (expr_=greaterp) or 
    (expr_=leq) or (expr_=lessp)) then t else nil;

procedure hasIneqRelop(expr_)$
  if(freeof(expr_, neq) and freeof(expr_, not) and
     freeof(expr_, geq) and freeof(expr_, greaterp) and
     freeof(expr_, leq) and freeof(expr_, lessp)) then nil else t;

procedure getReverseRelop(relop_)$
  if(relop_=equal) then equal
  else if(relop_=neq) then neq
  else if(relop_=geq) then leq
  else if(relop_=greaterp) then lessp
  else if(relop_=leq) then geq
  else if(relop_=lessp) then greaterp
  else nil;

procedure getInverseRelop(relop_)$
  if(relop_=equal) then neq
  else if(relop_=neq) then equal
  else if(relop_=geq) then lessp
  else if(relop_=greaterp) then leq
  else if(relop_=leq) then greaterp
  else if(relop_=lessp) then geq
  else nil;

procedure getReverseCons(cons_)$
begin;
  scalar reverseRelop_, lhs_, rhs_;

  reverseRelop_:= getReverseRelop(myHead(cons_));
  lhs_:= lhs(cons_);
  rhs_:= rhs(cons_);
  return reverseRelop_(rhs_, lhs_);
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

%---------------------------------------------------------------
% 論理演算子関連の関数
%---------------------------------------------------------------

procedure isLogicalOp(expr_)$
  if((expr_=or) or (expr_=and)) then t else nil;

procedure hasLogicalOp(expr_)$
  if(freeof(expr_, and) and freeof(expr_, or)) then nil else t;

%---------------------------------------------------------------
% 三角関数関連の関数
%---------------------------------------------------------------

procedure isTrigonometricFunc(expr_)$
  if((expr_=sin) or (expr_ = cos) or (expr_ = tan)) then t else nil;

procedure hasTrigonometricFunc(expr_)$
  if((not freeof(expr_, sin)) or (not freeof(expr_, cos)) or (not freeof(expr_, tan))) then t else nil;

procedure isInvTrigonometricFunc(expr_)$
  if((expr_=asin) or (expr_=acos) or (expr_=atan)) then t else nil;

procedure hasInvTrigonometricFunc(expr_)$
  if((not freeof(expr_, asin)) or (not freeof(expr_, acos)) or (not freeof(expr_, atan))) then t else nil;

%---------------------------------------------------------------
% 区間演算関連の関数
%---------------------------------------------------------------

procedure isInterval(expr_)$
  if(arglength(expr_)=-1) then nil
  else if(myHead(expr_)=interval) then t
  else nil;

procedure isZeroInterval(expr_)$
  if(expr_=makePointInterval(0)) then t else nil;

procedure getLbFromInterval(expr_)$
  if(isInterval(expr_)) then part(expr_, 1)
  else ERROR;

procedure getUbFromInterval(expr_)$
  if(isInterval(expr_)) then part(expr_, 2)
  else ERROR;

% TODO：等号の有無（開区間/閉区間）によって結果が変わる場合も正しく扱えるように
% （その場合、lbとubが等しくなっている区間は、閉区間と考えると実現可能か）
procedure compareInterval(interval1_, op_, interval2_)$
begin;
  debugWrite("in compareInterval", " ");
  debugWrite("interval1_: ", interval1_);
  debugWrite("op_: ", op_);
  debugWrite("interval2_: ", interval2_);

  if (op_ = geq) then
    if (getLbFromInterval(interval1_) >= getUbFromInterval(interval2_)) then return t 
    else if(getUbFromInterval(interval1_) < getLbFromInterval(interval2_)) then return nil
    else return unknown
  else if (op_ = greaterp) then
    if (getLbFromInterval(interval1_) > getUbFromInterval(interval2_)) then return t 
    else if(getUbFromInterval(interval1_) <= getLbFromInterval(interval2_)) then return nil
    else return unknown
  else if (op_ = leq) then
    if (getUbFromInterval(interval1_) <= getLbFromInterval(interval2_)) then return t 
    else if(getLbFromInterval(interval1_) > getUbFromInterval(interval2_)) then return nil
    else return unknown
  else if (op_ = lessp) then
    if (getUbFromInterval(interval1_) < getLbFromInterval(interval2_)) then return t 
    else if(getLbFromInterval(interval1_) >= getUbFromInterval(interval2_)) then return nil
    else return unknown;
end;

procedure plusInterval(interval1_, interval2_)$
  interval(getLbFromInterval(interval1_)+getLbFromInterval(interval2_), getUbFromInterval(interval1_)+getUbFromInterval(interval2_));

procedure timesInterval(interval1_, interval2_)$
begin;
  scalar compareList_, retInterval_;

  compareList_:= {getLbFromInterval(interval1_) * getLbFromInterval(interval2_),
                  getLbFromInterval(interval1_) * getUbFromInterval(interval2_),
                  getUbFromInterval(interval1_) * getLbFromInterval(interval2_),
                  getUbFromInterval(interval1_) * getUbFromInterval(interval2_)};
  retInterval_:= interval(myFindMinimumValue(INFINITY, compareList_), myFindMaximumValue(-INFINITY, compareList_));
  debugWrite("retInterval_ in timesInterval: ", retInterval_);

  return retInterval_;
end;

procedure quotientInterval(interval1_, interval2_)$
  if((getLbFromInterval(interval2_)=0) or (getUbFromInterval(interval2_)=0)) then ERROR
  else timesInterval(interval1_, interval(1/getUbFromInterval(interval2_), 1/getLbFromInterval(interval2_)));

% 無理数を含む定数式を区間値形式に変換する
% 前提：入力のvalue_は2以上の整数に限られる
procedure getSqrtInterval(value_, mode_)$
begin;
  scalar sqrtInterval_, sqrtLb_, sqrtUb_, midPoint_, loopCount_,
         tmpNewtonSol_, newTmpNewtonSol_, iIntervalList_;

  debugWrite("in getSqrtInterval", " ");
  debugWrite("value_: ", value_);
  debugWrite("mode_: ", mode_);

  debugWrite("irrationalNumberIntervalList__: ", irrationalNumberIntervalList__);
  
  iIntervalList_:= getIrrationalNumberInterval(sqrt(value_));
  debugWrite("iIntervalList_: ", iIntervalList_);
  if(iIntervalList_ neq {}) then return first(iIntervalList_);

  if(mode_=BINARY_SEARCH) then <<
    % 上下限の中点xにおいて、x^2-valueの正負を調べることで探索範囲を狭めていく
    sqrtLb_:= 0;
    sqrtUb_:= value_;
    loopCount_:= 0;
    while (sqrtUb_ - sqrtLb_ >= 1/10^intervalPrecision__) do <<
      midPoint_:= (sqrtLb_ + sqrtUb_)/2;
      debugWrite("midPoint_: ", midPoint_);
      if(midPoint_ * midPoint_ < value_) then sqrtLb_:= midPoint_
      else sqrtUb_:= sqrtUb_:= midPoint_;
      loopCount_:= loopCount_+1;
      debugWrite("loopCount_: ", loopCount_);
    >>;
    sqrtInterval_:= interval(sqrtLb_, sqrtUb_);
    putIrrationalNumberInterval(sqrt(value_), sqrtInterval_);
  >> else if(mode_=NEWTON) then <<
    % ニュートン法により求める
    newTmpNewtonSol_:= value_;
    loopCount_:= 0;
    repeat <<
      tmpNewtonsol_:= newTmpNewtonSol_;
      newTmpNewtonSol_:= 1/2*(tmpNewtonSol_+value_ / tmpNewtonSol_);
      debugWrite("newTmpNewtonSol_: ", newTmpNewtonSol_);
      loopCount_:= loopCount_+1;
      debugWrite("loopCount_: ", loopCount_);
    >> until (tmpNewtonSol_ - newTmpNewtonSol_ < 1/10^intervalPrecision__);
    sqrtLb_:= 2*newTmpNewtonSol_ - tmpNewtonSol_;
    sqrtUb_:= newTmpNewtonSol_;
    sqrtInterval_:= interval(sqrtLb_, sqrtUb_);
    putIrrationalNumberInterval(sqrt(value_), sqrtInterval_);
  >>;

  return sqrtInterval_;
end;

procedure makePointInterval(value_)$
  interval(value_, value_);

procedure convertValueToInterval(value_)$
begin;
  scalar head_, retInterval_, argsList_, insideSqrt_;

  debugWrite("in convertValueToInterval", " ");
  debugWrite("value_: ", value_);

  % 有理数なので区間にすることなく大小判定可能だが、上下限が等しい区間（点区間）として扱う
  if(numberp(value_)) then <<
    retInterval_:= makePointInterval(value_);
    debugWrite("retInterval_: ", retInterval_);
    debugWrite("(value_: )", value_);
    return retInterval_;
  >>;

  if(arglength(value_) neq -1) then <<
    head_:= myHead(value_);
    debugWrite("head_: ", head_);
    if(hasMinusHead(value_)) then <<
      % 負の数の場合
      retInterval_:= timesInterval(makePointInterval(-1), convertValueToInterval(part(value_, 1)));
    >> else if((head_=plus) or (head_= times)) then <<
      argsList_:= getArgsList(value_);
      debugWrite("argsList_: ", argsList_);
      if(head_=plus) then <<
        retInterval_:= interval(0, 0);
        for each x in argsList_ do
          retInterval_:= plusInterval(convertValueToInterval(x), retInterval_);
      >> else <<
        retInterval_:= interval(1, 1);
        for each x in argsList_ do
          retInterval_:= timesInterval(convertValueToInterval(x), retInterval_);
      >>;
    >> else if(head_=quotient) then <<
      retInterval_:= quotientInterval(part(value_, 1), part(value_, 2));
    >> else if(head_=expt) then <<
      % 前提：冪数は整数
      retInterval_:= makePointInterval(1);
      for i:=1 : part(value_, 2) do <<
        retInterval_:= timesInterval(retInterval_, convertValueToInterval(part(value_, 1)));
      >>;
    >> else if(head_=sqrt) then <<
      insideSqrt_:= part(value_, 1);
      retInterval_:= getSqrtInterval(insideSqrt_, NEWTON);
    >> else if(head_=sin) then <<
      retInterval_:= interval(-1, 1);
    >> else if(head_=cos) then <<
      retInterval_:= interval(-1, 1);
    >> else if(head_=tan) then <<
      retInterval_:= interval(-1, 1);
    >> else if(head_=asin) then <<
      % interval(-Pi, Pi)
      retInterval_:= interval(getLbFromInterval(timesInterval(makePointInterval(-1), piInterval__)), getUbFromInterval(piInterval__));
    >> else if(head_=acos) then <<
      retInterval_:= interval(getLbFromInterval(timesInterval(makePointInterval(-1), piInterval__)), getUbFromInterval(piInterval__));
    >> else if(head_=atan) then <<
      retInterval_:= interval(getLbFromInterval(timesInterval(makePointInterval(-1), piInterval__)), getUbFromInterval(piInterval__));
    >> else <<
      % TODO：他にどんな場合に無理数扱いになるかを調べる
      retInterval_:= interval(hoge, hoge);
    >>;
  >> else <<
    if(value_=pi) then retInterval_:= piInterval__
    else if(value_=e) then retInterval_:= eInterval__
    else retInterval_:= interval(hoge, hoge);
  >>;
  debugWrite("retInterval_: ", retInterval_);
  debugWrite("(value_: )", value_);

  return retInterval_;
end;

%---------------------------------------------------------------
% 実数以外の数関連の関数
%---------------------------------------------------------------

procedure hasImaginaryNum(value_)$
  if(not freeof(value_, i)) then t else nil;

procedure hasIndefinableNum(value_)$
begin;
  scalar retFlag_, head_, flagList_, insideInvTrigonometricFunc_;

%  debugWrite("in hasIndefinableNum", " ");
%  debugWrite("value_: ", value_);

  if(arglength(value_)=-1) then <<
%    debugWrite("retFlag_: ", nil);
%    debugWrite("(value_: )", value_);
    return nil;
  >>;

  head_:= myHead(value_);
%  debugWrite("head_: ", head_);
  if(hasMinusHead(value_)) then <<
    % 負の数の場合
    retFlag_:= hasIndefinableNum(part(value_, 1));
  >> else if((head_=plus) or (head_=times)) then <<
    flagList_:= union(for each x in getArgsList(value_) join
      if(hasIndefinableNum(x)) then {t} else {});
%    debugWrite("flagList_: ", flagList_);
    retFlag_:= if(flagList_={}) then nil else t;
  >> else if(isInvTrigonometricFunc(head_)) then <<
    insideInvTrigonometricFunc_:= part(value_, 1);
    retFlag_:= if(checkOrderingFormula(insideInvTrigonometricFunc_>=-1) and checkOrderingFormula(insideInvTrigonometricFunc_<=1)) then nil else t;
  >> else <<
    retFlag_:= nil;
  >>;
%  debugWrite("retFlag_: ", retFlag_);
%  debugWrite("(value_: )", value_);
  return retFlag_;
end;

procedure putIrrationalNumberInterval(value_, interval_)$
  irrationalNumberIntervalList__:= cons({value_, interval_}, irrationalNumberIntervalList__);

procedure getIrrationalNumberInterval(value_)$
  for each x in irrationalNumberIntervalList__ join 
    if(first(x)=value_) then {second(x)} else {};

%---------------------------------------------------------------
% パラメタ関連の関数
%---------------------------------------------------------------

% 式中にパラメタが含まれているかどうかを、psParameters__内の変数が含まれるかどうかで判定
procedure hasParameter(expr_)$
  if(collectParameters(expr_) neq {}) then t else nil;

% 式構造中のパラメタを、集める
procedure collectParameters(expr_)$
begin;
  scalar collectedParameters_;

%  debugWrite("in collectParameters", " ");
%  debugWrite("expr_: ", expr_);

%  debugWrite("psParameters__: ", psParameters__);
  collectedParameters_:= union({}, for each x in psParameters__ join if(not freeof(expr_, x)) then {x} else {});

%  debugWrite("collectedParameters_: ", collectedParameters_);
  return collectedParameters_;
end;

%---------------------------------------------------------------
% 大小判定関連の関数（定数式のみ、パラメタなし）
%---------------------------------------------------------------

checkOrderingFormulaCount_:= 0;
checkOrderingFormulaIrrationalNumberCount_:= 0;

procedure checkOrderingFormula(orderingFormula_)$
%入力: 論理式(特にsqrt(2), greaterp_, sin(2)などを含むようなもの), 精度
%出力: t or nil or -1
%      (xとyがほぼ等しい時 -1)
%geq_= >=, geq; greaterp_= >, greaterp; leq_= <=, leq; lessp_= <, lessp;
begin;
  scalar head_, x, op, y, bak_precision, ans, margin, xInterval_, yInterval_;

  debugWrite("in checkOrderingFormula", " ");
  debugWrite("orderingFormula_: ", orderingFormula_);
  checkOrderingFormulaCount_:= checkOrderingFormulaCount_+1;
  debugWrite("checkOrderingFormulaCount_: ", checkOrderingFormulaCount_);

  head_:= myHead(orderingFormula_);
  % 大小に関する論理式以外が入力されたらエラー
  if(hasLogicalOp(head_)) then return ERROR;

  x:= lhs(orderingFormula_);
  op:= head_;
  y:= rhs(orderingFormula_);

  debugWrite("-----checkOrderingFormula-----", " ");
  debugWrite("x: ", x);
  debugWrite("op: ", op);
  debugWrite("y: ", y);

  if(x=y) then <<
    debugWrite("x=y= ", x);
    if((op = geq) or (op = leq)) then return t
    else return nil
  >>;

  if(not freeof({x,y}, INFINITY)) then <<
    ans:= myInfinityIf(x, op, y);
    debugWrite("ans after myInfinityIf: ", ans);
    debugWrite("(orderingFormula_: )", orderingFormula_);
    return ans;
  >>;
  

  if(not numberp(x) or not(numberp(y))) then <<
    checkOrderingFormulaIrrationalNumberCount_:= checkOrderingFormulaIrrationalNumberCount_+1;
    debugWrite("checkOrderingFormulaIrrationalNumberCount_: ", checkOrderingFormulaIrrationalNumberCount_);
    % 無理数が含まれる場合
    if(optUseApproximateCompare__) then <<
      % 近似値を用いた大小比較
      bak_precision := precision 0;
      on rounded$ precision approxPrecision__$

      % xおよびyが有理数である時
      % 10^(3 + yかxの指数部の値 - 有効桁数)
      if(min(x,y)=0) then
        margin:=10 ^ (3 + floor log10 max(x, y) - approxPrecision__)
      else if(min(x,y)>0) then 
        margin:=10 ^ (3 + floor log10 min(x, y) - approxPrecision__)
      else
        margin:=10 ^ (3 - floor log10 abs min(x, y) - approxPrecision__);

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
    >> else <<
      % 区間値への変換を用いた大小比較
      xInterval_:= convertValueToInterval(x);
      yInterval_:= convertValueToInterval(y);
      debugWrite("xInterval_: ", xInterval_);
      debugWrite("yInterval_: ", yInterval_);
      ans:= compareInterval(xInterval_, op, yInterval_);
    >>;
  >> else <<
%    debugWrite("myApply(op, {x, y}): ", myApply(op, {x, y}));
%    if(myApply(op, {x, y})) then ans:= t else ans:= nil;
    if ((op = geq) or (op = greaterp)) then
      (if (x > y) then ans:=t else ans:=nil)
    else
      (if (x < y) then ans:=t else ans:=nil);
  >>;


  debugWrite("ans in checkOrderingFormula: ", ans);
  debugWrite("(orderingFormula_: )", orderingFormula_);
  if(ans=unknown) then <<
    % unknownが返った場合は精度を変えて再試行
    intervalPrecision__:= intervalPrecision__+4;
    ans:= checkOrderingFormula(orderingFormula_);
    intervalPrecision__:= intervalPrecision__-4;
  >>;
  return ans;
end;

procedure myInfinityIf(x, op, y)$
begin;
  scalar ans_, infinityTupleDNF_, retTuple_, i_, j_, andAns_;

  debugWrite("in myInfinityIf", " ");
  debugWrite("op(x, y): ", op(x, y));
  % INFINITY > -INFINITYとかの対応
  if(x=INFINITY or y=-INFINITY) then 
    if((op = geq) or (op = greaterp)) then ans_:=t else ans_:=nil
  else if(x=-INFINITY or y=INFINITY) then
    if((op = leq) or (op = lessp)) then ans_:=t else ans_:=nil
  else <<
    % 係数等への対応として、まずinfinity relop valueの形にしてから解きなおす
    infinityTupleDNF_:= exIneqSolve(op(x, y));
    debugWrite("infinityTupleDNF_: ", infinityTupleDNF_);
    if(isFalseDNF(infinityTupleDNF_)) then return nil;
    i_:= 1;
    ans_:= nil;
    while (i_<=length(infinityTupleDNF_) and (ans_ neq t)) do <<
      j_:= 1;
      andAns_:= t;
      while (j_<=length(part(infinityTupleDNF_, i_)) and (andAns_ = t)) do <<
        retTuple_:= part(part(infinityTupleDNF_, i_), j_);
        andAns_:= myInfinityIf(getVarNameFromTuple(retTuple_), getRelopFromTuple(retTuple_), getValueFromTuple(retTuple_));
        j_:= j_+1;
      >>;
      ans_:= andAns_;
      i_:= i_+1;
    >>;
  >>;

  return ans_;
end;

procedure mymin(x,y)$
%入力: 数値という前提
  if(checkOrderingFormula(x<y)) then x else y;

procedure mymax(x,y)$
%入力: 数値という前提
  if(checkOrderingFormula(x>y)) then x else y;

procedure myFindMinimumValue(x,lst)$
%入力: 現段階での最小値x, 最小値を見つけたい対象のリスト
%出力: リスト中の最小値
  if(lst={}) then x
  else if(mymin(x, first(lst)) = x) then myFindMinimumValue(x,rest(lst))
  else myFindMinimumValue(first(lst),rest(lst));

procedure myFindMaximumValue(x,lst)$
%入力: 現段階での最大値x, 最大値を見つけたい対象のリスト
%出力: リスト中の最大値
  if(lst={}) then x
  else if(mymax(x, first(lst)) = x) then myFindMaximumValue(x,rest(lst))
  else myFindMaximumValue(first(lst),rest(lst));

%---------------------------------------------------------------
% TC形式関連の関数
%---------------------------------------------------------------

% TC形式（時刻と条件DNFの組）におけるアクセス用関数
procedure getTimeFromTC(TC_)$
  part(TC_, 1);
procedure getCondDNFFromTC(TC_)$
  part(TC_, 2);

%---------------------------------------------------------------
% 大小判定関連の関数（パラメタを含む）
%---------------------------------------------------------------

procedure compareValueAndParameter(val_, paramExpr_, condDNF_)$
%入力: 比較する対象の値, パラメータを含む式, パラメータに関する条件
%出力: {「値」側が小さくなるためのパラメータの条件, 「パラメータを含む式」が小さくなるためのパラメータの条件}
% checkInfMinTimeの仕様変更により不要に
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

%---------------------------------------------------------------
% 不等式タプル関連の関数
%---------------------------------------------------------------

procedure isLbTuple(tuple_)$
  if((getRelopFromTuple(tuple_)=geq) or (getRelopFromTuple(tuple_)=greaterp)) then t else nil;

procedure isUbTuple(tuple_)$
  if((getRelopFromTuple(tuple_)=leq) or (getRelopFromTuple(tuple_)=lessp)) then t else nil;

procedure getLbTupleListFromConj(conj_)$
  for each x in conj_ join if(isLbTuple(x)) then {x} else {};

procedure getUbTupleListFromConj(conj_)$
  for each x in conj_ join if(isUbTuple(x)) then {x} else {};

% {左辺, relop, 右辺}のタプルから、式を作って返す
procedure makeExprFromTuple(tuple_)$
  myApply(getRelopFromTuple(tuple_), {getVarNameFromTuple(tuple_), getValueFromTuple(tuple_)});

procedure getVarNameFromTuple(tuple_)$
  part(tuple_, 1);

procedure getRelopFromTuple(tuple_)$
  part(tuple_, 2);

procedure getValueFromTuple(tuple_)$
  part(tuple_, 3);

%---------------------------------------------------------------
% 不等式タプルによるDNF関連の関数
%---------------------------------------------------------------

% 論理積にNotをつけてDNFにする
procedure getNotConjDNF(conj_)$
  for each tuple in conj_ collect
    {{getVarNameFromTuple(tuple), getInverseRelop(getRelopFromTuple(tuple)), getValueFromTuple(tuple)}};

% Equalを表す論理積になっているかどうか
procedure isEqualConj(conj_)$
begin;
  scalar value1_, value2_, relop1_, relop2_;

  if(length(conj_) neq 2) then return nil;

  value1_:= getValueFromTuple(first(conj_));
  value2_:= getValueFromTuple(second(conj_));
  if(value1_ neq value2_) then return nil;

  relop1_:= getRelopFromTuple(first(conj_));
  relop2_:= getRelopFromTuple(second(conj_));
  if(((relop1_=geq) and (relop2_=leq)) or ((relop1_=leq) and (relop2_=geq))) then return t
  else return nil;
end;

procedure isSameConj(conj1_, conj2_)$
begin;
  scalar flag_;

  debugWrite("in isSameConj", " ");
  debugWrite("conj1_: ", conj1_);
  debugWrite("conj2_: ", conj2_);

  if(length(conj1_) neq length(conj2_)) then return nil;

  flag_:= t;
  for each x in conj1_ do 
    if(freeof(conj2_, x)) then flag_:= nil;
  return flag_;
end;

procedure isFalseConj(conj_)$
  if(conj_={}) then t else nil;

procedure isTrueConj(conj_)$
  if(conj_={true}) then t else nil;

procedure getNotDNF(DNF_)$
begin;
  scalar retDNF_, i_;

  debugWrite("in getNotDNF", " ");
  debugWrite("DNF_: ", DNF_);

  retDNF_:= {{true}};
  i_:= 1;
  while (not isFalseDNF(retDNF_) and i_<=length(DNF_)) do <<
    retDNF_:= addCondDNFToCondDNF(getNotConjDNF(part(DNF_, i_)), retDNF_);
    debugWrite("retDNF_ in loop in getNotDNF: ", retDNF_);
    i_:= i_+1;
  >>;

  debugWrite("retDNF_ in getNotDNF: ", retDNF_);
  debugWrite("(DNF_: )", DNF_);
  return retDNF_;
end;

procedure isSameDNF(DNF1_, DNF2_)$
begin;
  scalar flag_, i_, conj1, flagList_;

  debugWrite("in isSameDNF", " ");
  debugWrite("DNF1_: ", DNF1_);
  debugWrite("DNF2_: ", DNF2_);

  if(length(DNF1_) neq length(DNF2_)) then return nil;

  flag_:= t;
  i_:= 1;
  while (flag_=t and i_<=length(DNF1_)) do <<
    conj1:= part(DNF1_, i_);
    flagList_:= for each conj2 in DNF2_ join
      if(isSameConj(conj1, conj2)) then {true} else {};
    if(flagList_={}) then flag_:= nil;
    i_:= i_+1;
  >>;
  return flag_;
end;

procedure isFalseDNF(DNF_)$
  if(DNF_={{}}) then t else nil;

procedure isTrueDNF(DNF_)$
  if(DNF_={{true}}) then t else nil;

% 前提：式expr_は1つの関係演算子のみを持つ
procedure makeTupleDNFFromEq(var_, value_)$
  {{ {var_, geq, value_}, {var_, leq, value_} }};

procedure makeTupleDNFFromNeq(var_, value_)$
  {{ {var_, lessp, value_}, {var_, greaterp, value_} }};

% DNF中の、実質等しい（重複した）論理積を削除
procedure simplifyDNF(DNF_)$
begin;
  scalar tmpRetDNF_, simplifiedDNF_;

  % myFoldLeft((if(isSameConj(first(#1), #2)) then #1 else cons(#2, #1))&, {first(DNF_)}, rest(DNF_))を実現
  tmpRetDNF_:= {first(DNF_)};
  for i:=1 : length(rest(DNF_)) do <<
    tmpRetDNF_:= if(isSameConj(first(tmpRetDNF_), part(rest(DNF_), i))) then tmpRetDNF_
                 else cons(part(rest(DNF_), i), tmpRetDNF_);
  >>;
  simplifiedDNF_:= tmpRetDNF_;

  debugWrite("simplifiedDNF_: ", simplifiedDNF_);
  return simplifiedDNF_;
end;

procedure addCondDNFToCondDNF(newCondDNF_, condDNF_)$
begin;
  scalar addedCondDNF_, tmpAddedCondDNF_, i_;

  debugWrite("in addCondDNFToCondDNF", " ");
  debugWrite("newCondDNF_: ", newCondDNF_);
  debugWrite("condDNF_: ", condDNF_);

  % Falseを追加しようとする場合はFalseを返す
  if(isFalseDNF(newCondDNF_)) then return {{}};

  addedCondDNF_:= for each conj in newCondDNF_ join <<
    i_:= 1;
    tmpAddedCondDNF_:= condDNF_;
    while (i_<=length(conj) and not isFalseDNF(tmpAddedCondDNF_)) do <<
      tmpAddedCondDNF_:= addCondTupleToCondDNF(part(conj, i_), tmpAddedCondDNF_);
      debugWrite("tmpAddedCondDNF_ in loop in addCondDNFToCondDNF: ", tmpAddedCondDNF_);
      i_:= i_+1;
    >>;
    if(not isFalseDNF(tmpAddedCondDNF_)) then simplifyDNF(tmpAddedCondDNF_) else {}
  >>;

  % newCondDNF_内のどのconjを追加してもFalseDNFになるような場合に、最終的に{}が得られているので修正する
  if(addedCondDNF_={})then addedCondDNF_:= {{}};

  debugWrite("addedCondDNF_ in addCondDNFToCondDNF: ", addedCondDNF_);
  debugWrite("(newCondDNF_: )", newCondDNF_);
  debugWrite("(condDNF_: )", condDNF_);
  return addedCondDNF_;
end;

procedure addCondTupleToCondDNF(newCondTuple_, condDNF_)$
%入力：追加する（パラメタの）条件タプルnewCondTuple_, （パラメタの）条件を表す論理式のリストcondDNF_
%出力：（パラメタの）条件を表す論理式のDNF
%注意：リストの要素1つ1つは論理和でつながり、要素内は論理積でつながっていることを表す。
begin;
  scalar addedCondDNF_, addedCondConj_;

  debugWrite("in addCondTupleToCondDNF", " ");
  debugWrite("newCondTuple_: ", newCondTuple_);
  debugWrite("condDNF_: ", condDNF_);

  addedCondDNF_:= union(for each x in condDNF_ join <<
    addedCondConj_:= addCondTupleToCondConj(newCondTuple_, x);
    debugWrite("addedCondConj_ in loop in addCondTupleToCondDNF: ", addedCondConj_);
    if(not isFalseConj(addedCondConj_)) then {addedCondConj_} else {}
  >>);

  if(addedCondDNF_={}) then addedCondDNF_:= {{}};
  debugWrite("addedCondDNF_ in end of addCondTupleToCondDNF: ", addedCondDNF_);
  debugWrite("(newCondTuple_: )", newCondTuple_);
  debugWrite("(condDNF_: )", condDNF_);
  return addedCondDNF_;
end;

procedure addCondTupleToCondConj(newCondTuple_, condConj_)$
begin;
  scalar addedCondConj_, varName_, relop_, value_,
         varTerms_, ubTuple_, lbTuple_, ub_, lb_,
         ubTupleList_, lbTupleList_, ineqSolDNF_;

  debugWrite("in addCondTupleToCondConj", " ");
  debugWrite("newCondTuple_: ", newCondTuple_);
  debugWrite("condConj_: ", condConj_);

  % trueを追加しようとする場合、追加しないのと同じ
  if(newCondTuple_=true) then return condConj_;
  % パラメタが入らない場合、単に大小判定をした結果が残る
  % （移項するとパラメタが入らなくなる場合も同様）
  if(not hasParameter(makeExprFromTuple(newCondTuple_)) and freeof(makeExprFromTuple(newCondTuple_), t)) then 
    if(checkOrderingFormula(makeExprFromTuple(newCondTuple_))) then return condConj_
    else return {};
  % falseに追加しようとする場合
  if(isFalseConj(condConj_)) then return {};
  % trueに追加しようとする場合
  if(isTrueConj(condConj_)) then return {newCondTuple_};


  % 場合によっては、タプルのVarName部分とValue部分の両方に変数名が入っていることもある
  % そのため、たとえ1次であっても一旦exIneqSolveで解く必要がある
  if((arglength(getVarNameFromTuple(newCondTuple_)) neq -1) or not numberp(getValueFromTuple(newCondTuple_))) then <<
    % VarName部分が1次の変数名で、かつ、Value部分が数値ならこの処理は不要
    ineqSolDNF_:= exIneqSolve(makeExprFromTuple(newCondTuple_));
    debugWrite("ineqSolDNF_: ", ineqSolDNF_);
    debugWrite("(newCondTuple_: )", newCondTuple_);
    debugWrite("(condConj_: )", condConj_);
    if(isFalseDNF(ineqSolDNF_)) then <<
      % falseを表すタプルを論理積に追加しようとした場合はfalseを表す論理積を返す
      addedCondConj_:= {};
      debugWrite("addedCondConj_: ", addedCondConj_);
      return addedCondConj_;
    >> else if(isTrueDNF(ineqSolDNF_)) then <<
      % trueを表すタプルを論理積に追加しようとした場合はcondConj_を返す
      addedCondConj_:= condConj_;
      debugWrite("addedCondConj_: ", addedCondConj_);
      return addedCondConj_;
    >>;
    % DNF形式で返るので、改めてタプルを取り出す
    if(length(first(ineqSolDNF_))>1) then <<
      % 「=」を表す形式の場合はgeqとleqの2つのタプルの論理積が返る
      tmpConj_:= condConj_;
      for i:=1 : length(first(ineqSolDNF_)) do <<
        tmpConj_:= addCondTupleToCondConj(part(first(ineqSolDNF_), i), tmpConj_);
      >>;
      return tmpConj_;
    >> else <<
      newCondTuple_:= first(first(ineqSolDNF_));
    >>
  >>;


  varName_:= getVarNameFromTuple(newCondTuple_);
  relop_:= getRelopFromTuple(newCondTuple_);
  value_:= getValueFromTuple(newCondTuple_);
  debugWrite("varName_: ", varName_);
  debugWrite("relop_: ", relop_);
  debugWrite("value_: ", value_);

  % 論理積の中から、追加する変数と同じ変数の項を探す
  varTerms_:= union(for each term in condConj_ join
    if(not freeof(term, varName_)) then {term} else {});
  debugWrite("varTerms_: ", varTerms_);


  % 下限・上限を得る
  ubTupleList_:= getUbTupleListFromConj(varTerms_);
  lbTupleList_:= getLbTupleListFromConj(varTerms_);
  % 実数のみ比較対象とする（パラメタとの比較は別の処理で行う）
  ubTupleList_:= for each x in ubTupleList_ join if(not hasParameter(getValueFromTuple(x))) then {x} else {};
  lbTupleList_:= for each x in lbTupleList_ join if(not hasParameter(getValueFromTuple(x))) then {x} else {};
  debugWrite("ubTupleList_: ", ubTupleList_);
  debugWrite("lbTupleList_: ", lbTupleList_);
  if(ubTupleList_={}) then ubTupleList_:= {{varName_, leq, INFINITY}};
  if(lbTupleList_={}) then lbTupleList_:= {{varName_, geq, -INFINITY}};
  ubTuple_:= first(ubTupleList_);
  lbTuple_:= first(lbTupleList_);

  % 更新が必要かどうかを、追加する不等式と上下限とを比較することで決定
  ub_:= getValueFromTuple(ubTuple_);
  debugWrite("ub_: ", ub_);
  lb_:= getValueFromTuple(lbTuple_);
  debugWrite("lb_: ", lb_);
  if((relop_=leq) or (relop_=lessp)) then <<
    % 上限側を調整
    if(mymin(value_, ub_)=value_) then <<
      % 更新を要するかどうかを判定。上限が同じ場合も、等号の有無によっては更新が必要
      if((value_ neq ub_) or freeof(ubTupleList_, lessp)) then
        addedCondConj_:= cons(newCondTuple_, condConj_ \ ubTupleList_)
      else addedCondConj_:= condConj_;
      ub_:= value_;
      ubTuple_:= newCondTuple_;
    >> else <<
      addedCondConj_:= condConj_;
    >>;
  >> else <<
    % 下限側を調整
    if(mymin(value_, lb_)=lb_) then <<
      % 更新を要するかどうかを判定。下限が同じ場合も、等号の有無によっては更新が必要
      if((value_ neq lb_) or freeof(lbTupleList_, greaterp)) then
        addedCondConj_:= cons(newCondTuple_, condConj_ \ lbTupleList_)
      else addedCondConj_:= condConj_;
      lb_:= value_;
      lbTuple_:= newCondTuple_;
    >> else <<
      addedCondConj_:= condConj_;
    >>;
  >>;
  debugWrite("addedCondConj_: ", addedCondConj_);
  debugWrite("lb_: ", lb_);
  debugWrite("lbTuple_: ", lbTuple_);

  % 最後にlb≦ubを確かめ、矛盾する場合は{}を返す（falseを表す論理積）
  % ubがデフォルト（INFINITY）のままなら確認不要
  if(ub_ neq INFINITY) then << 
    if(mymin(lb_, ub_)=lb_) then <<
      % lb=ubの場合、関係演算子がgeqとleqでなければならない
      if((lb_=ub_) and not isEqualConj({ubTuple_, lbTuple_})) then addedCondConj_:= {};
    >> else <<
      addedCondConj_:= {};
    >>;
  >>;


  debugWrite("addedCondConj_: ", addedCondConj_);
  debugWrite("(newCondTuple_: )", newCondTuple_);
  debugWrite("(condConj_: )", condConj_);
  return addedCondConj_;
end;

%---------------------------------------------------------------
% 論理式関連の関数
%---------------------------------------------------------------

%数式のリストをandで繋いだ論理式に変換する
procedure mymkand(lst)$
for i:=1:length(lst) mkand part(lst,i);

procedure mymkor(lst)$
for i:=1:length(lst) mkor part(lst, i);

procedure myex(lst,var)$
rlqe ex(var, mymkand(lst));

procedure myall(lst,var)$
rlqe all(var, mymkand(lst));

%---------------------------------------------------------------
% 求解・無矛盾性判定関連の関数
%---------------------------------------------------------------

% 有理化を行った上で解を返す
% TODO：実数解のみを返すようにする
procedure exSolve(exprs_, vars_)$
begin;
  scalar tmpSol_, retSol_;

  % 三角関数まわりの方程式を解いた場合、解は1つに限定してしまう
  % TODO：どうにかする？
  off allbranch;
  tmpSol_:= solve(exprs_, vars_);
  on allbranch;

  % 虚数解を除く
  tmpSol_:= for each x in tmpSol_ join
    if(myHead(x)=list) then <<
      {for each y in x join 
        if(hasImaginaryNum(rhs(y))) then {} else {y}
      }
    >> else <<
      if(hasImaginaryNum(rhs(x))) then {} else {x}
    >>;
  debugWrite("tmpSol_ after removing imaginary number: ", tmpSol_);

  % 実数上で定義できない値（asin(5)など）を除く
  tmpSol_:= for each x in tmpSol_ join
    if(myHead(x)=list) then <<
      {for each y in x join 
        if(hasIndefinableNum(rhs(y))) then {} else {y}
      }
    >> else <<
      if(hasIndefinableNum(rhs(x))) then {} else {x}
    >>;
  debugWrite("tmpSol_ after removing indefinable value: ", tmpSol_);

  % 有理化
  retSol_:= for each x in tmpSol_ collect
    if(myHead(x)=list) then map(rationalisation, x)
    else rationalisation(x);
  debugWrite("retSol_ in exSolve: ", retSol_);
  return retSol_;
end;

% 不等式を、左辺に正の変数名のみがある形式に整形し、それを表すタプルを作る
% (不等式の場合、必ず右辺が0になってしまう（自動的に移項してしまう）ことや、
% 不等式の向きが限定されてしまったりすることがあるため)
% 前提：入力される不等式も変数も1つ
% TODO：複数の不等式が渡された場合への対応？
% 2次不等式までは解ける
% TODO：3次以上への対応？
% 入力：1変数の2次以下の不等式1つ
% 出力：{左辺, relop, 右辺}の形式によるDNFリスト
% 等式の場合にも対応。geqとleqを一緒に使うことにより表現する
procedure exIneqSolve(ineqExpr_)$
begin;
  scalar lhs_, relop_, rhs_, adjustedLhs_, adjustedRelop_, 
         reverseRelop_, adjustedIneqExpr_, sol_, adjustedEqExpr_,
         retTuple_, exprVarList_, exprVar_, retTupleDNF_,
         boundList_, ub_, lb_, eqSol_, eqSolValue_, sqrtCondTuple_, lcofRet_, sqrtCoeffList_,
         sqrtCoeff_, insideSqrtExprs_;

  debugWrite("========== in exIneqSolve ==========", " ");
  debugWrite("ineqExpr_: ", ineqExpr_);

  lhs_:= lhs(ineqExpr_);
  relop_:= myHead(ineqExpr_);
  rhs_:= rhs(ineqExpr_);

  % 複数の変数が入っている場合への対応：tとusrVar→parameter→INFINITYの順に見る
  exprVarList_:= union(for each x in union(csVariables__, {t}) join
    if(not freeof(ineqExpr_, x)) then {x} else {});
  if(exprVarList_={}) then <<
    exprVarList_:= union(for each x in psParameters__ join
      if(not freeof(ineqExpr_, x)) then {x} else {});
  exprVarList_:= if(exprVarList_ neq {}) then exprVarList_ else {INFINITY}
  >>;
  debugWrite("exprVarList_: ", exprVarList_);
  exprVar_:= first(exprVarList_);
  debugWrite("exprVar_: ", exprVar_);


  % 右辺を左辺に移項する
  adjustedIneqExpr_:= myApply(relop_, {lhs_+(-1)*rhs_, 0});
  adjustedLhs_:= lhs(adjustedIneqExpr_);
  debugWrite("adjustedIneqExpr_: ", adjustedIneqExpr_);
  debugWrite("adjustedLhs_: ", adjustedLhs_);
  adjustedRelop_:= myHead(adjustedIneqExpr_);
  debugWrite("adjustedRelop_: ", adjustedRelop_);

  % 三角関数を含む場合、特別な符号判定が必要
  if(hasTrigonometricFunc(adjustedLhs_)) then <<
    compareIntervalRet_:= compareInterval(convertValueToInterval(adjustedLhs_), adjustedRelop_, makePointInterval(0));
    debugWrite("compareIntervalRet_: ", compareIntervalRet_);
    if(compareIntervalRet_=t) then <<
      retTupleDNF_:= {{true}};
      debugWrite("retTupleDNF_: ", retTupleDNF_);
      debugWrite("(ineqExpr_: )", ineqExpr_);
      return retTupleDNF_;
    >> else if(compareIntervalRet_=nil) then <<
      retTupleDNF_:= {{}};
      debugWrite("retTupleDNF_: ", retTupleDNF_);
      debugWrite("(ineqExpr_: )", ineqExpr_);
      return retTupleDNF_;
    >> else <<
      % unknownが返った
      retTupleDNF_:= {{unknown}};
      debugWrite("retTupleDNF_: ", retTupleDNF_);
      debugWrite("(ineqExpr_: )", ineqExpr_);
      return retTupleDNF_;
    >>;
  >>;

  % この時点でx+value relop 0または-x+value relop 0の形式（1次式）になっているので、
  % -x+value≦0の場合はx-value≧0の形にする必要がある（relopとして逆のものを得る必要がある）
  % サーバーモードだと≧を使った式表現は保持できないようなので、reverseするかどうかだけ分かれば良い
  % minusで条件判定を行うことがなぜかできないので、式に出現する変数名xを調べて、その係数の正負を調べる
  lcofRet_:= lcof(adjustedLhs_, exprVar_);
  if(not numberp(lcofRet_)) then lcofRet_:= 0;
  sqrtCoeffList_:= getSqrtList(adjustedLhs_, exprVar_, COEFF);
  % 前提：根号の中にパラメタがある項は多くても1つ
  % TODO：なんとかする
  if(sqrtCoeffList_={}) then sqrtCoeff_:= 0
  else if(length(sqrtCoeffList_)=1) then sqrtCoeff_:= first(sqrtCoeffList_);
  % TODO：厳密にはxorを使うべきか？
  if(checkOrderingFormula(lcofRet_<0) or checkOrderingFormula(sqrtCoeff_<0)) then <<
    adjustedRelop_:= getReverseRelop(adjustedRelop_);
  >>;
  debugWrite("adjustedRelop_: ", adjustedRelop_);

  % 変数にsqrtがついてる場合は、変数は0以上であるという条件を最後に追加する必要がある
  % TODO: sqrt(x-1)とかへの対応
  insideSqrtExprs_:= getSqrtList(adjustedLhs_, exprVar_, INSIDE);
  debugWrite("insideSqrtExprs_: ", insideSqrtExprs_);
  sqrtCondTupleList_:= for each x in insideSqrtExprs_ collect
    first(first(exIneqSolve(myApply(geq, {x, 0}))));
  debugWrite("sqrtCondTupleList_: ", sqrtCondTupleList_);
  if(sqrtCondTupleList_={}) then sqrtCondTupleList_:= {true};

  
  % ubまたはlbを求める
  off arbvars;
  on multiplicities;
  sol_:= exSolve(equal(adjustedLhs_, 0), exprVar_);
  debugWrite("sol_: ", sol_);
  on arbvars;
  off multiplicities;

  % TODO：複数変数への対応？
  % 複数変数が入ると2重リストになるはずだが、1変数なら不要か？
%  if(length(sol_)>1 and myHead(first(sol_))=list) then <<
%    % orでつながった中にandが入っている関係を表している
%      
%    adjustedEqExpr_:= first(first(sol_))
%
%  >> 

  if(sol_={}) then <<
    % adjustedLhs_=0に対する実数解がない場合
    % adjustedIneqExpr_が-t^2-1<0やt^2+5<0などのとき
    if((adjustedRelop_=geq) or (adjustedRelop_=greaterp) or (adjustedRelop_=neq)) then retTupleDNF_:= {{true}}
    else retTupleDNF_:= {{}};
  >> else if(length(sol_)>1) then <<
    % 2次方程式を解いた場合。厳密には長さ2のはず
    % TODO：3次以上への一般化？

    boundList_:= {rhs(part(sol_, 1)), rhs(part(sol_, 2))};
    debugWrite("boundList_: ", boundList_);
    if(not hasParameter(boundList_)) then <<
      if(length(union(boundList_))=1) then <<
        % 重解を表す場合
        lb_:= first(union(boundList_));
        ub_:= first(union(boundList_));
      >> else <<
        lb_:= myFindMinimumValue(INFINITY, boundList_);
        ub_:= first(boundList_ \ {lb_});
      >>;
      debugWrite("lb_ in exIneqSolve: ", lb_);
      debugWrite("ub_ in exIneqSolve: ", ub_);
    
      % relopによって、tupleDNFの構成を変える
      if((adjustedRelop_ = geq) or (adjustedRelop_ = greaterp)) then <<
        % ≧および＞の場合はorの関係になる
        retTupleDNF_:= {{ {exprVar_, getReverseRelop(adjustedRelop_), lb_} }, { {exprVar_, adjustedRelop_, ub_} }};
      >> else if((adjustedRelop_ = geq) or (adjustedRelop_ = greaterp)) then <<
        % ≦および＜の場合はandの関係になる
        retTupleDNF_:= {{ {exprVar_, getReverseRelop(adjustedRelop_), lb_},     {exprVar_, adjustedRelop_, ub_} }};
      >> else <<
        % ≠の場合
        retTupleDNF_:= {{ {exprVar_, lessp, lb_} }, { {exprVar_, greaterp, lb_}, {exprVar_, lessp, ub_} }, { {exprVar_, greaterp, ub_} }};
      >>;
    >> else <<
      % パラメタの解が得られた場合
      % どちらが下限でどちらが上限になるかをどのように調べるか？？
      % 調べるんじゃなくてそういう式を追加すれば良い      


      retTupleDNF_:= hogehogegege; % ←
    >>;
  >> else <<
    adjustedEqExpr_:= first(sol_);
    debugWrite("adjustedEqExpr_: ", adjustedEqExpr_);
    retTuple_:= {lhs(adjustedEqExpr_), adjustedRelop_, rhs(adjustedEqExpr_)};
    debugWrite("retTuple_: ", retTuple_);
    retTupleDNF_:= {{retTuple_}};
  >>;

  debugWrite("retTupleDNF_ before adding sqrtCondTuple_: ", retTupleDNF_);

  if(not isFalseDNF(retTupleDNF_)) then <<
    debugWrite("sqrtCondTupleList_: ", sqrtCondTupleList_);
    for i:=1 : length(sqrtCondTupleList_) do <<
      debugWrite("add : ", part(sqrtCondTupleList_, i));
      retTupleDNF_:= addCondTupleToCondDNF(part(sqrtCondTupleList_, i), retTupleDNF_);
      debugWrite("retTupleDNF_ in loop of adding sqrtCondTuple: ", retTupleDNF_);
    >>;
  >>;
  debugWrite("retTupleDNF_ after adding sqrtCondTuple_: ", retTupleDNF_);
  debugWrite("(ineqExpr_: )", ineqExpr_);
  debugWrite("========== end exIneqSolve ==========", " ");

  return retTupleDNF_;
end;

procedure exRlqe(formula_)$
begin;
  scalar appliedFormula_, header_, argsCount_, appliedArgsList_;
  debugWrite("formula_: ", formula_);

  % 引数を持たない場合（trueなど）
  if(arglength(formula_)=-1) then <<
    appliedFormula_:= rlqe(formula_);
    return appliedFormula_;
  >>;

  header_:= myHead(formula_);
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
    if(checkOrderingFormula(myApply(getInverseRelop(header_), {lhs(formula_), rhs(formula_)}))) then 
      appliedFormula_:= false 
    else appliedFormula_:= rlqe(formula_);
  >> else <<
    appliedFormula_:= rlqe(formula_);
  >>;

  debugWrite("appliedFormula_: ", appliedFormula_);
  return appliedFormula_;
end;

%---------------------------------------------------------------
% ラプラス変換関連の関数
%---------------------------------------------------------------

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


procedure getf(x,lst)$
if(lst={}) then nil
        else if(x=lhs(first(lst))) then rhs(first(lst))
                else getf(x,rest(lst));

procedure lgetf(x,llst)$
%入力: 変数名, 等式のリストのリスト(ex. {{x=1,y=2},{x=3,y=4},...})
%出力: 変数に対応する値のリスト
if(llst={}) then {}
        else if(rest(llst)={}) then getf(x,first(llst))
                else getf(x,first(llst)) . {lgetf(x,rest(llst))};

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

  debugWrite("table_: ", table_);
  % solveans_にsolvevars_の解が一つでも含まれない時 underconstraintと想定
  for each x in table_ do 
    if(freeof(solveans_, third(x))) then tmp_:=true;
  debugWrite("is under-constraint?: ", tmp_);
  if(tmp_=true) then return retunderconstraint___;

  debugWrite("table_: ", table_);
  
  % solveans_の逆ラプラス変換
  ans_:= for each table in table_ collect
      (first table) = invlap(lgetf((third table), solveans_),s,t);
  debugWrite("ans expr?: ", ans_);

  table_:={};
  return ans_;
end;

%---------------------------------------------------------------
% 特定の要素を抽出/削除したり複数の要素を分類したりする関数
%---------------------------------------------------------------

procedure getFrontTwoElemList(lst_)$
  if(length(lst_)<2) then {}
  else for i:=1 : 2 collect part(lst_, i);

procedure removeTrueList(patternList_)$
  for each x in patternList_ join if(rlqe(x)=true) then {} else {x};

% 論理式がtrueであるとき、現在はそのままtrueを返している
% TODO：なんとかする
procedure removeTrueFormula(formula_)$
  if(formula_=true) then true
  else myApply(and, for each x in getArgsList(formula_) join 
    if(rlqe(x)=true) then {} else {x});

% 制約リストから、等式以外を含む制約を抽出する
procedure getOtherExpr(exprs_)$
  for each x in exprs_ join if(hasIneqRelop(x) or hasLogicalOp(x)) then {x} else {};

% NDExpr（exDSolveで扱えないような制約式）であるかどうかを調べる
% 式の中にsinもcosも入っていなければfalse
procedure isNDExpr(expr_)$
  if(freeof(expr_, sin) and freeof(expr_, cos)) then nil else t;

procedure splitExprs(exprs_, vars_)$
begin;
  scalar otherExprs_, NDExprs_, NDExprVars_, DExprs_, DExprVars_;

  otherExprs_:= union(for each x in exprs_ join 
                  if(hasIneqRelop(x) or hasLogicalOp(x)) then {x} else {});
  NDExprs_ := union(for each x in (exprs_ \ otherExprs_) join 
                if(isNDExpr(x)) then {x} else {});
  NDExprVars_ := union(for each x in vars_ join if(not freeof(NDExprs_, x)) then {x} else {});
  DExprs_ := (exprs_ \ otherExprs_) \ NDExprs_;
  DExprVars_:= union(for each x in vars_ join if(not freeof(DExprs_, x)) then {x} else {});
  return {NDExprs_, NDExprVars_, DExprs_, DExprVars_, otherExprs_};
end;

% 前提：入力はinitxlhs=prev(x)の形
% TODO：なんとかする
procedure getInitVars(rcont_)$
  lhs(rcont_);

procedure getIneqBoundTCLists(ineqTCList_)$
begin;
  scalar lbTCList_, ubTCList_, timeExpr_, condExpr_, relop_, exprLhs_, lcofRet_, solveAns_;

  lbTCList_:= {};
  ubTCList_:= {};
  for each x in ineqTCList_ do <<
    timeExpr_:= getTimeFromTC(x);
    condExpr_:= getCondDNFFromTC(x);

    % timeExpr_は (t - value) op 0 または (-t - value) op 0 の形を想定
    relop_:= myHead(timeExpr_);
    exprLhs_:= lhs(timeExpr_);
    lcofRet_:= lcof(exprLhs_, t);
    if(checkOrderingFormula(lcofRet_<0)) then relop_:= getReverseRelop(relop_);
    solveAns_:= first(solve(exprLhs_=0, t));

    if((relop_ = geq) or (relop_ = greaterp)) then
      lbTCList_:= cons({rhs(solveAns_), condExpr_}, lbTCList_)
    else ubTCList_:= cons({rhs(solveAns_), condExpr_}, ubTCList_);
  >>;
  debugWrite("lbTCList_: ", lbTCList_);
  debugWrite("ubTCList_: ", ubTCList_);
  return {lbTCList_, ubTCList_};
end;

%---------------------------------------------------------------
% 待ち行列I関連の関数
%---------------------------------------------------------------

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

%---------------------------------------------------------------
% HydLa向け関数（df変数関連）
%---------------------------------------------------------------

procedure removeDifferentialVars(vars_)$
  union(for each x in vars_ join if(freeof(x, df)) then {x} else {});

procedure isDifferentialVar(var_)$
  if(arglength(var_)=-1) then nil
  else if(myHead(var_)=df) then t
  else nil;

%---------------------------------------------------------------
% HydLa向け関数（prev変数関連）
%---------------------------------------------------------------

procedure isPrevVariable(expr_)$
  if(arglength(expr_)=-1) then nil
  else if(myHead(expr_)=prev) then t
  else nil;

% prev変数の場合prevを外す
procedure removePrev(var_)$
  if(myHead(var_)=prev) then part(var_, 1) else var_;

% リストからprev変数を除く
procedure removePrevCons(consList_)$
  union(for each x in consList_ join if(freeof(x, prev)) then {x} else {});

%---------------------------------------------------------------
% HydLa向け関数（init変数関連）
%---------------------------------------------------------------

% REDUCEStringSenderから呼び出す
% @params vars_ init変数のリスト
procedure addInitVariables(vars_)$
begin;
  putLineFeed();

  initVariables__:= union(initVariables__, vars_);
  debugWrite("initVariables__: ", initVariables__);
end;

% initVariables__に含まれる変数かどうか調べる
procedure isInitVariable(var_)$
begin;
  scalar ret_;
  putLineFeed();

  if(arglength(var_) neq -1) then return nil;
  ret_ := nil;
  for each x in initVariables__ do if x = var_ then ret_ := t;
  return ret_;
end;

procedure removeInitCons(consList_)$
begin;
  putLineFeed();

  return union(consList_ \
    for each initVariable in initVariables__ join
      for each x in consList_ join 
        if(freeof(x, initVariable)) then {} else {x}
  );
end;

%---------------------------------------------------------------
% HydLa向け関数（共通して必要）
%---------------------------------------------------------------

operator prev;

rettrue___    := 1;
retfalse___   := 2;
retunknown___ := 3;


% 制約ストアのリセット
procedure resetConstraintStore()$
begin;
  putLineFeed();

  constraintStore__ := {};
  csVariables__ := {};
  parameterStore__:= {};
  prevConstraint__:= {};
  psParameters__:= {};
  isTemporary__:= nil;
  initConstraint__ := {};
  initTmpConstraint__:= {};
  tmpConstraint__:= {};
  tmpVariables__:= {};
  prevVariables__:= {};
  guard__:= {};
  guardVars__:= {};
  initVariables__:= {};
  debugWrite("constraintStore__: ", constraintStore__);
  debugWrite("csVariables__: ", csVariables__);
  debugWrite("parameterStore__: ", parameterStore__);
  debugWrite("prevConstraint__: ", prevConstraint__);
  debugWrite("prevVariables__: ", prevVariables__);
  debugWrite("psParameters__: ", psParameters__);
  debugWrite("isTemporary__", isTemporary__);
  debugWrite("initConstraint__", initConstraint__);
  debugWrite("initTmpConstraint__", initTmpConstraint__);
  debugWrite("tmpVariables__", tmpVariables__);
  debugWrite("initVariables__", initVariables__);
end;

procedure resetConstraintForVariable()$
begin;
  putLineFeed();

  constraintStore__ := {};
  csVariables__ := {};
  tmpVariables__:= {};
  prevVariables__:= {};
  debugWrite("constraintStore__: ", constraintStore__);
  debugWrite("csVariables__: ", csVariables__);
  debugWrite("tmpVariables__", tmpVariables__);
  debugWrite("prevVariables__: ", prevVariables__);
end;

% TODO co_にprevConstraint__を適用する
procedure addInitConstraint(co_, va_)$
begin;
  putLineFeed();

  debugWrite("prevConstraint__: ", prevConstraint__);
  debugWrite("co_: ", co_);
  debugWrite("va_: ", va_);

  if(isTemporary__) then
  <<
    tmpVariables__:= union(tmpVariables__, va_);
    initTmpConstraint__ := union(initTmpConstraint__, myExSub(prevConstraint__, co_));
  >> else
  <<
    csVariables__ := union(csVariables__, va_);
    initConstraint__ := union(initConstraint__, myExSub(prevConstraint__, co_));
  >>;

  debugWrite("tmpVariables__: ", tmpVariables__);
  debugWrite("initTmpConstraint__: ", initTmpConstraint__);
  debugWrite("csVariables__: ", csVariables__);
  debugWrite("initConstraint__: ", initConstraint__);
end;

procedure addPrevConstraint(cons_, vars_)$
begin;
  putLineFeed();

  prevConstraint__:= union(prevConstraint__, cons_);
  prevVariables__:= union(prevVariables__, vars_);

  debugWrite("prevConstraint__: ", prevConstraint__);
  debugWrite("prevVariables__: ", prevVariables__);
end;

procedure addGuard(gu_, vars_)$
begin;
  putLineFeed();
  if(part(gu_,0)=list) then
  <<
    guard__:= union(guard__, gu_); 
  >> else
  <<
    guard__:= union(guard__, {gu_}); 
  >>;

  guardVars__:= union(guardVars__, vars_);
  debugWrite("guard__: ", guard__);
  debugWrite("guardVars__: ", guardVars__);
end;

procedure setGuard(gu_, vars_)$
begin;
  putLineFeed();
  if(part(gu_,0)=list) then
  <<
    guard__:= gu_; 
  >> else
  <<
    guard__:= {gu_};
  >>;

  guardVars__:= vars_;
  debugWrite("guard__: ", guard__);
  debugWrite("guardVars__: ", guardVars__);
end;

procedure startTemporary()$
begin;
  putLineFeed();
  isTemporary__:= t;
end;

procedure endTemporary()$
begin;
  putLineFeed();
  isTemporary__:= nil;
  resetTemporaryConstraint();
end;

procedure resetTemporaryConstraint()$
begin;
  putLineFeed();
  tmpConstraint__:= {};
  initTmpConstraint__:= {};
  tmpVariables__:= {};
  guard__:= {};
  guardVars__:= {};
end;

% addConstraintは行わない
% PP/IPで共通のreset時に行う、制約ストアへの制約の追加
procedure addParameterConstraint(pcons_, pars_)$
begin;
  putLineFeed();
  debugWrite("in addParameterConstraint", " ");

  parameterStore__ := union(parameterStore__, pcons_);
  psParameters__ := union(psParameters__, pars_);
  debugWrite("new parameterStore__: ", parameterStore__);
  debugWrite("new psParameters__: ", psParameters__);

end;

%% 現在は代わりにaddParameterConstraintを使う
%% PP/IPで共通のreset時に行う、制約ストアへの制約の追加
%procedure addConstraintReset(cons_, vars_, pcons_, pars_)$
%begin;
%  putLineFeed();
%
%  debugWrite("in addConstraintReset", " ");
%  debugWrite("parameterStore__: ", parameterStore__);
%  debugWrite("psParameters__: ", psParameters__);
%  debugWrite("pcons_: ", pcons_);
%  debugWrite("pars_:", pars_);
%
%  parameterStore__ := union(parameterStore__, pcons_);
%  psParameters__ := union(psParameters__, pars_);
%  debugWrite("new parameterStore__: ", parameterStore__);
%  debugWrite("new psParameters__: ", psParameters__);
%
%  return addConstraint(cons_, vars_);
%
%end;

% 制約ストアへの制約の追加
procedure addConstraint(cons_, vars_)$
begin;
  putLineFeed();

  debugWrite("in addConstraint", " ");
  debugWrite("cons_: ", cons_);
  debugWrite("vars_: ", vars_);
  debugWrite("prevConstraint__: ", prevConstraint__);

  if(isTemporary__) then
  <<
    tmpVariables__:= union(tmpVariables__, vars_);
    tmpConstraint__:= union(tmpConstraint__, myExSub(prevConstraint__, cons_));
  >> else
  <<
    csVariables__:= union(csVariables__, vars_);
    constraintStore__:= union(constraintStore__, myExSub(prevConstraint__, cons_));
  >>;

  debugWrite("csVariables__: ", csVariables__);
  debugWrite("constraintStore__: ", constraintStore__);
  debugWrite("tmpVariables__: ", tmpVariables__);
  debugWrite("tmpConstraint__: ", tmpConstraint__);

  debugWrite("myExSub(prevConstraint__, cons_): ", myExSub(prevConstraint__, cons_));
  return if(isTemporary__) then tmpConstraint__ else constraintStore__;
end;

procedure getConstraintStore()$
begin;
  putLineFeed();

  debugWrite("constraintStore__:", constraintStore__);
  if(constraintStore__={}) then return {{}, parameterStore__};

  % 解を1つだけ得る
  % TODO: Orでつながった複数解への対応
  if(myHead(first(constraintStore__))=list) then return {first(constraintStore__), parameterStore__}
  else return {constraintStore__, parameterStore__};
end;


procedure checkLessThan(lhs_, rhs_)$
begin;
  scalar ret_;
  putLineFeed();

  ret_:= if(mymin(lhs_, rhs_) = lhs_) then rettrue___ else retfalse___;
  debugWrite("ret_:", ret_);

  return ret_;
end;

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

procedure exprTimeShift(expr_, time_)$
begin;
  scalar shiftedExpr_;
  putLineFeed();

  shiftedExpr_:= sub(t=t-time_, expr_);
  debugWrite("shiftedExpr_:", shiftedExpr_);

  return shiftedExpr_;
end;

%---------------------------------------------------------------
% HydLa向け関数（PPにおいて必要）
%---------------------------------------------------------------

% TODO!! check_consistency_result_tの要件をしっかり確認すること
% initConstraint__,  initTmpConstraint__, tmpVariables__ 対応版
% TODO: .m::checkConsistencyPoint[constraint && tmpConstraint && guard && initConstraint && initTmpConstraint, pConstraint, Union[variables, tmpVariables, guardVars]] の全変数に対応したい
% TODO; myLCont_にふるい分けるisInitVariableを正常に稼働させる
% @return {true, false} または{false, true}

procedure myCheckConsistencyPoint()$
begin;
  scalar ans_, ansBool_, trueMap_, falseMap_, tmpExprs_, myExpr_, myLCont_, myVars_;
  putLineFeed();
  tmpExprs_ := union(tmpConstraint__, guard__, initConstraint__, initTmpConstraint__);
  % 条件: tmpExprs_に含まれる連続性制約は必ず initxlhs=(数式) の格好である
  myExpr_ := for each x in tmpExprs_ join if(isInitVariable(lhs x)) then {} else {x};
  myLCont_ := for each x in tmpExprs_ join if(isInitVariable (lhs x)) then {x} else {};
  myVars_ := union(csVariables__, tmpVariables__, guardVars__);
  
  if(myExpr_ = {} and myLCont_ = {} and myVars_ = {} and constraintStore__ = {}) then
  <<
    ansBool_:= true;
    debugWrite("because of myExpr_ = {} and myLCont_ = {} and myVars_ = {} and constraintStore__ = {}, ansBool_: ", ansBool_);
  >> else <<
    ans_:= checkConsistencyBySolveOrRlqe(constraintStore__, myExpr_, myLCont_, parameterStore__, myVars_);
    debugWrite("ans_ in checkConsistencyBySolveOrRlqe: ", ans_);
    ansBool_:= rlqe(part(ans_, 1) = rettrue___);
  >>;

  if parameterStore__ <> {} then << 
    trueMap_:= rlqe(ansBool_ and mymkand(parameterStore__));
    falseMap_:= rlqe(not ansBool_ and mymkand(parameterStore__));

    % TODO prev変数の処理, t>0としてtの除去
    trueMap_:= if(myHead(trueMap_) = and) then map(makeConsTuple, getArgsList(trueMap_)) else trueMap_;
    falseMap_:= if(myHead(falseMap_) = and) then map(makeConsTuple, getArgsList(falseMap_)) else falseMap_;
    ans_:= {trueMap_, falseMap_};
  >> else if(ansBool_ = true) then ans_:= {true, false}
  else ans_:= {false, true};

  debugWrite("ans_ in myCheckConsistencyPoint: ", ans_);

  return ans_;
end;

% (制限 andを受け付けない) TODO 制限への対応
% (制限 trueを受け付けない) TODO 制限への対応
procedure checkConsistencyWithTmpCons(expr_, lcont_, vars_)$
begin;
  scalar ans_;
  putLineFeed();

  ans_:= {part(checkConsistencyBySolveOrRlqe(constraintStore__, expr_, lcont_, parameterStore__, vars_), 1)};
  debugWrite("ans_ in checkConsistencyWithTmpCons: ", ans_);

  return ans_;
end;

% PPにおける無矛盾性の判定
% 仕様 QE未使用 % (使用するなら, 変数は基本命題的に置き換え)
% @param cons_ constraintStore__
% @param expr_ 一時的な制約集合
% @param lcont_ 一時的な左連続性制約集合
% @param pCons_ 一時的なパラメータ制約の集合, 現状未使用
% @param vars_ 一時的な変数集合
% @return {ans, {{変数名 = 値},...}} の形式

procedure checkConsistencyBySolveOrRlqe(cons_, exprs_, lcont_, pCons_, vars_)$
begin;
  scalar exprList_, eqExprs_, otherExprs_, modeFlagList_, mode_, tmpSol_,
         solvedExprs_, solvedExprsQE_, ans_;

  debugWrite("checkConsistencyBySolveOrRlqe: ", " ");
  debugWrite("{cons_, exprs_, lcont_, pCons_, vars_}: ", {cons_, exprs_, lcont_, pCons_, vars_});

  exprList_:= union(cons_, exprs_);
  debugWrite("exprList_: ", exprList_);
  otherExprs_:= getOtherExpr(exprList_);
  debugWrite("otherExprs_: ", otherExprs_);
  eqExprs_:= exprList_ \ otherExprs_;
  debugWrite("eqExprs_: ", eqExprs_);
%  debugWrite("union(eqExprs_, lcont_):",  union(eqExprs_, lcont_));

  % 未知変数を追加しないようにする
  off arbvars;
  tmpSol_:= exSolve(union(eqExprs_, lcont_),  vars_);
  on arbvars;
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

  sol_:= checkConsistencyBySolveOrRlqe({}, {}, {}, {}, {});
  debugWrite("sol_ in checkConsistency: ", sol_);
  % ret_codeがrettrue___、つまり1であるかどうかをチェック
  if(part(sol_, 1) = 1) then constraintStore__:= part(sol_, 2);
  debugWrite("constraintStore__: ", constraintStore__);

end;

procedure applyPrevCons(csList_, retList_)$
begin;
  scalar firstCons_, newCsList_, ret_;
%  debugWrite("in applyPrevCons", " ");
  if(csList_={}) then return retList_;

  firstCons_:= first(csList_);
%  debugWrite("firstCons_: ", firstCons_);
  if(not freeof(lhs(firstCons_), prev)) then <<
    newCsList_:= union(for each x in rest(csList_) join {exSub({firstCons_}, x)});
    ret_:= applyPrevCons(rest(csList_), retList_);
  >> else if(not freeof(rhs(firstCons_), prev)) then <<
    ret_:= applyPrevCons(cons(getReverseCons(firstCons_), rest(csList_)), retList_);
  >> else <<
    ret_:= applyPrevCons(rest(csList_), cons(firstCons_, retList_));
  >>;
  return ret_;
end;

% convertCSToVM内で使う、整形用関数
procedure makeConsTuple(cons_)$
begin;
  scalar varName_, relopCode_, value_, tupleDNF_, retTuple_, adjustedCons_, sol_;

  debugWrite("in makeConsTuple", " ");
  debugWrite("cons_: ", cons_);
  
  % 左辺に変数名のみがある形式にする
  % 前提：等式はすでにこの形式になっている
  if(not hasIneqRelop(cons_)) then <<
    varName_:= lhs(cons_);
    relopCode_:= getExprCode(cons_);
    value_:= rhs(cons_);
  >> else <<
    tupleDNF_:= exIneqSolve(cons_);
    debugWrite("tupleDNF_: ", tupleDNF_);
    % 1次式になってるはずなので、解は1つなはず
    retTuple_:= first(first(tupleDNF_));
    
    varName_:= getVarNameFromTuple(retTuple_);
    relopCode_:= getExprCode(getRelopFromTuple(retTuple_));
    value_:= getValueFromTuple(retTuple_);
  >>;
  debugWrite("varName_: ", varName_);
  debugWrite("relopCode_: ", relopCode_);
  debugWrite("value_: ", value_);
  return {varName_, relopCode_, value_};
end;

% constraintStore__にinitConstraint__をunionしてconvertCSToVMに遷移する
procedure myConvertCSToVM()$
begin;
  putLineFeed();
  debugWrite("removePrevCons(initConstraint__): ", removePrevCons(initConstraint__));
  debugWrite("old constraintStore__: ", constraintStore__);

  constraintStore__:= union(constraintStore__, removePrevCons(initConstraint__));
  return convertCSToVM();
end;

% 前提：Orでつながってはいない
% TODO：なんとかする
procedure convertCSToVM()$
begin;
  scalar consTmpRet_, consRet_, paramDNFList_, paramRet_, tuple_, ret_;
  putLineFeed();

  debugWrite("constraintStore__:", constraintStore__);

  consTmpRet_:= applyPrevCons(constraintStore__, {});    
  debugWrite("consTmpRet_: ", consTmpRet_);

  % 式を{(変数名), (関係演算子コード), (値のフル文字列)}の形式に変換する
  consRet_:= map(makeConsTuple, consTmpRet_);
  paramDNFList_:= map(exIneqSolve, parameterStore__);
  paramRet_:= for each x in paramDNFList_ collect <<
    % 1つの項になっているはず
    tuple_:= first(first(x));
    {getVarNameFromTuple(tuple_), getExprCode(getRelopFromTuple(tuple_)), getValueFromTuple(tuple_)}
  >>;
  ret_:= union(consRet_, paramRet_);

  ret_:= getUsrVars(ret_, removePrevCons(csVariables__));

  debugWrite("ret_: ", ret_);
  return ret_;
end;

%---------------------------------------------------------------
% HydLa向け関数（IPにおいて必要）
%---------------------------------------------------------------

% TODO!! check_consistency_result_tの要件をしっかり確認すること
% initConstraint__,  initTmpConstraint__, tmpVariables__ 対応版
% TODO: .m::checkConsistencyPoint[constraint && tmpConstraint && guard && initConstraint && initTmpConstraint, pConstraint, Union[variables, tmpVariables, guardVars]] の全変数に対応したい
% @return {true, false} または{false, true}

procedure myCheckConsistencyInterval()$
begin;
  scalar ans_, ansBool_, trueMap_, falseMap_, tmpExprs_, myTmpCons_, myRCont_, myVars_;
  putLineFeed();
  tmpExprs_ := union(tmpConstraint__, guard__, initConstraint__, initTmpConstraint__);
  % 条件: tmpExprs_に含まれる連続性制約は必ず initxlhs=prev(x) の格好である
  myTmpCons_ := for each x in tmpExprs_ join if(isInitVariable(lhs x)) then {} else {x};
  myRCont_ :=   for each x in tmpExprs_ join if(isInitVariable(lhs x)) then {x} else {};
  myVars_ := union(csVariables__, tmpVariables__, guardVars__);
  
  ans_:= checkConsistencyInterval(constraintStore__, myTmpCons_, myRCont_, parameterStore__, myVars_);
  debugWrite("ans_ in checkConsistencyInterval: ", ans_);

  ansBool_:= rlqe(part(ans_, 1) = ICI_CONSISTENT___);

  if(part(ans_, 1)=ICI_UNKNOWN___) then ans_:= UNKNOWN
  else if parameterStore__ <> {} then << 
    trueMap_:= rlqe(ansBool_ and mymkand(parameterStore__));
    falseMap_:= rlqe(not ansBool_ and mymkand(parameterStore__));

    % TODO prev変数の処理, t>0としてtの除去
    trueMap_:= if(myHead(trueMap_) = and) then map(makeConsTuple, getArgsList(trueMap_)) else trueMap_;
    falseMap_:= if(myHead(falseMap_) = and) then map(makeConsTuple, getArgsList(falseMap_)) else falseMap_;
    ans_:= {trueMap_, falseMap_};
  >> else if(ansBool_ = true) then ans_:= {true, false}
  else ans_:= {false, true};

  debugWrite("ans_ in myCheckConsistencyInterval: ", ans_);

  return ans_;
end;

% 20110705 overconstraint___無し
ICI_SOLVER_ERROR___:= 0;
ICI_CONSISTENT___:= 1;
ICI_INCONSISTENT___:= 2;
ICI_UNKNOWN___:= 3; % 不要？

% TODO parameterStore__を仮引数に追加する
procedure checkConsistencyInterval(cons_, tmpCons_, rconts_, pCons_, vars_)$
begin;
  scalar tmpSol_, splitExprsResult_, NDExprs_, NDExprVars_, DExprs_, DExprVars_, otherExprs_,
         initCons_, initVars_, prevVars_, noPrevVars_, noDifferentialVars_, tmpVarMap_,
         DExprRconts_, DExprRcontsVars_,
         integTmp_, integTmpQE_, integTmpQEList_, integTmpEqualList_, integTmpIneqSolDNFList_, integTmpIneqSolDNF_, ans_;
  putLineFeed();

  debugWrite("{cons_, tmpCons_, rconts_, pCons_, vars_}: ", {cons_, tmpCons_, rconts_, pCons_, vars_});

  % SinやCosが含まれる場合はラプラス変換不可能なのでNDExpr扱いする
  % TODO:なんとかしたいところ？
  splitExprsResult_ := splitExprs(removePrevCons(cons_), vars_);
  NDExprs_ := part(splitExprsResult_, 1);
  debugWrite("NDExprs_: ", NDExprs_);
  NDExprVars_:= part(splitExprsResult_, 2);
  debugWrite("NDExprVars_: ", NDExprVars_);
  DExprs_ := part(splitExprsResult_, 3);
  debugWrite("DExprs_: ", DExprs_);
  DExprVars_ := part(splitExprsResult_, 4);
  debugWrite("DExprVars_: ", DExprVars_);
  otherExprs_:= part(splitExprsResult_, 5);
  debugWrite("otherExprs_: ", otherExprs_);

  DExprRconts_:= removeInitCons(rconts_);
  debugWrite("DExprRconts_: ", DExprRconts_);
  if(DExprRconts_ neq {}) then <<
    prevVars_:= for each x in vars_ join if(isPrevVariable(x)) then {x} else {};
    debugWrite("prevVars_: ", prevVars_);
    noPrevVars_:= union(for each x in prevVars_ collect part(x, 1));
    debugWrite("noPrevVars_: ", noPrevVars_);
    DExprRcontsVars_ := union(for each x in noPrevVars_ join if(not freeof(DExprRconts_, x)) then {x} else {});
    debugWrite("DExprRcontsVars_: ", DExprRcontsVars_);
    DExprs_:= union(DExprs_, DExprRconts_);
    DExprVars_:= union(DExprVars_, DExprRcontsVars_);
  >>;

  initCons_:= union(for each x in (rconts_ \ DExprRconts_) collect exSub(cons_, x));
  debugWrite("initCons_: ", initCons_);
  initVars_:= map(getInitVars, initCons_);
  debugWrite("initVars_: ", initVars_);

  noDifferentialVars_:= union(for each x in DExprVars_ collect if(isDifferentialVar(x)) then part(x, 1) else x);
  debugWrite("noDifferentialVars_: ", noDifferentialVars_);
  tmpSol_:= exDSolve(DExprs_, initCons_, noDifferentialVars_);
  debugWrite("tmpSol_ solved with exDSolve: ", tmpSol_);

  
  if(tmpSol_ = retsolvererror___) then return {ICI_SOLVER_ERROR___}
  else if(tmpSol_ = retoverconstraint___) then return {ICI_INCONSISTENT___};

  tmpVarMap_:= first(myFoldLeft(createIntegratedValue, {{},tmpSol_}, union(DExprVars_, (vars_ \ initVars_))));
  debugWrite("tmpVarMap_:", tmpVarMap_);
  tmpSol_:= for each x in tmpVarMap_ collect (part(x, 1)=part(x, 2));
  debugWrite("tmpSol_:", tmpSol_);


  % NDExpr_を連立
  if(union(tmpSol_, NDExprs_) neq {}) then <<
    debugWrite("union(DExprVars_, union(NDExprVars_, (vars_ \ initVars_))): ", 
               union(DExprVars_, union(NDExprVars_, (vars_ \ initVars_))));
    tmpSol_:= solve(union(tmpSol_, NDExprs_), union(DExprVars_, union(NDExprVars_, (vars_ \ initVars_))));
    debugWrite("tmpSol_ after solve with NDExpr_: ", tmpSol_);
    if(tmpSol_ = {}) then return {ICI_INCONSISTENT___};
  >>;

  tmpVarMap_:= first(myFoldLeft(createIntegratedValue, {{},tmpSol_}, union(DExprVars_, union(NDExprVars_, (vars_ \ initVars_)))));
  debugWrite("tmpVarMap_:", tmpVarMap_);
  tmpSol_:= for each x in tmpVarMap_ collect (first(x)=second(x));
  debugWrite("tmpSol_:", tmpSol_);


  % tmpCons_がない場合は無矛盾と判定して良い
  if(tmpCons_ = {}) then return {ICI_CONSISTENT___};

  % TODO このへんから分岐して不等式判定処理を追加するか？
  % if(tmpCons_ = {} and parameterStore__ = {}) then return {ICI_CONSISTENT___};

  integTmp_:= sub(tmpSol_, tmpCons_);
  debugWrite("integTmp_: ", integTmp_);

  % 前提：ParseTree構築時に分割されているはずなのでガードにorが入ることは考えない
  integTmpList_:= if(myHead(first(integTmp_))=and) then <<
    union(for each x in getArgsList(first(integTmp_)) join <<
      integTmpQE_:= rlqe(x);
      if(integTmpQE_=true) then {}
      else if(integTmpQE_=false) then {false}
      else {x}
    >>)
  >> else <<
    integTmpQE_:= rlqe(first(integTmp_));
    if(integTmpQE_=true) then {}
    else if(integTmpQE_=false) then {false}
    else integTmp_
  >>;
  debugWrite("integTmpList_: ", integTmpList_);

  % ガード条件全体がtrueの場合
  if(integTmpList_={}) then return {ICI_CONSISTENT___};
  % ガード条件内にfalseが入る場合
  if(not freeof(integTmpList_, false)) then return {ICI_INCONSISTENT___};

  integTmpEqualList_:= for each x in integTmpList_ join 
    if(myHead(x)=equal) then {x} 
    else {};
  debugWrite("integTmpEqualList_: ", integTmpEqualList_);
  % ガード条件判定においてはandでつながった等式がある場合、結果はfalse
  if(integTmpEqualList_ neq {}) then return {ICI_INCONSISTENT___};


  % それぞれの不等式について、1次にしてDNFにし、integTmpIneqSolDNFList_とする。
  % 注意：それぞれの要素間はandの関係である
  integTmpIneqSolDNFList_:= union(for each x in (integTmpList_ \ integTmpEqualList_) collect <<
                              ineqSolDNF_:= exIneqSolve(x);
                              debugWrite("ineqSolDNF_: ", ineqSolDNF_);
                              if(isFalseDNF(ineqSolDNF_)) then {{}}
                              else if(isTrueDNF(ineqSolDNF_)) then {{true}}
                              else ineqSolDNF_
                            >>);

  debugWrite("integTmpIneqSolDNFList_:", integTmpIneqSolDNFList_);

  integTmpIneqSolDNF_:= myFoldLeft(addCondDNFToCondDNF, {{{t, greaterp, 0}}}, integTmpIneqSolDNFList_);
  debugWrite("integTmpIneqSolDNF_:", integTmpIneqSolDNF_);

  % 不等式の場合、ここで初めて矛盾が見つかり、integTmpIneqSolDNF_がfalseになることがある
  if(isFalseDNF(integTmpIneqSolDNF_)) then return {ICI_INCONSISTENT___}
  % trueになったら無矛盾
  else if(isTrueDNF(integTmpIneqSolDNF_)) then return {ICI_CONSISTENT___};


  ans_:= checkInfUnitDNF(integTmpIneqSolDNF_);
  debugWrite("ans_: ", ans_);


  if(ans_=true) then return {ICI_CONSISTENT___}
  else if(ans_=false) then return {ICI_INCONSISTENT___}
  else <<
    debugWrite("rlqe ans: ", ans_);
    return {ICI_UNKNOWN___};
  >>;

end;

procedure checkInfUnitDNF(tDNF_)$
begin;
  scalar conj_, infCheckAns_, orArgsAnsList_, lbTupleList_, lbTuple_;

  conj_:= first(tDNF_);
  if(length(tDNF_)>1) then <<
    orArgsAnsList_:= union(for i:=1 : length(tDNF_) collect
      checkInfUnitDNF({part(tDNF_, 1)}));
    debugWrite("orArgsAnsList_: ", orArgsAnsList_);
    infCheckAns_:= if(orArgsAnsList_={false}) then false else true;
  >> else if(isEqualConj(conj_)) then <<
    % Equalを表す論理積の場合
    infCheckAns_:= false;
  >> else <<
    lbTupleList_:= getLbTupleListFromConj(conj_);
    % 前提：下限は1つ
    % TODO：パラメタによる下限もある場合への対応
    lbTuple_:= first(lbTupleList_);
    if((getRelopFromTuple(lbTuple_)=greaterp) and (getValueFromTuple(lbTuple_)=0)) then 
      infCheckAns_:= true
    else infCheckAns_:= false;
  >>;
  debugWrite("infCheckAns_: ", infCheckAns_);
  debugWrite("(tDNF_: )", tDNF_);

  return infCheckAns_;
end;

% 出力：時刻を表すDNFと条件の組（TC）のリスト
% TODO：ERROR処理
procedure checkInfMinTimeDNF(tDNF_, condDNF_)$
begin;
  scalar minTCList_, conj_, argsAnsTCListList_, minValue_, compareTCListList_, lbTuplelist_, ubTupleList_,
         lbParamTupleList_, ubParamTupleList_, lbValueTupleList_, ubValueTupleList_, lbValue_, ubValue_, 
         paramLeqValueCondDNF_, paramGreaterValueCondDNF_, checkDNF_, diffCondDNF_;
  debugWrite("in checkInfMinTimeDNF", " ");
  debugWrite("tDNF_: ", tDNF_);
  debugWrite("condDNF_: ", condDNF_);

  conj_:= first(tDNF_);
  if(length(tDNF_)>1) then <<
    argsAnsTCListList_:= union(for i:=1 : length(tDNF_) collect
      checkInfMinTimeDNF({part(tDNF_, i)}, condDNF_));
    debugWrite("argsAnsTCListList_: ", argsAnsTCListList_);
    minTCList_:= myFoldLeft(compareMinTimeList, first(argsAnsTCListList_), rest(argsAnsTCListList_));
  >> else if(isEqualConj(conj_)) then <<
    % Equalを表す論理積の場合
    if(not hasParameter(conj_)) then minTCList_:= {{getValueFromTuple(first(conj_)), condDNF_}}
    else <<
      % パラメタの場合、その値が0以下ならば結果はINFINITYになる
      minValue_:= getValueFromTuple(first(conj_));
      checkDNF_:= addCondTupleToCondDNF({minValue_, greaterp, 0}, condDNF_);
      debugWrite("checkDNF_: ", checkDNF_);
      if(not isFalseDNF(checkDNF_)) then << 
        if(isSameDNF(checkDNF_, condDNF_)) then <<
          minTCList_:= {{minValue_, checkDNF_}};
        >> else <<
          diffCondDNF_:= addCondDNFToCondDNF(getNotDNF(checkDNF_), condDNF_);
          debugWrite("diffCondDNF_: ", diffCondDNF_);
          minTCList_:= {{minValue_, checkDNF_}, {INFINITY, diffCondDNF_}};
        >>;
      >> else <<
        minTCList_:= {{INFINITY, condDNF_}};
      >>;
    >>;
  >> else <<
    if(not hasParameter(conj_)) then minTCList_:= {{getValueFromTuple(first(getLbTupleListFromConj(conj_))), condDNF_}}
    else << 
      % パラメタの入った下限もある場合
      % 前提：condDNF_内の定数の種類は1つまで？
      % TODO：なんとかする
      lbTupleList_:= getLbTupleListFromConj(conj_);
      ubTupleList_:= conj_ \ lbTupleList_;
      lbParamTupleList_:= for each x in lbTuplelist_ join if(hasParameter(x)) then {x} else {};
      ubParamTupleList_:= for each x in ubTuplelist_ join if(hasParameter(x)) then {x} else {};
      lbValueTupleList_:= lbTuplelist_ \ lbParamTupleList_;
      ubValueTupleList_:= ubTuplelist_ \ ubParamTupleList_;
      if(lbValueTupleList_={}) then lbValueTupleList_:= {{t, geq, -INFINITY}};
      if(ubValueTupleList_={}) then ubValueTupleList_:= {{t, leq,  INFINITY}};
      lbValue_:= getValueFromTuple(first(lbValueTupleList_));
      ubValue_:= getValueFromTuple(first(ubValueTupleList_));


      minTCList_:= {};
      % パラメタを含む下限がlbValue_より大きいかどうかを調べる
      % TODO：パラメタを含む下限同士の大小判定
      debugWrite("lbParamTupleList_: ", lbParamTupleList_);
      debugWrite("lbValue_: ", lbValue_);
      for each x in lbParamTupleList_ do <<
        debugWrite("x (lbParamTuple): ", x);
        checkDNF_:= addCondTupleToCondDNF({getValueFromTuple(x), greaterp, lbValue_}, condDNF_);
        debugWrite("checkDNF_: ", checkDNF_);
        if(not isFalseDNF(checkDNF_)) then <<
          if(isSameDNF(checkDNF_, condDNF_)) then <<
            minTCList_:= cons({getValueFromTuple(x), checkDNF_}, minTCList_);
          >> else <<
            diffCondDNF_:= addCondDNFToCondDNF(getNotDNF(checkDNF_), condDNF_);
            debugWrite("diffCondDNF_: ", diffCondDNF_);
            if(checkOrderingFormula(lbValue_ >0)) then <<
              minTCList_:= cons({getValueFromTuple(x), checkDNF_}, cons({lbValue_, diffCondDNF_}, minTCList_));
            >> else <<
              minTCList_:= cons({getValueFromTuple(x), checkDNF_}, cons({INFINITY, diffCondDNF_}, minTCList_));
            >>;
          >>;
          debugWrite("minTCList_: ", minTCList_);
        >>;
      >>;
      debugWrite("minTCList_ after add: ", minTCList_);


      debugWrite("========== check param-ub ==========", " ");
      % パラメタを含む上限は下限以上でなくてはならない
      debugWrite("ubParamTupleList_: ", ubParamTupleList_);
      if(minTCList_ neq {}) then <<
        for each x in ubParamTupleList_ do <<
          minTCList_:= union(for each y in minTCList_ join <<
            if(getTimeFromTC(y) neq INFINITY) then <<
              checkDNF_:= addCondTupleToCondDNF({getValueFromTuple(x), geq, getTimeFromTC(y)}, getCondDNFFromTC(y));
              debugWrite("checkDNF_: ", checkDNF_);
              if(not isFalseDNF(checkDNF_)) then {y} else {}
            >> else <<
              % 下限がINFINITYのとき（ある特定の範囲のパラメタによって離散変化が起きないパターン）は上下限確認不要
              % TODO：本当？
              {y}
            >>
          >>);
        >>;
        if(minTCList_={}) then minTCList_:= {{INFINITY, condDNF_}};
      >> else <<
        % パラメタの下限がなかった（lbParamTupleList_が空集合）ときはlbValue_とだけ比較
        for each x in ubParamTupleList_ do <<
          checkDNF_:= addCondTupleToCondDNF({getValueFromTuple(x), geq, lbValue_}, condDNF_);
          if(not isFalseDNF(checkDNF_) and checkOrderingFormula(lbValue_ >0)) then minTCList_:= {{lbValue_, condDNF_}}
          else minTCList_:= {{INFINITY, condDNF_}};
        >>;
      >>;
    >>;
  >>;

  debugWrite("minTCList_ in checkInfMinTimeDNF: ", minTCList_);
  debugWrite("(tDNF_: )", tDNF_);
  debugWrite("(condDNF_: )", condDNF_);
  return minTCList_;
end;

IC_SOLVER_ERROR___:= 0;
IC_NORMAL_END___:= 1;

procedure integrateCalc(rconts_, discCause_, vars_, maxTime_)$
begin;
  scalar tmpSol_, splitExprsResult_, NDExprs_, NDExprVars_, DExprs_, DExprVars_, otherExprs_, paramCondDNF_,
         initCons_, initVars_, prevVars_, noPrevVars_, noDifferentialVars_,
         DExprRconts_, DExprRcontsVars_,
         tmpDiscCause_, retCode_, tmpVarMap_, tmpMinTList_, integAns_, tmpIneqSolDNF_;
  putLineFeed();

  debugWrite("rconts_: ", rconts_);
  debugWrite("discCause_: ", discCause_);
  debugWrite("vars_: ", vars_);
  debugWrite("maxTime_: ", maxTime_);

  % SinやCosが含まれる場合はラプラス変換不可能なのでNDExpr扱いする
  % TODO:なんとかしたいところ？
  splitExprsResult_ := splitExprs(removePrevCons(constraintStore__), csVariables__);
  NDExprs_ := part(splitExprsResult_, 1);
  debugWrite("NDExprs_: ", NDExprs_);
  NDExprVars_ := part(splitExprsResult_, 2);
  debugWrite("NDExprVars_: ", NDExprVars_);
  DExprs_ := part(splitExprsResult_, 3);
  debugWrite("DExprs_: ", DExprs_);
  DExprVars_ := part(splitExprsResult_, 4);
  debugWrite("DExprVars_: ", DExprVars_);
  otherExprs_:= union(part(splitExprsResult_, 5), parameterStore__);
  debugWrite("otherExprs_: ", otherExprs_);
  % DNF形式にする
  % 空集合なら、{{true}}として扱う（trueを表すDNF）
  if(otherExprs_={}) then paramCondDNF_:= {{true}}
  else <<
    % paramCondDNF_:= myFoldLeft((addCondDNFToCondDNF(#1, exIneqSolve(#2)))&, {{true}}, otherExprs_);を実現
    tmpIneqSolDNF_:= {{true}};
    for i:=1 : length(otherExprs_) do <<
      tmpIneqSolDNF_:= addCondDNFToCondDNF(tmpIneqSolDNF_, exIneqSolve(part(otherExprs_, i)));
    >>;
    paramCondDNF_:= tmpIneqSolDNF_;
  >>;
  debugWrite("paramCondDNF_: ", paramCondDNF_);


  DExprRconts_:= removePrevCons(rconts_);
  debugWrite("DExprRconts_: ", DExprRconts_);
  if(DExprRconts_ neq {}) then <<
    prevVars_:= for each x in csVariables__ join if(isPrevVariable(x)) then {x} else {};
    debugWrite("prevVars_: ", prevVars_);
    noPrevVars_:= union(for each x in prevVars_ collect part(x, 1));
    debugWrite("noPrevVars_: ", noPrevVars_);
    DExprRcontsVars_ := union(for each x in noPrevVars_ join if(not freeof(DExprRconts_, x)) then {x} else {});
    debugWrite("DExprRcontsVars_: ", DExprRcontsVars_);
    DExprs_:= union(DExprs_, DExprRconts_);
    DExprVars_:= union(DExprVars_, DExprRcontsVars_);
  >>;

  initCons_:= union(for each x in (rconts_ \ DExprRconts_) collect exSub(constraintStore__, x));
  debugWrite("initCons_: ", initCons_);
  initVars_:= map(getInitVars, initCons_);
  debugWrite("initVars_: ", initVars_);

  noDifferentialVars_:= union(for each x in DExprVars_ collect if(isDifferentialVar(x)) then part(x, 1) else x);
  debugWrite("noDifferentialVars_: ", noDifferentialVars_);
  tmpSol_:= exDSolve(DExprs_, initCons_, noDifferentialVars_);
  debugWrite("tmpSol_ solved with exDSolve: ", tmpSol_);


  tmpVarMap_:= first(myFoldLeft(createIntegratedValue, {{},tmpSol_}, union(DExprVars_, (vars_ \ initVars_))));
  debugWrite("tmpVarMap_:", tmpVarMap_);
  tmpSol_:= for each x in tmpVarMap_ collect (first(x)=second(x));
  debugWrite("tmpSol_:", tmpSol_);


  % NDExpr_を連立
  if(union(tmpSol_, NDExprs_) neq {}) then <<
    debugWrite("union(DExprVars_, union(NDExprVars_, (vars_ \ initVars_))): ", 
               union(DExprVars_, union(NDExprVars_, (vars_ \ initVars_))));
    tmpSol_:= solve(union(tmpSol_, NDExprs_), union(DExprVars_, union(NDExprVars_, (vars_ \ initVars_))));
    debugWrite("tmpSol_ after solve with NDExpr_: ", tmpSol_);
    if(tmpSol_ = {}) then return {ICI_INCONSISTENT___};
  >>;

  tmpVarMap_:= first(myFoldLeft(createIntegratedValue, {{},tmpSol_}, union(DExprVars_, union(NDExprVars_, (vars_ \ initVars_)))));
  debugWrite("tmpVarMap_:", tmpVarMap_);
  tmpSol_:= for each x in tmpVarMap_ collect (part(x, 1)=part(x, 2));
  debugWrite("tmpSol_:", tmpSol_);


  tmpDiscCause_:= union(sub(tmpSol_, discCause_));
  debugWrite("tmpDiscCause_:", tmpDiscCause_);

  tmpVarMap_:= first(myFoldLeft(createIntegratedValue, {{},tmpSol_}, union(DExprVars_, union(NDExprVars_, (vars_ \ initVars_)))));
  debugWrite("tmpVarMap_:", tmpVarMap_);

  tmpMinTList_:= calcNextPointPhaseTime(maxTime_, tmpDiscCause_, paramCondDNF_);
  debugWrite("tmpMinTList_:", tmpMinTList_);
  if(tmpMinTList_ = {error}) then retCode_:= IC_SOLVER_ERROR___
  else retCode_:= IC_NORMAL_END___;

  integAns_:= {retCode_, tmpVarMap_, tmpMinTList_};
  debugWrite("integAns_: ", integAns_);
  
  return integAns_;
end;

procedure createIntegratedValue(pairInfo_, variable_)$
begin;
  scalar retList_, integRule_, integExpr_, newRetList_;

  retList_:= first(pairInfo_);
  integRule_:= second(pairInfo_);

  integExpr_:= {variable_, sub(integRule_, variable_)};
%  debugWrite("integExpr_: ", integExpr_);

  newRetList_:= cons(integExpr_, retList_);
%  debugWrite("newRetList_: ", newRetList_);

  return {newRetList_, integRule_};
end;

procedure calcNextPointPhaseTime(maxTime_, discCauseList_, condDNF_)$
begin;
  scalar minTCondListList_, comparedMinTCondList_, 
         minTTime_, minTCondDNF_, maxTimeFlag_, ans_;

  debugWrite("in calcNextPointPhaseTime", " ");
  debugWrite("discCauseList_: ", discCauseList_);
  debugWrite("condDNF_: ", condDNF_);

  minTCondListList_:= union(for each x in discCauseList_ collect findMinTime(x, condDNF_));
  debugWrite("minTCondListList_ in calcNextPointPhaseTime: ", minTCondListList_);

  if(not freeof(minTCondListList_, error)) then return {error};

  debugWrite("============================== before compareMinTimeList ==============================", " ");
  comparedMinTCondList_:= myFoldLeft(compareMinTimeList, {{maxTime_, condDNF_}}, minTCondListList_);
  debugWrite("comparedMinTCondList_ in calcNextPointPhaseTime: ", comparedMinTCondList_);

  ans_:= union(for each x in comparedMinTCondList_ collect <<
    minTTime_:= getTimeFromTC(x);
    minTCondDNF_:= getCondDNFFromTC(x);
    % Condがtrueの場合は空集合として扱う
    if(isTrueDNF(minTCondDNF_)) then minTCondDNF_:= {{}};
    % 演算子部分をコードに置き換える
    % TODO DNFが"x=1 and x=2"と "x+1 or x=2"を正しく区別できているか?
    minTCondDNF_:= for each conj in minTCondDNF_ collect
     for each term in conj collect {getVarNameFromTuple(term), getExprCode(getRelopFromTuple(term)), getValueFromTuple(term)};
    maxTimeFlag_:= if(minTTime_ neq maxTime_) then 0 else 1;
    {minTTime_, minTCondDNF_, maxTimeFlag_}
  >>);
  debugWrite("ans_ in calcNextPointPhaseTime: ", ans_);
  debugWrite("============================= end of calcNextPointPhaseTime ==============================", " ");
  return ans_;
end;

procedure findMinTime(integAsk_, condDNF_)$
begin;
  scalar integAskQE_, integAskList_, integAskIneqSolDNFList_, integAskIneqSolDNF_,
         minTCList_, tmpSol_, ineqSolDNF_;

  debugWrite("in findMinTime", " ");
  debugWrite("integAsk_: ", integAsk_);
  debugWrite("condDNF_: ", condDNF_);

  % t>0と連立してfalseになるような場合はMinTimeを考える必要がない
  if(rlqe(integAsk_ and t>0) = false) then return {{INFINITY, condDNF_}};


  % 前提：ParseTree構築時に分割されているはずなのでガードにorが入ることは考えない
  % TODO：¬gの形だと入ることがあるのでは？？
  integAskList_:= if(myHead(integAsk_)=and) then <<
    union(for each x in getArgsList(integAsk_) join <<
      integAskQE_:= rlqe(x);
      if(integAskQE_=true) then {error}
      else if(integAskQE_=false) then {false}
      else {x}
    >>)
  >> else <<
    integAskQE_:= rlqe(integAsk_);
    if(integAskQE_=true) then {error}
    else if(integAskQE_=false) then {false}
    else {integAsk_}
  >>;
  debugWrite("integAskList_: ", integAskList_);

  % ガード条件全体がtrueの場合とガード条件内にfalseが入る場合はエラー
  if(integAskList_={error} or not freeof(integAskList_, false)) then return {error};

%  integAskQE_:= rlqe(integAsk_);
%  debugWrite("integAskQE_: ", integAskQE_);
%
%  %%%%%%%%%%%% TODO:この辺から、%%%%%%%%%%%%%%
%  % まず、andでつながったtmp制約をリストに変換
%  if(myHead(integAskQE_)=and) then integAskList_:= getArgsList(integAskQE_)
%  else integAskList_:= {integAskQE_};
%  debugWrite("integAskList_:", integAskList_);


  integAskIneqSolDNFList_:= union(for each x in integAskList_ collect
                              if(not isIneqRelop(myHead(x))) then <<
                                tmpSol_:= exSolve(x, t);
                                if(length(tmpSol_)>1) then for each y in tmpSol_ join makeTupleDNFFromEq(lhs(y), rhs(y))
                                else makeTupleDNFFromEq(lhs(first(tmpSol_)), rhs(first(tmpSol_)))
                              >> else <<
                                ineqSolDNF_:= exIneqSolve(x);
                                debugWrite("ineqSolDNF_: ", ineqSolDNF_);
                                if(isFalseDNF(ineqSolDNF_)) then {{}}
                                else if(isTrueDNF(ineqSolDNF_)) then {{error}} % ←
                                else ineqSolDNF_
                              >>
                            );
  debugWrite("integAskIneqSolDNFList_:", integAskIneqSolDNFList_);
  % unknownが含まれたらエラーを返す
  % TODO：要検討
  if(not freeof(integAskIneqSolDNFList_, unknown)) then return {error}
  % trueになったらエラー
  else if(not freeof(integAskIneqSolDNFList_, error)) then return {error};


  debugWrite("========== add t>0 ==========", " ");
  integAskIneqSolDNF_:= myFoldLeft(addCondDNFToCondDNF, {{{t, greaterp, 0}}}, integAskIneqSolDNFList_);
  debugWrite("========== end add t>0 ==========", " ");
  debugWrite("integAskIneqSolDNF_:", integAskIneqSolDNF_);

  % 不等式の場合、ここで初めて矛盾が見つかり、integAskIneqSolDNF_がfalseになることがある
  if(isFalseDNF(integAskIneqSolDNF_)) then return {{INFINITY, condDNF_}};

  %%%%%%%%%%%% TODO:この辺までを1つの処理にまとめたい%%%%%%%%%%%%


  minTCList_:= checkInfMinTimeDNF(integAskIneqSolDNF_, condDNF_);
  debugWrite("minTCList_ in findMinTime: ", minTCList_);
  debugWrite("=================== end of findMinTime ====================", " ");

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

  % Mapではなく、Joinを使う方が正しそうなのでそうしている
%  comparedList_:= for each x in candidateTCList_ collect compareParamTime(newTC_, x, MIN);
  comparedList_:= for each x in candidateTCList_ join compareParamTime(newTC_, x, MIN);
  debugWrite("comparedList_ in makeMapAndUnion: ", comparedList_);
  ret_:= union(comparedList_, retTCList_);

  debugWrite("ret_ in makeMapAndUnion: ", ret_);
  return ret_;
end;

procedure compareParamTime(TC1_, TC2_, mode_)$
begin;
  scalar TC1Time_, TC1Cond_, TC2Time_, TC2Cond_,
         intersectionCondDNF_, TC1LessTC2CondDNF_, TC1GeqTC2CondDNF_,
         retTCList_;

  debugWrite("in compareParamTime", " ");
  debugWrite("TC1_: ", TC1_);
  debugWrite("TC2_: ", TC2_);
  debugWrite("mode_: ", mode_);

  TC1Time_:= getTimeFromTC(TC1_);
  TC1Cond_:= getCondDNFFromTC(TC1_);
  TC2Time_:= getTimeFromTC(TC2_);
  TC2Cond_:= getCondDNFFromTC(TC2_);


  % それぞれの条件部分について論理積を取り、falseなら空集合
  intersectionCondDNF_:= addCondDNFToCondDNF(TC1Cond_, TC2Cond_);
  debugWrite("intersectionCondDNF_: ", intersectionCondDNF_);
  if(isFalseDNF(intersectionCondDNF_)) then return {};

  if(TC1Time_ = infinity) then return {{TC2Time_, intersectionCondDNF_}};
  if(TC2Time_ = infinity) then return {{TC1Time_, intersectionCondDNF_}};

  % 条件の共通部分と時間に関する条件との論理積を取る
  % TC1Time_＜TC2Time_という条件
  debugWrite("========== make TC1LessTC2CondDNF_ ==========", " ");
  TC1LessTC2CondDNF_:= addCondTupleToCondDNF({TC1Time_, lessp, TC2Time_}, intersectionCondDNF_);
  debugWrite("TC1LessTC2CondDNF_: ", TC1LessTC2CondDNF_);
  % TC1Time_≧TC2Time_という条件
  debugWrite("========== make TC1GeqTC2CondDNF_ ==========", " ");
  TC1GeqTC2CondDNF_:= addCondTupleToCondDNF({TC1Time_, geq, TC2Time_}, intersectionCondDNF_);
  debugWrite("TC1GeqTC2CondDNF_: ", TC1GeqTC2CondDNF_);


  retTCList_:= {};
  % それぞれ、falseでなければretTCList_に追加
  if(not isFalseDNF(TC1LessTC2CondDNF_)) then 
    if(mode_=MIN) then retTCList_:= cons({TC1Time_, TC1LessTC2CondDNF_}, retTCList_)
    else if(mode_=MAX) then retTCList_:= cons({TC2Time_, TC1LessTC2CondDNF_}, retTCList_);
  if(not isFalseDNF(TC1GeqTC2CondDNF_)) then 
    if(mode_=MIN) then retTCList_:= cons({TC2Time_, TC1GeqTC2CondDNF_}, retTCList_)
    else if(mode_=MAX) then retTCList_:= cons({TC1Time_, TC1GeqTC2CondDNF_}, retTCList_);

  debugWrite("retTCList_ in compareParamTime: ", retTCList_);
  return retTCList_;
end;

% 未使用かつ文法エラーのためコメントアウト
%procedure solveParameterIneq(ineqList_)$
%begin;
%  scalar paramNameList_, paramName_, ret_;
%
%  paramNameList_:= collectParameters(ineqList_);
%  debugWrite("paramNameList_: ", paramNameList_);
%
%  % 2種類以上のパラメタが含まれていると扱えない
%  % TODO：なんとかする？
%  if(length(paramNameList_)>1) then return ERROR;
%  paramName_:= first(paramNameList_);
%
%  ret_:= exIneqSolve(ineqList_, paramName_);
%  return ret_;
%end;

defaultPrec_ := 0;

procedure getRealVal(value_, prec_)$
begin;
  scalar tmp_;
  putLineFeed();

  defaultPrec:= precision(0)$
  precision(prec_);
  tmp_:= value_;
  debugWrite("tmp_:", tmp_);
  precision(defaultPrec_)$
  write("<redeval> end:");
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

% 前提：Orでつながってはいない
% TODO：exDSolveを含めた実行をする
procedure convertCSToVMInterval()$
begin;
  scalar consTmpRet_, consRet_, paramDNFList_, paramRet_, tuple_, ret_;
  scalar tmpCons_, rconts_;
  scalar tmpSol_, splitExprsResult_, DExprs_, DExprVars_, 
         initVars_, prevVars_, noPrevVars_, noDifferentialVars_, tmpVarMap_,
         DExprRconts_, DExprRcontsVars_;

  putLineFeed();

  tmpCons_ := for each x in initConstraint__ join if(isInitVariable(lhs x)) then {} else {x};
  if(tmpCons_ neq {}) then return {SOLVER_ERROR___};
  rconts_ := for each x in initConstraint__ join if(isInitVariable(lhs x)) then {x} else {};
  
  splitExprsResult_ := splitExprs(removePrevCons(constraintStore__), csVariables__);
  DExprs_ := part(splitExprsResult_, 3);
  DExprVars_ := part(splitExprsResult_, 4);

  DExprRconts_:= removeInitCons(rconts_);
  if(DExprRconts_ neq {}) then <<
    prevVars_:= for each x in csVariables__ join if(isPrevVariable(x)) then {x} else {};
    noPrevVars_:= union(for each x in prevVars_ collect part(x, 1));
    DExprRcontsVars_ := union(for each x in noPrevVars_ join if(not freeof(DExprRconts_, x)) then {x} else {});
    DExprs_:= union(DExprs_, DExprRconts_);
    DExprVars_:= union(DExprVars_, DExprRcontsVars_);
  >>;

  initCons_:= union(for each x in (rconts_ \ DExprRconts_) collect exSub(constraintStore__, x));
  initVars_:= map(getInitVars, initCons_);
  noDifferentialVars_:= union(for each x in DExprVars_ collect if(isDifferentialVar(x)) then part(x, 1) else x);

  tmpSol_:= exDSolve(DExprs_, initCons_, noDifferentialVars_);
  debugWrite("tmpSol_ solved with exDSolve: ", tmpSol_);
 
  if(tmpSol_ = retsolvererror___) then return {SOLVER_ERROR___}
  else if(tmpSol_ = retoverconstraint___) then return {ICI_INCONSISTENT___};

  tmpVarMap_:= first(myFoldLeft(createIntegratedValue, {{},tmpSol_}, union(DExprVars_, (csVariables__ \ initVars_))));
  tmpSol_:= for each x in tmpVarMap_ collect (first(x)=second(x));
  consTmpRet_:= applyPrevCons(union(constraintStore__, tmpSol_), {});    
  debugWrite("consTmpRet_: ", consTmpRet_);

  % 式を{(変数名), (関係演算子コード), (値のフル文字列)}の形式に変換する
  consRet_:= map(makeConsTuple, consTmpRet_);
  paramDNFList_:= map(exIneqSolve, parameterStore__);
  paramRet_:= for each x in paramDNFList_ collect <<
    % 1つの項になっているはず
    tuple_:= first(first(x));
    {getVarNameFromTuple(tuple_), getExprCode(getRelopFromTuple(tuple_)), getValueFromTuple(tuple_)}
  >>;
  ret_:= union(consRet_, paramRet_);

  ret_:= getUsrVars(ret_, removePrevCons(union(DExprVars_, (csVariables__ \ initVars_))));

  debugWrite("ret_: ", ret_);
  return ret_;
end;

% vcs_math_sourceにおけるgetTimeVars
% convertCSToVMIntervalのretから記号定数を取り除く
procedure getUsrVars(ret_, vars_);
begin;
  % TODO
  return for each x in ret_ join if(contains(vars_, first x)) then {x} else {}; 
end;

% {df(usrvary,t,2),df(usrvary,t),usrvary}

procedure contains(vars_, var_);
begin;
  return if((for each x in vars_ sum if(x = var_) then 1 else 0) > 0) then t else nil;
end;

% 次のポイントフェーズに移行する時刻を求める
procedure  calculateNextPointPhaseTime(maxTime_, discCause_);
begin;
  scalar ans_;
  putLineFeed();

  ans_:=  calculateNextPointPhaseTimeMain(maxTime_, discCause_, constraintStore__, initConstraint__, parameterStore__, csVariables__);
  debugWrite("ans_ in calculateNextPointPhaseTime:", ans_);
  return ans_;
end;

% 次のポイントフェーズに移行する時刻を求める
% 戻り値の形式: {time_t, {}(parameter_map_t), true(bool)},...}
procedure calculateNextPointPhaseTimeMain(maxTime_, discCause_, cons_, initCons_, pCons_, vars_);
begin;
  scalar tmpSol_, splitExprsResult_, NDExprs_, NDExprVars_, DExprs_, DExprVars_, otherExprs_, paramCondDNF_,
         initCons_, initVars_, prevVars_, noPrevVars_, noDifferentialVars_,
         DExprRconts_, DExprRcontsVars_,
         tmpDiscCause_, retCode_, tmpVarMap_, tmpMinTList_, integAns_, tmpIneqSolDNF_;
  putLineFeed();
  debugWrite("in calculateNextPointPhaseTimeMain", " ");

  % TODO initCons_を使ってない？
  debugWrite("{maxTime_ ,discCause_, cons_, initCons_, pCons_, vars_}: ", {maxTime_ ,discCause_, cons_, initCons_, pCons_, vars_});
  debugWrite("{csVariables__, psParameters__}: ", {csVariables__, psParameters__});

  splitExprsResult_ := splitExprs(removePrevCons(cons_), vars_);
  NDExprs_ := part(splitExprsResult_, 1);
  NDExprVars_ := part(splitExprsResult_, 2);
  DExprs_ := part(splitExprsResult_, 3);
  DExprVars_ := part(splitExprsResult_, 4);
  otherExprs_:= union(part(splitExprsResult_, 5), pCons_);
  % DNF形式にする
  % 空集合なら、{{true}}として扱う（trueを表すDNF）
  if(otherExprs_={}) then paramCondDNF_:= {{true}}
  else <<
    % paramCondDNF_:= myFoldLeft((addCondDNFToCondDNF(#1, exIneqSolve(#2)))&, {{true}}, otherExprs_);を実現
    tmpIneqSolDNF_:= {{true}};
    for i:=1 : length(otherExprs_) do <<
      tmpIneqSolDNF_:= addCondDNFToCondDNF(tmpIneqSolDNF_, exIneqSolve(part(otherExprs_, i)));
    >>;
    paramCondDNF_:= tmpIneqSolDNF_;
  >>;
  debugWrite("paramCondDNF_: ", paramCondDNF_);

  % TODO DExprs_, NDExprs_の処理
  tmpDiscCause_:= union(sub(cons_, discCause_));
  debugWrite("tmpDiscCause_:", tmpDiscCause_);

  tmpMinTList_:= calcNextPointPhaseTime(maxTime_, tmpDiscCause_, paramCondDNF_);
  debugWrite("tmpMinTList_:", tmpMinTList_);
  if(tmpMinTList_ = {error}) then retCode_:= IC_SOLVER_ERROR___

  putLineFeed();
  return tmpMinTList_;
end;

%---------------------------------------------------------------
% シミュレーションに直接は関係ないが処理系の都合で必要な関数
%---------------------------------------------------------------

% デバッグ用メッセージ出力関数
% TODO:任意長の引数に対応したい
procedure debugWrite(arg1_, arg2_)$
  if(optUseDebugPrint__) then <<
    write(arg1_, arg2_);
  >> 
  else <<
    1$
  >>;

% 関数呼び出しはredevalを経由させる
% <redeval> end:の次が最終行
symbolic procedure redeval(foo_)$
begin scalar ans_;

  debugWrite("<redeval> reval :", (car foo_));
  ans_ :=(reval foo_);
  write("<redeval> end:");

  return ans_;
end;

procedure putLineFeed()$
begin;
  write("");
end;

%---------------------------------------------------------------
% 現時点では使用していないが有用そうな関数
%---------------------------------------------------------------

procedure bball_out()$
% gnuplot用出力, 未完成
% 正規表現 {|}|
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

%procedure myout(x,t)$

procedure getSExpFromString(str_)$
begin;
  scalar retSExp_;
  putLineFeed();

  retSExp_:= str_;
  debugWrite("retSExp_:", retSExp_);

  return retSExp_;
end;




%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%

% グローバル変数の初期化とputLineFeed
symbolic redeval '(resetConstraintStore);

;end;
