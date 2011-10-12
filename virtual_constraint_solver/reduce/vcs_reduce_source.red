load_package sets;

% �O���[�o���ϐ�
% constraintStore_: ���݈����Ă��鐧��W���i���X�g�`���j
constraintStore_ = {};


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

  write "-----myif-----";

  if(x=y) then <<
    write "x=y= ", x;
    if(op = geq_ or op = leq_) then return t
    else return nil
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

  write "margin:= ", margin;

  write "x:= ", x;
  write "y:= ", y;
  write "abs(x-y):= ", abs(x-y);
%x��y���قړ�������
  if(abs(x-y)<margin) then <<off rounded$ precision bak_precision$ write -1; return -1>>;

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

procedure myfind(x,lst)$
%����: IP�J�n���̎���t, ���̎������̃��X�g
%�o��: ����PP�J�n���̎���
if(rest(lst)={}) then
  if(myif(x,lessp_,first(lst),30)) then first(lst) else {}
else if(myif(x,lessp_,first(lst),30))
    then mymin(first(lst),myfind(x,rest(lst)))
else myfind(x,rest(lst))$

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
write t=lt;
write y=ly;
write v=lv;
write fy=lfy;
write fv=lfv;
write ";end;";
shut "out";
on nat;
>>$

%procegure myout(x,t)$

%---------------------------------------------------------------
% HydLa�����֐�
%---------------------------------------------------------------

operator prev;

rettrue___ := "RETTRUE___";
retfalse___ := "RETFALSE___";

% �֐��Ăяo����redeval���o�R������
% <redeval> end:�̎����ŏI�s

symbolic procedure redeval(foo_)$
begin scalar ans_;

  write "<redeval> reval ", (car foo_), ":";
  ans_ :=(reval foo_);
  write "<redeval> end:";

  return ans_;
end;


% PP�ɂ����閳�������̔���
% �Ԃ�l��{ans, {{�ϐ��� = �l},...}} �̌`��
% �d�l QE���g�p % (�g�p����Ȃ�, �ϐ��͊�{����I�ɒu������)

% (���� and���󂯕t���Ȃ�) TODO �����ւ̑Ή�
% (���� true���󂯕t���Ȃ�) TODO �����ւ̑Ή�

procedure isConsistent(expr_,vars_)$
begin;
  scalar flag_, ans_, tmp_, mode_;
write "isConsistent: ";

% TODO sqrt(2)<>0�����ėp�I�Ȓl�ɓK�p����
% tmp_:=rlqe(ex(vars_, mymkand(expr_) and sqrt(2)<>0));
% write "tmp_: ", tmp_;
%  flag_:= if(tmp_ = true) then rettrue___ else if(tmp_ = false) then retfalse___;
% �ʈ� true�ȊO�̉��͑S��false�Ɣ���
% flag_:= if(ws = true) then rettrue___ else retfalse___;
%  tmp_:=rlatl(rlqe(mymkand(expr_)));
%  write "tmp_: ", tmp_;

  % �����������Ă��邩�ǂ����ɂ��A�����̂Ɏg�p����֐�������
  % TODO: ROQE���[�h�ł�����X�g�A��Ԃ��K�v������ꍇ�ւ̑Ή�

  if(hasInequality(expr_)) then mode_:= RLQE else mode_:= SOLVE;
  write("mode_:", mode_);

  if(mode_=SOLVE) then
  <<
    write("expr_:", expr_);
    write("vars_:", vars_);
    ans_:=solve(expr_, vars_);
    write "ans_: ", ans_;
    flag_:= if(ans_ <> {}) then rettrue___ else retfalse___;
    write "flag_: ", flag_;

    return {flag_, ans_};
  >> else
  <<    
    write("expr_:", expr_);
    ans_:= rlqe(mymkand(expr_));
    write "ans_: ", ans_;
    flag_:= if(ans_ <> false) then rettrue___ else retfalse___;
    write "flag_: ", flag_;

    return {flag_};
  >>;
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
  sol_:=rlqe(guard_ and mymkand store_);
%  nsol_:=rlqe(not guard_ and mymkand store_);
   nsol_:=rlqe(not sol_ and mymkand store_);
write "sol_: ", sol_;
write "nsol_: ", nsol_;

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
% TODO prev�́H
table_:={};

% operator�錾���ꂽargs_���L������O���[�o���ϐ�
loadedOperator:={};

%args_:={v,lapv}
procedure LaplaceLetUnit(args_)$
begin;
  scalar arg_, LAParg_;
  arg_:= first args_;
  LAParg_:= second args_;

  % �d���̔���
  if(freeof(loadedOperator,arg_)) then 
    << 
     operator arg_, LAParg_;
     loadedOperator:= arg_ . loadedOperator;

     let{
       laplace(df(arg_(~x),x),x) => il!&*laplace(arg_(x),x) - mkid(mkid(INIT,arg_),lhs),
       laplace(df(arg_(~x),x,~n),x) => il!&**n*laplace(arg_(x),x) -
       for i:=n-1 step -1 until 0 sum
         sub(x=0, df(arg_(x),x,n-1-i)) * il!&**i,
       laplace(arg_(~x),x) =>LAParg_(il!&)
     };
    >>;

% {{v, v(t), lapv(s)},...}�̑Ή��\
  table_:= {arg_, arg_(t), LAParg_(s)} . table_;
  write("table_: ", table_);
end;

% vars_����df�����������̂�Ԃ�
procedure removedf(vars_)$
begin;
  exceptdfvars_:={};
  for each x in vars_ collect
    if(freeof(x,df)) then exceptdfvars_:=x . exceptdfvars_;
  return exceptdfvars_;
end;

retsolvererror___ := "RETSOLVERERROR___";
retoverconstraint___ := "RETOVERCONSTRAINT___";
retunderconstraint___ := "RETUNDERCONSTRAINT___";

procedure exDSolve(expr_, init_, vars_)$
  begin;
    scalar flag_, ans_, tmp_;
 
  exceptdfvars_:= removedf(vars_);
 
  tmp_:= for each x in exceptdfvars_ collect {x,mkid(lap,x)};
  map(LaplaceLetUnit, tmp_);

  %TODO ht => ht(t)�u��
  tmp_:=map(first(~w)=second(~w), table_);
  write("MAP: ", tmp_);

  tmp_:= sub(tmp_, expr_);
  write("SUB: ", tmp_);
  % expr_�𓙎����獷���`����
  
  diffexpr_:={};
  for each x in tmp_ do 
    if(not freeof(x, equal))
      then diffexpr_:= append(diffexpr_, {lhs(x)-rhs(x)})
    % not contained equal case
    else diffexpr_:= append(diffexpr_, {lhs(x)-rhs(x)});
    
    
  LAPexpr_:=map(laplace(~w,t,s), diffexpr_);
  
  % laplace���Z�q�ŃG���[���Alaplace���Z�q���݂̎����Ԃ�Ƒz��
  if(not freeof(LAPexpr_, laplace)) then return retsolvererror___;

%  LAPexpr_:=map(laplace(~w,t,s), diffexpr_);
  write "LAPexpr_: ", LAPexpr_;

  % s�Ɋւ��ĉ����A�t���v���X

  % init_�����LaplaceLetUnit�Ɍ����ϐ����ƑΉ�������
  solveexpr_:= append(LAPexpr_, init_);
  write("solveexpr_:", solveexpr_);

  solvevars_:= append(append(map(third, table_), map(lhs, init_)), {s});
  write("solvevars_:", solvevars_);

  solveans_ := solve(solveexpr_, solvevars_);
  write "solve: ", solveans_;
  % solve���𖳂��̎� overconstraint�Ƒz��
  if(solveans_={}) then return retoverconstraint___;

  % solveans_��solvevars_�̉�����ł��܂܂�Ȃ��� underconstraint�Ƒz��
  for each x in table_ do 
    if(freeof(solveans_, third(x))) then tmp_:=true;
  if(tmp_=true) then return retunderconstraint___;
  
  % write("solvevars is not Free");
  
  % solve����x�Ɋ܂܂�郉�v���X�ϊ��΂���A���ꂼ��̕ϐ��ɑ΂�������������o���B
  ans_:= for each table in table_ collect
      (first table) = invlap(lgetf((third table), solveans_),s,t);
  write("ans expr?: ", ans_);

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


% 20110705 overconstraint___����
%ICI_SOLVER_ERROR___:= {0};
%ICI_ENTAILED___:= {1};
%ICI_CONSTRAINT_ERROR___:= {2};

procedure isConsistentInterval(tmpCons_, expr_, pexpr_, init_, vars_)$
begin;
  scalar tmpSol_, integTmp_, integTmpQE_, integTmpSol_, infList_, ans_;
  tmpSol_:= exDSolve(expr_, init_, vars_);
  write("tmpSol_: ", tmpSol_);
  
  if(tmpSol_ = retsolvererror___) then return {0}
  else if(tmpSol_ = retoverconstraint___) then return {2};

  % tmpCons_���Ȃ��ꍇ�͖������Ɣ��肵�ėǂ�
  if(tmpCons_ = {}) then return {1};

  integTmp_:= sub(tmpSol_, tmpCons_);
  write("integTmp_: ", integTmp_);

  integTmpQE_:= rlqe (mymkand(integTmp_));
  write("integTmpQE_: ", integTmpQE_);

  % ������true��false�͂��̂܂ܔ��茋�ʂƂȂ�
  if(integTmpQE_ = true) then return {1}
  else if(integTmpQE_ = false) then return {2};

  % �Ƃ肠����t�Ɋւ��ĉ����i�����̌`����O��Ƃ��Ă���j
  % TODO:�K�[�h�������s�����̏ꍇ��solve�łȂ��K�؂Ȋ֐��ŉ����K�v������
  % TODO:�K�[�h�����ɓ����ƕs���������݂��Ă�����A�������Ă��炩�H
  % TODO:�_���ςłȂ������`�ւ̑Ή�
  integTmpSol_:= solve(integTmpQE_,t);

  infList_:= union(for each x in integTmpSol_ join checkInfUnit(x, ENTAILMENT___));
  write("infList_: ", infList_);

  % �p�����^�����Ȃ����1�ɂȂ�͂�
  if(length(infList_) neq 1) then return {0};
  ans_:= first(infList_);
  write("ans_: ", ans_);

  if(ans_=true) then return {1}
  else if(ans_=false) then return {2}
  else 
  <<
    write("rlqe ans: ", ans_);
    return CEI_UNKNOWN___;
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
  tmp_:= exDSolve(store_, init_, vars_);

  write("tmp_: ", tmp_);
  
  otherExpr_:={};

  % "=" ������ => �����ȊO�̐��� �Ƒz��
  for each x in store_ do
    if(freeof(x, equal))
      then otherExpr_:= append(otherExpr_, {x});

  tGuard_:= sub(tmp_,guard_);
  write("tGuard_: ", tGuard_);

  tGuardQE_:= rlqe (tGuard_);
  write("tGuardQE_: ", tGuardQE_);

  % ������true��false�͂��̂܂ܔ��茋�ʂƂȂ�
  if(tGuardQE_ = true) then return CEI_ENTAILED___
  else if(tGuardQE_ = false) then return CEI_NOT_ENTAILED___;

  % �Ƃ肠����t�Ɋւ��ĉ����i�����̌`����O��Ƃ��Ă���j
  % TODO:�K�[�h�������s�����̏ꍇ��solve�łȂ��K�؂Ȋ֐��ŉ����K�v������
  % TODO:�K�[�h�����ɓ����ƕs���������݂��Ă�����A�������Ă��炩�H
  % TODO:�_���ςłȂ������`�ւ̑Ή�
  tGuardSol_:= solve(tGuardQE_,t);

  infList_:= union(for each x in tGuardSol_ join checkInfUnit(x, ENTAILMENT___));
  write("infList_: ", infList_);

  % �p�����^�����Ȃ����1�ɂȂ�͂�
  if(length(infList_) neq 1) then return CEI_SOLVER_ERROR___;
  ans_:= first(infList_);
  write("ans_: ", ans_);


  %% tGuard ���� otherExpr ���� t>0 �̓��o
  %example_:=sub(tmp_, guard_) and mymkand sub(tmp_, otherExpr_) and t
  %> 0;
  %%  - 5*t**2 + 10 = 0 and true and t > 0$

  %% bballPP���ڂɊւ���OK, 2��ڈȍ~�͉����璷�ɂȂ�ERROR
  %ans_:=rlqe ex(t,example_);

  %write("rlqe ex(t,example_): ",rlqe ex(t,example_));


  if(ans_=true) then return CEI_ENTAILED___
  else if(ans_=false) then return CEI_NOT_ENTAILED___
  else 
    << 
     write("rlqe ans: ", ans_);
     return CEI_UNKNOWN___;
    >>;

end;



procedure checkInfUnit(tExpr_, mode_)$
begin;
  scalar infCheckAns_;
  % �O��Frelop(t, �l)�̌`��
  write("tExpr_: ", tExpr_);

  if(part(tExpr_,0)=equal) then 
    % �K�[�h��������ɂ����Ă͓����̏ꍇ��false
    if(mode_ = ENTAILMENT___) then infCheckAns_:= {false}
    else if(mode_ = MINTIME___) then
      if(mymin(part(tExpr_,2),0) neq part(tExpr_,2)) then infCheckAns_:= {part(tExpr_, 2)}
      else infCheckAns_:= {}
  % TODO:�s�����̏ꍇ�ւ̑Ή�
  else 
    if(mode_ = ENTAILMENT___) then infCheckAns_:= {false}
    else if(mode_ = MINTIME___) then infCheckAns_:= {};
  write("infCheckAns_: ", infCheckAns_);

  return infCheckAns_;
end;



IC_SOLVER_ERROR___:= 0;
IC_NORMAL_END___:= 1;

procedure integrateCalc(cons_, init_, discCause_, vars_, maxTime_)$
begin;
  scalar tmpSol_, tmpDiscCause_, 
         retCode_, tmpVarMap_, tmpMinT_, integAns_;
  tmpSol_:= exDSolve(cons_, init_, vars_);
  write("tmpSol_:", tmpSol_);

  % TODO:Solver error����

  tmpDiscCause_:= sub(tmpSol_, discCause_);
  write("tmpDiscCause_:", tmpDiscCause);

  tmpVarMap_:= first(myFoldLeft(createIntegratedValue, {{},tmpSol_}, vars_)); 
  write("tmpVarMap_:", tmpVarMap_);

  tmpMinT_:= calcNextPointPhaseTime(maxTime_, tmpDiscCause_);
  write("tmpMinT_:", tmpMinT_);
  if(tmpMinT_ = error) then retCode_:= IC_SOLVER_ERROR___
  else retCode_:= IC_NORMAL_END___;

  % TODO:tmpMinT_�͕�������������悤�ɂ���
  integAns_:= {retCode_, tmpVarMap_, {tmpMinT_}};
  write("integAns_", integAns_);
  
  return integAns_;
end;



procedure createIntegratedValue(pairInfo_, variable_)$
begin;
  scalar retList_, integRule_, integExpr_, newRetList_;
  retList_:= first(pairInfo_);
  integRule_:= second(pairInfo_);

  integExpr_:= {variable_, sub(integRule_, variable_)};
  write("integExpr_: ", integExpr_);

  newRetList_:= cons(integExpr_, retList_);
  write("newRetList_: ", newRetList_);

  return {newRetList_, integRule_};
end;



procedure calcNextPointPhaseTime(maxTime_, discCause_)$
begin;
  scalar minTList_, minT_, ans_;

  minTList_:= union(for each x in discCause_ join calcMinTime(x));
  write("minTList_: ", minTList_);

  if(not freeof(minTList_, error)) then return error;

  minT_:= myfind(0, minTList_);
  write("minT_: ", minT_);

  if(mymin(minT_, maxTime_) = minT_) then ans_:= {minT_, 0}
  else ans_:= {maxTime_, 1}; 
  write("ans_: ", ans_);

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
  scalar sol_, minTList_;
  write("in calcMinTime");
  write("integAsk_: ", integAsk_);

  if(integAsk_ = false) then return {}; % ����Ԃ��ׂ����H

  % �Ƃ肠����t�Ɋւ��ĉ����i�����̌`����O��Ƃ��Ă���j
  % TODO:�K�[�h�������s�����̏ꍇ��solve�łȂ��K�؂Ȋ֐��ŉ����K�v������
  % TODO:�K�[�h�����ɓ����ƕs���������݂��Ă�����A�������Ă��炩�H
  % TODO:�_���ςłȂ������`�ւ̑Ή�
  sol_:= solve(integAsk_, t);

  minTList_:= union(for each x in sol_ join checkInfUnit(x, MINTIME___));  
  write("minTList_: ", minTList_);

  % �p�����^�����Ȃ����1�ɂȂ�͂�
  if(length(minTList_) neq 1) then return {error};
  return minTList_;

end;



procedure getRealVal(value_, prec_)$
begin;
  scalar tmp_, defaultPrec_;
  defaultPrec:= precision(0)$
  on rounded$
  precision(prec_);
  tmp_:= value_;
  write("tmp_:", tmp_);
  precision(defaultPrec_)$
  off rounded$

  return tmp_;
end;



%TODO �G���[���o�i�K�p�������ʎ����ȊO�ɂȂ����ꍇ���j

procedure applyTime2Expr(expr_, time_)$
begin;
  scalar appliedExpr_;
  appliedExpr_:= sub(t=time_, expr_);
  write("appliedExpr_:", appliedExpr_);

  return {1, appliedExpr_};
end;



procedure exprTimeShift(expr_, time_)$
begin;
  scalar shiftedExpr_;
  shiftedExpr_:= sub(t=t-time_, expr_);
  write("shiftedExpr_:", shiftedExpr_);

  return shiftedExpr_;
end;



%load_package "assist";

procedure simplifyExpr(expr_)$
begin;
  scalar simplifiedExpr_;
  % TODO:simplify�֐����g��
%  simplifiedExpr_:= simplify(expr_);
  simplifiedExpr_:= expr_;
  write("simplifiedExpr_:", simplifiedExpr_);

  return simplifiedExpr_;
end;



procedure checkLessThan(lhs_, rhs_)$
begin;
  scalar ret_;
  ret_:= if(mymin(lhs_, rhs_) = lhs_) then rettrue___ else retfalse___;
  write("ret_:", ret_);

  return ret_;
end;



procedure getSExpFromString(str_)$
begin;
  scalar retSExp_;
  retSExp_:= str_;
  write("retSExp_:", retSExp_);

  return retSExp_;
end;


%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%
expr_:= {yyy = -10, y = 10, yy = 0, z = y, zz = yy}; vars_:={y, z, yy, zz, yyy, y, z, yy, zz};
symbolic redeval '(isconsistent expr_ vars_);
clear expr_, pexpr_, vars_;


;end;
