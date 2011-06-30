procedure myif(x,op,y,approx_precision)$
%����: ������(ex. sqrt(2), greaterp_, sin(2)), ����
%����: t or nil or -1
%      (x��y���ۤ��������� -1)
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

% 10^(3 + y��x�λؿ������� - ͭ�����)
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
%x��y���ۤ���������
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
%����: �ѿ�̾, �����Υꥹ�ȤΥꥹ��(ex. {{x=1,y=2},{x=3,y=4},...})
%����: �ѿ����б������ͤΥꥹ��
if(llst={}) then {}
	else if(rest(llst)={}) then getf(x,first(llst))
		else getf(x,first(llst)) . {lgetf(x,rest(llst))}$

procedure mymin(x,y)$
if(x={}) then
	if(y={}) then {} else y
  else	if(y={}) then x
	else if(myif(x,greaterp_,y,30)) then y else x$

procedure myfind(x,lst)$
%����: IP���ϻ��λ���t, ���λ������Υꥹ��
%����: ����PP���ϻ��λ���
if(rest(lst)={}) then
  if(myif(x,lessp_,first(lst),30)) then first(lst) else {}
else if(myif(x,lessp_,first(lst),30))
    then mymin(first(lst),myfind(x,rest(lst)))
else myfind(x,rest(lst))$

%�Ԥ�����I�ط�
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

%�����Υꥹ�Ȥ�and�ǷҤ������������Ѵ�����
procedure mymkand(lst)$
for i:=1:length(lst) mkand part(lst,i);

procedure myex(lst,var)$
rlqe ex(var, mymkand(lst));

procedure myall(lst,var)$
rlqe all(var, mymkand(lst));


procedure bball_out()$
% gnuplot�ѽ���, ̤����
% ����ɽ�� {|}|\n
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
% HydLa�����ؿ�
%---------------------------------------------------------------

operator prev;
%TODO depend���˴�
depend {y, ht, v}, t;

rettrue___ := "RETTRUE___";
retfalse___ := "RETFALSE___";

% �ؿ��ƤӽФ���redeval���ͳ������
% <redeval> end:�μ����ǽ���

symbolic procedure redeval(foo_)$
begin scalar ans_;

  write "<redeval> reval ", (car foo_), ":";
  ans_ :=(reval foo_);
  write "<redeval> end:";

  return ans_;
end;


% PP�ˤ�����̵̷������Ƚ��
% �֤��ͤ�{ans, {{�ѿ�̾ = ��},...}} �η���
% ���� QE̤���� % (���Ѥ���ʤ�, �ѿ��ϴ���̿��Ū���֤�����)

% (���� and������դ��ʤ�) TODO ���¤ؤ��б�
% (���� true������դ��ʤ�) TODO ���¤ؤ��б�

procedure isConsistent(vars_,pexpr_,expr_)$
begin;
  scalar flag_, ans_, tmp_;
write "isConsistent: ";

% TODO sqrt(2)<>0��������Ū���ͤ�Ŭ�Ѥ���
% tmp_:=rlqe(ex(vars_, mymkand(expr_) and sqrt(2)<>0));
% write "tmp_: ", tmp_;
%  flag_:= if(tmp_ = true) then rettrue___ else if(tmp_ = false) then retfalse___;
% �̰� true�ʳ��β������false��Ƚ��
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


%TODO ������
%TODO �ѥ�᡼����ʬ����褿�Ȥ�

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
% ��Ĺ����
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
% TODO ��������֤�����

% orexqr_:={df(prev(y),t,1) = 0 and df(y,t,1) = 0 and df(y,t,2) = -10 and prev(y) = 10 and y = 10};
% symbolic reval '(createcstovm orexpr_);
procedure createcstovm(orexpr_)$
begin;
  scalar flag_, ans_, tmp_;

  

  return {0, -10, 10};
end;








load_package "laplace";
% �ե�ץ饹�Ѵ�����ͤ�sin, cos��ɽ�����륹���å�
on ltrig;

% �ѿ��Ȥ��Υ�ץ饹�Ѵ��Ф��б�ɽ
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

% {{v, v(t), lapv(s)},...}���б�ɽ
  table_:= {arg_, arg_(t), LAParg_(s)} . table_;
  write("table_: ", table_);
end;

% vars_����df���������Τ��֤�
procedure removedf(vars_)$
begin;
  exceptdfvars_:={};
  for each x in vars_ collect
    if(freeof(x,df)) then exceptdfvars_:=x . exceptdfvars_;
  return exceptdfvars_;
end;

retsolvererror___ := "RETSOLVERERROR___";

procedure exDSolve(expr_, init_, vars_)$
  begin;
    scalar flag_, ans_, tmp_;
 
  exceptdfvars_:= removedf(vars_);
 
  tmp_:= for each x in exceptdfvars_ collect {x,mkid(lap,x)};
  map(LaplaceLetUnit, tmp_);

  %TODO ht => ht(t)�ִ�
  tmp_:=map(first(~w)=second(~w), table_);
  write("MAP: ", tmp_);

  tmp_:= sub(tmp_, expr_);
  write("SUB: ", tmp_);
  % expr_���������麹��������
  
  diffexpr_:={};
  for each x in tmp_ do 
    if(not freeof(x, equal))
      then diffexpr_:= append(diffexpr_, {lhs(x)-rhs(x)})
    % not contained equal case
    else diffexpr_:= append(diffexpr_, {lhs(x)-rhs(x)});
    
    
  LAPexpr_:=map(laplace(~w,t,s), diffexpr_);
  
  % laplace�黻�Ҥǥ��顼����laplace�黻�ҹ��ߤμ����֤������
  if(not freeof(LAPexpr_, laplace)) then return retsolvererror___;

%  LAPexpr_:=map(laplace(~w,t,s), diffexpr_);
  write "LAPexpr_: ", LAPexpr_;

  % s�˴ؤ��Ʋ򤯡��ե�ץ饹

  % init_�����LaplaceLetUnit�˸�����ѿ�̾���б�������
  solveexpr_:= append(LAPexpr_, init_);
  write("solveexpr_:", solveexpr_);

  solvevars_:= append(append(map(third, table_), map(lhs, init_)), {s});
  write("solvevars_:", solvevars_);

  solveans_ := solve(solveexpr_, solvevars_);
  write "solve: ", solveans_;
  % solve����̵���λ�(overconstraint?)
  if(solveans_={}) then return "SOLVEERROR_";

  % solve���x�˴ޤޤ���ץ饹�Ѵ��Ф��顢���줾����ѿ����Ф�������������Ф���
  ans_:= for each table in table_ collect
      (first table) = invlap(lgetf((third table), solveans_),s,t);
  write("ans expr?: ", ans_);

  table_:={};
  return ans_;
end;

%depend ht,v;
%expr_:={df(ht,t) = v,
%        df(v,t) = -10
%       };
%init_:={inithtlhs = 10,
%        initvlhs = 0
%       };
%vars_:={ht,v,df(ht,t),df(v,t)};
%exDSolve(expr_, init_, vars_);





%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%
expr_:= {yyy = -10, y = 10, yy = 0, z = y, zz = yy}; vars_:={y, z, yy, zz, yyy, y, z, yy, zz};
symbolic redeval '(isconsistent vars_ pexpr_ expr_);
clear expr_, pexpr_, vars_;


;end;

