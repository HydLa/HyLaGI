load_package sets;

% �O���[�o���ϐ�
% constraintStore_: ���݈����Ă��鐧��W���i���X�g�`���APP�̒萔���Ή��j
% csVariables_: ����X�g�A���ɏo������ϐ��̈ꗗ�i���X�g�`���APP�̒萔���Ή��j
% parameterStore_: ���݈����Ă���A�萔����̏W���i���X�g�`���AIP�̂ݎg�p�j
% psParameters_: �萔����̏W���ɏo������萔�̈ꗗ�i���X�g�`���AIP�̂ݎg�p�j
%
% optUseDebugPrint_: �f�o�b�O�o�͂����邩�ǂ���
% approxPrecision_: checkOrderingFormula���ŁA�����𐔒l�ɋߎ�����ۂ̐��x
%

% �O���[�o���ϐ�������
% TODO:�v����
approxPrecision_:= 30;

% �f�o�b�O�p���b�Z�[�W�o�͊֐�
% TODO:�C�Ӓ��̈����ɑΉ�������
procedure debugWrite(arg1_, arg2_)$
  if(optUseDebugPrint_) then <<
    write(arg1_, arg2_);
  >> 
  else <<
    1$
  >>;

%Mathematica�ł���Head�֐�
procedure myHead(expr_)$
  if(arglength(expr_)=-1) then nil
  else part(expr_, 0)$

%Mathematica�ł���Fold�֐�
procedure myFoldLeft(func_, init_, list_)$
  if(list_ = {}) then init_
  else myFoldLeft(func_, func_(init_, first(list_)), rest(list_))$

procedure getArgsList(expr_)$
  if(arglength(expr_)=-1) then {}
  else for i:=1 : arglength(expr_) collect part(expr_, i)$

%Mathematica�ł���Apply�֐�
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
%����: �_����(����sqrt(2), greaterp_, sin(2)�Ȃǂ��܂ނ悤�Ȃ���), ���x
%�o��: t or nil or -1
%      (x��y���قړ������� -1)
%geq_= >=, geq; greaterp_= >, greaterp; leq_= <=, leq; lessp_= <, lessp;
begin;
  scalar head_, x, op, y, bak_precision, ans, margin;

  debugWrite("in checkOrderingFormula", " ");
  debugWrite("orderingFormula_: ", orderingFormula_);

  head_:= myHead(orderingFormula_);
  % �召�Ɋւ���_�����ȊO�����͂��ꂽ��G���[
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


  debugWrite("ans in checkOrderingFormula: ", ans);
  return ans;
end;


procedure myInfinityIf(x, op, y)$
begin;
  scalar ans, tmpAns_, tupleDNF_, retTuple_;

  debugWrite("in myInfinityIf", " ");
  % INFINITY > -INFINITY�Ƃ��̑Ή�
  if(x=INFINITY or y=-INFINITY) then 
    if((op = geq) or (op = greaterp)) then ans:=t else ans:=nil
  else if(x=-INFINITY or y=INFINITY) then
    if((op = leq) or (op = lessp)) then ans:=t else ans:=nil
  else <<
    % �W�����ւ̑Ή��Ƃ��āA�܂�infinity relop value�̌`�ɂ��Ă�������Ȃ���
    tupleDNF_:= exIneqSolve(op(x, y));
    debugWrite("tupleDNF_: ", tupleDNF_);
    % 1�����ł���Ƃ��́A����1�Ȃ͂�
    % TODO�FINFINITY^2�Ȃǂւ̑Ή�
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
%����: �ϐ���, �����̃��X�g�̃��X�g(ex. {{x=1,y=2},{x=3,y=4},...})
%�o��: �ϐ��ɑΉ�����l�̃��X�g
if(llst={}) then {}
	else if(rest(llst)={}) then getf(x,first(llst))
		else getf(x,first(llst)) . {lgetf(x,rest(llst))}$

procedure mymin(x,y)$
%����: ���l�Ƃ����O��
if(checkOrderingFormula(x<y)) then x else y$

procedure mymax(x,y)$
%����: ���l�Ƃ����O��
if(checkOrderingFormula(x>y)) then x else y$

procedure myFindMinimumValue(x,lst)$
%����: ���i�K�ł̍ŏ��lx, �ŏ��l�����������Ώۂ̃��X�g
%�o��: ���X�g���̍ŏ��l
if(lst={}) then x
else if(mymin(x, first(lst)) = x) then myFindMinimumValue(x,rest(lst))
else myFindMinimumValue(first(lst),rest(lst))$

procedure myFindMaximumValue(x,lst)$
%����: ���i�K�ł̍ő�lx, �ő�l�����������Ώۂ̃��X�g
%�o��: ���X�g���̍ő�l
if(lst={}) then x
else if(mymax(x, first(lst)) = x) then myFindMaximumValue(x,rest(lst))
else myFindMaximumValue(first(lst),rest(lst))$

procedure compareValueAndParameter(val_, paramExpr_, condDNF_)$
%����: ��r����Ώۂ̒l, �p�����[�^���܂ގ�, �p�����[�^�Ɋւ������
%�o��: {�u�l�v�����������Ȃ邽�߂̃p�����[�^�̏���, �u�p�����[�^���܂ގ��v���������Ȃ邽�߂̃p�����[�^�̏���}
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

% ���́F�f�t�H���g�̍ŏ��l�Ɋւ���ut�̎��Ə����̑g�vdefaultTC_, ���ƂȂ�ut�̎��Ə����̑g�v�̃��X�gTCList_
% �o�́F(�ŏ��l, �����^����萔�̏���)�̑g�̃��X�g
% �Ԃ��l�͕K��0���傫�����̂ɂȂ�悤�ɂ��Ă���_�ɒ��Ӂi0�ȉ��̓G���[�����j
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


  % �s�����̏W������A�ŏ��l�����߂�
  ineqList_:= for each x in ineqTCList_ collect part(x, 1);
  debugWrite("ineqList_: ", ineqList_);
  splitIneqsResult_:= getIneqBoundLists(ineqList_);

  % ���ׂĂ̏����0��菬�����Ȃ��Ă͂Ȃ�Ȃ�
  ubList_:= part(splitIneqsResult_, 2);
  maxUb_:= myfindMaximumValue(0, ubList_);
  debugWrite("maxUb_: ", maxUb_);
  if(maxUb_ neq 0) then return {{ERROR, defaultCond_}};

  % �ł������������͍ŏ��l�ƂȂ肤��
  lbList_:= part(splitIneqsResult_, 1);
  minLb_:= myfindMinimumValue(INFINITY, lbList_);
  debugWrite("minLb_: ", minLb_);

  % value��default_�ƕs�����̒��ł̍ŏ��l�����߂�
  minValue_:= mymin(part(defaultTC_, 1), minLb_);
  minValueTCList_:= for each x in numberTCList_ join 
    if(mymin(minValue_, part(x, 1)) = part(x, 1)) then {x} else {};
  debugWrite("minValueTCList_: ", minValueTCList_);

  % param���Ȃ��ꍇ�͂��̌�̏����͕s�v
  if(paramTCList_={}) then return minValueTCList_;


  % paramTC��minValueTC�Ƃ̔�r�icondition_�ɂ���Č��ʂ��ς�邱�Ƃ�����j
  % �O��Fcondition_���̒萔�̎�ނ�1�܂ŁH
  % TODO�F�Ȃ�Ƃ�����
  if(minValueTCList_ neq {}) then
    retTCList_:= for each x in paramTCList_ join     
      for each y in minValueTCList_ join compareMinTime(x, y)
  else retTCList_:= myFoldLeft(compareMinTime, {INFINITY, defaultCond_}, paramTCList_); %�H�H�H

  debugWrite("retTCList_: ", retTCList_);
  return retTCList_;
end;

%�҂��s��I�֌W
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

%�����̃��X�g��and�Ōq�����_�����ɕϊ�����
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

  % �����������Ȃ��ꍇ�itrue�Ȃǁj
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
    % ��������sqrt�������Ă��鎞�̂݁A�����ȑ召��r���L���ƂȂ�
    % TODO:���Y�̐������ɕϐ����������ۂɂ��������������ł���悤�ɂ���
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

% TODO:3�捪�ȏ�ւ̑Ή�
procedure rationalisation(expr_)$
begin;
  scalar head_, denominator_, numerator_, denominatorHead_, argsList_,
         frontTwoElemList_, restElemList_, timesRhs_, conjugate_, 
         rationalisedArgsList_, rationalisedExpr_, flag_;

  debugWrite("expr_: ", expr_);
  if(getArgsList(expr_)={}) then return expr_;

  % �z�肷��ΏہF����̍�����4�܂�
  % TODO:����ʓI�Ȍ`�ւ̑Ή���5���ȏ�H
  % TODO:3�捪�ȏ�ւ̑Ή�

  head_:= myHead(expr_);
  debugWrite("head_: ", head_);

  if(head_=quotient) then <<
    numerator_:= part(expr_, 1);
    denominator_:= part(expr_, 2);
    % ����ɖ��������Ȃ���ΗL�����K�v�Ȃ�
    if(numberp(denominator_)) then return expr_;

    denominatorHead_:= myHead(denominator_);
    debugWrite("denominatorHead_: ", denominatorHead_);
    if((denominatorHead_=plus) or (denominatorHead_=times)) then <<
      if(denominatorHead_=plus) then argsList_:=
    getArgsList(denominator_)
      else argsList_:= getArgsList(part(denominator_, 2));
      debugWrite("argsList_: ", argsList_);

      % ������3�ȏ�̏ꍇ�A�m���ɖ�����������悤�ɍH�v���ċ��𐔂����߂�
      if(length(argsList_)>2) then <<
        frontTwoElemList_:= getFrontTwoElemList(argsList_);
        debugWrite("frontTwoElemList_: ", frontTwoElemList_);
        restElemList_:= argsList_ \ frontTwoElemList_;
        debugWrite("restElemList_: ", restElemList_);
        if(denominatorHead_=plus) then <<
          conjugate_:= plus(myApply(plus, frontTwoElemList_), -1*(myApply(plus, restElemList_)));
        >> else <<
          % �O��F�ς̉E�ӂ͂��ׂ�plus�łȂ����Ă���`��(-5��+(-5)�̂悤��)
          % TODO�F�����łȂ��ꍇ�ł����C�Ȃ悤�ɁH
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
    % ���𐔂𕪕�q�ɂ�����
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

%---------------------------------------------------------------
% HydLa�����֐�
%---------------------------------------------------------------

operator prev;

%rettrue___ := "RETTRUE___";
%retfalse___ := "RETFALSE___";
rettrue___ := 1;
retfalse___ := 2;

% �֐��Ăяo����redeval���o�R������
% <redeval> end:�̎����ŏI�s

symbolic procedure redeval(foo_)$
begin scalar ans_;

  debugWrite("<redeval> reval :", (car foo_));
  ans_ :=(reval foo_);
  write("<redeval> end:");

  return ans_;
end;



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

procedure getConstraintStore()$
begin;
  scalar ret_;
  putLineFeed();

  ret_:= {constraintStore_, parameterStore_};
  debugWrite("ret_: ", ret_);
  return ret_;
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

procedure removeTrueList(patternList_)$
  for each x in patternList_ join if(rlqe(x)=true) then {} else {x}$

% �_������true�ł���Ƃ��A���݂͂��̂܂�true��Ԃ��Ă���
% TODO�F�Ȃ�Ƃ�����
procedure removeTrueFormula(formula_)$
  if(formula_=true) then true
  else myApply(and, for each x in getArgsList(formula_) join 
    if(rlqe(x)=true) then {} else {x})$

% expr_���ɓ����ȊO��_�����Z�q���܂܂��ꍇ�ɗp����u���֐�
procedure exSub(patternList_, expr_)$
begin;
  scalar subAppliedExpr_, head_, subAppliedLeft_, subAppliedRight_, 
         argCount_, subAppliedExprList_, test_;

  debugWrite("in exSub", " ");
  debugWrite("patternList_: ", patternList_);
  debugWrite("expr_: ", expr_);
  
  % expr_�������������Ȃ��ꍇ
  if(arglength(expr_)=-1) then <<
    subAppliedExpr_:= sub(patternList_, expr_);
    return subAppliedExpr_;
  >>;
  % patternList_����True���Ӗ����鐧�������
  patternList_:= removeTrueList(patternList_);
  debugWrite("patternList_: ", patternList_);

  head_:= myHead(expr_);
  debugWrite("head_: ", head_);

  % or�Ō����������̓��m�����ʂł�����Ȃ��ƁAneq�Ƃ����Ⴄ�����̂�����������\������
  if((head_=neq) or (head_=geq) or (head_=greaterp) or (head_=leq) or (head_=lessp)) then <<
    % �����ȊO�̊֌W���Z�q�̏ꍇ
    subAppliedLeft_:= exSub(patternList_, part(expr_, 1));
    debugWrite("subAppliedLeft_:", subAppliedLeft_);
    subAppliedRight_:= exSub(patternList_, part(expr_, 2));
    debugWrite("subAppliedRight_:", subAppliedRight_);
    % �Ȃ����G���[���N����悤�ɂȂ����H�H
    % subAppliedExpr_:= head_(subAppliedLeft_, subAppliedRight_);
    subAppliedExpr_:= if(head_=neq) then neq(subAppliedLeft_, subAppliedRight_)
                      else if(head_=geq) then geq(subAppliedLeft_, subAppliedRight_)
                      else if(head_=greaterp) then greaterp(subAppliedLeft_, subAppliedRight_)
                      else if(head_=leq) then leq(subAppliedLeft_, subAppliedRight_)
                      else if(head_=lessp) then lessp(subAppliedLeft_, subAppliedRight_);
  >> else if((head_=and) or (head_=or)) then <<
    % �_�����Z�q�̏ꍇ
    argCount_:= arglength(expr_);
    debugWrite("argCount_: ", argCount_);
    subAppliedExprList_:= for i:=1 : argCount_ collect exSub(patternList_, part(expr_, i));
    debugWrite("subAppliedExprList_:", subAppliedExprList_);
    subAppliedExpr_:= myApply(head_, subAppliedExprList_);

  >> else <<
    % ������A�ϐ����Ȃǂ�factor�̏ꍇ
    % TODO:expr_�����āA����X�g�A�i���邢��csvars�j���ɂ���悤�Ȃ�A����Ƒ΂��Ȃ��l�i�����̉E�Ӂj��K�p
    subAppliedExpr_:= sub(patternList_, expr_);
  >>;

  debugWrite("subAppliedExpr_:", subAppliedExpr_);
  return subAppliedExpr_;
end;


procedure exSolve(exprs_, vars_)$
begin;
  % �������݂̂�Ԃ��悤�ɂ���


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
  debugWrite("union(eqExprs_, lcont_):",  union(eqExprs_, lcont_));
  debugWrite("union(csVariables_, vars_):", union(csVariables_, vars_));

  % ���m�ϐ���ǉ����Ȃ��悤�ɂ���
  off arbvars;
  tmpSol_:= solve(union(eqExprs_, lcont_),  union(csVariables_, vars_));  on arbvars;
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

% convertCSToVM���Ŏg���A���`�p�֐�
procedure makeConsTuple(cons_)$
begin;
  scalar varName_, relopCode_, value_, tupleDNF_, retTuple_, adjustedCons_, sol_;

  debugWrite("in makeConsTuple", " ");
  debugWrite("cons_: ", cons_);
  
  % ���ӂɕϐ����݂̂�����`���ɂ���
  % �O��F�����͂��łɂ��̌`���ɂȂ��Ă���
  if(not hasInequality(cons_)) then <<
    varName_:= part(cons_, 1);
    relopCode_:= getExprCode(cons_);
    value_:= part(cons_, 2);
  >> else <<
    tupleDNF_:= exIneqSolve(cons_);
    debugWrite("tupleDNF_: ", tupleDNF_);
    % 1�����ɂȂ��Ă�͂��Ȃ̂ŁA����1�Ȃ͂�
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

% �O��FOr�łȂ����Ă͂��Ȃ�
% TODO�F�Ȃ�Ƃ�����
procedure convertCSToVM()$
begin;
  scalar tmpRet_, ret_;
  putLineFeed();

  debugWrite("constraintStore_:", constraintStore_);
  if(constraintStore_={}) then return {};

  tmpRet_:= applyPrevCons(constraintStore_, {});    
  debugWrite("tmpRet_: ", tmpRet_);

  % ����{(�ϐ���), (�֌W���Z�q�R�[�h), (�l�̃t��������)}�̌`���ɕϊ�����
  ret_:= map(makeConsTuple, tmpRet_);
  debugWrite("ret_: ", ret_);
  return ret_;
end;


procedure returnCS()$
begin;
  putLineFeed();

  debugWrite("constraintStore_:", constraintStore_);
  if(constraintStore_={}) then return {};

  % ����1��������
  % TODO: Or�łȂ������������ւ̑Ή�
  % 2�d���X�g��ԂȂ�1���x��������Ԃ��B1�d���X�g�Ȃ炻�̂܂ܕԂ�
  if(myHead(part(constraintStore_, 1))=list) then return part(constraintStore_, 1)
  else return constraintStore_;
end;


procedure hasInequality(expr_)$
  if(freeof(expr_, neq) and freeof(expr_, not) and
    freeof(expr_, geq) and freeof(expr_, greaterp) and
    freeof(expr_, leq) and freeof(expr_, lessp)) then nil else t$

procedure hasLogicalOp(expr_)$
  if(freeof(expr_, and) and freeof(expr_, or)) then nil else t$

% ���񃊃X�g����A�����ȊO���܂ސ���𒊏o����
procedure getOtherExpr(exprs_)$
  for each x in exprs_ join if(hasInequality(x) or hasLogicalOp(x)) then {x} else {}$

% �����Ƀp�����^���܂܂�Ă��邩�ǂ������ApsParameters_���̕ϐ����܂܂�邩�ǂ����Ŕ���
procedure hasParameter(expr_)$
  if(collectParameters(expr_) neq {}) then t else nil$

% ���\�����̃p�����^���A�W�߂�
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


%% ���\�����̒萔�i�������E�p�����^�j���W�߂�
%procedure collectParameters(expr_, lst_)$
%begin;
%  scalar retList_, lhsRet_, rhsRet_, newLst_;
%
%  if((hasInequality(expr_)) or (myHead(expr_)=equal)) then <<
%    lhsRet_:= collectParameters(part(expr_, 1), lst_);
%    rhsRet_:= collectParameters(part(expr_, 2), lst_);
%    retList_:= union(lhsRet_, rhsRet_);
%  >> else if(hasParameter(expr_)) then <<
%    % expr_��lst_�ɐV�����o�^����
%    % ���łɓo�^���Ă��钆�Ɋ܂܂��ꍇ�͂���������������Ŏ�������
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
  % solveans_��solvevars_�̉�����ł��܂܂�Ȃ��� underconstraint�Ƒz��
  for each x in table_ do 
    if(freeof(solveans_, third(x))) then tmp_:=true;
  if(tmp_=true) then return retunderconstraint___;
  
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


% NDExpr�iexDSolve�ň����Ȃ��悤�Ȑ��񎮁j�ł��邩�ǂ����𒲂ׂ�
% ���̒���sin��cos�������Ă��Ȃ����false
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

% �O��Finitxlhs=prev(x)�̌`
% TODO�F�Ȃ�Ƃ�����
procedure getInitVars(rcont_)$
  part(rcont_, 1)$

% 20110705 overconstraint___����
ICI_SOLVER_ERROR___:= 0;
ICI_CONSISTENT___:= 1;
ICI_INCONSISTENT___:= 2;
ICI_UNKNOWN___:= 3; % �s�v�H

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

  % Sin��Cos���܂܂��ꍇ�̓��v���X�ϊ��s�\�Ȃ̂�NDExpr��������
  % TODO:�Ȃ�Ƃ��������Ƃ���H
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

  % NDExpr_��A��
  debugWrite("getNoDifferentialVars(union(DExprVars_, (vars_ \ initVars_))): ", 
             getNoDifferentialVars(union(DExprVars_, (vars_ \ initVars_))));
  tmpSol_:= solve(union(tmpSol_, NDExprs_), 
                  getNoDifferentialVars(union(DExprVars_, (vars_ \ initVars_))));
  debugWrite("tmpSol_ after solve: ", tmpSol_);

  % tmpCons_���Ȃ��ꍇ�͖������Ɣ��肵�ėǂ�
  if(tmpCons_ = {}) then return {ICI_CONSISTENT___};

  integTmp_:= sub(tmpSol_, tmpCons_);
  debugWrite("integTmp_: ", integTmp_);

  integTmpQE_:= rlqe(mymkand(integTmp_));
  debugWrite("integTmpQE_: ", integTmpQE_);

  % ������true��false�͂��̂܂ܔ��茋�ʂƂȂ�
  if(integTmpQE_ = true) then return {ICI_CONSISTENT___}
  else if(integTmpQE_ = false) then return {ICI_INCONSISTENT___};


  % �܂��Aand�łȂ�����tmp��������X�g�ɕϊ�
  integTmpQEList_:= if(myHead(integTmpQE_)=and) then getArgsList(integTmpQE_)
                    else {integTmpQE_};
  debugWrite("integTmpQEList_:", integTmpQEList_);

  % ���ꂼ��ɂ��āA�����Ȃ��solve����integTmpSolList_�Ƃ���B�s�����Ȃ�Ό�񂵁B
  integTmpSolList_:= union(for each x in integTmpQEList_ join 
                         if(not hasInequality(x) and not hasLogicalOp(x)) then solve(x, t) else {x});
  debugWrite("integTmpSolList_:", integTmpSolList_);

  % integTmpSolList_�̊e�v�f�ɂ��āAcheckInfUnit���āAinfList_�𓾂�
  % TODO:integTmpQEList_�̗v�f����or�������Ă���ꍇ���l����
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
    % �K�[�h��������ɂ����Ă͓����̏ꍇ��false
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
    % (t - value) op 0 �̌`��z��
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

% �o�́F�����Ə����̑g�iTC�j�̃��X�g
% �O��FtExpr_�͓������s������t��1�����ɂȂ��Ă���
% TODO�FERROR����
procedure checkInfMinTime(tExpr_, condDNF_)$
begin;
  scalar head_, tExprSol_, argsAnsTCList_, ineqTCList_, ineqList_, eqTCList_,
         minTCList_, andEqTCArgsCount_,
         lbList_, ubList_, maxLb_, minUb_, minTValue_, compareRet_;

  debugWrite("in checkInfMinTime", " ");
  debugWrite("tExpr_: ", tExpr_);
  debugWrite("condDNF_: ", condDNF_);

  % �����������Ȃ��ꍇ
  if(arglength(tExpr_)=-1) then return {{tExpr_, condDNF_}};

  head_:= myHead(tExpr_);
  debugWrite("head_: ", head_);
  ineqTCList_:={};
  eqTCList_:={};

  if(hasLogicalOp(head_)) then <<
    argsAnsTCList_:= union(for i:=1 : arglength(tExpr_) join
      checkInfMinTime(part(tExpr_, i), condDNF_));
    debugWrite("argsAnsTCList_: ", argsAnsTCList_);
    % ����1�̃��X�g�Ȃ炻�̂܂ܕԂ�
    if(length(argsAnsTCList_)=1) then return argsAnsTCList_;

    for each x in argsAnsTCList_ do
      if(hasInequality(x)) then ineqTCList_:= cons(x, ineqTCList_);
    debugWrite("ineqTCList_: ", ineqTCList_);
    eqTCList_:= argsAnsTCList_ \ ineqTCList_;
    debugWrite("eqTCList_: ", eqTCList_);
    % INFINITY����
    eqTCList_:= for each x in eqTCList_ join 
      if(freeof(part(x, 1), INFINITY)) then {x} else {};

    if(head_=or) then <<
      minTCList_:= myFindMinimumValueCond({INFINITY, condDNF_}, eqTCList_);
    >> else if(head_=and) then <<
      andEqTCArgsCount_:= length(eqTCList_);
      debugWrite("andEqTCArgsCount_:", andEqTCArgsCount_);
      % 2�ȏ�̓������_���ςłȂ����Ă�����G���[
      if(andEqTCArgsCount_ > 1) then return {{ERROR}};

      % lb��ub�Ƃŕ�����
      ineqList_:= for each x in ineqTCList_ collect part(x, 1);
      splitIneqsResult_:= getIneqBoundLists(ineqList_);
      lbList_:= part(splitIneqsResult_, 1);
      ubList_:= part(splitIneqsResult_, 2);
      debugWrite("lbList_: ", lbList_);
      debugWrite("ubList_: ", ubList_);
      % lb�̍ő�l��ub�̍ŏ��l�����߂�
      maxLb_:= myfindMaximumValue(0, lbList_); 
      minUb_:= myfindMinimumValue(INFINITY, ubList_);
      debugWrite("maxLb_: ", maxLb_);
      debugWrite("minUb_: ", minUb_);

      if(andEqTCArgsCount_ = 1) then <<
        % minTValue_�����݂���̂ŁAlb<pt����pt<ub�ł��邱�Ƃ��m���߂�
        minTValue_:= part(first(eqTCList_), 1);
        debugWrite("minTValue_: ", minTValue_);
        if((mymin(maxLb_, minTValue_) neq maxLb_) or (mymin(minTValue_, minUb_) neq minTValue_)) then
          minTCList_:= {{INFINITY, condDNF_}}
        else minTCList_:= {{minTValue_, part(first(eqTCList_), 2)}};
      >> else <<
        % �s���������Ȃ̂ŁAlb<ub����lb>0���m���߂�
        if((mymin(maxLb_, minUb_) = maxLb_) and (mymin(0, maxLb_) = 0)) then 
          minTCList_:= {{maxLb_, condDNF_}}
        else minTCList_:= {{INFINITY, condDNF_}};
      >>;
    >>;
  >> else if(head_=equal) then <<
    tExprSol_:= first(solve(tExpr_, t));
    debugWrite("tExprSol_:", tExprSol_);
    % t>0�łȂ���΁i�A������false�Ȃ�j�AINFINITY��Ԃ�
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
    % �s�����̏ꍇ�͂��̂܂ܕԂ�
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

  % Sin��Cos���܂܂��ꍇ�̓��v���X�ϊ��s�\�Ȃ̂�NDExpr��������
  % TODO:�Ȃ�Ƃ��������Ƃ���H
  splitExprsResult_ := splitExprs(removePrevCons(constraintStore_), csVariables_);
  NDExprs_ := part(splitExprsResult_, 1);
  debugWrite("NDExprs_: ", NDExprs_);
  DExprs_ := part(splitExprsResult_, 2);
  debugWrite("DExprs_: ", DExprs_);
  DExprVars_ := part(splitExprsResult_, 3);
  debugWrite("DExprVars_: ", DExprVars_);
  otherExprs_:= union(part(splitExprsResult_, 4), parameterStore_);
  % ��W���Ȃ�A{{true}}�Ƃ��Ĉ����itrue��\��DNF�j
  if(otherExprs_={}) then otherExprs_:= {{true}};
  debugWrite("otherExprs_: ", otherExprs_);

  initCons_:= union(for each x in rconts_ join {exSub(constraintStore_, x)});
  debugWrite("initCons_: ", initCons_);
  initVars_:= map(getInitVars, rconts_);
  debugWrite("initVars_: ", initVars_);

  tmpSol_:= exDSolve(DExprs_, initCons_, union(DExprVars_, (vars_ \ initVars_)));
  debugWrite("tmpSol_: ", tmpSol_);

  % NDExprs_��A��
  tmpSol_:= solve(union(tmpSol_, NDExprs_), getNoDifferentialVars(vars_ \ initVars_));
  debugWrite("tmpSol_ after solve: ", tmpSol_);

  % TODO:Solver error����

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

  % ���U�ω����N�����Ȃ��ꍇ�́AmaxTime_�܂Ŏ��s���ďI���
  if(discCauseList_ = {}) then return {{maxTime_, condDNF_, 1}};

  minTCondListList_:= union(for each x in discCauseList_ collect findMinTime(x, condDNF_));
  debugWrite("minTCondListList_ in calcNextPointPhaseTime: ", minTCondListList_);

  if(not freeof(minTCondListList_, error)) then return {error};

  comparedMinTCondList_:= myFoldLeft(compareMinTimeList, {{maxTime_, condDNF_}}, minTCondListList_);
  debugWrite("comparedMinTCondList_ in calcNextPointPhaseTime: ", comparedMinTCondList_);

  ans_:= union(for each x in comparedMinTCondList_ collect <<
    minTTime_:= part(x, 1);
    minTCond_:= part(x, 2);
    % Cond��true�̏ꍇ�͋�W���Ƃ��Ĉ���
    if(isTrueDNF(minTCond_)) then minTCond_:= {};
    % ���Z�q�������R�[�h�ɒu��������
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

% �s�������A���ӂɐ��̕ϐ����݂̂�����`���ɐ��`���A�����\���^�v�������
% (�s�����̏ꍇ�A�K���E�ӂ�0�ɂȂ��Ă��܂��i�����I�Ɉڍ����Ă��܂��j���Ƃ�A
% �s�����̌��������肳��Ă��܂����肷�邱�Ƃ����邽��)
% �O��F���͂����s�������ϐ���1��
% TODO�F�����̕s�������n���ꂽ�ꍇ�ւ̑Ή��H
% 2���s�����܂ł͉�����
% TODO�F3���ȏ�ւ̑Ή��H
% ���́F1�ϐ���2���ȉ��̕s����1��
% �o�́F{����, relop, �E��}�̌`���ɂ��DNF���X�g
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
  % �E�ӂ����ӂɈڍ�����
  adjustedIneqExpr_:= myApply(relop_, {lhs_+(-1)*rhs_, 0});
  adjustedLhs_:= lhs(adjustedIneqExpr_);
  debugWrite("adjustedIneqExpr_: ", adjustedIneqExpr_);
  debugWrite("adjustedLhs_: ", adjustedLhs_);
  adjustedRelop_:= myHead(adjustedIneqExpr_);


  % ���̎��_��x+value relop 0�܂���-x+value relop 0�̌`���ɂȂ��Ă���̂ŁA
  % -x+value��0�̏ꍇ��x-value��0�̌`�ɂ���K�v������irelop�Ƃ��ċt�̂��̂𓾂�K�v������j
  % �T�[�o�[���[�h���Ɓ����g�������\���͕ێ��ł��Ȃ��悤�Ȃ̂ŁAreverse���邩�ǂ�������������Ηǂ�
  % minus���Ȃ����g���Ȃ��̂ŁA���ɏo������ϐ����𒲂ׂāA�u-x�v�̌`���Ȃ����ǂ������ׂ�
  exprVarList_:= union(for each x in union(union(csVariables_, psParameters_), {INFINITY}) join
    if(not freeof(adjustedLhs_, x)) then {x} else {});
  debugWrite("exprVarList_: ", exprVarList_);
  % TODO�F�����̕ϐ��������Ă���ꍇ�ւ̑Ή��H
  exprVar_:= first(exprVarList_);
  debugWrite("exprVar_: ", exprVar_);
  if(not freeof(adjustedLhs_, -1*exprVar_)) then <<
    adjustedRelop_:= getReverseRelop(myHead(adjustedIneqExpr_));
  >>;

  % �ϐ���sqrt�����Ă�ꍇ��2�悵�Ă���
  if(not freeof(adjustedLhs_, sqrt(exprVar_))) then <<
    adjustedLhs_:= adjustedLhs_*adjustedLhs_;
    debugWrite("adjustedLhs_ after squaring: ", adjustedLhs_);
  >>;

  
  % ub�܂���lb�����߂�
  off arbvars;
  % INFINITY�̂Ƃ��͕ϐ��Ƃ���infinity���w�肵�A����ȊO�͐���X�g�A�̕ϐ����w�肷��
  % TODO�Felse�̂Ƃ��̕ϐ������������Ǝw�肷��
  if(not freeof(adjustedLhs_, INFINITY)) then 
    sol_:= solve(equal(adjustedLhs_, 0), INFINITY)
  else sol_:= solve(equal(adjustedLhs_, 0), exprVar_);
  debugWrite("sol_: ", sol_);
  on arbvars;

  % TODO�F�����ϐ��ւ̑Ή��H
  % �����ϐ��������2�d���X�g�ɂȂ�͂������A1�ϐ��Ȃ�s�v���H
%  if(length(sol_)>1 and myHead(first(sol_))=list) then <<
%    % or�łȂ���������and�������Ă���֌W��\���Ă���
%      
%    adjustedEqExpr_:= first(first(sol_))
%
%  >> 
  if(length(sol_)>1) then <<
    % 2�����������������ꍇ�B�����ɂ͒���2�̂͂�
    % TODO�F3���ȏ�ւ̈�ʉ��H

    boundList_:= {rhs(part(sol_, 1)), rhs(part(sol_, 2))};
    lb_:= myFindMinimumValue(INFINITY, boundList_);
    ub_:= first(boundList_ \ {lb_});
    
    % relop�ɂ���āAtupleDNF�̍\����ς���
    if((adjustedRelop_ = geq) or (adjustedRelop_ = greaterp)) then <<
      % ������с��̏ꍇ��or�̊֌W�ɂȂ�
      retTupleDNF_:= {{ {lb_, adjustedRelop_, exprVar_} }, { {exprVar_, adjustedRelop_, ub_} }};
    >> else <<
      % ������с��̏ꍇ��and�̊֌W�ɂȂ�
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

% ineq_solve�œ���ꂽ���X�g�`���̏o�͂��Ԍ`���̏o�͂𐮌`���Aor��and�łȂ������`���ɂ���
% TODO�For��and�ŗǂ��̂��H�H���ǂ��Ȃ��B�Ȃ̂Ŏg��Ȃ�
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
    % x=(lb_ .. ub_)�̌`
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

  % t>0�ƘA������false�ɂȂ�悤�ȏꍇ��MinTime���l����K�v���Ȃ�
  if(rlqe(integAsk_ and t>0) = false) then return {{INFINITY, condDNF_}};

  %%%%%%%%%%%% TODO:���̕ӂ���A%%%%%%%%%%%%%%
  % �܂��Aand�łȂ�����tmp��������X�g�ɕϊ�
  if(myHead(integAsk_)=and) then integAskList_:= getArgsList(integAsk_)
  else integAskList_:= {integAsk_};
  debugWrite("integAskList_:", integAskList_);

  % ���ꂼ��ɂ��āA�����Ȃ��solve�E�s�����Ȃ��1���ɂ���integAskSolList_�Ƃ���B
  integAskSolList_:= union(for each x in integAskList_ join
                       if(not hasInequality(x)) then <<
                         tmpSol_:= solve(x, t);
                         if(length(tmpSol_)>1) then {map(rationalisation, tmpSol_)} 
                         else map(rationalisation, tmpSol_)
                       >> else <<
                         ineqSolDNF_:= exIneqSolve(x);
                         debugWrite("ineqSolDNF_: ", ineqSolDNF_);
                         % DNF�`�����炽����or�`���ɒ���
                         for each x in ineqSolDNF_ collect first(x)
                       >>
                     );
  debugWrite("integAskSolList_:", integAskSolList_);

  % �_�����`���ɕϊ�
  integAskSolFormula_:= rlqe(mymkand(for each x in integAskSolList_ collect
                          if(myHead(x)=list) then rlqe(mymkor(x)) else x
                        ));
  debugWrite("integAskSolFormula_: ", integAskSolFormula_);
  %%%%%%%%%%%% TODO:���̕ӂ܂ł�1�̏����ɂ܂Ƃ߂���%%%%%%%%%%%%

  minTCList_:= checkInfMinTime(integAskSolFormula_, condDNF_);
  debugWrite("minTCList_ in findMinTime: ", minTCList_);

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

  % Map�ł͂Ȃ��AJoin���g���������������H
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
  % ���ꂼ��̏��������ɂ��Ę_���ς����Afalse�Ȃ��W��
  intersectionCondDNF_:= addCondDNFToCondDNF(TC1Cond_, TC2Cond_);
  debugWrite("intersectionCondDNF_: ", intersectionCondDNF_);
  if(isFalseDNF(intersectionCondDNF_)) then return retTCList_;

  % �����̋��ʕ����Ǝ��ԂɊւ�������Ƃ̘_���ς����
  % TC1Time_��TC2Time_�Ƃ�������
  TC1LeqTC2CondDNF_:= addCondTupleToCondDNF({TC1Time_, leq, TC2Time_}, intersectionCondDNF_);
  debugWrite("TC1LeqTC2CondDNF_: ", TC1LeqTC2CondDNF_);
  % TC1Time_��TC2Time_�Ƃ�������
  TC1GreaterTC2CondDNF_:= addCondTupleToCondDNF({TC1Time_, greaterp, TC2Time_}, intersectionCondDNF_);
  debugWrite("TC1GreaterTC2CondDNF_: ", TC1GreaterTC2CondDNF_);


  % ���ꂼ��Afalse�łȂ����retTCList_�ɒǉ�
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
%���́F�ǉ�����i�p�����^�́j�����^�v��newCondTuple_, �i�p�����^�́j������\���_�����̃��X�gcondDNF_
%�o�́F�i�p�����^�́j������\���_�����̃��X�g
%���ӁF���X�g�̗v�f1��1�͘_���a�łȂ���A�v�f���͘_���ςłȂ����Ă��邱�Ƃ�\���B
begin;
  scalar addedCondDNF_, addedCondConj_;

  %�ǉ��������ʁA�v�f�iand�łȂ��������j��false�ɂȂ�����A���̗v�f�͏���
  addedCondDNF_:= for each x in condDNF_ join <<
    addedCondConj_:= addCondTupleToCondConj(newCondTuple_, x);
    debugWrite("addedCondConj_ in loop in addCondTupleToCondDNF: ", addedCondConj_);
    if(addedCondConj_=false) then {} else {addedCondConj_}
  >>;

  %%��W���ɂȂ��Ă��܂�����false��Ԃ�
  %if(addedCondDNF_={}) then addedCondDNF_:= false;

  debugWrite("addedCondDNF_ in addCondTupleToCondDNF: ", addedCondDNF_);
  return addedCondDNF_;
end;

% {����, relop, �E��}�̃^�v������A��������ĕԂ�
procedure makeFormulaFromTuple(tuple_)$
  myApply(part(tuple_, 2), {part(tuple_, 1), part(tuple_, 3)});

procedure addCondTupleToCondConj(newCondTuple_, condConj_)$
begin;
  scalar addedCondConj_, varName_, relop_, value_,
         varTerms_, ubTuple_, lbTuple_, ub_, lb_;

  debugWrite("in addCondTupleToCondConj", " ");
  debugWrite("newCondTuple_: ", newCondTuple_);
  debugWrite("condConj_: ", condConj_);

  % true��ǉ����悤�Ƃ���ꍇ�A�ǉ����Ȃ��̂Ɠ���
  if(newCondTuple_=true) then return condConj_;
  % �p�����^������Ȃ��ꍇ�A�P�ɑ召������������ʂ��c��
  if(not hasParameter(newCondTuple_)) then 
    if(checkOrderingFormula(makeFormulaFromTuple(newCondTuple_))) then return condConj_
    else return false;
  % true�ɒǉ����悤�Ƃ���ꍇ
  if(condConj_={true}) then return {newCondTuple_};


  varName_:= part(newCondTuple_, 1);
  relop_:= part(newCondTuple_, 2);
  value_:= part(newCondTuple_, 3);

  % �_���ς̒�����A�ǉ�����ϐ��Ɠ����ϐ��̍���T��
  varTerms_:= union(for each x in condConj_ join
    if(not freeof(x, varName_)) then {x} else {});

  % �����E����𓾂�
  % length(varTerms_)<=2��z��
  ubTuple_:= first(for each x in varTerms_ join
    if((not freeof(x, leq)) or (not freeof(x, lessp))) then {x} else {});
  lbTuple_:= first(varTerms_ \ {ubTuple_});
  ub_:= part(ubTuple_, 3);
  lb_:= part(lbTuple_, 3);

  % �ǉ�����s�����Ə㉺���Ƃ��r���A�X�V����
  if((relop_=leq) or (relop_=lessp)) then <<
    if(mymin(value_, ub_)=value_) then ub_:= value_
  >> else <<
    if(mymin(value_, lb_)=lb_) then lb_:= value_
  >>;

  % lb<ub���m���߂�
  if((mymin(lb_, ub_) = lb_)) then addedCondConj_:= {}
  else addedCondConj_:= false;
				  
  debugWrite("addedCondConj_: ", addedCondConj_);
  return addedCondConj_;
end;

% �p�����^�Ɋւ���s�������A�^�v���`���ɕϊ�����
% exIneqSolve�ő�p�ł���̂ŁA�g��Ȃ�
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

% �s�������Ƀp�����^�ϐ����܂܂��ꍇ�Ɏg���֐�
% exIneqSolve�̎d�l�ύX�i�ϐ��w�肪�s�v�ƂȂ����j�ɂ��A�g��Ȃ�
procedure solveParameterIneq(ineqList_)$
begin;
  scalar paramNameList_, paramName_, ret_;

  debugWrite("in solveParameterIneq", " ");
  debugWrite("ineqList_: ", ineqList_);

  paramNameList_:= collectParameters(ineqList_);
  debugWrite("paramNameList_: ", paramNameList_);
  % 2��ވȏ�̃p�����^���܂܂�Ă���ƈ����Ȃ�
  % TODO�F�Ȃ�Ƃ�����H
  if(length(paramNameList_)>1) then return ERROR;
  paramName_:= first(paramNameList_);

  % exIneqSolve�ł�1�������s�����������Ȃ��̂ŁA�ʁX�ɉ���
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



%TODO �G���[���o�i�K�p�������ʎ����ȊO�ɂȂ����ꍇ���j

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

  % TODO:simplify�֐����g��
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
