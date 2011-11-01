load_package sets;

% グローバル変数
% constraintStore_: 現在扱っている制約集合（リスト形式）
% csVariables_: 制約ストア内に出現する変数の一覧（定数未対応）
%
% optUseDebugPrint_: デバッグ出力をするかどうか
%

% デバッグ用メッセージ出力関数
% TODO:任意長の引数に対応したい
procedure debugWrite(arg1_, arg2_)$
  if(optUseDebugPrint_) then <<
    write(arg1_, arg2_);
  >> 
  else <<
    1$
  >>$


%MathematicaでいうFold関数
procedure myFoldLeft(func_, init_, list_)$
  if(list_ = {}) then init_
  else myFoldLeft(func_, func_(init_, first(list_)), rest(list_))$

procedure applyUnitAndFlatten(funcName_, appliedExpr_, newArg_)$
  if(neqArg_ = {}) then appliedExpr_
  else if(part(appliedExpr_, 0) neq funcName_) then funcName_(appliedExpr_, newArg_)
  else applyUnitAndFlatten(funcName_, funcName_(part(appliedExpr_, 1), part(appliedExpr_, 2), first(newArg_)), rest(newArg_))$

%MathematicaでいうApply関数
procedure myApply(func_, expr_)$
begin;
  scalar argsCount_, argsList_, ret_;
  argsCount_:= arglength(expr_);
  if(argsCount_=1) then return func_(part(expr_, 1));

  argsList_:= for i:=1 : argsCount_ collect part(expr_, i);
  ret_:= applyUnitAndFlatten(func_, first(argsList_), rest(argsList_));
  
  % TODO:便利そうなので作る
end;

procedure getInverseRelop(relop_)$
  if(relop_=equal) then neq
  else if(relop_=neq) then equal
  else if(relop_=geq) then lessp
  else if(relop_=greaterp) then leq
  else if(relop_=leq) then greaterp
  else if(relop_=lessp) then geq
  else nil$

procedure myif(x,op,y,approx_precision)$
%入力: 論理式(ex. sqrt(2), greaterp_, sin(2)), 精度
%出力: t or nil or -1
%      (xとyがほぼ等しい時 -1)
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

% 10^(3 + yかxの指数部の値 - 有効桁数)
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
%xとyがほぼ等しい時
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
  if(x=INFINITY) then 
    (if ((op = geq) or (op = greaterp)) then ans:=t else ans:=nil)
  else
    (if ((op = leq) or (op = lessp)) then ans:=t else ans:=nil);
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
	else if(myif(x,greaterp,y,30)) then y else x$

procedure myFindMinimumNatPPTime(x,lst)$
%入力: 現段階での最小PP時刻x, 次の時刻候補のリスト
%出力: 次のPP開始時の時刻
% 0より小さい値はlstに渡されないという前提（他の処理で実現されている）
if(rest(lst)={}) then
<<
  if(myif(x,lessp,first(lst),30)) then x else first(lst)
>>
else 
<<
  if(myif(x,lessp,first(lst),30)) then 
  <<
    myFindMinimumNatPPTime(x,rest(lst))
  >>
  else 
  <<
    myFindMinimumNatPPTime(first(lst),rest(lst))
  >>
>>$


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

  % 引数を持たない場合（trueなど）
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
    % 数式内にsqrtが入っている時のみ、myif関数による大小比較が有効となる
    % TODO:当該の数式内に変数が入った際にも正しく処理ができるようにする
    if(myif(part(formula_, 1), getInverseRelop(header_), part(formula_, 2), 30)) then appliedFormula_:= false 
    else appliedFormula_:= rlqe(formula_)
  >> else <<
    appliedFormula_:= rlqe(formula_)
  >>;

  debugWrite("appliedFormula_: ", appliedFormula_);
  return appliedFormula_;

end;


procedure bball_out()$
% gnuplot用出力, 未完成
% 正規表現 {|}|\n
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
% HydLa向け関数
%---------------------------------------------------------------

operator prev;

%rettrue___ := "RETTRUE___";
%retfalse___ := "RETFALSE___";
rettrue___ := 1;
retfalse___ := 2;

% 関数呼び出しはredevalを経由させる
% <redeval> end:の次が最終行

symbolic procedure redeval(foo_)$
begin scalar ans_;

  write("<redeval> reval ", (car foo_), ":");
  ans_ :=(reval foo_);
  write("<redeval> end:");

  return ans_;
end;



% PPにおける制約ストアのリセット

procedure resetConstraintStore()$
begin;
  putLineFeed();

  constraintStore_ := {};
  csVariables_ := {};
  debugWrite("constraintStore_: ", constraintStore_);
  debugWrite("csVariables_: ", csVariables_);

end;

% PPにおける制約ストアへの制約の追加

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


% (制限 andを受け付けない) TODO 制限への対応
% (制限 trueを受け付けない) TODO 制限への対応

procedure checkConsistencyWithTmpCons(expr_, lcont_, vars_)$
begin;
  scalar ans_;
  putLineFeed();

  ans_:= {part(checkConsistencyBySolveOrRlqe(expr_, lcont_, vars_), 1)};
  debugWrite("ans_ in checkConsistencyWithTmpCons: ", ans_);

  return ans_;
end;


% expr_中に等式以外や論理演算子が含まれる場合に用いる置換関数

procedure exSub(patternList_, expr_)$
begin;
  scalar subAppliedExpr_, head_, subAppliedLeft_, subAppliedRight_, 
         argCount_, subAppliedExprList_, test_;

  debugWrite("in exSub", " ");
  debugWrite("expr_:", expr_);
  
  % 引数を持たない場合
  if(arglength(expr_)=-1) then <<
    subAppliedExpr_:= sub(patternList_, expr_);
    return subAppliedExpr_;
  >>;

  head_:= part(expr_, 0);

  % orで結合されるもの同士を括弧でくくらないと、neqとかが違う結合のしかたをする可能性あり
  if((head_=neq) or (head_=geq) or (head_=greaterp) or (head_=leq) or (head_=lessp)) then <<
    % 等式以外の関係演算子の場合
    subAppliedLeft_:= exSub(patternList_, part(expr_, 1));
    debugWrite("subAppliedLeft_:", subAppliedLeft_);
    subAppliedRight_:= exSub(patternList_, part(expr_, 2));
    debugWrite("subAppliedRight_:", subAppliedRight_);
    subAppliedExpr_:= head_(subAppliedLeft_, subAppliedRight_);
  >> else if((head_=and) or (head_=or)) then <<
    % 論理演算子の場合
    argCount_:= arglength(expr_);
    debugWrite("argCount_: ", argCount_);
    subAppliedExprList_:= for i:=1 : argCount_ collect exSub(patternList_, part(expr_, i));
    debugWrite("subAppliedExprList_:", subAppliedExprList_);    

    % できない
    %subAppliedExpr_:= myApply(head_, subAppliedExprList_);

    % 引数は2つと仮定
    % TODO: なんとかする
    subAppliedExpr_:= if(head_=and) then 
                        and(part(subAppliedExprList_, 1), part(subAppliedExprList_, 2))
                      else if(head_=or) then 
                        or(part(subAppliedExprList_, 1), part(subAppliedExprList_, 2));
  >> else <<
    % 等式や、変数名などのfactorの場合
    % TODO:expr_を見て、制約ストア（あるいはcsvars）内にあるようなら、それと対をなす値（等式の右辺）を適用
    subAppliedExpr_:= sub(patternList_, expr_);
  >>;

  debugWrite("subAppliedExpr_:", subAppliedExpr_);
  return subAppliedExpr_;
end;


procedure convertEqToRule(expr_)$
begin;
  scalar convertedRule_, lhs_, rhs_;
  lhs_:= part(expr_, 1);
  convertedRule_:= replaceby(part(expr_, 1), part(expr_, 2));
  return convertedRule_;

end;

% 等式で表されるルールeqRule_を式exprs_に適用する
procedure applyEqRuleLet(exprs_, eqRule_)$
begin;
  scalar appliedExprs_, ruleRhs_, letEq_;
  debugWrite("in applyEqRuleLet", " ");
  debugWrite("exprs_: ", exprs_);
  debugWrite("eqRule_: ", eqRule_);

  ruleRhs_:= part(eqRule_, 2);
  letEq_:= equal(part(eqRule_, 1), ruleRhs_);
  debugWrite("letEq_: ", letEq_);
  let letEq_;
  appliedExprs_:= exprs_;
  debugWrite("appliedExprs_: ", appliedExprs_);
  debugWrite("part(eqRule_, 1): ", part(eqRule_, 1));
  clear part(eqRule_, 1);
  debugWrite("appliedExprs_ after clear: ", appliedExprs_);

  return appliedExprs_;
end;

procedure applyEqRuleWhere(exprs_, eqRuleLhs_, eqRuleRhs_)$
begin;
  scalar appliedExprs_;
  debugWrite("in applyEqRuleWhere", " ");
  debugWrite("exprs_: ", exprs_);
  debugWrite("eqRuleLhs_: ", eqRuleLhs_);
  debugWrite("eqRuleRhs_: ", eqRuleRhs_);

  appliedExprs_:= exprs_ where (eqRuleLhs_ => eqRuleRhs_);
  debugWrite("appliedExprs_: ", appliedExprs_);
  return appliedExprs_;
end;

procedure applyEqRuleWhere(exprs_, eqRule_)$
begin;
  scalar appliedExprs_;
  debugWrite("in applyEqRuleWhere", " ");
  debugWrite("exprs_: ", exprs_);
  debugWrite("eqRule_: ", eqRule_);

  appliedExprs_:= exprs_ where eqRule_;
  debugWrite("appliedExprs_: ", appliedExprs_);
  return appliedExprs_;
end;

procedure applyEqRuleSet(exprs_, eqRuleLhs_, eqRuleRhs_)$
begin;
  scalar appliedExprs_;
  debugWrite("in applyEqRuleSet", " ");
  debugWrite("exprs_: ", exprs_);
  debugWrite("eqRuleLhs_: ", eqRuleLhs_);

  eqRuleLhs_:=eqRuleRhs_;
  appliedExprs_:= exprs_;
  debugWrite("appliedExprs_: ", appliedExprs_);
  return appliedExprs_;
end;

% PPにおける無矛盾性の判定
% 返り値は{ans, {{変数名 = 値},...}} の形式
% 仕様 QE未使用 % (使用するなら, 変数は基本命題的に置き換え)

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
  % TODO:複数解得られた場合への対応
  tmpSol_:= first(tmpSol_);


  % exprs_に等式以外が入っているかどうかにより、解くのに使用する関数を決定
  % TODO: ROQEモードでも制約ストアを返す必要がある場合への対応
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
    % subの拡張版を用いる手法
    solvedExprs_:= union(for each x in exprs_ join {exSub(tmpSol_, x)});

    % 制約ストアの等式をルールに変換（不要？）
    % csRule_:= map(convertEqToRule, constraintStore_);
    % debugWrite("csRule_:", csRule_);
    % solvedExprs_:= where(exprs_, csRule_);

    %solvedExprs_:= where(exprs_, constraintStore_);
    %solvedExprs_:= union(for each x in constraintStore_ join exprs_ where x);
    %solvedExprs_:= union(for each x in constraintStore_ join applyEqRuleLet(exprs_, x));
    %solvedExprs_:= union(for each x in constraintStore_ join
    %                 applyEqRuleWhere(exprs_, part(x, 1),part(x, 2)));
    %solvedExprs_:= union(for each x in constraintStore_ join applyEqRuleWhere(exprs_, x));
    %solvedExprs_:= union(for each x in constraintStore_ join
    %                 applyEqRuleSet(exprs_, part(x, 1), part(x, 2)));

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
  % ret_codeがrettrue___、つまり1であるかどうかをチェック
  if(part(sol_, 1) = 1) then constraintStore_:= part(sol_, 2);
  debugWrite("constraintStore_: ", constraintStore_);

end;


% 式を{(変数名), (関係演算子コード), (値のフル文字列)}の形式に変換する

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

  % 解を1つだけ得る
  % TODO: Orでつながった複数解への対応
  % 2重リスト状態なら1レベル内側を返す。1重リストならそのまま返す
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
table_:={};

% operator宣言されたargs_を記憶するグローバル変数
loadedOperator:={};

% 初期条件init○_○lhsを作成
procedure makeInitId(f,i)$
if(i=0) then
  mkid(mkid(INIT,f),lhs)
else
  mkid(mkid(mkid(mkid(INIT,f),_),i),lhs);

%laprule用、mkidしたｆを演算子とする
procedure setMkidOperator(f,x)$
  f(x);

% ラプラス変換の変換規則の作成
% {{v, v(t), lapv(s)},...}の対応表table_の作成
procedure LaplaceLetUnit(args_)$
begin;
  scalar arg_, LAParg_, laprule_;

  arg_:= first args_;
  LAParg_:= second args_;

  % arg_が重複してないか判定
  if(freeof(loadedOperator,arg_)) then 
    << 
     operator arg_, LAParg_;
     loadedOperator:= arg_ . loadedOperator;
     operator !~f;

     % makeInitId(f,i)版
     laprule_ :={
       laplace(df(~f(~x),x),x) => il!&*laplace(f(x),x) - makeInitId(f,0),
       laplace(df(~f(~x),x,~n),x) => il!&**n*laplace(f(x),x) -
         for i:=n-1 step -1 until 0 sum
	   makeInitId(f,n-1-i) * il!&**i,
       laplace(~f(~x),x) => setMkidOperator(mkid(lap,f),il!&)
     };
%     % sub版
%     laprule_ :={
%       laplace(df(~f(~x),x),x) => il!&*laplace(f(x),x) - sub(x=0,f(x)),
%       laplace(df(~f(~x),x,~n),x) => il!&**n*laplace(f(x),x) -
%       for i:=n-1 step -1 until 0 sum
%         sub(~x=0, df(f(~x),x,n-1-i)) * il!&**i,
%       laplace(~f(~x),x) => setMkidOperator(mkid(lap,f),il!&)
%     };
     
     let laprule_;
    >>;

  % {{v, v(t), lapv(s)},...}の対応表
  table_:= {arg_, arg_(t), LAParg_(s)} . table_;
  debugWrite("table_: ", table_);
end;

% vars_からdfを除いたものを返す
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
  % ラプラス変換規則の作成
  map(LaplaceLetUnit, tmp_);

  %ht => ht(t)置換
  tmp_:=map(first(~w)=second(~w), table_);
  debugWrite("MAP: ", tmp_);

  tmp_:= sub(tmp_, expr_);
  debugWrite("SUB: ", tmp_);

  % expr_を等式から差式形式に
  diffexpr_:={};
  for each x in tmp_ do 
    if(not freeof(x, equal))
      then diffexpr_:= append(diffexpr_, {lhs(x)-rhs(x)})
    else diffexpr_:= append(diffexpr_, {lhs(x)-rhs(x)});
      
  % laplace演算子でエラー時、laplace演算子込みの式が返ると想定
  if(not freeof(LAPexpr_, laplace)) then return retsolvererror___;

  % ラプラス変換
  LAPexpr_:=map(laplace(~w,t,s), diffexpr_);
  debugWrite("LAPexpr_: ", LAPexpr_);

  % init_制約をLAPexpr_に適応
  solveexpr_:= append(LAPexpr_, init_);
  debugWrite("solveexpr_:", solveexpr_);

  % 逆ラプラス変換の対象
  solvevars_:= append(append(map(third, table_), map(lhs, init_)), {s});
  debugWrite("solvevars_:", solvevars_);

  % 変換対と初期条件を連立して解く
  solveans_ := solve(solveexpr_, solvevars_);
  debugWrite("solveans_: ", solveans_);

  % solveが解無しの時 overconstraintと想定
  if(solveans_={}) then return retoverconstraint___;
  % sがarbcomplexでない値を持つ時 overconstraintと想定
  if(freeof(lgetf(s, solveans_), arbcomplex)) then  return retoverconstraint___;
  % solveans_にsolvevars_の解が一つでも含まれない時 underconstraintと想定
  for each x in table_ do 
    if(freeof(solveans_, third(x))) then tmp_:=true;
  if(tmp_=true) then return retunderconstraint___;
  
  % solveans_の逆ラプラス変換
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


% NDExpr（exDSolveで扱えないような制約式）であるかどうかを調べる
% 式の中にsinもcosも入っていなければfalse
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


% 20110705 overconstraint___無し
ICI_SOLVER_ERROR___:= 0;
ICI_CONSISTENT___:= 1;
ICI_INCONSISTENT___:= 2;
ICI_UNKNOWN___:= 3; % 不要？

procedure checkConsistencyInterval(tmpCons_, exprs_, pexpr_, init_, vars_)$
begin;
  scalar tmpSol_, splitExprsResult_, NDExprs_, DExprs_, DExprVars_, otherExprs_,
         integTmp_, integTmpQE_, andArgsCount_, integTmpSol_, infList_, ans_;
  putLineFeed();

  % SinやCosが含まれる場合はラプラス変換不可能なのでNDExpr扱いする
  % TODO:なんとかしたいところ？
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

  % NDExpr_を連立
  tmpSol_:= solve(union(tmpSol_, NDExprs_), getNoDifferentialVars(vars_));
  debugWrite("tmpSol_ after solve: ", tmpSol_);

  % tmpCons_がない場合は無矛盾と判定して良い
  if(tmpCons_ = {}) then return {ICI_CONSISTENT___};

  integTmp_:= sub(tmpSol_, tmpCons_);
  debugWrite("integTmp_: ", integTmp_);

  integTmpQE_:= rlqe (mymkand(integTmp_));
  debugWrite("integTmpQE_: ", integTmpQE_);

  % ただのtrueやfalseはそのまま判定結果となる
  if(integTmpQE_ = true) then return {ICI_CONSISTENT___}
  else if(integTmpQE_ = false) then return {ICI_INCONSISTENT___};


%  % t>0を連立させてtrue/false判定
%  ans_:= rlqe(integTmpQE_ and t>0);
%  debugWrite("ans_:", ans_);
%  if(ans_=false) then return {ICI_INCONSISTENT___} else return {ICI_CONSISTENT___};


  if(not hasLogicalOp(integTmpQE_)) then <<
    % とりあえずtに関して解く（等式の形式を前提としている）
    integTmpSol_:= solve(integTmpQE_,t);

    infList_:= union(for each x in integTmpSol_ join checkInfUnit(x, ENTAILMENT___));
    debugWrite("infList_: ", infList_);

    % パラメタ無しなら解は1つになるはず
    if(length(infList_) neq 1) then return {ICI_SOLVER_ERROR};
    ans_:= first(infList_);

  >> else <<
    % まず、andでつながったtmp制約をリストに変換
    andArgsCount_:= arglength(integTmpQE_);
    integTmpQEList_:= for i:=1 : andArgsCount_ collect part(integTmpQE_, i);
    debugWrite("integTmpQEList_:", integTmpQEList_);

    % それぞれについて、等式ならばsolveしてintegTmpSolList_とする。不等式ならば後回し。
    integTmpSolList_:= union(for each x in integTmpQEList_ join 
                         if(not hasInequality(x) and not hasLogicalOp(x)) then solve(x, t) else {x});
    debugWrite("integTmpSolList_:", integTmpSolList_);

    % integTmpSolList_の各要素について、checkInfUnitして、infList_を得る
    % TODO:integTmpQEList_の要素内にorが入っている場合を考える
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
  scalar head_, infCheckAns_, exprLhs_, solveAns_, orArgsAnsList_;

  % 前提：relop(t, 値)の形式
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
      % TODO: ちゃんと作る
      infCheckAns_:= hoge
    >>
  >> else if(head_=equal) then <<
    % ガード条件判定においては等式の場合はfalse
    if(mode_ = ENTAILMENT___) then infCheckAns_:= {false}
    else if(mode_ = MINTIME___) then
      if(mymin(part(tExpr_,2),0) neq part(tExpr_,2)) then infCheckAns_:= {part(tExpr_, 2)}
      else infCheckAns_:= {INFINITY}
  >> else <<
    if(mode_ = ENTAILMENT___) then
      if(rlqe(tExpr_ and t>0) = false) then infCheckAns_:= {false} 
      else infCheckAns_:= {true}
    else if(mode_ = MINTIME___) then
      % >および>=の場合は処理しない
      % TODO:パラメタ対応
      if((head_ = geq) or (head_ = greaterp)) then infCheckAns_:= {INFINITY}

      % (t - value) op 0  の形を想定
      % TODO:2次式以上への対応
      else <<
        exprLhs_:= part(tExpr_, 1);
        solveAns_:= part(solve(exprLhs_=0, t), 1);
        infCheckAns_:= {part(solveAns_, 2)};
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

  % SinやCosが含まれる場合はラプラス変換不可能なのでNDExpr扱いする
  % TODO:なんとかしたいところ？
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

  % NDExpr_を連立
  tmpSol_:= solve(union(tmpSol_, NDExpr_), getNoDifferentialVars(vars_));
  debugWrite("tmpSol_ after solve: ", tmpSol_);

  % TODO:Solver error処理

  tmpDiscCause_:= sub(tmpSol_, discCause_);
  debugWrite("tmpDiscCause_:", tmpDiscCause_);

  tmpVarMap_:= first(myFoldLeft(createIntegratedValue, {{},tmpSol_}, vars_)); 
  debugWrite("tmpVarMap_:", tmpVarMap_);

  tmpMinT_:= calcNextPointPhaseTime(maxTime_, tmpDiscCause_);
  debugWrite("tmpMinT_:", tmpMinT_);
  if(tmpMinT_ = error) then retCode_:= IC_SOLVER_ERROR___
  else retCode_:= IC_NORMAL_END___;

  % TODO:tmpMinT_は複数時刻扱えるようにする
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

  % 離散変化が起きえない場合は、maxTime_まで実行して終わり
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



% Fold用

procedure calcMinTime(currentMinPair_, newTriple_)$
begin;
  scalar currentMinT_, currentMinTriple_, sol_, minT_;


end;



% Map用

procedure calcMinTime(integAsk_)$
begin;
  scalar andArgsCount_, integAskList_, integAskSolList_, 
         minTList_, singletonMinTList_;

  debugWrite("in calcMinTime", " ");
  debugWrite("integAsk_: ", integAsk_);

  % falseになるような場合はMinTimeを考える必要がない
  if(rlqe(integAsk_) = false) then return {INFINITY};

  % まず、andでつながったtmp制約をリストに変換
  andArgsCount_:= arglength(integAsk_);
  integAskList_:= for i:=1 : andArgsCount_ collect part(integAsk_, i);
  debugWrite("integAskList_:", integAskList_);

  % それぞれについて、等式ならばsolveしてintegAskSolList_とする。不等式ならば後回し。
  integAskSolList_:= union(for each x in integAskList_ join
                         if(not hasInequality(x)) then solve(x, t) else {x});
  debugWrite("integAskSolList_:", integAskSolList_);

  % integAskSolList_の各要素について、checkInfUnitして、minTList_を得る
  minTList_:= union(for each x in integAskSolList_ join checkInfUnit(x, MINTIME___));  
  debugWrite("minTList_ in calcMinTime: ", minTList_);

  singletonMinTList_:= {myFindMinimumNatPPTime(Infinity, minTList_)};
  debugWrite("singletonMinTList_: ", singletonMinTList_);

  % パラメタ無しなら解は1つになるはず
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



%TODO エラー検出（適用した結果実数以外になった場合等）

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

  % TODO:simplify関数を使う
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
