load_package sets;

% グローバル変数
% constraintStore_: 現在扱っている制約集合（リスト形式）
constraintStore_ = {};


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

procedure isConsistent(expr_,vars_)$
begin;
  scalar flag_, ans_, tmp_, mode_;
write "isConsistent: ";

% TODO sqrt(2)<>0をより汎用的な値に適用する
% tmp_:=rlqe(ex(vars_, mymkand(expr_) and sqrt(2)<>0));
% write "tmp_: ", tmp_;
%  flag_:= if(tmp_ = true) then rettrue___ else if(tmp_ = false) then retfalse___;
% 別案 true以外の解は全てfalseと判定
% flag_:= if(ws = true) then rettrue___ else retfalse___;
%  tmp_:=rlatl(rlqe(mymkand(expr_)));
%  write "tmp_: ", tmp_;

  % 等式が入っているかどうかにより、解くのに使用する関数を決定
  % TODO: ROQEモードでも制約ストアを返す必要がある場合への対応

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

% {{v, v(t), lapv(s)},...}の対応表、グローバル変数
% exDSolveのexceptdfvars_に対応させるため、exDSolve最後で空集合を代入し初期化している
% TODO prevは？
table_:={};

% operator宣言されたargs_を記憶するグローバル変数
loadedOperator:={};

%args_:={v,lapv}
procedure LaplaceLetUnit(args_)$
begin;
  scalar arg_, LAParg_;
  arg_:= first args_;
  LAParg_:= second args_;

  % 重複の判定
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

procedure isConsistentInterval(tmpCons_, expr_, pexpr_, init_, vars_)$
begin;
  scalar tmpSol_, integTmp_, integTmpQE_, integTmpSol_, infList_, ans_;
  tmpSol_:= exDSolve(expr_, init_, vars_);
  write("tmpSol_: ", tmpSol_);
  
  if(tmpSol_ = retsolvererror___) then return {0}
  else if(tmpSol_ = retoverconstraint___) then return {2};

  % tmpCons_がない場合は無矛盾と判定して良い
  if(tmpCons_ = {}) then return {1};

  integTmp_:= sub(tmpSol_, tmpCons_);
  write("integTmp_: ", integTmp_);

  integTmpQE_:= rlqe (mymkand(integTmp_));
  write("integTmpQE_: ", integTmpQE_);

  % ただのtrueやfalseはそのまま判定結果となる
  if(integTmpQE_ = true) then return {1}
  else if(integTmpQE_ = false) then return {2};

  % とりあえずtに関して解く（等式の形式を前提としている）
  % TODO:ガード条件が不等式の場合はsolveでなく適切な関数で解く必要がある
  % TODO:ガード条件に等式と不等式が混在していたら、分解してからか？
  % TODO:論理積でつながった形への対応
  integTmpSol_:= solve(integTmpQE_,t);

  infList_:= union(for each x in integTmpSol_ join checkInfUnit(x, ENTAILMENT___));
  write("infList_: ", infList_);

  % パラメタ無しなら解は1つになるはず
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

  % とりあえずtに関して解く（等式の形式を前提としている）
  % TODO:ガード条件が不等式の場合はsolveでなく適切な関数で解く必要がある
  % TODO:ガード条件に等式と不等式が混在していたら、分解してからか？
  % TODO:論理積でつながった形への対応
  tGuardSol_:= solve(tGuardQE_,t);

  infList_:= union(for each x in tGuardSol_ join checkInfUnit(x, ENTAILMENT___));
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



procedure checkInfUnit(tExpr_, mode_)$
begin;
  scalar infCheckAns_;
  % 前提：relop(t, 値)の形式
  write("tExpr_: ", tExpr_);

  if(part(tExpr_,0)=equal) then 
    % ガード条件判定においては等式の場合はfalse
    if(mode_ = ENTAILMENT___) then infCheckAns_:= {false}
    else if(mode_ = MINTIME___) then
      if(mymin(part(tExpr_,2),0) neq part(tExpr_,2)) then infCheckAns_:= {part(tExpr_, 2)}
      else infCheckAns_:= {}
  % TODO:不等式の場合への対応
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

  % TODO:Solver error処理

  tmpDiscCause_:= sub(tmpSol_, discCause_);
  write("tmpDiscCause_:", tmpDiscCause);

  tmpVarMap_:= first(myFoldLeft(createIntegratedValue, {{},tmpSol_}, vars_)); 
  write("tmpVarMap_:", tmpVarMap_);

  tmpMinT_:= calcNextPointPhaseTime(maxTime_, tmpDiscCause_);
  write("tmpMinT_:", tmpMinT_);
  if(tmpMinT_ = error) then retCode_:= IC_SOLVER_ERROR___
  else retCode_:= IC_NORMAL_END___;

  % TODO:tmpMinT_は複数時刻扱えるようにする
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



% Fold用

procedure calcMinTime(currentMinPair_, newTriple_)$
begin;
  scalar currentMinT_, currentMinTriple_, sol_, minT_;


end;



% Map用

procedure calcMinTime(integAsk_)$
begin;
  scalar sol_, minTList_;
  write("in calcMinTime");
  write("integAsk_: ", integAsk_);

  if(integAsk_ = false) then return {}; % ∞を返すべきか？

  % とりあえずtに関して解く（等式の形式を前提としている）
  % TODO:ガード条件が不等式の場合はsolveでなく適切な関数で解く必要がある
  % TODO:ガード条件に等式と不等式が混在していたら、分解してからか？
  % TODO:論理積でつながった形への対応
  sol_:= solve(integAsk_, t);

  minTList_:= union(for each x in sol_ join checkInfUnit(x, MINTIME___));  
  write("minTList_: ", minTList_);

  % パラメタ無しなら解は1つになるはず
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
  % TODO:simplify関数を使う
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
