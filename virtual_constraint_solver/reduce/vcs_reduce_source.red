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


%Mathematica�ł���Fold�֐�
procedure myFoldLeft(func_, init_, list_)$
  if(list_ = {}) then init_
  else myFoldLeft(func_, func_(init_, first(list_)), rest(list_))$

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
    if(op = geq_ or op = leq_) then return t
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

if (op = geq_) then
  (if (x >= y) then ans:=t else ans:=nil)
else if (op = greaterp_) then
  (if (x > y) then ans:=t else ans:=nil)
else if (op = leq_) then
  (if (x <= y) then ans:=t else ans:=nil)
else if (op = lessp_) then
  (if (x < y) then ans:=t else ans:=nil);

  off rounded$ precision bak_precision$

  return ans;
end;


procedure myInfinityIf(x, op, y)$
begin;
  scalar ans;
  if(x=INFINITY) then 
    (if (op = geq_ or op = greaterp_) then ans:=t else ans:=nil)
  else
    (if (op = leq_ or op = lessp_) then ans:=t else ans:=nil);
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
if(x={}) then
	if(y={}) then {} else y
  else	if(y={}) then x
	else if(myif(x,greaterp_,y,30)) then y else x$

procedure myFindMinimumNatPPTime(x,lst)$
%����: ���i�K�ł̍ŏ�PP����x, ���̎������̃��X�g
%�o��: ����PP�J�n���̎���
% 0��菬�����l��lst�ɓn����Ȃ��Ƃ����O��i���̏����Ŏ�������Ă���j
if(rest(lst)={}) then
<<
  if(myif(x,lessp_,first(lst),30)) then x else first(lst)
>>
else 
<<
  if(myif(x,lessp_,first(lst),30)) then 
  <<
    myFindMinimumNatPPTime(x,rest(lst))
  >>
  else 
  <<
    myFindMinimumNatPPTime(first(lst),rest(lst))
  >>
>>$


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

procedure myex(lst,var)$
rlqe ex(var, mymkand(lst));

procedure myall(lst,var)$
rlqe all(var, mymkand(lst));


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

  write("<redeval> reval ", (car foo_), ":");
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
  debugWrite("cons_: ", cons_);
  debugWrite("vars_:", vars_);

  constraintStore_ := union(constraintStore_, cons_);
  csVariables_ := union(csVariables_, vars_);
  debugWrite("constraintStore_: ", constraintStore_);
  debugWrite("csVariables_: ", csVariables_);

end;


% (���� and���󂯕t���Ȃ�) TODO �����ւ̑Ή�
% (���� true���󂯕t���Ȃ�) TODO �����ւ̑Ή�

procedure checkConsistencyWithTmpCons(expr_,vars_)$
begin;
  scalar ans_;
  putLineFeed();

  ans_:= {part(checkConsistencyBySolveOrRlqe(expr_, vars_), 1)};
  debugWrite("ans_ in checkConsistencyWithTmpCons: ", ans_);

  return ans_;
end;


% PP�ɂ����閳�������̔���
% �Ԃ�l��{ans, {{�ϐ��� = �l},...}} �̌`��
% �d�l QE���g�p % (�g�p����Ȃ�, �ϐ��͊�{����I�ɒu������)

procedure checkConsistencyBySolveOrRlqe(expr_, vars_)$
begin;
  scalar flag_, ans_, tmp_, mode_;

  debugWrite("checkConsistencyBySolveOrRlqe: ", " ");

% TODO sqrt(2)<>0�����ėp�I�Ȓl�ɓK�p����
% tmp_:=rlqe(ex(vars_, mymkand(expr_) and sqrt(2)<>0));
% debugWrite("tmp_: ", tmp_);
% flag_:= if(tmp_ = true) then rettrue___ else if(tmp_ = false) then retfalse___;
% �ʈ� true�ȊO�̉��͑S��false�Ɣ���
% flag_:= if(ws = true) then rettrue___ else retfalse___;
% tmp_:=rlatl(rlqe(mymkand(expr_)));
% debugWrite("tmp_: ", tmp_);

  % �����������Ă��邩�ǂ����ɂ��A�����̂Ɏg�p����֐�������
  % TODO: ROQE���[�h�ł�����X�g�A��Ԃ��K�v������ꍇ�ւ̑Ή�

  if(hasInequality(expr_)) then mode_:= RLQE else mode_:= SOLVE;
  debugWrite("mode_:", mode_);

  if(mode_=SOLVE) then
  <<
    debugWrite("union(constraintStore_, expr_):", union(constraintStore_, expr_));
    debugWrite("union(csVariables_, vars_):", union(csVariables_, vars_));
    ans_:=solve(union(constraintStore_, expr_), union(csVariables_, vars_));
    debugWrite("ans_ in checkConsistencyBySolveOrRlqe: ", ans_);
    if(ans_ <> {}) then return {rettrue___, ans_} else return {retfalse___};
  >> else
  <<    
    debugWrite("union(constraintStore_, expr_):", union(constraintStore_, expr_));
    ans_:= rlqe(mymkand(union(constraintStore_, expr_)));
    debugWrite("ans_: ", ans_);
    if(ans_ <> false) then return {rettrue___} else return {retfalse___};
  >>;
end;


procedure checkConsistency()$
begin;
  scalar sol_;
  putLineFeed();

  sol_:= checkConsistencyBySolveOrRlqe(constraintStore_, csVariables_);
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
    freeof(expr_, geq) and freeof(expr_,greaterp) and
    freeof(expr_, leq) and freeof(expr_, lessp)) then nil else t$


%TODO �s����
%TODO �p�����[�^������ɗ����Ƃ�

% CCP_SOLVER_ERROR___:= {0};
% CCP_ENTAILED___:= {1};
% CCP_NOT_ENTAILED___:= {2};
% CCP_UNKNOWN___:= {3};

procedure checkentailment(guard_, store_, vars_)$
begin;
  scalar flag_, ans_, sol_, nsol_;
  putLineFeed();

  sol_:=rlqe(guard_ and mymkand store_);
%  nsol_:=rlqe(not guard_ and mymkand store_);
   nsol_:=rlqe(not sol_ and mymkand store_);
  debugWrite("sol_: ", sol_);
  debugWrite("nsol_: ", nsol_);

  if(sol_ neq false) then
% �璷����
    if(nsol_ = false) then
      return CCP_ENTAILED___
    else return {CCP_UNKNOWN___}
  else return CCP_NOT_ENTAILED___;
% Solver Error
  return CCP_SOLVER_ERROR___;
end;

% guard_:=prev(y) = 0;
% store_:={df(prev(y),t,1) = 0 and df(y,t,1) = 0 and df(y,t,2) = -10 and prev(y) = 10 and y = 10};
% vars_:={prev(y), y, prev(y), df(y,t,1), df(prev(y),t,1), df(y,t,2)};
% checkentailment(guard_, store_, vars_);


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
loadedOperator:={};

% ��������init��_��lhs���쐬
procedure makeInitId(f,i)$
if(i=0) then
  mkid(mkid(INIT,f),lhs)
else
  mkid(mkid(mkid(mkid(INIT,f),_),i),lhs);

%laprule�p�Amkid�����������Z�q�Ƃ���
procedure setMkidOperator(f,x)$
  f(x);

% ���v���X�ϊ��̕ϊ��K���̍쐬
% {{v, v(t), lapv(s)},...}�̑Ή��\table_�̍쐬
procedure LaplaceLetUnit(args_)$
begin;
  scalar arg_, LAParg_, laprule_;

  arg_:= first args_;
  LAParg_:= second args_;

  % arg_���d�����ĂȂ�������
  if(freeof(loadedOperator,arg_)) then 
    << 
     operator arg_, LAParg_;
     loadedOperator:= arg_ . loadedOperator;
     operator !~f;

     % makeInitId(f,i)��
     laprule_ :={
       laplace(df(~f(~x),x),x) => il!&*laplace(f(x),x) - makeInitId(f,0),
       laplace(df(~f(~x),x,~n),x) => il!&**n*laplace(f(x),x) -
         for i:=n-1 step -1 until 0 sum
	   makeInitId(f,n-1-i) * il!&**i,
       laplace(~f(~x),x) => setMkidOperator(mkid(lap,f),il!&)
     };
%     % sub��
%     laprule_ :={
%       laplace(df(~f(~x),x),x) => il!&*laplace(f(x),x) - sub(x=0,f(x)),
%       laplace(df(~f(~x),x,~n),x) => il!&**n*laplace(f(x),x) -
%       for i:=n-1 step -1 until 0 sum
%         sub(~x=0, df(f(~x),x,n-1-i)) * il!&**i,
%       laplace(~f(~x),x) => setMkidOperator(mkid(lap,f),il!&)
%     };
     
     let laprule_;
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
  scalar NDExpr_, DExpr_, DExprVars_;

  NDExpr_ := union(for each x in exprs_ join if(isNDExpr(x)) then {x} else {});
  DExpr_ := setdiff(expr_, ndExpr_);
  DExprVars_:= union(for each x in vars_ join if(not freeof(DExpr_, x)) then {x} else {});
  return {NDExpr_, DExpr_, DExprVars_};
end;


procedure getNoDifferentialVars(vars_)$
  union(for each x in vars_ join if(freeof(x, df)) then {x} else {})$


% 20110705 overconstraint___����
ICI_SOLVER_ERROR___:= 0;
ICI_CONSISTENT___:= 1;
ICI_INCONSISTENT___:= 2;
ICI_UNKNOWN___:= 3; % �s�v�H

procedure isConsistentInterval(tmpCons_, expr_, pexpr_, init_, vars_)$
begin;
  scalar tmpSol_, splitExprsResult_, NDExpr_, DExpr_, DExprVars_,
         integTmp_, integTmpQE_, integTmpSol_, infList_, ans_;
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
  
  if(tmpSol_ = retsolvererror___) then return {ICI_SOLVER_ERROR___}
  else if(tmpSol_ = retoverconstraint___) then return {ICI_INCONSISTENT___};

  % NDExpr_��A��
  tmpSol_:= solve(union(tmpSol_, NDExpr_), getNoDifferentialVars(vars_));
  debugWrite("tmpSol_ after solve: ", tmpSol_);

  % tmpCons_���Ȃ��ꍇ�͖������Ɣ��肵�ėǂ�
  if(tmpCons_ = {}) then return {ICI_CONSISTENT___};

  integTmp_:= sub(tmpSol_, tmpCons_);
  debugWrite("integTmp_: ", integTmp_);

  integTmpQE_:= rlqe (mymkand(integTmp_));
  debugWrite("integTmpQE_: ", integTmpQE_);

  % ������true��false�͂��̂܂ܔ��茋�ʂƂȂ�
  if(integTmpQE_ = true) then return {ICI_CONSISTENT___}
  else if(integTmpQE_ = false) then return {ICI_INCONSISTENT___};

  % �Ƃ肠����t�Ɋւ��ĉ����i�����̌`����O��Ƃ��Ă���j
  % TODO:�K�[�h�������s�����̏ꍇ��solve�łȂ��K�؂Ȋ֐��ŉ����K�v������
  % TODO:�K�[�h�����ɓ����ƕs���������݂��Ă�����A�������Ă��炩�H
  % TODO:�_���ςłȂ������`�ւ̑Ή�
  integTmpSol_:= solve(integTmpQE_,t);

  infList_:= union(for each x in integTmpSol_ join checkInfUnit(x, ENTAILMENT___));
  debugWrite("infList_: ", infList_);

  % �p�����^�����Ȃ����1�ɂȂ�͂�
  if(length(infList_) neq 1) then return {ICI_SOLVER_ERROR};
  ans_:= first(infList_);
  debugWrite("ans_: ", ans_);

  if(ans_=true) then return {ICI_CONSISTENT}
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
%symbolic redeval '(isConsistentInterval expr_ init_ vars_);



%%CEI_SOLVER_ERROR___:= {0};
%CEI_ENTAILED___:= {1};
%CEI_NOT_ENTAILED___:= {2};
%CEI_UNKNOWN___:= {3};


procedure checkEntailmentInterval(guard_, store_, init_, vars_, pars_)$
begin;
  scalar tmp_, otherExpr_, tGuard_, tGuardQE_, tGuardSol_, infList_, ans_;
  putLineFeed();

  tmp_:= exDSolve(store_, init_, vars_);
  debugWrite("tmp_: ", tmp_);
  
  otherExpr_:={};

  % "=" ������ => �����ȊO�̐��� �Ƒz��
  for each x in store_ do
    if(freeof(x, equal))
      then otherExpr_:= append(otherExpr_, {x});

  tGuard_:= sub(tmp_,guard_);
  debugWrite("tGuard_: ", tGuard_);

  tGuardQE_:= rlqe (tGuard_);
  debugWrite("tGuardQE_: ", tGuardQE_);

  % ������true��false�͂��̂܂ܔ��茋�ʂƂȂ�
  if(tGuardQE_ = true) then return CEI_ENTAILED___
  else if(tGuardQE_ = false) then return CEI_NOT_ENTAILED___;

  % �Ƃ肠����t�Ɋւ��ĉ����i�����̌`����O��Ƃ��Ă���j
  % TODO:�K�[�h�������s�����̏ꍇ��solve�łȂ��K�؂Ȋ֐��ŉ����K�v������
  % TODO:�K�[�h�����ɓ����ƕs���������݂��Ă�����A�������Ă��炩�H
  % TODO:�_���ςłȂ������`�ւ̑Ή�
  tGuardSol_:= solve(tGuardQE_,t);

  infList_:= union(for each x in tGuardSol_ join checkInfUnit(x, ENTAILMENT___));
  debugWrite("infList_: ", infList_);

  % �p�����^�����Ȃ����1�ɂȂ�͂�
  if(length(infList_) neq 1) then return CEI_SOLVER_ERROR___;
  ans_:= first(infList_);
  debugWrite("ans_: ", ans_);


  %% tGuard ���� otherExpr ���� t>0 �̓��o
  %example_:=sub(tmp_, guard_) and mymkand sub(tmp_, otherExpr_) and t
  %> 0;
  %%  - 5*t**2 + 10 = 0 and true and t > 0$

  %% bballPP���ڂɊւ���OK, 2��ڈȍ~�͉����璷�ɂȂ�ERROR
  %ans_:=rlqe ex(t,example_);

  %debugWrite("rlqe ex(t,example_): ",rlqe ex(t,example_));


  if(ans_=true) then return CEI_ENTAILED___
  else if(ans_=false) then return CEI_NOT_ENTAILED___
  else 
    << 
     debugWrite("rlqe ans: ", ans_);
     return CEI_UNKNOWN___;
    >>;

end;



procedure checkInfUnit(tExpr_, mode_)$
begin;
  scalar infCheckAns_;

  % �O��Frelop(t, �l)�̌`��
  debugWrite("tExpr_: ", tExpr_);

  if(part(tExpr_,0)=equal) then 
    % �K�[�h��������ɂ����Ă͓����̏ꍇ��false
    if(mode_ = ENTAILMENT___) then infCheckAns_:= {false}
    else if(mode_ = MINTIME___) then
      if(mymin(part(tExpr_,2),0) neq part(tExpr_,2)) then infCheckAns_:= {part(tExpr_, 2)}
      else infCheckAns_:= {INFINITY}
  % TODO:�s�����̏ꍇ�ւ̑Ή�
  else 
    if(mode_ = ENTAILMENT___) then infCheckAns_:= {false}
    else if(mode_ = MINTIME___) then infCheckAns_:= {};
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

  minT_:= myFindMinimumNatPPTime(INFINITY, minTList_);
  debugWrite("minT_: ", minT_);

  if(mymin(minT_, maxTime_) neq maxTime_) then ans_:= {minT_, 0}
  else ans_:= {maxTime_, 1}; 
  debugWrite("ans_: ", ans_);

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
  scalar sol_, minTList_, singletonMinTList_;

  debugWrite("in calcMinTime", " ");
  debugWrite("integAsk_: ", integAsk_);

  % false�ɂȂ�悤�ȏꍇ��MinTime���l����K�v���Ȃ�
  if(rlqe(integAsk_) = false) then return {INFINITY};

  % �Ƃ肠����t�Ɋւ��ĉ����i�����̌`����O��Ƃ��Ă���j
  % TODO:�K�[�h�������s�����̏ꍇ��solve�łȂ��K�؂Ȋ֐��ŉ����K�v������
  % TODO:�K�[�h�����ɓ����ƕs���������݂��Ă�����A�������Ă��炩�H
  % TODO:�_���ςłȂ������`�ւ̑Ή�
  sol_:= solve(integAsk_, t);

  minTList_:= union(for each x in sol_ join checkInfUnit(x, MINTIME___));  
  debugWrite("minTList_ in calcMinTime: ", minTList_);

  singletonMinTList_:= {myFindMinimumNatPPTime(Infinity, minTList_)};
  debugWrite("singletonMinTList_: ", singletonMinTList_);

  % �p�����^�����Ȃ����1�ɂȂ�͂�
  if(length(singletonMinTList_) neq 1) then return {error};
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
