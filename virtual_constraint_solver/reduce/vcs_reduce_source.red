load_package sets;

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

procedure isConsistent(vars_,pexpr_,expr_)$
begin;
  scalar flag_, ans_, tmp_;
write "isConsistent: ";

% TODO sqrt(2)<>0�����ėp�I�Ȓl�ɓK�p����
% tmp_:=rlqe(ex(vars_, mymkand(expr_) and sqrt(2)<>0));
% write "tmp_: ", tmp_;
%  flag_:= if(tmp_ = true) then rettrue___ else if(tmp_ = false) then retfalse___;
% �ʈ� true�ȊO�̉��͑S��false�Ɣ���
% flag_:= if(ws = true) then rettrue___ else retfalse___;
%  tmp_:=rlatl(rlqe(mymkand(expr_)));
%  write "tmp_: ", tmp_;

%  ans_:=solve(expr_, vars_);
  ans_:=solve(expr_, vars_);
write "ans_: ", ans_;
  flag_:= if(ans_ <> {}) then rettrue___ else retfalse___;
write "flag_: ", flag_;

  return {flag_, ans_};
end;


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

% �ϐ��Ƃ��̃��v���X�ϊ��΂̑Ή��\
table_:={};

%args_:={{v,lapv},...}
procedure LaplaceLetUnit(args_)$
  begin;
    scalar arg_, LAParg_;
  arg_:= first args_;
  LAParg_:= second args_;
  operator arg_, LAParg_;

  let{
    laplace(df(arg_(~x),x),x) => il!&*laplace(arg_(x),x) - mkid(mkid(INIT,arg_),lhs),
    laplace(df(arg_(~x),x,~n),x) => il!&**n*laplace(arg_(x),x) -
    for i:=n-1 step -1 until 0 sum
      sub(x=0, df(arg_(x),x,n-1-i)) * il!&**i,
    laplace(arg_(~x),x) =>LAParg_(il!&)
  };

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

procedure isConsistentInterval(expr_, init_, vars_)$
begin;
  scalar tmp_;
  tmp_:= exDSolve(expr_, init_, vars_);
  
  if(tmp_ = retsolvererror___) then return {0}
  else if(tmp_ = retoverconstraint___) then return {2}
  else return {1};
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

  % �Ƃ肠����t�Ɋւ��ĉ���
  % TODO:�K�[�h�������s�����̏ꍇ��solve�łȂ��K�؂Ȋ֐��ŉ����K�v����
  % ��
  % TODO:�K�[�h�����ɓ����ƕs���������݂��Ă�����A�������Ă��炩�H
  tGuardSol_:= solve(tGuardQE_,t);

  infList_:= union(for each x in tGuardSol_ join checkInfUnit(x));
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



procedure checkInfUnit(tExpr_)$
begin;
  scalar infCheckAns_;
  write("tExpr_: ", tExpr_);
%  if(mymin(part(tExpr_,2),0) = 0) then return {};
  % �����̏ꍇ��false
  if(part(tExpr_,0)=equal) then infCheckAns_:= {false}
  % TODO:�s�����̏ꍇ�ւ̑Ή�
  else infCheckAns_:= {false};
  write("infCheckAns_: ", infCheckAns_);

  return infCheckAns_;
end;



%IC_SOLVER_ERROR___:= {0};
%IC_NORMAL_END___:= {1};

procedure integrateCalc(cons_, init_, posAsk_, negAsk_, NACons_, vars_, maxTime_)$
begin;
  scalar tmpSol_, tmpPosAsk_, tmpNegAsk_, tmpNACons_, 
         tmpVarMap_, tmpMinT_, integAns_;
  tmpSol_:= exDSolve(cons_, init_, vars_);
  write("tmpSol_:", tmpSol_);

  % TODO:Solver error����

  let{tmpSol_};
  tmpPosAsk_:= posAsk_;
  tmpNegAsk_:= negAsk_;
  tmpNACons_:= NACons_;
  write("tmpPosAsk_:", tmpPosAsk_, 
        "tmpNegAsk_:", tmpNegAsk_,
        "tmpNACons_:", tmpNACons_);

  tmpVarMap_:=tmpSol_;
  write("tmpVarMap_:", tmpVarMap_);

  tmpMinT_:= calcNextPointPhaseTime(maxTime_, tmpPosAsk_, tmpNegAsk_, tmpNACons_);
  write("tmpMinT_:", tmpMinT_);

  integAns_:= {1, tmpVarMap_, tmpMinT_};
  write("integAns_", integAns_);
  
  return integAns_;
end;



procedure calcNextPointPhaseTime(maxTime_, posAsk_, negAsk_, NACons_)$
begin;
  scalar minTList_, ans_;


%  % TODO:list�������Ȃ�Ƃ�����
%  ans_:= fold(calcMinTime, {maxTime_, {}}, {posAsk_, negAsk_, NACons_});

  minTList_:= map(calcMinTime, union(union(posAsk_, negAsk_), NACons_));
  ans_:= myfind(0, minTList_);
  write("ans_", ans_);

  return ans_;
end;



% Fold�p

procedure calcMinTime(currentMinPair_, newTriple_)$
begin;
  scalar currentMinT_, currentMinTriple_, sol_, minT_;



end;



procedure getRealVal(value_, prec_)$
begin;
  scalar tmp_;
  on rounded;
  precision(prec_);
  tmp_:= value_;
  write("tmp_:", tmp_);
  off rounded;

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
  simplifiedExpr_:= simplify(expr_);
  write("simplifiedExpr_:", simplifiedExpr_);

  return simplifiedExpr_;
end;



%TODO true�܂���false��Ԃ��悤�ɂ���

procedure checkLessThan(lhs_, rhs_)$
begin;
  scalar ret_;
  ret_:= if(lhs_ < rhs_) then rettrue___ else retfalse___;
  write("ret_:", ret_);

  return ret_;
end;


%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%
expr_:= {yyy = -10, y = 10, yy = 0, z = y, zz = yy}; vars_:={y, z, yy, zz, yyy, y, z, yy, zz};
symbolic redeval '(isconsistent vars_ pexpr_ expr_);
clear expr_, pexpr_, vars_;


;end;
