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
%TODO depend��j��
depend {y, ht, v}, t;

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
  sol_:=rlqe(guard_ and first store_);
%  nsol_:=rlqe(not guard_ and first store_);
   nsol_:=rlqe(not sol_ and first store_);
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


% in "/home/yysaki/workspace/HydLa/virtual_constraint_solver/reduce/vcs_reduce_source.red";

load_package "laplace";

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
  }
end;

%�P���V���E����̎��̊֐���
procedure exDSolve(fv, v_0, fy, y_0)$
begin;
  scalar flag_, ans_, tmp_;

%STEP1. load, let, operator�錾
vars_:={y,v};
tmp_:= for each x in vars_ collect {x,mkid(lap,x)};
map(LaplaceLetUnit, tmp_);

%STEP2. ���v���X�ϊ�
	%memo: foreach��mkid�ň�ʉ��o����
LAPEXPRv:= laplace(fv,t,s);
write "LAPEXPRv: ", LAPEXPRv;

INITv   := INITvlhs - v_0;
write "INITv: ", INITv;
LAPEXPRy:= laplace(fy,t,s);
write "LAPEXPRy: ", LAPEXPRy;
INITy   := INITylhs - y_0;
write "INITy: ", INITy;

%STEP3. s�Ɋւ��ĉ����A�t���v���X
	%memo: lapv(s)�̕\�L���l�b�N
x := solve({LAPEXPRv, LAPEXPRy, INITv, INITy}, {lapv(s),lapy(s),INITvlhs,INITylhs,s});
write "solve: ", x;
tmp := lgetf(lapv(s), x);
vexp := invlap(tmp,s,t);

tmp := lgetf(lapy(s), x);
yexp := invlap(tmp,s,t);

return {v(t)=vexp, y(t)=yexp};
end;

operator y,v;
fv := (df(v(t),t) - (-10)); v_0:=0;
fy := (df(y(t),t) - v(t));  y_0:=10;
%fv := (df(v(t),t,2) - (-10));
exDSolve(fv, v_0, fy, y_0);

clear fv, v_0, fy, y_0;




%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%
expr_:= {yyy = -10, y = 10, yy = 0, z = y, zz = yy}; vars_:={y, z, yy, zz, yyy, y, z, yy, zz};
symbolic redeval '(isconsistent vars_ pexpr_ expr_);
clear expr_, pexpr_, vars_;

;end;

