load_package sets;

% �O���[�o���ϐ�
% constraintStore_: ���݈����Ă��鐧��W���i���X�g�`���j
% csVariables_: ����X�g�A���ɏo������ϐ��̈ꗗ�i�萔���Ή��j
%
% optUseDebugPrint_: �f�o�b�O�o�͂����邩�ǂ���
%

% �f�o�b�O�p���b�Z�[�W�o�͊֐�
% TODO:�C�Ӓ��̈����ɑΉ�������
procedure debugWrite(arg1_, arg2_)$
  if(optUseDebugPrint_) then <<
    write(arg1_, arg2_);
  >> 
  else <<
    1$
  >>$

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

procedure applyUnitAndFlatten(funcName_, appliedExpr_, newArg_)$
  if(neqArg_ = {}) then appliedExpr_
  else if(part(appliedExpr_, 0) neq funcName_) then funcName_(appliedExpr_, newArg_)
  else applyUnitAndFlatten(funcName_, funcName_(part(appliedExpr_, 1), part(appliedExpr_, 2), first(newArg_)), rest(newArg_))$

%Mathematica�ł���Apply�֐�
procedure myApply(func_, expr_)$
part(expr_, 0):= func_$

procedure getInverseRelop(relop_)$
  if(relop_=equal) then neq
  else if(relop_=neq) then equal
  else if(relop_=geq) then lessp
  else if(relop_=greaterp) then leq
  else if(relop_=leq) then greaterp
  else if(relop_=lessp) then geq
  else nil$

procedure myif(x,op,y,approx_precision)$
%����: �_����(ex. sqrt(2), greaterp_, sin(2)), ���x
%�o��: t or nil or -1
%      (x��y���قړ������� -1)
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

% 10^(3 + y��x�̎w�����̒l - �L������)
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

  return ans;
end;


procedure myInfinityIf(x, op, y)$
begin;
  scalar ans;
  % INFINITY > -INFINITY�Ƃ��̑Ή�
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
%����: �ϐ���, �����̃��X�g�̃��X�g(ex. {{x=1,y=2},{x=3,y=4},...})
%�o��: �ϐ��ɑΉ�����l�̃��X�g
if(llst={}) then {}
	else if(rest(llst)={}) then getf(x,first(llst))
		else getf(x,first(llst)) . {lgetf(x,rest(llst))}$

procedure mymin(x,y)$
%����: ���l�Ƃ����O��
if(myif(x,lessp,y,30)) then x else y$

procedure mymax(x,y)$
%����: ���l�Ƃ����O��
if(myif(x,greaterp,y,30)) then x else y$

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
    % ��������sqrt�������Ă��鎞�̂݁Amyif�֐��ɂ��召��r���L���ƂȂ�
    % TODO:���Y�̐������ɕϐ����������ۂɂ��������������ł���悤�ɂ���
    if(myif(part(formula_, 1), getInverseRelop(header_), part(formula_, 2), 30)) then appliedFormula_:= false 
    else appliedFormula_:= rlqe(formula_)
  >> else <<
    appliedFormula_:= rlqe(formula_)
  >>;

  debugWrite("appliedFormula_: ", appliedFormula_);
  return appliedFormula_;

end;

procedure rationalise(expr_)$
begin;
  scalar head_, denominator_, numerator_, denominatorHead_, conjugate_, 
         rationalisedArgsList_, rationalisedExpr_, flag_;

  debugWrite("expr_: ", expr_);
  if(getArgsList(expr_)={}) then return expr_;

  % �z�肷��ΏہF����̍�����2�܂�
  % TODO:����ʓI�Ȍ`�ւ̑Ή������ꂪsqrt(a)+sqrt(b)+c�̌`(a,b>0)�Ƃ�
  % TODO:3�捪�ȏ�ւ̑Ή�

  head_:= part(expr_, 0);
  debugWrite("head_: ", head_);

  if(head_=quotient) then <<
    numerator_:= part(expr_, 1);
    denominator_:= part(expr_, 2);
    % ����ɖ��������Ȃ���ΗL�����K�v�Ȃ�
    if(numberp(denominator_)) then return expr_;

    denominatorHead_:= part(denominator_, 0);
    if((denominatorHead_=plus) or (denominatorHead_=difference)) then <<
      conjugate_:= denominatorHead_(part(denominator_, 1), -1*part(denominator_, 2));
    >> else <<
      conjugate_:= -1*denominator_;
    >>;
    % ���𐔂𕪕�q�ɂ�����
    numerator_:= numerator_ * conjugate_;
    denominator_:= denominator_ * conjugate_;
    rationalisedExpr_:= numerator_ / denominator_;
    flag_:= true;
  >> else if(length(expr_)>1) then <<
    rationalisedArgsList_:= map(rationalise, getArgsList(expr_));
    debugWrite("rationalisedArgsList_: ", rationalisedArgsList_);
    rationalisedExpr_:= myApply(head_, rationalisedArgsList_);
  >> else <<
    rationalisedExpr_:= expr_;
  >>;

  debugWrite("rationalisedExpr_: ", rationalisedExpr_);
  debugWrite("flag_; ", flag_);
  if(flag_=true) then rationalisedExpr_:= rationalise(rationalisedExpr_);
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



% PP�ɂ����鐧��X�g�A�̃��Z�b�g

procedure resetConstraintStore()$
begin;
  putLineFeed();

  constraintStore_ := {};
  csVariables_ := {};
  debugWrite("constraintStore_: ", constraintStore_);
  debugWrite("csVariables_: ", csVariables_);

end;

% PP�ɂ����鐧��X�g�A�ւ̐���̒ǉ�

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


% expr_���ɓ����ȊO��_�����Z�q���܂܂��ꍇ�ɗp����u���֐�

procedure exSub(patternList_, expr_)$
begin;
  scalar subAppliedExpr_, head_, subAppliedLeft_, subAppliedRight_, 
         argCount_, subAppliedExprList_, test_;

  debugWrite("in exSub", " ");
  debugWrite("expr_:", expr_);
  
  % �����������Ȃ��ꍇ
  if(arglength(expr_)=-1) then <<
    subAppliedExpr_:= sub(patternList_, expr_);
    return subAppliedExpr_;
  >>;

  head_:= part(expr_, 0);

  % or�Ō����������̓��m�����ʂł�����Ȃ��ƁAneq�Ƃ����Ⴄ�����̂�����������\������
  if((head_=neq) or (head_=geq) or (head_=greaterp) or (head_=leq) or (head_=lessp)) then <<
    % �����ȊO�̊֌W���Z�q�̏ꍇ
    subAppliedLeft_:= exSub(patternList_, part(expr_, 1));
    debugWrite("subAppliedLeft_:", subAppliedLeft_);
    subAppliedRight_:= exSub(patternList_, part(expr_, 2));
    debugWrite("subAppliedRight_:", subAppliedRight_);
    subAppliedExpr_:= head_(subAppliedLeft_, subAppliedRight_);
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


% PP�ɂ����閳�������̔���
% �Ԃ�l��{ans, {{�ϐ��� = �l},...}} �̌`��
% �d�l QE���g�p % (�g�p����Ȃ�, �ϐ��͊�{����I�ɒu������)

procedure checkConsistencyBySolveOrRlqe(exprs_, lcont_, vars_)$
begin;
  scalar flag_, ans_, modeFlagList_, mode_, csRule_, tmpSol_,
         solvedExprs_, solvedExprsQE_;

  debugWrite("checkConsistencyBySolveOrRlqe: ", " ");
  debugWrite("exprs_: ", exprs_);
  debugWrite("lcont_: ", lcont_);
  debugWrite("vars_: ", vars_);


  debugWrite("union(constraintStore_, lcont_):",  union(constraintStore_, lcont_));
  debugWrite("union(csVariables_, vars_):", union(csVariables_, vars_));
  tmpSol_:= solve(union(constraintStore_, lcont_),  union(csVariables_, vars_));
  debugWrite("tmpSol_: ", tmpSol_);

  if(tmpSol_={}) then return {retfalse___};
  % 2�d���X�g�̎��̂�first�œ���
  % TODO:�����𓾂�ꂽ�ꍇ�ւ̑Ή�
  if(part(first(tmpSol_), 0)=list) then tmpSol_:= first(tmpSol_);


  % exprs_�ɓ����ȊO�������Ă��邩�ǂ����ɂ��A�����̂Ɏg�p����֐�������
  modeFlagList_:= for each x in exprs_ join 
    if(hasInequality(x) or hasLogicalOp(x)) then {false} else {true};
  debugWrite("modeFlagList_:", modeFlagList_);
  mode_:= if(rlqe(mymkand(modeFlagList_))=false) then RLQE else SOLVE;
  debugWrite("mode_:", mode_);

  if(mode_=SOLVE) then
  <<
    debugWrite("union(tmpSol_, exprs_):",  union(tmpSol_, exprs_));
    debugWrite("union(csVariables_, vars_):", union(csVariables_, vars_));
    ans_:=solve(union(tmpSol_, exprs_), union(csVariables_, vars_));
    debugWrite("ans_ in checkConsistencyBySolveOrRlqe: ", ans_);
    if(ans_ <> {}) then return {rettrue___, ans_} else return {retfalse___};
  >> else
  <<
    % sub�̊g���ł�p�����@
    solvedExprs_:= union(for each x in exprs_ join {exSub(tmpSol_, x)});
    debugWrite("solvedExprs_:", solvedExprs_);
    solvedExprsQE_:= exRlqe(mymkand(solvedExprs_));
    debugWrite("solvedExprsQE_:", solvedExprsQE_);
    debugWrite("union(tmpSol_, solvedExprsQE_):", union(tmpSol_, {solvedExprsQE_}));
%    ans_:= exRlqe(mymkand(union(tmpSol_, {solvedExprs_})));
    ans_:= rlqe(mymkand(union(tmpSol_, {solvedExprsQE_})));
    debugWrite("ans_ in checkConsistencyBySolveOrRlqe: ", ans_);
    if(ans_ <> false) then return {rettrue___, ans_} else return {retfalse___};
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


% ����{(�ϐ���), (�֌W���Z�q�R�[�h), (�l�̃t��������)}�̌`���ɕϊ�����

procedure convertCSToVM()$
begin;
  putLineFeed();

  debugWrite("constraintStore_:", constraintStore_);


end;


procedure returnCS()$
begin;
  putLineFeed();

  debugWrite("constraintStore_:", constraintStore_);
  if(constraintStore_={}) then return {};

  % ����1��������
  % TODO: Or�łȂ������������ւ̑Ή�
  % 2�d���X�g��ԂȂ�1���x��������Ԃ��B1�d���X�g�Ȃ炻�̂܂ܕԂ�
  if(part(part(constraintStore_, 1), 0)=list) then return part(constraintStore_, 1)
  else return constraintStore_;
end;


procedure hasInequality(expr_)$
  if(freeof(expr_, neq) and freeof(expr_, not) and
    freeof(expr_, geq) and freeof(expr_, greaterp) and
    freeof(expr_, leq) and freeof(expr_, lessp)) then nil else t$

procedure hasLogicalOp(expr_)$
  if(freeof(expr_, and) and freeof(expr_, or)) then nil else t$


% createCStoVM
% TODO �~�萔��Ԃ�����

% orexqr_:={df(prev(y),t,1) = 0 and df(y,t,1) = 0 and df(y,t,2) = -10 and prev(y) = 10 and y = 10};
% symbolic reval '(createcstovm orexpr_);
procedure createcstovm(orexpr_)$
begin;
  scalar flag_, ans_, tmp_;

  

  return {0, -10, 10};
end;








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
 
  exceptdfvars_:= removedf(vars_);
  tmp_:= for each x in exceptdfvars_ collect {x,mkid(lap,x)};
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


% 20110705 overconstraint___����
ICI_SOLVER_ERROR___:= 0;
ICI_CONSISTENT___:= 1;
ICI_INCONSISTENT___:= 2;
ICI_UNKNOWN___:= 3; % �s�v�H

procedure checkConsistencyInterval(tmpCons_, exprs_, pexpr_, init_, vars_)$
begin;
  scalar tmpSol_, splitExprsResult_, NDExprs_, DExprs_, DExprVars_, otherExprs_,
         integTmp_, integTmpQE_, integTmpSol_, infList_, ans_;
  putLineFeed();

  % Sin��Cos���܂܂��ꍇ�̓��v���X�ϊ��s�\�Ȃ̂�NDExpr��������
  % TODO:�Ȃ�Ƃ��������Ƃ���H
  splitExprsResult_ := splitExprs(exprs_, vars_);
  NDExprs_ := part(splitExprsResult_, 1);
  debugWrite("NDExprs_: ", NDExprs_);
  DExprs_ := part(splitExprsResult_, 2);
  debugWrite("DExprs_: ", DExprs_);
  DExprVars_ := part(splitExprsResult_, 3);
  debugWrite("DExprVars_: ", DExprVars_);
  otherExprs_:= part(splitExprsResult_, 4);
  debugWrite("otherExprs_: ", otherExprs_);

%  tmpSol_:= exDSolve(DExpr_, init_, getNoDifferentialVars(DExprVars_));
  tmpSol_:= exDSolve(DExprs_, init_, DExprVars_);
  debugWrite("tmpSol_: ", tmpSol_);
  
  if(tmpSol_ = retsolvererror___) then return {ICI_SOLVER_ERROR___}
  else if(tmpSol_ = retoverconstraint___) then return {ICI_INCONSISTENT___};

  % NDExpr_��A��
  tmpSol_:= solve(union(tmpSol_, NDExprs_), getNoDifferentialVars(vars_));
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


%  % t>0��A��������true/false����
%  ans_:= rlqe(rlex(integTmpQE_ and t>0));
%  debugWrite("ans_:", ans_);
%  if(ans_=false) then return {ICI_INCONSISTENT___};
%%  if(ans_=false) then return {ICI_INCONSISTENT___} else return {ICI_CONSISTENT___};


  if(not hasLogicalOp(integTmpQE_)) then <<
    % �Ƃ肠����t�Ɋւ��ĉ����i�����̌`����O��Ƃ��Ă���j
    integTmpSol_:= solve(integTmpQE_,t);

    infList_:= union(for each x in integTmpSol_ join checkInfUnit(x, ENTAILMENT___));
    debugWrite("infList_: ", infList_);

    % �p�����^�����Ȃ����1�ɂȂ�͂�
    if(length(infList_) neq 1) then return {ICI_SOLVER_ERROR};
    ans_:= first(infList_);

  >> else <<
    % �܂��Aand�łȂ�����tmp��������X�g�ɕϊ�
    integTmpQEList_:= getArgsList(integTmpQE_);
    debugWrite("integTmpQEList_:", integTmpQEList_);

    % ���ꂼ��ɂ��āA�����Ȃ��solve����integTmpSolList_�Ƃ���B�s�����Ȃ�Ό�񂵁B
    integTmpSolList_:= union(for each x in integTmpQEList_ join 
                         if(not hasInequality(x) and not hasLogicalOp(x)) then solve(x, t) else {x});
    debugWrite("integTmpSolList_:", integTmpSolList_);

    % integTmpSolList_�̊e�v�f�ɂ��āAcheckInfUnit���āAinfList_�𓾂�
    % TODO:integTmpQEList_�̗v�f����or�������Ă���ꍇ���l����
    infList_:= union(for each x in integTmpSolList_ join checkInfUnit(x, ENTAILMENT___));
    debugWrite("infList_: ", infList_);

    ans_:= rlqe(mymkand(infList_));
  >>;

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




procedure checkInfUnit(tExpr_, mode_)$
begin;
  scalar head_, infCheckAns_, exprLhs_, solveAns_, tExprSol_,
         orArgsAnsList_, andArgsAnsList_;

  debugWrite("tExpr_: ", tExpr_);
  debugWrite("mode_: ", mode_);

  head_:= part(tExpr_, 0);
  debugWrite("head_: ", head_);

  if(head_=or) then <<
    if(mode_ = ENTAILMENT___) then <<
      orArgsAnsList_:= for i:=1 : arglength(tExpr_) join
        checkInfUnit(part(tExpr_, i), mode_);
      debugWrite("orArgsAnsList_: ", orArgsAnsList_);
      infCheckAns_:= {rlqe(mymkor(orArgsAnsList_))};
    >> else if(mode_ = MINTIME___) then <<
      orArgsAnsList_:= union(for i:=1 : arglength(tExpr_) collect
        checkInfUnit(part(tExpr_, i), mode_));
      debugWrite("orArgsAnsList_: ", orArgsAnsList_);
      infCheckAns_:= cons(or, orArgsAnsList_);
    >>
  >> else if(head_=and) then <<
    if(mode_ = ENTAILMENT___) then <<
      andArgsAnsList_:= for i:=1 : arglength(tExpr_) join
        checkInfUnit(part(tExpr_, i), mode_);
      debugWrite("andArgsAnsList_: ", andArgsAnsList_);
      infCheckAns_:= {rlqe(mymkand(andArgsAnsList_))};
    >> else if(mode_ = MINTIME___) then <<
      andArgsAnsList_:= union(for i:=1 : arglength(tExpr_) collect
        checkInfUnit(part(tExpr_, i), mode_));
      debugWrite("andArgsAnsList_: ", andArgsAnsList_);
      infCheckAns_:= cons(and, andArgsAnsList_);
    >>
  >> else if(head_=equal) then <<
    % �K�[�h��������ɂ����Ă͓����̏ꍇ��false
    if(mode_ = ENTAILMENT___) then infCheckAns_:= {false}
    else if(mode_ = MINTIME___) then <<
      % TODO:2�����ȏ�ւ̑Ή��H
      tExprSol_:= first(solve(tExpr_, t));
      debugWrite("tExprSol_:", tExprSol_);
      if(mymin(part(tExprSol_,2),0) neq part(tExprSol_,2)) then infCheckAns_:= part(tExprSol_, 2)
      else infCheckAns_:= INFINITY;
    >>
  >> else <<
    if(mode_ = ENTAILMENT___) then
      if(rlqe(tExpr_ and t>0) = false) then infCheckAns_:= {false} 
      else infCheckAns_:= {true}
    else if(mode_ = MINTIME___) then <<
      % �s�����̏ꍇ�͂��̂܂ܕԂ�
      infCheckAns_:= tExpr_;
    >>
  >>;
  debugWrite("infCheckAns_: ", infCheckAns_);

  return infCheckAns_;
end;



IC_SOLVER_ERROR___:= 0;
IC_NORMAL_END___:= 1;

procedure integrateCalc(cons_, init_, discCause_, vars_, maxTime_)$
begin;
  scalar ndExpr_, tmpSol_, tmpDiscCause_, 
         retCode_, tmpVarMap_, tmpMinT_, integAns_;
  putLineFeed();

  % Sin��Cos���܂܂��ꍇ�̓��v���X�ϊ��s�\�Ȃ̂�NDExpr��������
  % TODO:�Ȃ�Ƃ��������Ƃ���H
  splitExprsResult_ := splitExprs(expr_, vars_);
  NDExpr_ := part(splitExprsResult_, 1);
  debugWrite("NDExpr_: ", NDExpr_);
  DExpr_ := part(splitExprsResult_, 2);
  debugWrite("DExpr_: ", DExpr_);
  DExprVars_ := part(splitExprsResult_, 3);
  debugWrite("DExprVars_: ", DExprVars_);

%  tmpSol_:= exDSolve(DExpr_, init_, getNoDifferentialVars(DExprVars_));
  tmpSol_:= exDSolve(DExpr_, init_, DExprVars_);
  debugWrite("tmpSol_: ", tmpSol_);

  % NDExpr_��A��
  tmpSol_:= solve(union(tmpSol_, NDExpr_), getNoDifferentialVars(vars_));
  debugWrite("tmpSol_ after solve: ", tmpSol_);

  % TODO:Solver error����

  tmpDiscCause_:= sub(tmpSol_, discCause_);
  debugWrite("tmpDiscCause_:", tmpDiscCause_);

  tmpVarMap_:= first(myFoldLeft(createIntegratedValue, {{},tmpSol_}, vars_)); 
  debugWrite("tmpVarMap_:", tmpVarMap_);

  tmpMinT_:= calcNextPointPhaseTime(maxTime_, tmpDiscCause_);
  debugWrite("tmpMinT_:", tmpMinT_);
  if(tmpMinT_ = error) then retCode_:= IC_SOLVER_ERROR___
  else retCode_:= IC_NORMAL_END___;

  % TODO:tmpMinT_�͕�������������悤�ɂ���
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



procedure calcNextPointPhaseTime(maxTime_, discCause_)$
begin;
  scalar minTList_, minT_, ans_;

  % ���U�ω����N�����Ȃ��ꍇ�́AmaxTime_�܂Ŏ��s���ďI���
  if(discCause_ = {}) then return {maxTime_, 1};

  minTList_:= union(for each x in discCause_ join calcMinTime(x));
  debugWrite("minTList_ in calcNextPointPhaseTime: ", minTList_);

  if(not freeof(minTList_, error)) then return error;

  minT_:= myFindMinimumValue(INFINITY, minTList_);
  debugWrite("minT_: ", minT_);

  if(mymin(minT_, maxTime_) neq maxTime_) then ans_:= {minT_, 0}
  else ans_:= {maxTime_, 1}; 
  debugWrite("ans_ in calcNextPointPhaseTime: ", ans_);

  return ans_;
end;



% Fold�p

procedure calcMinTime(currentMinPair_, newTriple_)$
begin;
  scalar currentMinT_, currentMinTriple_, sol_, minT_;


end;



% Map�p

procedure calcMinTime(integAsk_)$
begin;
  scalar integAskList_, integAskSolList_, integAskSolFormula_,
         minTList_, singletonMinTList_, tmpSol_;

  debugWrite("in calcMinTime", " ");
  debugWrite("integAsk_: ", integAsk_);

  % t>0�ƘA������false�ɂȂ�悤�ȏꍇ��MinTime���l����K�v���Ȃ�
  if(rlqe(integAsk_ and t>0) = false) then return {INFINITY};

  %%%%%%%%%%%% TODO:���̕ӂ���A%%%%%%%%%%%%%%
  % �܂��Aand�łȂ�����tmp��������X�g�ɕϊ�
  if(part(integAsk_, 0)=and) then integAskList_:= getArgsList(integAsk_)
  else integAskList_:= {integAsk_};
  debugWrite("integAskList_:", integAskList_);

  % ���ꂼ��ɂ��āA�����Ȃ��solve����integAskSolList_�Ƃ���B�s�����Ȃ�Ό�񂵁B
  integAskSolList_:= union(for each x in integAskList_ join
                       if(not hasInequality(x)) then <<
                         tmpSol_:= solve(x, t);
                         if(length(tmpSol_)>1) then {tmpSol_} else tmpSol_
                       >> else <<
                         {x}
                       >>
                     );
  debugWrite("integAskSolList_:", integAskSolList_);

  % �_�����`���ɕϊ�
  integAskSolFormula_:= rlqe(mymkand(for each x in integAskSolList_ collect
                          if(part(x, 0)=list) then rlqe(mymkor(x)) else x
                        ));
  debugWrite("integAskSolFormula_: ", integAskSolFormula_);
  %%%%%%%%%%%% TODO:���̕ӂ܂ł�1�̏����ɂ܂Ƃ߂���%%%%%%%%%%%%

  minTList_:= checkInfUnit(integAskSolFormula_, MINTIME___);
  debugWrite("minTList_ in calcMinTime: ", minTList_);

  % minTList_�̐���
  singletonMinTList_:= {regulateMinTExpr(minTList_)};
  debugWrite("singletonMinTList_: ", singletonMinTList_);

  % �p�����^�����Ȃ����1�ɂȂ�͂�
  if(length(singletonMinTList_) neq 1) then return {error};
  % ERROR���Ԃ��Ă�error
  if(singletonMinList_ = {ERROR}) then return {error};
  return singletonMinTList_;

end;

procedure regulateMinTExpr(minTExpr_)$
begin;
  scalar head_, ineqList_, eqList_, argsList_, regulatedMinTList_, andEqArgsCount_,
         lbList_, ubList_, maxLb_, minUb_, exprLhs_, solveAns_;

  debugWrite("in regulateMinTExpr", " ");
  debugWrite("minTExpr_: ", minTExpr_);

  % �����������Ȃ��ꍇ
  if(arglength(minTExpr_)=-1) then return minTExpr_;

  head_:= part(minTExpr_, 0);
  debugWrite("head_: ", head_);
  ineqList_:={};
  eqList_:={};

  if(head_=list) then <<
    % or�܂���and��\���v�f

    argsList_:= getArgsList(minTExpr_) \ {and, or};
    debugWrite("argsList_: ", argsList_);
    % ����1�̃��X�g�Ȃ炻�̗v�f��Ԃ�
    if(length(argsList_)=1) then return first(argsList_);

    for each x in argsList_ do 
      if(hasInequality(x)) then ineqList_:= cons(x, ineqList_);
    debugWrite("ineqList_: ", ineqList_);
    eqList_:= argsList_ \ ineqList_;
    debugWrite("eqList_: ", eqList_);

    if(part(minTExpr_, 1)=or) then <<

      if(ineqList_ neq {}) then return ERROR;

      % INFINITY����
      eqList_:= eqList_ \ {INFINITY};

      regulatedMinTList_:= for each x in eqList_ collect regulateMinTExpr(x);
      debugWrite("regulatedMinTList_: ", regulatedMinTList_);
      minTValue_:= myFindMinimumValue(INFINITY, regulatedMinTList_);
    >> else if(part(minTExpr_, 1)=and) then <<
      % INFINITY����
      eqList_:= eqList_ \ {INFINITY};

      regulatedMinTList_:= for each x in eqList_ collect regulateMinTExpr(x);
      debugWrite("regulatedMinTList_: ", regulatedMinTList_);
      andEqArgsCount_:= length(regulatedMinTList_);
      debugWrite("andEqArgsCount_:", andEqArgsCount_);
      if(andEqArgsCount_ > 1) then return ERROR;
      % lb��ub�Ƃŕ�����
      lbList_:= {};
      ubList_:= {};

      for each x in ineqList_ do <<
        % (t - value) op 0  �̌`��z��
        % TODO:�p�����^�Ή�
        % TODO:2�����ȏ�ւ̑Ή��H
        exprLhs_:= part(x, 1);
        solveAns_:= part(solve(exprLhs_=0, t), 1);

        if((not freeof(x, geq)) or (not freeof(x, greaterp))) then
          lbList_:= cons(part(solveAns_, 2), lbList_)
        else ubList_:= cons(part(solveAns_, 2), ubList_);
      >>;
      debugWrite("lbList_: ", lbList_);
      debugWrite("ubList_: ", ubList_);
      % lb�̍ő�l��ub�̍ŏ��l�����߂�
      maxLb_:= myfindMaximumValue(0, lbList_);
      minUB_:= myfindMinimumValue(INFINITY, ubList_);
      debugWrite("maxLb_: ", maxLb_);
      debugWrite("minUb_: ", minUb_);

      if(andEqArgsCount_ = 1) then <<
        % minTValue_�����݂���̂ŁAlb<pt����pt<ub�ł��邱�Ƃ��m���߂�
        minTValue_:= first(regulatedMinTList_);
        debugWrite("minTValue_: ", minTValue_);
        if(mymin(maxLb_, minTValue_) neq maxLb_ or 
          mymin(minTValue_, minUb_) neq minTValue_) then minTValue_:= INFINITY;
      >> else <<
        % �s���������Ȃ̂ŁAlb<ub����lb>0���m���߂�
        if(mymin(maxLb_, minUb_) = maxLb_ and mymin(0, maxLb_) = 0) then minTValue_:= maxLb_ 
        else minTValue_:= INFINITY;
      >>;
    >>;
    debugWrite("minTValue_: ", minTValue_);
    return minTValue_;
  >> else <<
    % and��or�ɂȂ��炸�ɁA<�����<=�ɂ��ǂ蒅�����ꍇ��ERROR����
    if((not freeof(minTExpr_, leq)) or (not freeof(minTExpr_, lessp))) then return ERROR
    else return minTExpr_;
  >>;
  
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
