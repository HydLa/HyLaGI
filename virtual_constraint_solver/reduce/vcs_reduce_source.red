load_package sets;

%MathematicaでいうFold関数
procedure myFoldLeft(func_, init_, list_)$
  if(list_ = {}) then init_
  else myFoldLeft(func_, func_(init_, first(list_)), rest(list_))$

procedure myif(x,op,y,approx_precision)$
%入力: 論理式(ex. sqrt(2), greaterp_, sin(2)), 精度
%出力: t or nil or -1
%      (xとyがほぼ等しい時 -1)
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

% 10^(3 + yかxの指数部の値 - 有効桁数)
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
%xとyがほぼ等しい時
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
%入力: 変数名, 等式のリストのリスト(ex. {{x=1,y=2},{x=3,y=4},...})
%出力: 変数に対応する値のリスト
if(llst={}) then {}
	else if(rest(llst)={}) then getf(x,first(llst))
		else getf(x,first(llst)) . {lgetf(x,rest(llst))}$

procedure mymin(x,y)$
if(x={}) then
	if(y={}) then {} else y
  else	if(y={}) then x
	else if(myif(x,greaterp_,y,30)) then y else x$

procedure myfind(x,lst)$
%入力: IP開始時の時刻t, 次の時刻候補のリスト
%出力: 次のPP開始時の時刻
if(rest(lst)={}) then
  if(myif(x,lessp_,first(lst),30)) then first(lst) else {}
else if(myif(x,lessp_,first(lst),30))
    then mymin(first(lst),myfind(x,rest(lst)))
else myfind(x,rest(lst))$

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

procedure myex(lst,var)$
rlqe ex(var, mymkand(lst));

procedure myall(lst,var)$
rlqe all(var, mymkand(lst));


procedure bball_out()$
% gnuplot用出力, 未完成
% 正規表現 {|}|\n
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
% HydLa向け関数
%---------------------------------------------------------------

operator prev;

rettrue___ := "RETTRUE___";
retfalse___ := "RETFALSE___";

% 関数呼び出しはredevalを経由させる
% <redeval> end:の次が最終行

symbolic procedure redeval(foo_)$
begin scalar ans_;

  write "<redeval> reval ", (car foo_), ":";
  ans_ :=(reval foo_);
  write "<redeval> end:";

  return ans_;
end;


% PPにおける無矛盾性の判定
% 返り値は{ans, {{変数名 = 値},...}} の形式
% 仕様 QE未使用 % (使用するなら, 変数は基本命題的に置き換え)

% (制限 andを受け付けない) TODO 制限への対応
% (制限 trueを受け付けない) TODO 制限への対応

procedure isConsistent(vars_,pexpr_,expr_)$
begin;
  scalar flag_, ans_, tmp_;
write "isConsistent: ";

% TODO sqrt(2)<>0をより汎用的な値に適用する
% tmp_:=rlqe(ex(vars_, mymkand(expr_) and sqrt(2)<>0));
% write "tmp_: ", tmp_;
%  flag_:= if(tmp_ = true) then rettrue___ else if(tmp_ = false) then retfalse___;
% 別案 true以外の解は全てfalseと判定
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


%TODO 不等式
%TODO パラメータが分母に来たとき

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
% 冗長かも
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
% TODO ×定数を返すだけ

% orexqr_:={df(prev(y),t,1) = 0 and df(y,t,1) = 0 and df(y,t,2) = -10 and prev(y) = 10 and y = 10};
% symbolic reval '(createcstovm orexpr_);
procedure createcstovm(orexpr_)$
begin;
  scalar flag_, ans_, tmp_;

  

  return {0, -10, 10};
end;








load_package "laplace";
% 逆ラプラス変換後の値をsin, cosで表示するスイッチ
on ltrig;

% 変数とそのラプラス変換対の対応表
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

% {{v, v(t), lapv(s)},...}の対応表
  table_:= {arg_, arg_(t), LAParg_(s)} . table_;
  write("table_: ", table_);
end;

% vars_からdfを除いたものを返す
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

  %TODO ht => ht(t)置換
  tmp_:=map(first(~w)=second(~w), table_);
  write("MAP: ", tmp_);

  tmp_:= sub(tmp_, expr_);
  write("SUB: ", tmp_);
  % expr_を等式から差式形式に
  
  diffexpr_:={};
  for each x in tmp_ do 
    if(not freeof(x, equal))
      then diffexpr_:= append(diffexpr_, {lhs(x)-rhs(x)})
    % not contained equal case
    else diffexpr_:= append(diffexpr_, {lhs(x)-rhs(x)});
    
    
  LAPexpr_:=map(laplace(~w,t,s), diffexpr_);
  
  % laplace演算子でエラー時、laplace演算子込みの式が返ると想定
  if(not freeof(LAPexpr_, laplace)) then return retsolvererror___;

%  LAPexpr_:=map(laplace(~w,t,s), diffexpr_);
  write "LAPexpr_: ", LAPexpr_;

  % sに関して解く、逆ラプラス

  % init_制約をLaplaceLetUnitに現れる変数名と対応させる
  solveexpr_:= append(LAPexpr_, init_);
  write("solveexpr_:", solveexpr_);

  solvevars_:= append(append(map(third, table_), map(lhs, init_)), {s});
  write("solvevars_:", solvevars_);

  solveans_ := solve(solveexpr_, solvevars_);
  write "solve: ", solveans_;
  % solveが解無しの時 overconstraintと想定
  if(solveans_={}) then return retoverconstraint___;

  % solveans_にsolvevars_の解が一つでも含まれない時 underconstraintと想定
  for each x in table_ do 
    if(freeof(solveans_, third(x))) then tmp_:=true;
  if(tmp_=true) then return retunderconstraint___;
  
  % write("solvevars is not Free");
  
  % solve結果xに含まれるラプラス変換対から、それぞれの変数に対する方程式を取り出す。
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


% 20110705 overconstraint___無し
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

  % "=" が無い => 等式以外の制約 と想定
  for each x in store_ do
    if(freeof(x, equal))
      then otherExpr_:= append(otherExpr_, {x});

  tGuard_:= sub(tmp_,guard_);
  write("tGuard_: ", tGuard_);

  tGuardQE_:= rlqe (tGuard_);
  write("tGuardQE_: ", tGuardQE_);

  % ただのtrueやfalseはそのまま判定結果となる
  if(tGuardQE_ = true) then return CEI_ENTAILED___
  else if(tGuardQE_ = false) then return CEI_NOT_ENTAILED___;

  % とりあえずtに関して解く
  % TODO:ガード条件が不等式の場合はsolveでなく適切な関数で解く必要があ
  % る
  % TODO:ガード条件に等式と不等式が混在していたら、分解してからか？
  tGuardSol_:= solve(tGuardQE_,t);

  infList_:= union(for each x in tGuardSol_ join checkInfUnit(x));
  write("infList_: ", infList_);

  % パラメタ無しなら解は1つになるはず
  if(length(infList_) neq 1) then return CEI_SOLVER_ERROR___;
  ans_:= first(infList_);
  write("ans_: ", ans_);


  %% tGuard かつ otherExpr かつ t>0 の導出
  %example_:=sub(tmp_, guard_) and mymkand sub(tmp_, otherExpr_) and t
  %> 0;
  %%  - 5*t**2 + 10 = 0 and true and t > 0$

  %% bballPP一回目に関してOK, 2回目以降は解が冗長になりERROR
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
  % 等式の場合はfalse
  if(part(tExpr_,0)=equal) then infCheckAns_:= {false}
  % TODO:不等式の場合への対応
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

  % TODO:Solver error処理

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


%  % TODO:list部分をなんとかする
%  ans_:= fold(calcMinTime, {maxTime_, {}}, {posAsk_, negAsk_, NACons_});

  minTList_:= map(calcMinTime, union(union(posAsk_, negAsk_), NACons_));
  ans_:= myfind(0, minTList_);
  write("ans_", ans_);

  return ans_;
end;



% Fold用

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



%TODO エラー検出（適用した結果実数以外になった場合等）

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



%TODO trueまたはfalseを返すようにする

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
