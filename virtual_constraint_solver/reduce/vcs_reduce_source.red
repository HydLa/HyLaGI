load_package sets;
load_package redlog; rlset R;
load_package "laplace";
load_package "ineq";
load_package "numeric";
%load_package "assist";


operator interval;


% �O���[�o���ϐ�
% constraintStore_: ���݈����Ă��鐧��W���i���X�g�`���APP�̒萔���Ή��j
% csVariables_: ����X�g�A���ɏo������ϐ��̈ꗗ�i���X�g�`���APP�̒萔���Ή��j
% parameterStore_: ���݈����Ă���A�萔����̏W���i���X�g�`���AIP�̂ݎg�p�j
% psParameters_: �萔����̏W���ɏo������萔�̈ꗗ�i���X�g�`���AIP�̂ݎg�p�j
%
% irrationalNumberIntervalList_: �������ƁA��Ԓl�ɂ��ߎ��\���̑g�̃��X�g
%
% optUseDebugPrint_: �f�o�b�O�o�͂����邩�ǂ���
% optUseApproximateCompare_: �ߎ��l��p�����召������s�����ǂ���
% approxPrecision_: checkOrderingFormula���ŁA�����𐔒l�ɋߎ�����ۂ̐��x
% intervalPrecision_: ��Ԓl�ւ̕ϊ���p�����召��r�ɂ�����A��Ԃ̐��x
%
%
% piInterval_: �~����Pi��\�����
% eInterval_: �l�C�s�A��E��\�����
%


% �O���[�o���ϐ�������
irrationalNumberIntervalList_:= {};
optUseApproximateCompare_:= nil;
approxPrecision_:= 30; % TODO:�v����
intervalPrecision_:= 2; % TODO:�v����
piInterval_:= interval(3141592/1000000, 3141593/1000000);
eInterval_:= interval(2718281/1000000, 2718282/1000000);


%---------------------------------------------------------------
% ��{�I�Ȑ�������֐�
%---------------------------------------------------------------

%Mathematica�ł���Head�֐�
procedure myHead(expr_)$
  if(arglength(expr_)=-1) then nil
  else part(expr_, 0);

%Mathematica�ł���Fold�֐�
procedure myFoldLeft(func_, init_, list_)$
  if(list_ = {}) then init_
  else myFoldLeft(func_, func_(init_, first(list_)), rest(list_));

procedure getArgsList(expr_)$
  if(arglength(expr_)=-1) then {}
  else for i:=1 : arglength(expr_) collect part(expr_, i);

%Mathematica�ł���Apply�֐�
procedure myApply(func_, expr_)$
  part(expr_, 0):= func_;

% ���̓������}�C�i�X���ǂ���
% ������expr�������̏ꍇ�͕��ł����Ă��}�C�i�X�������Ȃ�
% TODO�F�s�s��������΁A�Ή�����
procedure hasMinusHead(expr_)$
  if(arglength(expr_)=-1) then nil
  else if(part(expr_, 1) = -1*expr_) then t
  else nil;

% TODO:3�捪�ȏ�ւ̑Ή�
procedure rationalisation(expr_)$
begin;
  scalar head_, denominator_, numerator_, denominatorHead_, denomPlusArgsList_,
         frontTwoElemList_, restElemList_, timesRhs_, conjugate_, 
         rationalisedArgsList_, rationalisedExpr_, flag_;

%  debugWrite("in rationalisation", " ");
%  debugWrite("expr_: ", expr_);
  if(getArgsList(expr_)={}) then return expr_;

  % �z�肷��ΏہF����̍�����4�܂�
  % TODO:����ʓI�Ȍ`�ւ̑Ή���5���ȏ�H
  % TODO:3�捪�ȏ�ւ̑Ή�

  head_:= myHead(expr_);
%  debugWrite("head_: ", head_);

  if(head_=quotient) then <<
    numerator_:= part(expr_, 1);
    denominator_:= part(expr_, 2);
    % ����ɖ��������Ȃ���ΗL�����K�v�Ȃ�
    if(numberp(denominator_)) then return expr_;

    denominatorHead_:= myHead(denominator_);
%    debugWrite("denominatorHead_: ", denominatorHead_);
    if((denominatorHead_=plus) or (denominatorHead_=times)) then <<
      denomPlusArgsList_:= if(denominatorHead_=plus) then getArgsList(denominator_)
      else << 
        % denominatorHead_=times�̂Ƃ�
        if(myHead(part(denominator_, 2))=plus) then getArgsList(part(denominator_, 2))
        else {part(denominator_, 2)}
      >>;
%      debugWrite("denomPlusArgsList_: ", denomPlusArgsList_);

      % ������3�ȏ�̏ꍇ�A�m���ɖ�����������悤�ɍH�v���ċ��𐔂����߂�
      if(length(denomPlusArgsList_)>2) then <<
        frontTwoElemList_:= getFrontTwoElemList(denomPlusArgsList_);
%        debugWrite("frontTwoElemList_: ", frontTwoElemList_);
        restElemList_:= denomPlusArgsList_ \ frontTwoElemList_;
%        debugWrite("restElemList_: ", restElemList_);
        if(denominatorHead_=plus) then <<
          conjugate_:= plus(myApply(plus, frontTwoElemList_), -1*(myApply(plus, restElemList_)));
        >> else <<
          % �O��F�ς̉E�ӂ͂��ׂ�plus��(-5��+(-5)�̂悤��)�Ȃ����Ă���`��
          % TODO�F�����łȂ��ꍇ�ł����C�Ȃ悤�ɁH
          timesRhs_:= plus(myApply(plus, frontTwoElemList_), -1*(myApply(plus, restElemList_)));
          conjugate_:= part(denominator_, 1) * timesRhs_;
        >>;
      >> else if(length(denomPlusArgsList_)=2) then <<
        if(denominatorHead_=plus) then <<
          conjugate_:= plus(part(denomPlusArgsList_, 1), -1*part(denomPlusArgsList_, 2));
        >> else <<
          % denominatorHead_=times�̂Ƃ�
          timesRhs_:= plus(part(denomPlusArgsList_, 1), -1*part(denomPlusArgsList_, 2));
          conjugate_:= part(denominator_, 1) * timesRhs_;
        >>;
      >> else <<
        % denomPlusArgsList_�̒�����1�̂͂��A���̂Ƃ�denominatorHead_��times�ł���͂�
        conjugate_:= -1*first(denomPlusArgsList_);
      >>;
    >> else if(denominatorHead_=difference) then <<
      conjugate_:= difference(part(denominator_, 1), -1*part(denominator_, 2));
    >> else <<
      conjugate_:= -1*denominator_;
    >>;
%    debugWrite("conjugate_: ", conjugate_);
    % ���𐔂𕪕�q�ɂ�����
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

% expr_���ɓ����ȊO��_�����Z�q���܂܂��ꍇ�ɂ��Ή��ł���u���֐�
procedure exSub(patternList_, expr_)$
begin;
  scalar subAppliedExpr_, head_, subAppliedLeft_, subAppliedRight_, 
         argCount_, subAppliedExprList_, test_;

%  debugWrite("in exSub", " ");
%  debugWrite("patternList_: ", patternList_);
%  debugWrite("expr_: ", expr_);
  
  % expr_�������������Ȃ��ꍇ
  if(arglength(expr_)=-1) then <<
    subAppliedExpr_:= sub(patternList_, expr_);
    return subAppliedExpr_;
  >>;
  % patternList_����True���Ӗ����鐧�������
  patternList_:= removeTrueList(patternList_);
%  debugWrite("patternList_: ", patternList_);

  head_:= myHead(expr_);
%  debugWrite("head_: ", head_);

  % or�Ō����������̓��m�����ʂł�����Ȃ��ƁAneq�Ƃ����Ⴄ�����̂�����������\������
  if(isIneqRelop(head_)) then <<
    % �����ȊO�̊֌W���Z�q�̏ꍇ
    subAppliedLeft_:= exSub(patternList_, lhs(expr_));
%    debugWrite("subAppliedLeft_:", subAppliedLeft_);
    subAppliedRight_:= exSub(patternList_, rhs(expr_));
%    debugWrite("subAppliedRight_:", subAppliedRight_);
    subAppliedExpr_:= myApply(head_, {subAppliedLeft_, subAppliedRight_});
  >> else if(isLogicalOp(head_)) then <<
    % �_�����Z�q�̏ꍇ
    argCount_:= arglength(expr_);
%    debugWrite("argCount_: ", argCount_);
    subAppliedExprList_:= for i:=1 : argCount_ collect exSub(patternList_, part(expr_, i));
%    debugWrite("subAppliedExprList_:", subAppliedExprList_);
    subAppliedExpr_:= myApply(head_, subAppliedExprList_);

  >> else <<
    % ������A�ϐ����Ȃǂ�factor�̏ꍇ
    % TODO:expr_�����āA����X�g�A�i���邢��csvars�j���ɂ���悤�Ȃ�A����Ƒ΂��Ȃ��l�i�����̉E�Ӂj��K�p
    subAppliedExpr_:= sub(patternList_, expr_);
  >>;

%  debugWrite("subAppliedExpr_:", subAppliedExpr_);
  return subAppliedExpr_;
end;

% expr_���ɏo������Asqrt(var_)�Ɋւ��鍀�̌W���𓾂�
% ���������݂���ꍇ���l�����A���X�g�`���ŕԂ�
procedure getSqrtList(expr_, var_, mode_)$
begin;
  scalar head_, argsAnsList_, coefficientList_, exprList_, insideSqrt_;

%  debugWrite("in getSqrtList", " ");
%  debugWrite("expr_: ", expr_);
%  debugWrite("var_: ", var_);
%  debugWrite("mode_: ", mode_);


  % �ϐ���sqrt���܂܂�Ȃ���΍l����K�v�Ȃ�
  if(freeof(expr_, var_) or freeof(expr_, sqrt)) then return {};

  head_:= myHead(expr_);
%  debugWrite("head_: ", head_);
  if(hasMinusHead(expr_)) then <<
    % TODO�F�����̒���var_����������ꍇ�ւ̑Ή��H
    coefficientList_:= {-1*first(getSqrtList(part(expr_, 1), var_, mode_))};
    exprList_:= getSqrtList(part(expr_, 1), var_, mode_);
  >> else if(head_=plus) then <<
    % �������̏ꍇ
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
      % lcof�ŌW�������߂�
      coefficientList_:= {lcof(insideSqrt_, var_)};
    >> else <<
      % �����̒��������sqrt���܂ޑ������ɂȂ��Ă���ꍇ
      argsAnsList_:= for each x in getArgsList(insideSqrt_) join getSqrtList(x, var_, mode_);
%      debugWrite("argsAnsList_: ", argsAnsList_);
      % TODO�F��������ꍇ�ւ̑Ή��H
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
% �֌W���Z�q�֘A�̊֐�
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

  % relop�������Ƃ��Ē��ړn���ꂽ�ꍇ�ւ��Ή�
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
% �_�����Z�q�֘A�̊֐�
%---------------------------------------------------------------

procedure isLogicalOp(expr_)$
  if((expr_=or) or (expr_=and)) then t else nil;

procedure hasLogicalOp(expr_)$
  if(freeof(expr_, and) and freeof(expr_, or)) then nil else t;

%---------------------------------------------------------------
% �O�p�֐��֘A�̊֐�
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
% ��ԉ��Z�֘A�̊֐�
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

% TODO�F�����̗L���i�J���/��ԁj�ɂ���Č��ʂ��ς��ꍇ��������������悤��
% �i���̏ꍇ�Alb��ub���������Ȃ��Ă����Ԃ́A��Ԃƍl����Ǝ����\���j
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

% ���������܂ޒ萔������Ԓl�`���ɕϊ�����
% �O��F���͂�value_��2�ȏ�̐����Ɍ�����
procedure getSqrtInterval(value_, mode_)$
begin;
  scalar sqrtInterval_, sqrtLb_, sqrtUb_, midPoint_, loopCount_,
         tmpNewtonSol_, newTmpNewtonSol_, iIntervalList_;

  debugWrite("in getSqrtInterval", " ");
  debugWrite("value_: ", value_);
  debugWrite("mode_: ", mode_);

  debugWrite("irrationalNumberIntervalList_: ", irrationalNumberIntervalList_);
  
  iIntervalList_:= getIrrationalNumberInterval(sqrt(value_));
  debugWrite("iIntervalList_: ", iIntervalList_);
  if(iIntervalList_ neq {}) then return first(iIntervalList_);

  if(mode_=BINARY_SEARCH) then <<
    % �㉺���̒��_x�ɂ����āAx^2-value�̐����𒲂ׂ邱�ƂŒT���͈͂����߂Ă���
    sqrtLb_:= 0;
    sqrtUb_:= value_;
    loopCount_:= 0;
    while (sqrtUb_ - sqrtLb_ >= 1/10^intervalPrecision_) do <<
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
    % �j���[�g���@�ɂ�苁�߂�
    newTmpNewtonSol_:= value_;
    loopCount_:= 0;
    repeat <<
      tmpNewtonsol_:= newTmpNewtonSol_;
      newTmpNewtonSol_:= 1/2*(tmpNewtonSol_+value_ / tmpNewtonSol_);
      debugWrite("newTmpNewtonSol_: ", newTmpNewtonSol_);
      loopCount_:= loopCount_+1;
      debugWrite("loopCount_: ", loopCount_);
    >> until (tmpNewtonSol_ - newTmpNewtonSol_ < 1/10^intervalPrecision_);
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

  % �L�����Ȃ̂ŋ�Ԃɂ��邱�ƂȂ��召����\�����A�㉺������������ԁi�_��ԁj�Ƃ��Ĉ���
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
      % ���̐��̏ꍇ
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
      % �O��F�p���͐���
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
      retInterval_:= interval(getLbFromInterval(timesInterval(makePointInterval(-1), piInterval_)), getUbFromInterval(piInterval_));
    >> else if(head_=acos) then <<
      retInterval_:= interval(getLbFromInterval(timesInterval(makePointInterval(-1), piInterval_)), getUbFromInterval(piInterval_));
    >> else if(head_=atan) then <<
      retInterval_:= interval(getLbFromInterval(timesInterval(makePointInterval(-1), piInterval_)), getUbFromInterval(piInterval_));
    >> else <<
      % TODO�F���ɂǂ�ȏꍇ�ɖ����������ɂȂ邩�𒲂ׂ�
      retInterval_:= interval(hoge, hoge);
    >>;
  >> else <<
    if(value_=pi) then retInterval_:= piInterval_
    else if(value_=e) then retInterval_:= eInterval_
    else retInterval_:= interval(hoge, hoge);
  >>;
  debugWrite("retInterval_: ", retInterval_);
  debugWrite("(value_: )", value_);

  return retInterval_;
end;

%---------------------------------------------------------------
% �����ȊO�̐��֘A�̊֐�
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
    % ���̐��̏ꍇ
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
  irrationalNumberIntervalList_:= cons({value_, interval_}, irrationalNumberIntervalList_);

procedure getIrrationalNumberInterval(value_)$
  for each x in irrationalNumberIntervalList_ join 
    if(first(x)=value_) then {second(x)} else {};

%---------------------------------------------------------------
% �p�����^�֘A�̊֐�
%---------------------------------------------------------------

% �����Ƀp�����^���܂܂�Ă��邩�ǂ������ApsParameters_���̕ϐ����܂܂�邩�ǂ����Ŕ���
procedure hasParameter(expr_)$
  if(collectParameters(expr_) neq {}) then t else nil;

% ���\�����̃p�����^���A�W�߂�
procedure collectParameters(expr_)$
begin;
  scalar collectedParameters_;

%  debugWrite("in collectParameters", " ");
%  debugWrite("expr_: ", expr_);

%  debugWrite("psParameters_: ", psParameters_);
  collectedParameters_:= union({}, for each x in psParameters_ join if(not freeof(expr_, x)) then {x} else {});

%  debugWrite("collectedParameters_: ", collectedParameters_);
  return collectedParameters_;
end;

%---------------------------------------------------------------
% �召����֘A�̊֐��i�萔���̂݁A�p�����^�Ȃ��j
%---------------------------------------------------------------

checkOrderingFormulaCount_:= 0;
checkOrderingFormulaIrrationalNumberCount_:= 0;

procedure checkOrderingFormula(orderingFormula_)$
%����: �_����(����sqrt(2), greaterp_, sin(2)�Ȃǂ��܂ނ悤�Ȃ���), ���x
%�o��: t or nil or -1
%      (x��y���قړ������� -1)
%geq_= >=, geq; greaterp_= >, greaterp; leq_= <=, leq; lessp_= <, lessp;
begin;
  scalar head_, x, op, y, bak_precision, ans, margin, xInterval_, yInterval_;

  debugWrite("in checkOrderingFormula", " ");
  debugWrite("orderingFormula_: ", orderingFormula_);
  checkOrderingFormulaCount_:= checkOrderingFormulaCount_+1;
  debugWrite("checkOrderingFormulaCount_: ", checkOrderingFormulaCount_);

  head_:= myHead(orderingFormula_);
  % �召�Ɋւ���_�����ȊO�����͂��ꂽ��G���[
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
    % ���������܂܂��ꍇ
    if(optUseApproximateCompare_) then <<
      % �ߎ��l��p�����召��r
      bak_precision := precision 0;
      on rounded$ precision approxPrecision_$

      % x�����y���L�����ł��鎞
      % 10^(3 + y��x�̎w�����̒l - �L������)
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
      %x��y���قړ�������
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
      % ��Ԓl�ւ̕ϊ���p�����召��r
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
    % unknown���Ԃ����ꍇ�͐��x��ς��čĎ��s
    intervalPrecision_:= intervalPrecision_+4;
    ans:= checkOrderingFormula(orderingFormula_);
    intervalPrecision_:= intervalPrecision_-4;
  >>;
  return ans;
end;

procedure myInfinityIf(x, op, y)$
begin;
  scalar ans_, infinityTupleDNF_, retTuple_, i_, j_, andAns_;

  debugWrite("in myInfinityIf", " ");
  debugWrite("op(x, y): ", op(x, y));
  % INFINITY > -INFINITY�Ƃ��̑Ή�
  if(x=INFINITY or y=-INFINITY) then 
    if((op = geq) or (op = greaterp)) then ans_:=t else ans_:=nil
  else if(x=-INFINITY or y=INFINITY) then
    if((op = leq) or (op = lessp)) then ans_:=t else ans_:=nil
  else <<
    % �W�����ւ̑Ή��Ƃ��āA�܂�infinity relop value�̌`�ɂ��Ă�������Ȃ���
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
%����: ���l�Ƃ����O��
  if(checkOrderingFormula(x<y)) then x else y;

procedure mymax(x,y)$
%����: ���l�Ƃ����O��
  if(checkOrderingFormula(x>y)) then x else y;

procedure myFindMinimumValue(x,lst)$
%����: ���i�K�ł̍ŏ��lx, �ŏ��l�����������Ώۂ̃��X�g
%�o��: ���X�g���̍ŏ��l
  if(lst={}) then x
  else if(mymin(x, first(lst)) = x) then myFindMinimumValue(x,rest(lst))
  else myFindMinimumValue(first(lst),rest(lst));

procedure myFindMaximumValue(x,lst)$
%����: ���i�K�ł̍ő�lx, �ő�l�����������Ώۂ̃��X�g
%�o��: ���X�g���̍ő�l
  if(lst={}) then x
  else if(mymax(x, first(lst)) = x) then myFindMaximumValue(x,rest(lst))
  else myFindMaximumValue(first(lst),rest(lst));

%---------------------------------------------------------------
% TC�`���֘A�̊֐�
%---------------------------------------------------------------

% TC�`���i�����Ə���DNF�̑g�j�ɂ�����A�N�Z�X�p�֐�
procedure getTimeFromTC(TC_)$
  part(TC_, 1);
procedure getCondDNFFromTC(TC_)$
  part(TC_, 2);

%---------------------------------------------------------------
% �召����֘A�̊֐��i�p�����^���܂ށj
%---------------------------------------------------------------

procedure compareValueAndParameter(val_, paramExpr_, condDNF_)$
%����: ��r����Ώۂ̒l, �p�����[�^���܂ގ�, �p�����[�^�Ɋւ������
%�o��: {�u�l�v�����������Ȃ邽�߂̃p�����[�^�̏���, �u�p�����[�^���܂ގ��v���������Ȃ邽�߂̃p�����[�^�̏���}
% checkInfMinTime�̎d�l�ύX�ɂ��s�v��
begin;
  scalar valueLeqParamCondSol_, valueGreaterParamCondSol_, 
         ret_;

  debugWrite("in compareValueAndParameter", " ");
  debugWrite("val_: ", val_);
  debugWrite("paramExpr_: ", paramExpr_);
  debugWrite("condDNF_: ", condDNF_);


  % minValue_��param�̏ꍇ�̏�����
  valueLeqParamCondSol_:= addCondTupleToCondDNF({val_, leq, paramExpr_}, condDNF_);
  debugWrite("valueLeqParamCondSol_: ", valueLeqParamCondSol_);

  % minValue_��param�̏ꍇ�̏�����
  valueGreaterParamCondSol_:= addCondTupleToCondDNF({val_, greaterp, paramExpr_}, condDNF_);
  debugWrite("valueGreaterParamCondSol_: ", valueGreaterParamCondSol_);

  return {valueLeqParamCondSol_, valueGreaterParamCondSol_};
end;

%---------------------------------------------------------------
% �s�����^�v���֘A�̊֐�
%---------------------------------------------------------------

procedure isLbTuple(tuple_)$
  if((getRelopFromTuple(tuple_)=geq) or (getRelopFromTuple(tuple_)=greaterp)) then t else nil;

procedure isUbTuple(tuple_)$
  if((getRelopFromTuple(tuple_)=leq) or (getRelopFromTuple(tuple_)=lessp)) then t else nil;

procedure getLbTupleListFromConj(conj_)$
  for each x in conj_ join if(isLbTuple(x)) then {x} else {};

procedure getUbTupleListFromConj(conj_)$
  for each x in conj_ join if(isUbTuple(x)) then {x} else {};

% {����, relop, �E��}�̃^�v������A��������ĕԂ�
procedure makeExprFromTuple(tuple_)$
  myApply(getRelopFromTuple(tuple_), {getVarNameFromTuple(tuple_), getValueFromTuple(tuple_)});

procedure getVarNameFromTuple(tuple_)$
  part(tuple_, 1);

procedure getRelopFromTuple(tuple_)$
  part(tuple_, 2);

procedure getValueFromTuple(tuple_)$
  part(tuple_, 3);

%---------------------------------------------------------------
% �s�����^�v���ɂ��DNF�֘A�̊֐�
%---------------------------------------------------------------

% �_���ς�Not������DNF�ɂ���
procedure getNotConjDNF(conj_)$
  for each tuple in conj_ collect
    {{getVarNameFromTuple(tuple), getInverseRelop(getRelopFromTuple(tuple)), getValueFromTuple(tuple)}};

% Equal��\���_���ςɂȂ��Ă��邩�ǂ���
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

% �O��F��expr_��1�̊֌W���Z�q�݂̂�����
procedure makeTupleDNFFromEq(var_, value_)$
  {{ {var_, geq, value_}, {var_, leq, value_} }};

procedure makeTupleDNFFromNeq(var_, value_)$
  {{ {var_, lessp, value_}, {var_, greaterp, value_} }};

% DNF���́A�����������i�d�������j�_���ς��폜
procedure simplifyDNF(DNF_)$
begin;
  scalar tmpRetDNF_, simplifiedDNF_;

  % myFoldLeft((if(isSameConj(first(#1), #2)) then #1 else cons(#2, #1))&, {first(DNF_)}, rest(DNF_))������
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

  % False��ǉ����悤�Ƃ���ꍇ��False��Ԃ�
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

  % newCondDNF_���̂ǂ�conj��ǉ����Ă�FalseDNF�ɂȂ�悤�ȏꍇ�ɁA�ŏI�I��{}�������Ă���̂ŏC������
  if(addedCondDNF_={})then addedCondDNF_:= {{}};

  debugWrite("addedCondDNF_ in addCondDNFToCondDNF: ", addedCondDNF_);
  debugWrite("(newCondDNF_: )", newCondDNF_);
  debugWrite("(condDNF_: )", condDNF_);
  return addedCondDNF_;
end;

procedure addCondTupleToCondDNF(newCondTuple_, condDNF_)$
%���́F�ǉ�����i�p�����^�́j�����^�v��newCondTuple_, �i�p�����^�́j������\���_�����̃��X�gcondDNF_
%�o�́F�i�p�����^�́j������\���_������DNF
%���ӁF���X�g�̗v�f1��1�͘_���a�łȂ���A�v�f���͘_���ςłȂ����Ă��邱�Ƃ�\���B
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

  % true��ǉ����悤�Ƃ���ꍇ�A�ǉ����Ȃ��̂Ɠ���
  if(newCondTuple_=true) then return condConj_;
  % �p�����^������Ȃ��ꍇ�A�P�ɑ召������������ʂ��c��
  % �i�ڍ�����ƃp�����^������Ȃ��Ȃ�ꍇ�����l�j
  if(not hasParameter(makeExprFromTuple(newCondTuple_)) and freeof(makeExprFromTuple(newCondTuple_), t)) then 
    if(checkOrderingFormula(makeExprFromTuple(newCondTuple_))) then return condConj_
    else return {};
  % false�ɒǉ����悤�Ƃ���ꍇ
  if(isFalseConj(condConj_)) then return {};
  % true�ɒǉ����悤�Ƃ���ꍇ
  if(isTrueConj(condConj_)) then return {newCondTuple_};


  % �ꍇ�ɂ���ẮA�^�v����VarName������Value�����̗����ɕϐ����������Ă��邱�Ƃ�����
  % ���̂��߁A���Ƃ�1���ł����Ă���UexIneqSolve�ŉ����K�v������
  if((arglength(getVarNameFromTuple(newCondTuple_)) neq -1) or not numberp(getValueFromTuple(newCondTuple_))) then <<
    % VarName������1���̕ϐ����ŁA���AValue���������l�Ȃ炱�̏����͕s�v
    ineqSolDNF_:= exIneqSolve(makeExprFromTuple(newCondTuple_));
    debugWrite("ineqSolDNF_: ", ineqSolDNF_);
    debugWrite("(newCondTuple_: )", newCondTuple_);
    debugWrite("(condConj_: )", condConj_);
    if(isFalseDNF(ineqSolDNF_)) then <<
      % false��\���^�v����_���ςɒǉ����悤�Ƃ����ꍇ��false��\���_���ς�Ԃ�
      addedCondConj_:= {};
      debugWrite("addedCondConj_: ", addedCondConj_);
      return addedCondConj_;
    >> else if(isTrueDNF(ineqSolDNF_)) then <<
      % true��\���^�v����_���ςɒǉ����悤�Ƃ����ꍇ��condConj_��Ԃ�
      addedCondConj_:= condConj_;
      debugWrite("addedCondConj_: ", addedCondConj_);
      return addedCondConj_;
    >>;
    % DNF�`���ŕԂ�̂ŁA���߂ă^�v�������o��
    if(length(first(ineqSolDNF_))>1) then <<
      % �u=�v��\���`���̏ꍇ��geq��leq��2�̃^�v���̘_���ς��Ԃ�
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

  % �_���ς̒�����A�ǉ�����ϐ��Ɠ����ϐ��̍���T��
  varTerms_:= union(for each term in condConj_ join
    if(not freeof(term, varName_)) then {term} else {});
  debugWrite("varTerms_: ", varTerms_);


  % �����E����𓾂�
  ubTupleList_:= getUbTupleListFromConj(varTerms_);
  lbTupleList_:= getLbTupleListFromConj(varTerms_);
  % �����̂ݔ�r�ΏۂƂ���i�p�����^�Ƃ̔�r�͕ʂ̏����ōs���j
  ubTupleList_:= for each x in ubTupleList_ join if(not hasParameter(getValueFromTuple(x))) then {x} else {};
  lbTupleList_:= for each x in lbTupleList_ join if(not hasParameter(getValueFromTuple(x))) then {x} else {};
  debugWrite("ubTupleList_: ", ubTupleList_);
  debugWrite("lbTupleList_: ", lbTupleList_);
  if(ubTupleList_={}) then ubTupleList_:= {{varName_, leq, INFINITY}};
  if(lbTupleList_={}) then lbTupleList_:= {{varName_, geq, -INFINITY}};
  ubTuple_:= first(ubTupleList_);
  lbTuple_:= first(lbTupleList_);

  % �X�V���K�v���ǂ������A�ǉ�����s�����Ə㉺���Ƃ��r���邱�ƂŌ���
  ub_:= getValueFromTuple(ubTuple_);
  debugWrite("ub_: ", ub_);
  lb_:= getValueFromTuple(lbTuple_);
  debugWrite("lb_: ", lb_);
  if((relop_=leq) or (relop_=lessp)) then <<
    % ������𒲐�
    if(mymin(value_, ub_)=value_) then <<
      % �X�V��v���邩�ǂ����𔻒�B����������ꍇ���A�����̗L���ɂ���Ă͍X�V���K�v
      if((value_ neq ub_) or freeof(ubTupleList_, lessp)) then
        addedCondConj_:= cons(newCondTuple_, condConj_ \ ubTupleList_)
      else addedCondConj_:= condConj_;
      ub_:= value_;
      ubTuple_:= newCondTuple_;
    >> else <<
      addedCondConj_:= condConj_;
    >>;
  >> else <<
    % �������𒲐�
    if(mymin(value_, lb_)=lb_) then <<
      % �X�V��v���邩�ǂ����𔻒�B�����������ꍇ���A�����̗L���ɂ���Ă͍X�V���K�v
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

  % �Ō��lb��ub���m���߁A��������ꍇ��{}��Ԃ��ifalse��\���_���ρj
  % ub���f�t�H���g�iINFINITY�j�̂܂܂Ȃ�m�F�s�v
  if(ub_ neq INFINITY) then << 
    if(mymin(lb_, ub_)=lb_) then <<
      % lb=ub�̏ꍇ�A�֌W���Z�q��geq��leq�łȂ���΂Ȃ�Ȃ�
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
% �_�����֘A�̊֐�
%---------------------------------------------------------------

%�����̃��X�g��and�Ōq�����_�����ɕϊ�����
procedure mymkand(lst)$
for i:=1:length(lst) mkand part(lst,i);

procedure mymkor(lst)$
for i:=1:length(lst) mkor part(lst, i);

procedure myex(lst,var)$
rlqe ex(var, mymkand(lst));

procedure myall(lst,var)$
rlqe all(var, mymkand(lst));

%---------------------------------------------------------------
% �����E������������֘A�̊֐�
%---------------------------------------------------------------

% �L�������s������ŉ���Ԃ�
% TODO�F�������݂̂�Ԃ��悤�ɂ���
procedure exSolve(exprs_, vars_)$
begin;
  scalar tmpSol_, retSol_;

  % �O�p�֐��܂��̕��������������ꍇ�A����1�Ɍ��肵�Ă��܂�
  % TODO�F�ǂ��ɂ�����H
  off allbranch;
  tmpSol_:= solve(exprs_, vars_);
  on allbranch;

  % ������������
  tmpSol_:= for each x in tmpSol_ join
    if(myHead(x)=list) then <<
      {for each y in x join 
        if(hasImaginaryNum(rhs(y))) then {} else {y}
      }
    >> else <<
      if(hasImaginaryNum(rhs(x))) then {} else {x}
    >>;
  debugWrite("tmpSol_ after removing imaginary number: ", tmpSol_);

  % ������Œ�`�ł��Ȃ��l�iasin(5)�Ȃǁj������
  tmpSol_:= for each x in tmpSol_ join
    if(myHead(x)=list) then <<
      {for each y in x join 
        if(hasIndefinableNum(rhs(y))) then {} else {y}
      }
    >> else <<
      if(hasIndefinableNum(rhs(x))) then {} else {x}
    >>;
  debugWrite("tmpSol_ after removing indefinable value: ", tmpSol_);

  % �L����
  retSol_:= for each x in tmpSol_ collect
    if(myHead(x)=list) then map(rationalisation, x)
    else rationalisation(x);
  debugWrite("retSol_ in exSolve: ", retSol_);
  return retSol_;
end;

% �s�������A���ӂɐ��̕ϐ����݂̂�����`���ɐ��`���A�����\���^�v�������
% (�s�����̏ꍇ�A�K���E�ӂ�0�ɂȂ��Ă��܂��i�����I�Ɉڍ����Ă��܂��j���Ƃ�A
% �s�����̌��������肳��Ă��܂����肷�邱�Ƃ����邽��)
% �O��F���͂����s�������ϐ���1��
% TODO�F�����̕s�������n���ꂽ�ꍇ�ւ̑Ή��H
% 2���s�����܂ł͉�����
% TODO�F3���ȏ�ւ̑Ή��H
% ���́F1�ϐ���2���ȉ��̕s����1��
% �o�́F{����, relop, �E��}�̌`���ɂ��DNF���X�g
% �����̏ꍇ�ɂ��Ή��Bgeq��leq���ꏏ�Ɏg�����Ƃɂ��\������
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

  % �����̕ϐ��������Ă���ꍇ�ւ̑Ή��Ft��usrVar��parameter��INFINITY�̏��Ɍ���
  exprVarList_:= union(for each x in union(csVariables_, {t}) join
    if(not freeof(ineqExpr_, x)) then {x} else {});
  if(exprVarList_={}) then <<
    exprVarList_:= union(for each x in psparameters_ join
      if(not freeof(ineqExpr_, x)) then {x} else {});
  exprVarList_:= if(exprVarList_ neq {}) then exprVarList_ else {INFINITY}
  >>;
  debugWrite("exprVarList_: ", exprVarList_);


  exprVar_:= first(exprVarList_);
  debugWrite("exprVar_: ", exprVar_);

  % �����̏ꍇ�̏���
  if(relop_=equal) then <<
    eqSol_:= exSolve(ineqExpr_, exprVar_);
    eqSolValue_:= rhs(first(eqSol_));          % 2���ȏ�͍l���Ȃ��ėǂ��̂��H
    debugWrite("eqSolValue_: ", eqSolValue_);
    return hogegege;
%    return makeTupleDNFFromEq(exprVar_, eqSolValue_);
  >>;

  % �E�ӂ����ӂɈڍ�����
  adjustedIneqExpr_:= myApply(relop_, {lhs_+(-1)*rhs_, 0});
  adjustedLhs_:= lhs(adjustedIneqExpr_);
  debugWrite("adjustedIneqExpr_: ", adjustedIneqExpr_);
  debugWrite("adjustedLhs_: ", adjustedLhs_);
  adjustedRelop_:= myHead(adjustedIneqExpr_);
  debugWrite("adjustedRelop_: ", adjustedRelop_);

  % �O�p�֐����܂ޏꍇ�A���ʂȕ������肪�K�v
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
      % unknown���Ԃ���
      retTupleDNF_:= {{unknown}};
      debugWrite("retTupleDNF_: ", retTupleDNF_);
      debugWrite("(ineqExpr_: )", ineqExpr_);
      return retTupleDNF_;
    >>;
  >>;

  % ���̎��_��x+value relop 0�܂���-x+value relop 0�̌`���i1�����j�ɂȂ��Ă���̂ŁA
  % -x+value��0�̏ꍇ��x-value��0�̌`�ɂ���K�v������irelop�Ƃ��ċt�̂��̂𓾂�K�v������j
  % �T�[�o�[���[�h���Ɓ����g�������\���͕ێ��ł��Ȃ��悤�Ȃ̂ŁAreverse���邩�ǂ�������������Ηǂ�
  % minus�ŏ���������s�����Ƃ��Ȃ����ł��Ȃ��̂ŁA���ɏo������ϐ���x�𒲂ׂāA���̌W���̐����𒲂ׂ�
  lcofRet_:= lcof(adjustedLhs_, exprVar_);
  if(not numberp(lcofRet_)) then lcofRet_:= 0;
  sqrtCoeffList_:= getSqrtList(adjustedLhs_, exprVar_, COEFF);
  % �O��F�����̒��Ƀp�����^�����鍀�͑����Ă�1��
  % TODO�F�Ȃ�Ƃ�����
  if(sqrtCoeffList_={}) then sqrtCoeff_:= 0
  else if(length(sqrtCoeffList_)=1) then sqrtCoeff_:= first(sqrtCoeffList_);
  % TODO�F�����ɂ�xor���g���ׂ����H
  if(checkOrderingFormula(lcofRet_<0) or checkOrderingFormula(sqrtCoeff_<0)) then <<
    adjustedRelop_:= getReverseRelop(adjustedRelop_);
  >>;
  debugWrite("adjustedRelop_: ", adjustedRelop_);

  % �ϐ���sqrt�����Ă�ꍇ�́A�ϐ���0�ȏ�ł���Ƃ����������Ō�ɒǉ�����K�v������
  % TODO: sqrt(x-1)�Ƃ��ւ̑Ή�
  insideSqrtExprs_:= getSqrtList(adjustedLhs_, exprVar_, INSIDE);
  debugWrite("insideSqrtExprs_: ", insideSqrtExprs_);
  sqrtCondTupleList_:= for each x in insideSqrtExprs_ collect
    first(first(exIneqSolve(myApply(geq, {x, 0}))));
  debugWrite("sqrtCondTupleList_: ", sqrtCondTupleList_);
  if(sqrtCondTupleList_={}) then sqrtCondTupleList_:= {true};

  
  % ub�܂���lb�����߂�
  off arbvars;
  on multiplicities;
  sol_:= exSolve(equal(adjustedLhs_, 0), exprVar_);
  debugWrite("sol_: ", sol_);
  on arbvars;
  off multiplicities;

  % TODO�F�����ϐ��ւ̑Ή��H
  % �����ϐ��������2�d���X�g�ɂȂ�͂������A1�ϐ��Ȃ�s�v���H
%  if(length(sol_)>1 and myHead(first(sol_))=list) then <<
%    % or�łȂ���������and�������Ă���֌W��\���Ă���
%      
%    adjustedEqExpr_:= first(first(sol_))
%
%  >> 

  if(sol_={}) then <<
    % adjustedLhs_=0�ɑ΂�����������Ȃ��ꍇ
    % adjustedIneqExpr_��-t^2-1<0��t^2+5<0�Ȃǂ̂Ƃ�
    if((adjustedRelop_=geq) or (adjustedRelop_=greaterp) or (adjustedRelop_=neq)) then retTupleDNF_:= {{true}}
    else retTupleDNF_:= {{}};
  >> else if(length(sol_)>1) then <<
    % 2�����������������ꍇ�B�����ɂ͒���2�̂͂�
    % TODO�F3���ȏ�ւ̈�ʉ��H

    boundList_:= {rhs(part(sol_, 1)), rhs(part(sol_, 2))};
    debugWrite("boundList_: ", boundList_);
    if(not hasParameter(boundList_)) then <<
      if(length(union(boundList_))=1) then <<
        % �d����\���ꍇ
        lb_:= first(union(boundList_));
        ub_:= first(union(boundList_));
      >> else <<
        lb_:= myFindMinimumValue(INFINITY, boundList_);
        ub_:= first(boundList_ \ {lb_});
      >>;
      debugWrite("lb_ in exIneqSolve: ", lb_);
      debugWrite("ub_ in exIneqSolve: ", ub_);
    
      % relop�ɂ���āAtupleDNF�̍\����ς���
      if((adjustedRelop_ = geq) or (adjustedRelop_ = greaterp)) then <<
        % ������с��̏ꍇ��or�̊֌W�ɂȂ�
        retTupleDNF_:= {{ {exprVar_, getReverseRelop(adjustedRelop_), lb_} }, { {exprVar_, adjustedRelop_, ub_} }};
      >> else if((adjustedRelop_ = geq) or (adjustedRelop_ = greaterp)) then <<
        % ������с��̏ꍇ��and�̊֌W�ɂȂ�
        retTupleDNF_:= {{ {exprVar_, getReverseRelop(adjustedRelop_), lb_},     {exprVar_, adjustedRelop_, ub_} }};
      >> else <<
        % ���̏ꍇ
        retTupleDNF_:= {{ {exprVar_, lessp, lb_}, {exprVar_, greaterp, lb_}, {exprVar_, lessp, ub_}, {exprVar_, greaterp, ub_} }};
      >>;
    >> else <<
      % �p�����^�̉�������ꂽ�ꍇ
      % �ǂ��炪�����łǂ��炪����ɂȂ邩���ǂ̂悤�ɒ��ׂ邩�H�H
      % ���ׂ�񂶂�Ȃ��Ă�����������ǉ�����Ηǂ�      


      retTupleDNF_:= hogehogegege; % ��
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

  % �����������Ȃ��ꍇ�itrue�Ȃǁj
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
    % ��������sqrt�������Ă��鎞�̂݁A�����ȑ召��r���L���ƂȂ�
    % TODO:���Y�̐������ɕϐ����������ۂɂ��������������ł���悤�ɂ���
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
% ���v���X�ϊ��֘A�̊֐�
%---------------------------------------------------------------

% �t���v���X�ϊ���̒l��sin, cos�ŕ\������X�C�b�`
on ltrig;

% {{v, v(t), lapv(s)},...}�̑Ή��\�A�O���[�o���ϐ�
% exDSolve��exceptdfvars_�ɑΉ������邽�߁AexDSolve�Ō�ŋ�W�����������������Ă���
table_:={};

% operator�錾���ꂽargs_���L������O���[�o���ϐ�
loadedOperator_:={};

% ��������init��_��lhs���쐬
procedure makeInitId(f,i)$
if(i=0) then
  mkid(mkid(INIT,f),lhs)
else
  mkid(mkid(mkid(mkid(INIT,f),_),i),lhs);

%laprule_�p�Amkid�����������Z�q�Ƃ��ĕԂ�
procedure setMkidOperator(f,x)$
  f(x);

%laprule_�p�̎��R���Z�q
operator !~f$

% �����Ɋւ���ϊ��K��laprule_, let�͈�x��
let {
  laplace(df(~f(~x),x),x) => il!&*laplace(f(x),x) - makeInitId(f,0),
  laplace(df(~f(~x),x,~n),x) => il!&**n*laplace(f(x),x) -
    for i:=n-1 step -1 until 0 sum
      makeInitId(f,n-1-i) * il!&**i,
  laplace(~f(~x),x) => setMkidOperator(mkid(lap,f),il!&)
}$

% ���v���X�ϊ��΂̍쐬, �I�y���[�^�錾
% {{v, v(t), lapv(s)},...}�̑Ή��\table_�̍쐬
procedure LaplaceLetUnit(args_)$
begin;
  scalar arg_, LAParg_;

  arg_:= first args_;
  LAParg_:= second args_;

  % arg_���d�����ĂȂ�������
  if(freeof(loadedOperator_,arg_)) then 
    << 
     operator arg_, LAParg_;
     loadedOperator_:= arg_ . loadedOperator_;
    >>;

  % {{v, v(t), lapv(s)},...}�̑Ή��\
  table_:= {arg_, arg_(t), LAParg_(s)} . table_;
  debugWrite("table_: ", table_);
end;

% vars_����df�����������̂�Ԃ�
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
%����: �ϐ���, �����̃��X�g�̃��X�g(ex. {{x=1,y=2},{x=3,y=4},...})
%�o��: �ϐ��ɑΉ�����l�̃��X�g
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
  % ���v���X�ϊ��K���̍쐬
  map(LaplaceLetUnit, tmp_);

  %ht => ht(t)�u��
  tmp_:=map(first(~w)=second(~w), table_);
  debugWrite("MAP: ", tmp_);

  tmp_:= sub(tmp_, expr_);
  debugWrite("SUB: ", tmp_);

  % expr_�𓙎����獷���`����
  diffexpr_:={};
  for each x in tmp_ do 
    if(not freeof(x, equal))
      then diffexpr_:= append(diffexpr_, {lhs(x)-rhs(x)})
    else diffexpr_:= append(diffexpr_, {lhs(x)-rhs(x)});
      
  % laplace���Z�q�ŃG���[���Alaplace���Z�q���݂̎����Ԃ�Ƒz��
  if(not freeof(LAPexpr_, laplace)) then return retsolvererror___;

  % ���v���X�ϊ�
  LAPexpr_:=map(laplace(~w,t,s), diffexpr_);
  debugWrite("LAPexpr_: ", LAPexpr_);

  % init_�����LAPexpr_�ɓK��
  solveexpr_:= append(LAPexpr_, init_);
  debugWrite("solveexpr_:", solveexpr_);

  % �t���v���X�ϊ��̑Ώ�
  solvevars_:= append(append(map(third, table_), map(lhs, init_)), {s});
  debugWrite("solvevars_:", solvevars_);

  % �ϊ��΂Ə���������A�����ĉ���
  solveans_ := solve(solveexpr_, solvevars_);
  debugWrite("solveans_: ", solveans_);

  % solve���𖳂��̎� overconstraint�Ƒz��
  if(solveans_={}) then return retoverconstraint___;
  % s��arbcomplex�łȂ��l������ overconstraint�Ƒz��
  if(freeof(lgetf(s, solveans_), arbcomplex)) then  return retoverconstraint___;

  debugWrite("table_: ", table_);
  % solveans_��solvevars_�̉�����ł��܂܂�Ȃ��� underconstraint�Ƒz��
  for each x in table_ do 
    if(freeof(solveans_, third(x))) then tmp_:=true;
  debugWrite("is under-constraint?: ", tmp_);
  if(tmp_=true) then return retunderconstraint___;

  debugWrite("table_: ", table_);
  
  % solveans_�̋t���v���X�ϊ�
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

%---------------------------------------------------------------
% ����̗v�f�𒊏o/�폜�����蕡���̗v�f�𕪗ނ����肷��֐�
%---------------------------------------------------------------

procedure getFrontTwoElemList(lst_)$
  if(length(lst_)<2) then {}
  else for i:=1 : 2 collect part(lst_, i);

procedure removeTrueList(patternList_)$
  for each x in patternList_ join if(rlqe(x)=true) then {} else {x};

% �_������true�ł���Ƃ��A���݂͂��̂܂�true��Ԃ��Ă���
% TODO�F�Ȃ�Ƃ�����
procedure removeTrueFormula(formula_)$
  if(formula_=true) then true
  else myApply(and, for each x in getArgsList(formula_) join 
    if(rlqe(x)=true) then {} else {x});

% ���񃊃X�g����A�����ȊO���܂ސ���𒊏o����
procedure getOtherExpr(exprs_)$
  for each x in exprs_ join if(hasIneqRelop(x) or hasLogicalOp(x)) then {x} else {};

% NDExpr�iexDSolve�ň����Ȃ��悤�Ȑ��񎮁j�ł��邩�ǂ����𒲂ׂ�
% ���̒���sin��cos�������Ă��Ȃ����false
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

% �O��F���͂�initxlhs=prev(x)�̌`
% TODO�F�Ȃ�Ƃ�����
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

    % timeExpr_�� (t - value) op 0 �܂��� (-t - value) op 0 �̌`��z��
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
% �҂��s��I�֘A�̊֐�
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
% HydLa�����֐��idf�ϐ��֘A�j
%---------------------------------------------------------------

procedure removeDifferentialVars(vars_)$
  union(for each x in vars_ join if(freeof(x, df)) then {x} else {});

procedure isDifferentialVar(var_)$
  if(arglength(var_)=-1) then nil
  else if(myHead(var_)=df) then t
  else nil;

%---------------------------------------------------------------
% HydLa�����֐��iprev�ϐ��֘A�j
%---------------------------------------------------------------

procedure isPrevVariable(expr_)$
  if(arglength(expr_)=-1) then nil
  else if(myHead(expr_)=prev) then t
  else nil;

procedure removePrev(var_)$
  if(myHead(var_)=prev) then part(var_, 1) else var_;

procedure removePrevVars(varsList_)$
  union(for each x in varsList_ join if(freeof(x, prev)) then {x} else {});

procedure removePrevCons(consList_)$
  union(for each x in consList_ join if(freeof(x, prev)) then {x} else {});

%---------------------------------------------------------------
% HydLa�����֐��i���ʂ��ĕK�v�j
%---------------------------------------------------------------

operator prev;

%rettrue___ := "RETTRUE___";
%retfalse___ := "RETFALSE___";
rettrue___ := 1;
retfalse___ := 2;


% ����X�g�A�̃��Z�b�g
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

% PP/IP�ŋ��ʂ�reset���ɍs���A����X�g�A�ւ̐���̒ǉ�
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

% ����X�g�A�ւ̐���̒ǉ�
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

procedure getConstraintStore()$
begin;
  putLineFeed();

  debugWrite("constraintStore_:", constraintStore_);
  if(constraintStore_={}) then return {{}, parameterStore_};

  % ����1��������
  % TODO: Or�łȂ������������ւ̑Ή�
  if(myHead(first(constraintStore_))=list) then return {first(constraintStore_), parameterStore_}
  else return {constraintStore_, parameterStore_};
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

  % TODO:simplify�֐����g��
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
% HydLa�����֐��iPP�ɂ����ĕK�v�j
%---------------------------------------------------------------

% (���� and���󂯕t���Ȃ�) TODO �����ւ̑Ή�
% (���� true���󂯕t���Ȃ�) TODO �����ւ̑Ή�
procedure checkConsistencyWithTmpCons(expr_, lcont_, vars_)$
begin;
  scalar ans_;
  putLineFeed();

  ans_:= {part(checkConsistencyBySolveOrRlqe(expr_, lcont_, vars_), 1)};
  debugWrite("ans_ in checkConsistencyWithTmpCons: ", ans_);

  return ans_;
end;

% PP�ɂ����閳�������̔���
% �Ԃ�l��{ans, {{�ϐ��� = �l},...}} �̌`��
% �d�l QE���g�p % (�g�p����Ȃ�, �ϐ��͊�{����I�ɒu������)

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
%  debugWrite("union(eqExprs_, lcont_):",  union(eqExprs_, lcont_));
%  debugWrite("union(csVariables_, vars_):", union(csVariables_, vars_));

  % ���m�ϐ���ǉ����Ȃ��悤�ɂ���
  off arbvars;
  tmpSol_:= exSolve(union(eqExprs_, lcont_),  union(csVariables_, vars_));
  on arbvars;
  debugWrite("tmpSol_: ", tmpSol_);

  if(tmpSol_={}) then return {retfalse___};
  % 2�d���X�g�̎��̂�first�œ���
  % TODO:�����𓾂�ꂽ�ꍇ�ւ̑Ή�
  if(myHead(first(tmpSol_))=list) then tmpSol_:= first(tmpSol_);


  % exprs_����ѐ���X�g�A�ɓ����ȊO�������Ă��邩�ǂ����ɂ��A�����̂Ɏg�p����֐�������
  mode_:= if(otherExprs_={}) then SOLVE else RLQE;
  debugWrite("mode_:", mode_);

  if(mode_=SOLVE) then
  <<
    ans_:=tmpSol_;
    debugWrite("ans_ in checkConsistencyBySolveOrRlqe: ", ans_);
    if(ans_ <> {}) then return {rettrue___, ans_} else return {retfalse___};
  >> else
  <<
    % sub�̊g���ł�p�����@
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
  % ret_code��rettrue___�A�܂�1�ł��邩�ǂ������`�F�b�N
  if(part(sol_, 1) = 1) then constraintStore_:= part(sol_, 2);
  debugWrite("constraintStore_: ", constraintStore_);

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

% convertCSToVM���Ŏg���A���`�p�֐�
procedure makeConsTuple(cons_)$
begin;
  scalar varName_, relopCode_, value_, tupleDNF_, retTuple_, adjustedCons_, sol_;

  debugWrite("in makeConsTuple", " ");
  debugWrite("cons_: ", cons_);
  
  % ���ӂɕϐ����݂̂�����`���ɂ���
  % �O��F�����͂��łɂ��̌`���ɂȂ��Ă���
  if(not hasIneqRelop(cons_)) then <<
    varName_:= lhs(cons_);
    relopCode_:= getExprCode(cons_);
    value_:= rhs(cons_);
  >> else <<
    tupleDNF_:= exIneqSolve(cons_);
    debugWrite("tupleDNF_: ", tupleDNF_);
    % 1�����ɂȂ��Ă�͂��Ȃ̂ŁA����1�Ȃ͂�
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

% �O��FOr�łȂ����Ă͂��Ȃ�
% TODO�F�Ȃ�Ƃ�����
procedure convertCSToVM()$
begin;
  scalar consTmpRet_, consRet_, paramDNFList_, paramRet_, tuple_, ret_;
  putLineFeed();

  debugWrite("constraintStore_:", constraintStore_);

  consTmpRet_:= applyPrevCons(constraintStore_, {});    
  debugWrite("consTmpRet_: ", consTmpRet_);

  % ����{(�ϐ���), (�֌W���Z�q�R�[�h), (�l�̃t��������)}�̌`���ɕϊ�����
  consRet_:= map(makeConsTuple, consTmpRet_);
  paramDNFList_:= map(exIneqSolve, parameterStore_);
  paramRet_:= for each x in paramDNFList_ collect <<
    % 1�̍��ɂȂ��Ă���͂�
    tuple_:= first(first(x));
    {getVarNameFromTuple(tuple_), getExprCode(getRelopFromTuple(tuple_)), getValueFromTuple(tuple_)}
  >>;
  ret_:= union(consRet_, paramRet_);

  debugWrite("ret_: ", ret_);
  return ret_;
end;

%---------------------------------------------------------------
% HydLa�����֐��iIP�ɂ����ĕK�v�j
%---------------------------------------------------------------

% 20110705 overconstraint___����
ICI_SOLVER_ERROR___:= 0;
ICI_CONSISTENT___:= 1;
ICI_INCONSISTENT___:= 2;
ICI_UNKNOWN___:= 3; % �s�v�H

procedure checkConsistencyInterval(tmpCons_, rconts_, vars_)$
begin;
  scalar tmpSol_, splitExprsResult_, NDExprs_, NDExprVars_, DExprs_, DExprVars_, otherExprs_,
         initCons_, initVars_, prevVars_, noPrevVars_, noDifferentialVars_, tmpVarMap_,
         DExprRconts_, DExprRcontsVars_,
         integTmp_, integTmpQE_, integTmpQEList_, integTmpEqualList_, integTmpIneqSolDNFList_, integTmpIneqSolDNF_, ans_;
  putLineFeed();

%  debugWrite("constraintStore_: ", constraintStore_);
%  debugWrite("csVariables_: ", csVariables_);
  debugWrite("tmpCons_: ", tmpCons_);
  debugWrite("rconts_: ", rconts_);
  debugWrite("vars_: ", vars_);

  % Sin��Cos���܂܂��ꍇ�̓��v���X�ϊ��s�\�Ȃ̂�NDExpr��������
  % TODO:�Ȃ�Ƃ��������Ƃ���H
  splitExprsResult_ := splitExprs(removePrevCons(constraintStore_), csVariables_);
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

  DExprRconts_:= removePrevCons(rconts_);
  debugWrite("DExprRconts_: ", DExprRconts_);
  if(DExprRconts_ neq {}) then <<
    prevVars_:= for each x in csVariables_ join if(isPrevVariable(x)) then {x} else {};
    debugWrite("prevVars_: ", prevVars_);
    noPrevVars_:= union(for each x in prevVars_ collect part(x, 1));
    debugWrite("noPrevVars_: ", noPrevVars_);
    DExprRcontsVars_ := union(for each x in noPrevVars_ join if(not freeof(DExprRconts_, x)) then {x} else {});
    debugWrite("DExprRcontsVars_: ", DExprRcontsVars_);
    DExprs_:= union(DExprs_, DExprRconts_);
    DExprVars_:= union(DExprVars_, DExprRcontsVars_);
  >>;

  initCons_:= union(for each x in (rconts_ \ DExprRconts_) collect exSub(constraintStore_, x));
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


  % NDExpr_��A��
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


  % tmpCons_���Ȃ��ꍇ�͖������Ɣ��肵�ėǂ�
  if(tmpCons_ = {}) then return {ICI_CONSISTENT___};

  integTmp_:= sub(tmpSol_, tmpCons_);
  debugWrite("integTmp_: ", integTmp_);

  % �O��FParseTree�\�z���ɕ�������Ă���͂��Ȃ̂ŃK�[�h��or�����邱�Ƃ͍l���Ȃ�
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

  % �K�[�h�����S�̂�true�̏ꍇ
  if(integTmpList_={}) then return {ICI_CONSISTENT___};
  % �K�[�h��������false������ꍇ
  if(not freeof(integTmpList_, false)) then return {ICI_INCONSISTENT___};

  integTmpEqualList_:= for each x in integTmpList_ join 
    if(myHead(x)=equal) then {x} 
    else {};
  debugWrite("integTmpEqualList_: ", integTmpEqualList_);
  % �K�[�h��������ɂ����Ă�and�łȂ���������������ꍇ�A���ʂ�false
  if(integTmpEqualList_ neq {}) then return {ICI_INCONSISTENT___};


  % ���ꂼ��̕s�����ɂ��āA1���ɂ���DNF�ɂ��AintegTmpIneqSolDNFList_�Ƃ���B
  % ���ӁF���ꂼ��̗v�f�Ԃ�and�̊֌W�ł���
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

  % �s�����̏ꍇ�A�����ŏ��߂Ė�����������AintegTmpIneqSolDNF_��false�ɂȂ邱�Ƃ�����
  if(isFalseDNF(integTmpIneqSolDNF_)) then return {ICI_INCONSISTENT___}
  % true�ɂȂ����疳����
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

%depend {ht,v}, t;
%expr_:={df(ht,t) = v,
%        df(v,t) = -10
%       };
%init_:={inithtlhs = 10,
%        initvlhs = 0
%       };
%vars_:={ht,v,df(ht,t),df(v,t)};
%symbolic redeval '(checkConsistencyInterval expr_ init_ vars_);

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
    % Equal��\���_���ς̏ꍇ
    infCheckAns_:= false;
  >> else <<
    lbTupleList_:= getLbTupleListFromConj(conj_);
    % �O��F������1��
    % TODO�F�p�����^�ɂ�鉺��������ꍇ�ւ̑Ή�
    lbTuple_:= first(lbTupleList_);
    if((getRelopFromTuple(lbTuple_)=greaterp) and (getValueFromTuple(lbTuple_)=0)) then 
      infCheckAns_:= true
    else infCheckAns_:= false;
  >>;
  debugWrite("infCheckAns_: ", infCheckAns_);
  debugWrite("(tDNF_: )", tDNF_);

  return infCheckAns_;
end;

% �o�́F������\��DNF�Ə����̑g�iTC�j�̃��X�g
% TODO�FERROR����
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
    % Equal��\���_���ς̏ꍇ
    if(not hasParameter(conj_)) then minTCList_:= {{getValueFromTuple(first(conj_)), condDNF_}}
    else <<
      % �p�����^�̏ꍇ�A���̒l��0�ȉ��Ȃ�Ό��ʂ�INFINITY�ɂȂ�
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
      % �p�����^�̓���������������ꍇ
      % �O��FcondDNF_���̒萔�̎�ނ�1�܂ŁH
      % TODO�F�Ȃ�Ƃ�����
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
      % �p�����^���܂މ�����lbValue_���傫�����ǂ����𒲂ׂ�
      % TODO�F�p�����^���܂މ������m�̑召����
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
      % �p�����^���܂ޏ���͉����ȏ�łȂ��Ă͂Ȃ�Ȃ�
      debugWrite("ubParamTupleList_: ", ubParamTupleList_);
      if(minTCList_ neq {}) then <<
        for each x in ubParamTupleList_ do <<
          minTCList_:= union(for each y in minTCList_ join <<
            if(getTimeFromTC(y) neq INFINITY) then <<
              checkDNF_:= addCondTupleToCondDNF({getValueFromTuple(x), geq, getTimeFromTC(y)}, getCondDNFFromTC(y));
              debugWrite("checkDNF_: ", checkDNF_);
              if(not isFalseDNF(checkDNF_)) then {y} else {}
            >> else <<
              % ������INFINITY�̂Ƃ��i�������͈̔͂̃p�����^�ɂ���ė��U�ω����N���Ȃ��p�^�[���j�͏㉺���m�F�s�v
              % TODO�F�{���H
              {y}
            >>
          >>);
        >>;
        if(minTCList_={}) then minTCList_:= {{INFINITY, condDNF_}};
      >> else <<
        % �p�����^�̉������Ȃ������ilbParamTupleList_����W���j�Ƃ���lbValue_�Ƃ�����r
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

  % Sin��Cos���܂܂��ꍇ�̓��v���X�ϊ��s�\�Ȃ̂�NDExpr��������
  % TODO:�Ȃ�Ƃ��������Ƃ���H
  splitExprsResult_ := splitExprs(removePrevCons(constraintStore_), csVariables_);
  NDExprs_ := part(splitExprsResult_, 1);
  debugWrite("NDExprs_: ", NDExprs_);
  NDExprVars_ := part(splitExprsResult_, 2);
  debugWrite("NDExprVars_: ", NDExprVars_);
  DExprs_ := part(splitExprsResult_, 3);
  debugWrite("DExprs_: ", DExprs_);
  DExprVars_ := part(splitExprsResult_, 4);
  debugWrite("DExprVars_: ", DExprVars_);
  otherExprs_:= union(part(splitExprsResult_, 5), parameterStore_);
  debugWrite("otherExprs_: ", otherExprs_);
  % DNF�`���ɂ���
  % ��W���Ȃ�A{{true}}�Ƃ��Ĉ����itrue��\��DNF�j
  if(otherExprs_={}) then paramCondDNF_:= {{true}}
  else <<
    % paramCondDNF_:= myFoldLeft((addCondDNFToCondDNF(#1, exIneqSolve(#2)))&, {{true}}, otherExprs_);������
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
    prevVars_:= for each x in csVariables_ join if(isPrevVariable(x)) then {x} else {};
    debugWrite("prevVars_: ", prevVars_);
    noPrevVars_:= union(for each x in prevVars_ collect part(x, 1));
    debugWrite("noPrevVars_: ", noPrevVars_);
    DExprRcontsVars_ := union(for each x in noPrevVars_ join if(not freeof(DExprRconts_, x)) then {x} else {});
    debugWrite("DExprRcontsVars_: ", DExprRcontsVars_);
    DExprs_:= union(DExprs_, DExprRconts_);
    DExprVars_:= union(DExprVars_, DExprRcontsVars_);
  >>;

  initCons_:= union(for each x in (rconts_ \ DExprRconts_) collect exSub(constraintStore_, x));
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


  % NDExpr_��A��
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
    % Cond��true�̏ꍇ�͋�W���Ƃ��Ĉ���
    if(isTrueDNF(minTCondDNF_)) then minTCondDNF_:= {{}};
    % ���Z�q�������R�[�h�ɒu��������
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

  % t>0�ƘA������false�ɂȂ�悤�ȏꍇ��MinTime���l����K�v���Ȃ�
  if(rlqe(integAsk_ and t>0) = false) then return {{INFINITY, condDNF_}};


  % �O��FParseTree�\�z���ɕ�������Ă���͂��Ȃ̂ŃK�[�h��or�����邱�Ƃ͍l���Ȃ�
  % TODO�F��g�̌`���Ɠ��邱�Ƃ�����̂ł́H�H
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
    else integAsk_
  >>;
  debugWrite("integAskList_: ", integAskList_);

  % �K�[�h�����S�̂�true�̏ꍇ�ƃK�[�h��������false������ꍇ�̓G���[
  if(integAskList_={error} or not freeof(integAskList_, false)) then return {error};

%  integAskQE_:= rlqe(integAsk_);
%  debugWrite("integAskQE_: ", integAskQE_);
%
%  %%%%%%%%%%%% TODO:���̕ӂ���A%%%%%%%%%%%%%%
%  % �܂��Aand�łȂ�����tmp��������X�g�ɕϊ�
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
                                else if(isTrueDNF(ineqSolDNF_)) then {{error}} % ��
                                else ineqSolDNF_
                              >>
                            );
  debugWrite("integAskIneqSolDNFList_:", integAskIneqSolDNFList_);
  % unknown���܂܂ꂽ��G���[��Ԃ�
  % TODO�F�v����
  if(not freeof(integAskIneqSolDNFList_, unknown)) then return {error}
  % true�ɂȂ�����G���[
  else if(not freeof(integAskIneqSolDNFList_, error)) then return {error};


  debugWrite("========== add t>0 ==========", " ");
  integAskIneqSolDNF_:= myFoldLeft(addCondDNFToCondDNF, {{{t, greaterp, 0}}}, integAskIneqSolDNFList_);
  debugWrite("========== end add t>0 ==========", " ");
  debugWrite("integAskIneqSolDNF_:", integAskIneqSolDNF_);

  % �s�����̏ꍇ�A�����ŏ��߂Ė�����������AintegAskIneqSolDNF_��false�ɂȂ邱�Ƃ�����
  if(isFalseDNF(integAskIneqSolDNF_)) then return {{INFINITY, condDNF_}};

  %%%%%%%%%%%% TODO:���̕ӂ܂ł�1�̏����ɂ܂Ƃ߂���%%%%%%%%%%%%


  minTCList_:= checkInfMinTimeDNF(integAskIneqSolDNF_, condDNF_);
  debugWrite("minTCList_ in findMinTime: ", minTCList_);
  debugWrite("=================== end of findMinTime ====================", " ");

  % ERROR���Ԃ��Ă�����error
  if(not freeof(minTCList_, ERROR)) then return {error};
  return minTCList_;
end;

procedure compareMinTimeList(candidateTCList_, newTCList_)$
begin;
  scalar tmpRet_, arg2_, arg3_, ret_;

  debugWrite("in compareMinTimeList", " ");
  debugWrite("candidateTCList_: ", candidateTCList_);
  debugWrite("newTCList_: ", newTCList_);

  %ret_:= myFoldLeft((makeMapAndUnion(candidateTCList_, #1, #2))&, {}, newTCList_);������
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

  % Map�ł͂Ȃ��AJoin���g���������������Ȃ̂ł������Ă���
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
  % ���ꂼ��̏��������ɂ��Ę_���ς����Afalse�Ȃ��W��
  intersectionCondDNF_:= addCondDNFToCondDNF(TC1Cond_, TC2Cond_);
  debugWrite("intersectionCondDNF_: ", intersectionCondDNF_);
  if(isFalseDNF(intersectionCondDNF_)) then return {};

  % �����̋��ʕ����Ǝ��ԂɊւ�������Ƃ̘_���ς����
  % TC1Time_��TC2Time_�Ƃ�������
  debugWrite("========== make TC1LessTC2CondDNF_ ==========", " ");
  TC1LessTC2CondDNF_:= addCondTupleToCondDNF({TC1Time_, lessp, TC2Time_}, intersectionCondDNF_);
  debugWrite("TC1LessTC2CondDNF_: ", TC1LessTC2CondDNF_);
  % TC1Time_��TC2Time_�Ƃ�������
  debugWrite("========== make TC1GeqTC2CondDNF_ ==========", " ");
  TC1GeqTC2CondDNF_:= addCondTupleToCondDNF({TC1Time_, geq, TC2Time_}, intersectionCondDNF_);
  debugWrite("TC1GeqTC2CondDNF_: ", TC1GeqTC2CondDNF_);


  retTCList_:= {};
  % ���ꂼ��Afalse�łȂ����retTCList_�ɒǉ�
  if(not isFalseDNF(TC1LessTC2CondDNF_)) then 
    if(mode_=MIN) then retTCList_:= cons({TC1Time_, TC1LessTC2CondDNF_}, retTCList_)
    else if(mode_=MAX) then retTCList_:= cons({TC2Time_, TC1LessTC2CondDNF_}, retTCList_);
  if(not isFalseDNF(TC1GeqTC2CondDNF_)) then 
    if(mode_=MIN) then retTCList_:= cons({TC2Time_, TC1GeqTC2CondDNF_}, retTCList_)
    else if(mode_=MAX) then retTCList_:= cons({TC1Time_, TC1GeqTC2CondDNF_}, retTCList_);

  debugWrite("retTCList_ in compareParamTime: ", retTCList_);
  return retTCList_;
end;

%TODO �G���[���o�i�K�p�������ʎ����ȊO�ɂȂ����ꍇ���j
procedure applyTime2Expr(expr_, time_)$
begin;
  scalar appliedExpr_;
  putLineFeed();

  appliedExpr_:= sub(t=time_, expr_);
  debugWrite("appliedExpr_:", appliedExpr_);

  return {1, appliedExpr_};
end;

%---------------------------------------------------------------
% �V�~�����[�V�����ɒ��ڂ͊֌W�Ȃ��������n�̓s���ŕK�v�Ȋ֐�
%---------------------------------------------------------------

% �f�o�b�O�p���b�Z�[�W�o�͊֐�
% TODO:�C�Ӓ��̈����ɑΉ�������
procedure debugWrite(arg1_, arg2_)$
  if(optUseDebugPrint_) then <<
    write(arg1_, arg2_);
  >> 
  else <<
    1$
  >>;

% �֐��Ăяo����redeval���o�R������
% <redeval> end:�̎����ŏI�s
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
% �����_�ł͎g�p���Ă��Ȃ����L�p�����Ȋ֐�
%---------------------------------------------------------------

procedure bball_out()$
% gnuplot�p�o��, ������
% ���K�\�� {|}|\n
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

symbolic redeval '(putLineFeed);

;end;
