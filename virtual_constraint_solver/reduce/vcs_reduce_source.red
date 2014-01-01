%---------------------------------------------------------------
% 初期設定
%---------------------------------------------------------------

load_package "ineq";
load_package "laplace";
load_package "numeric";
load_package "redlog"; rlset R;
load_package "sets";

operator interval;
operator prev;

% 出力の設定
off nat;
linelength(100000000);

% 逆ラプラス変換後の値をsin, cosで表示する
on ltrig;

%---------------------------------------------------------------
% グローバル変数
%---------------------------------------------------------------

% constraint__: 現在扱っている制約集合（リスト形式、PPの定数未対応）
% pConstraint__: 現在扱っている、定数制約の集合（リスト形式、IPのみ使用）
% prevConstraint__: 左極限値を設定する制約
% initConstraint__: 初期値制約
% variables__: 制約ストア内に出現する変数の一覧（リスト形式、PPの定数未対応）
% parameters__: 定数制約の集合に出現する定数の一覧（リスト形式、IPのみ使用）
% isTemporary__：制約の追加を一時的なものとするか
% tmpConstraint__: 一時的に追加された変数
% initTmpConstraint__: 一時的に追加された初期値制約
% tmpVariables__: 一時制約に出現する変数のリスト
% guard__: ガード制約
% guardVars__: ガード制約に含まれる変数のリスト
% 
% initVariables__: init変数(init◯◯)のリスト 
% loadedOperator__: ラプラス変換向け, operator宣言されたargs_を記憶する
%
% irrationalNumberIntervalList__: 無理数と、区間値による近似表現の組のリスト
% optUseDebugPrint__: デバッグ出力をするかどうか
% optUseApproximateCompare__: 近似値を用いた大小判定を行うかどうか
% approxPrecision__: checkOrderingFormula内で、数式を数値に近似する際の精度
% intervalPrecision__: 区間値への変換を用いた大小比較における、区間の精度
%
% piInterval__: 円周率Piを表す区間
% eInterval__: ネイピア数Eを表す区間

% グローバル変数初期化
irrationalNumberIntervalList__:= {};
optUseApproximateCompare__:= nil;
approxPrecision__:= 30; % TODO:要検討
intervalPrecision__:= 2; % TODO:要検討
piInterval__:= interval(3141592/1000000, 3141593/1000000);
eInterval__:= interval(2718281/1000000, 2718282/1000000);

loadedOperator__:={};

%---------------------------------------------------------------
% 基本的な数式操作関数
%---------------------------------------------------------------

%MathematicaでいうHead関数
procedure head(expr_)$
  if(arglength(expr_)=-1) then nil
  else part(expr_, 0);

%MathematicaでいうFold関数
procedure foldLeft(func_, init_, list_)$
  if(list_ = {}) then init_
  else foldLeft(func_, func_(init_, first(list_)), rest(list_));

procedure getArgsList(expr_)$
  if(arglength(expr_)=-1) then {}
  else for i:=1 : arglength(expr_) collect part(expr_, i);

%MathematicaでいうApply関数
procedure myApply(func_, expr_)$
  part(expr_, 0):= func_;

% 式の頭部がマイナスかどうか
% ただしexprが整数の場合は負であってもマイナス扱いしない
% TODO：不都合があれば、対応する
procedure hasMinusHead(expr_)$
  if(arglength(expr_)=-1) then nil
  else if(part(expr_, 1) = -1*expr_) then t
  else nil;

% 多項式の有理化を行う
% @param expr_ 分母の項数が4までの多項式
procedure rationalise(expr_)$
begin;
  scalar ans_;

  ans_:= rationaliseMain(expr_);
  debugWrite("ans in rationalise: ", ans_);
  return ans_;
end;

% TODO:3乗根以上への対応
% TODO:より一般的な形への対応, 5項以上？
procedure rationaliseMain(expr_)$
begin;
  scalar head_, denominator_, numerator_, denominatorHead_, denomPlusArgsList_,
         frontTwoElemList_, restElemList_, timesRhs_, conjugate_, 
         rationalisedArgsList_, rationalisedExpr_, flag_;

  if(getArgsList(expr_)={}) then return expr_;

  head_:= head(expr_);
  if(head_=quotient) then <<
    numerator_:= part(expr_, 1);
    denominator_:= part(expr_, 2);
    % 分母に無理数がなければ有理化必要なし
    if(numberp(denominator_)) then return expr_;

    denominatorHead_:= head(denominator_);
    if((denominatorHead_=plus) or (denominatorHead_=times)) then <<
      denomPlusArgsList_:= if(denominatorHead_=plus) then getArgsList(denominator_)
      else << 
        % denominatorHead_=timesのとき
        if(head(part(denominator_, 2))=plus) then getArgsList(part(denominator_, 2))
        else {part(denominator_, 2)}
      >>;

      % 項数が3以上の場合、確実に無理数が減るように工夫して共役数を求める
      if(length(denomPlusArgsList_)>2) then <<
        frontTwoElemList_:= getFrontTwoElemList(denomPlusArgsList_);
        restElemList_:= denomPlusArgsList_ \ frontTwoElemList_;
        if(denominatorHead_=plus) then <<
          conjugate_:= plus(myApply(plus, frontTwoElemList_), -1*(myApply(plus, restElemList_)));
        >> else <<
          % 前提：積の右辺はすべてplusで(-5は+(-5)のように)つながっている形式
          % TODO：そうでない場合でも平気なように？
          timesRhs_:= plus(myApply(plus, frontTwoElemList_), -1*(myApply(plus, restElemList_)));
          conjugate_:= part(denominator_, 1) * timesRhs_;
        >>;
      >> else if(length(denomPlusArgsList_)=2) then <<
        if(denominatorHead_=plus) then <<
          conjugate_:= plus(part(denomPlusArgsList_, 1), -1*part(denomPlusArgsList_, 2));
        >> else <<
          % denominatorHead_=timesのとき
          timesRhs_:= plus(part(denomPlusArgsList_, 1), -1*part(denomPlusArgsList_, 2));
          conjugate_:= part(denominator_, 1) * timesRhs_;
        >>;
      >> else <<
        % denomPlusArgsList_の長さは1のはず、このときdenominatorHead_はtimesであるはず
        conjugate_:= -1*first(denomPlusArgsList_);
      >>;
    >> else if(denominatorHead_=difference) then <<
      conjugate_:= difference(part(denominator_, 1), -1*part(denominator_, 2));
    >> else <<
      conjugate_:= -1*denominator_;
    >>;
    % 共役数を分母子にかける
    numerator_:= numerator_ * conjugate_;
    denominator_:= denominator_ * conjugate_;
    rationalisedExpr_:= numerator_ / denominator_;
    flag_:= true;
  >> else if(length(expr_)>1) then <<
    rationalisedArgsList_:= map(rationaliseMain, getArgsList(expr_));
    rationalisedExpr_:= myApply(head_, rationalisedArgsList_);
  >> else <<
    rationalisedExpr_:= expr_;
  >>;

  if(flag_=true) then rationalisedExpr_:= rationaliseMain(rationalisedExpr_);

  return rationalisedExpr_;
end;


% exSubMainの第二引数をリスト形式に対応させた置換関数
procedure exSub(patternList_, exprs_)$
  if(head(exprs_) = list) then
    for each x in exprs_ collect exSub(patternList_, x)
  else exSubMain(patternList_, exprs_);

% expr_中に等式以外や論理演算子が含まれる場合にも対応できる置換関数
% 不等式には未対応
procedure exSubMain(patternList_, expr_)$
begin;
  scalar subAppliedExpr_, head_, subAppliedLeft_, subAppliedRight_, 
         argCount_, subAppliedExprList_, test_;

  % expr_が引数を持たない場合
  if(arglength(expr_)=-1) then <<
    subAppliedExpr_:= sub(patternList_, expr_);
    return subAppliedExpr_;
  >>;
  % patternList_からTrueを意味する制約を除く
  patternList_:= removeTrueList(patternList_);
  head_:= head(expr_);

  % orで結合されるもの同士を括弧でくくらないと、neqとかが違う結合のしかたをする可能性あり
  if(isIneqRelop(head_)) then <<
    % 等式以外の関係演算子の場合
    subAppliedLeft_:= exSubMain(patternList_, lhs(expr_));
    subAppliedRight_:= exSubMain(patternList_, rhs(expr_));
    subAppliedExpr_:= myApply(head_, {subAppliedLeft_, subAppliedRight_});
  >> else if(isLogicalOp(head_)) then <<
    % 論理演算子の場合
    argCount_:= arglength(expr_);
    subAppliedExprList_:= for i:=1 : argCount_ collect exSubMain(patternList_, part(expr_, i));
    subAppliedExpr_:= myApply(head_, subAppliedExprList_);

  >> else <<
    % 等式や、変数名などのfactorの場合
    % TODO:expr_を見て、制約ストア（あるいはcsvars）内にあるようなら、それと対をなす値（等式の右辺）を適用
    subAppliedExpr_:= sub(patternList_, expr_);
  >>;

  return subAppliedExpr_;
end;

% expr_中に出現する、sqrt(var_)に関する項の係数を得る
% 複数項存在する場合も考慮し、リスト形式で返す
procedure getSqrtList(expr_, var_, mode_)$
begin;
  scalar head_, argsAnsList_, coefficientList_, exprList_, insideSqrt_;

  % 変数やsqrtが含まれなければ考える必要なし
  if(freeof(expr_, var_) or freeof(expr_, sqrt)) then return {};

  head_:= head(expr_);
  if(hasMinusHead(expr_)) then <<
    % TODO：負号の中にvar_が複数個ある場合への対応？
    coefficientList_:= {-1*first(getSqrtList(part(expr_, 1), var_, mode_))};
    exprList_:= getSqrtList(part(expr_, 1), var_, mode_);
  >> else if(head_=plus) then <<
    % 多項式の場合
    argsAnsList_:= for each x in getArgsList(expr_) join getSqrtList(x, var_, mode_);
    coefficientList_:= argsAnsList_;
    exprList_:= argsAnsList_;
  >> else if(head_=times) then <<
    argsAnsList_:= for each x in getArgsList(expr_) join getSqrtList(x, var_, mode_);
    coefficientList_:= {foldLeft(times, 1, argsAnsList_)};
    exprList_:= argsAnsList_;
  >> else if(head_=sqrt) then <<
    insideSqrt_:= part(expr_, 1);
    exprList_:= {insideSqrt_};

    if(freeof(insideSqrt_, sqrt)) then <<
      % lcofで係数を求める
      coefficientList_:= {lcof(insideSqrt_, var_)};
    >> else <<
      % 根号の中がさらにsqrtを含む多項式になっている場合
      argsAnsList_:= for each x in getArgsList(insideSqrt_) join getSqrtList(x, var_, mode_);
      % TODO：複数個ある場合への対応？
      coefficientList_:= {first(argsAnsList_)};
      exprList_:= union(exprList_, argsAnsList_);
    >>;
  >> else <<
    coefficientList_:= hogehoge;
    exprList_:= fugafuga;
  >>;

  if(mode_=COEFF) then return coefficientList_
  else if(mode_=INSIDE) then return exprList_;
end;

%---------------------------------------------------------------
% 関係演算子関連の関数
%---------------------------------------------------------------

procedure isIneqRelop(expr_)$
  if((expr_=neq) or (expr_=not) or
    (expr_=geq) or (expr_=greaterp) or 
    (expr_=leq) or (expr_=lessp)) then t else nil;

procedure hasIneqRelop(expr_)$
  if(freeof(expr_, neq) and freeof(expr_, not) and
     freeof(expr_, geq) and freeof(expr_, greaterp) and
     freeof(expr_, leq) and freeof(expr_, lessp)) then nil else t;

procedure getReverseRelop(relop_)$
  if(relop_=equal) then equal
  else if(relop_=neq) then neq
  else if(relop_=geq) then leq
  else if(relop_=greaterp) then lessp
  else if(relop_=leq) then geq
  else if(relop_=lessp) then greaterp
  else nil;

procedure getInverseRelop(relop_)$
  if(relop_=equal) then neq
  else if(relop_=neq) then equal
  else if(relop_=geq) then lessp
  else if(relop_=greaterp) then leq
  else if(relop_=leq) then greaterp
  else if(relop_=lessp) then geq
  else nil;

procedure getReverseCons(cons_)$
begin;
  scalar reverseRelop_, lhs_, rhs_;

  reverseRelop_:= getReverseRelop(head(cons_));
  lhs_:= lhs(cons_);
  rhs_:= rhs(cons_);
  return reverseRelop_(rhs_, lhs_);
end;

procedure getExprCode(cons_)$
begin;
  scalar head_;

  % relopが引数として直接渡された場合へも対応
  if(arglength(cons_)=-1) then head_:= cons_
  else head_:= head(cons_);

  if(head_=equal) then return 0
  else if(head_=lessp) then return 1
  else if(head_=greaterp) then return 2
  else if(head_=leq) then return 3
  else if(head_=geq) then return 4
  else return nil;
end;

%---------------------------------------------------------------
% 論理演算子関連の関数
%---------------------------------------------------------------

procedure isLogicalOp(expr_)$
  if((expr_=or) or (expr_=and)) then t else nil;

procedure hasLogicalOp(expr_)$
  if(freeof(expr_, and) and freeof(expr_, or)) then nil else t;

%---------------------------------------------------------------
% 三角関数関連の関数
%---------------------------------------------------------------

procedure isTrigonometricFunc(expr_)$
  if((expr_=sin) or (expr_ = cos) or (expr_ = tan)) then t else nil;

procedure hasTrigonometricFunc(expr_)$
  if((not freeof(expr_, sin)) or (not freeof(expr_, cos)) or (not freeof(expr_, tan))) then t else nil;

procedure isInvTrigonometricFunc(expr_)$
  if((expr_=asin) or (expr_=acos) or (expr_=atan)) then t else nil;

procedure hasInvTrigonometricFunc(expr_)$
  if((not freeof(expr_, asin)) or (not freeof(expr_, acos)) or (not freeof(expr_, atan))) then t else nil;

%---------------------------------------------------------------
% 区間演算関連の関数
%---------------------------------------------------------------

procedure isInterval(expr_)$
  if(arglength(expr_)=-1) then nil
  else if(head(expr_)=interval) then t
  else nil;

procedure isZeroInterval(expr_)$
  if(expr_=makePointInterval(0)) then t else nil;

procedure getLbFromInterval(expr_)$
  if(isInterval(expr_)) then part(expr_, 1)
  else ERROR;

procedure getUbFromInterval(expr_)$
  if(isInterval(expr_)) then part(expr_, 2)
  else ERROR;

% TODO：等号の有無（開区間/閉区間）によって結果が変わる場合も正しく扱えるように
% （その場合、lbとubが等しくなっている区間は、閉区間と考えると実現可能か）
procedure compareInterval(interval1_, op_, interval2_)$
begin;
  debugWrite("in compareInterval", " ");
  debugWrite("interval1_: ", interval1_);
  debugWrite("op_: ", op_);
  debugWrite("interval2_: ", interval2_);

  if (op_ = geq) then
    if (getLbFromInterval(interval1_) >= getUbFromInterval(interval2_)) then return t 
    else if(getUbFromInterval(interval1_) < getLbFromInterval(interval2_)) then return nil
    else return unknown
  else if (op_ = greaterp) then
    if (getLbFromInterval(interval1_) > getUbFromInterval(interval2_)) then return t 
    else if(getUbFromInterval(interval1_) <= getLbFromInterval(interval2_)) then return nil
    else return unknown
  else if (op_ = leq) then
    if (getUbFromInterval(interval1_) <= getLbFromInterval(interval2_)) then return t 
    else if(getLbFromInterval(interval1_) > getUbFromInterval(interval2_)) then return nil
    else return unknown
  else if (op_ = lessp) then
    if (getUbFromInterval(interval1_) < getLbFromInterval(interval2_)) then return t 
    else if(getLbFromInterval(interval1_) >= getUbFromInterval(interval2_)) then return nil
    else return unknown;
end;

procedure plusInterval(interval1_, interval2_)$
  interval(getLbFromInterval(interval1_)+getLbFromInterval(interval2_), getUbFromInterval(interval1_)+getUbFromInterval(interval2_));

procedure timesInterval(interval1_, interval2_)$
begin;
  scalar compareList_, retInterval_;

  compareList_:= {getLbFromInterval(interval1_) * getLbFromInterval(interval2_),
                  getLbFromInterval(interval1_) * getUbFromInterval(interval2_),
                  getUbFromInterval(interval1_) * getLbFromInterval(interval2_),
                  getUbFromInterval(interval1_) * getUbFromInterval(interval2_)};
  retInterval_:= interval(findMinimumValue(INFINITY, compareList_), findMaximumValue(-INFINITY, compareList_));
  debugWrite("ans in timesInterval: ", retInterval_);

  return retInterval_;
end;

procedure quotientInterval(interval1_, interval2_)$
  if((getLbFromInterval(interval2_)=0) or (getUbFromInterval(interval2_)=0)) then ERROR
  else timesInterval(interval1_, interval(1/getUbFromInterval(interval2_), 1/getLbFromInterval(interval2_)));

% 無理数を含む定数式を区間値形式に変換する
% 前提：入力のvalue_は2以上の整数に限られる
% @param mode_: BINARY_SEARCH(二分探索) or newton(ニュートン法) 
procedure getSqrtInterval(value_, mode_)$
begin;
  scalar sqrtInterval_, sqrtLb_, sqrtUb_, midPoint_, loopCount_,
         tmpNewtonSol_, newTmpNewtonSol_, iIntervalList_;

  debugWrite("in getSqrtInterval: ", {value_, mode_});
  
  iIntervalList_:= getIrrationalNumberInterval(sqrt(value_));
  if(iIntervalList_ neq {}) then return first(iIntervalList_);

  if(mode_=BINARY_SEARCH) then <<
    % 上下限の中点xにおいて、x^2-valueの正負を調べることで探索範囲を狭めていく
    sqrtLb_:= 0;
    sqrtUb_:= value_;
    loopCount_:= 1;
    while (sqrtUb_ - sqrtLb_ >= 1/10^intervalPrecision__) do <<
      debugWrite("loopCount_: ", loopCount_);
      midPoint_:= (sqrtLb_ + sqrtUb_)/2;
      debugWrite("midPoint_: ", midPoint_);
      if(midPoint_ * midPoint_ < value_) then sqrtLb_:= midPoint_
      else sqrtUb_:= sqrtUb_:= midPoint_;
      loopCount_:= loopCount_+1;
    >>;
    sqrtInterval_:= interval(sqrtLb_, sqrtUb_);
    putIrrationalNumberInterval(sqrt(value_), sqrtInterval_);
  >> else if(mode_=NEWTON) then <<
    % ニュートン法により求める
    newTmpNewtonSol_:= value_;
    loopCount_:= 1;
    repeat <<
      tmpNewtonsol_:= newTmpNewtonSol_;
      newTmpNewtonSol_:= 1/2*(tmpNewtonSol_+value_ / tmpNewtonSol_);
      debugWrite("{loopCount_, new Newton Sol}: ", {loopCount_, newTmpNewtonSol_});
      loopCount_:= loopCount_+1;
    >> until (tmpNewtonSol_ - newTmpNewtonSol_ < 1/10^intervalPrecision__);
    sqrtLb_:= 2*newTmpNewtonSol_ - tmpNewtonSol_;
    sqrtUb_:= newTmpNewtonSol_;
    sqrtInterval_:= interval(sqrtLb_, sqrtUb_);
    putIrrationalNumberInterval(sqrt(value_), sqrtInterval_);
  >>;

  debugWrite("ans in getSqrtInterval: ", sqrtInterval_);
  return sqrtInterval_;
end;

procedure makePointInterval(value_)$
  interval(value_, value_);

procedure convertValueToInterval(value_)$
begin;
  scalar head_, retInterval_, argsList_, insideSqrt_;

  debugWrite("in convertValueToInterval: ", value_);

  % 有理数なので区間にすることなく大小判定可能だが、上下限が等しい区間（点区間）として扱う
  if(numberp(value_)) then <<
    retInterval_:= makePointInterval(value_);
    debugWrite("ans in convertValueInterval", retInterval_);
    return retInterval_;
  >>;

  if(arglength(value_) neq -1) then <<
    head_:= head(value_);
    debugWrite("head_: ", head_);
    if(hasMinusHead(value_)) then <<
      % 負の数の場合
      retInterval_:= timesInterval(makePointInterval(-1), convertValueToInterval(part(value_, 1)));
    >> else if((head_=plus) or (head_= times)) then <<
      argsList_:= getArgsList(value_);
      debugWrite("argsList_: ", argsList_);
      if(head_=plus) then <<
        retInterval_:= interval(0, 0);
        for each x in argsList_ do
          retInterval_:= plusInterval(convertValueToInterval(x), retInterval_);
      >> else <<
        retInterval_:= interval(1, 1);
        for each x in argsList_ do
          retInterval_:= timesInterval(convertValueToInterval(x), retInterval_);
      >>;
    >> else if(head_=quotient) then <<
      retInterval_:= quotientInterval(part(value_, 1), part(value_, 2));
    >> else if(head_=expt) then <<
      % 前提：冪数は整数
      retInterval_:= makePointInterval(1);
      for i:=1 : part(value_, 2) do <<
        retInterval_:= timesInterval(retInterval_, convertValueToInterval(part(value_, 1)));
      >>;
    >> else if(head_=sqrt) then <<
      insideSqrt_:= part(value_, 1);
      retInterval_:= getSqrtInterval(insideSqrt_, NEWTON);
    >> else if(head_=sin) then <<
      retInterval_:= interval(-1, 1);
    >> else if(head_=cos) then <<
      retInterval_:= interval(-1, 1);
    >> else if(head_=tan) then <<
      retInterval_:= interval(-1, 1);
    >> else if(head_=asin) then <<
      % interval(-Pi, Pi)
      retInterval_:= interval(getLbFromInterval(timesInterval(makePointInterval(-1), piInterval__)), getUbFromInterval(piInterval__));
    >> else if(head_=acos) then <<
      retInterval_:= interval(getLbFromInterval(timesInterval(makePointInterval(-1), piInterval__)), getUbFromInterval(piInterval__));
    >> else if(head_=atan) then <<
      retInterval_:= interval(getLbFromInterval(timesInterval(makePointInterval(-1), piInterval__)), getUbFromInterval(piInterval__));
    >> else <<
      % TODO：他にどんな場合に無理数扱いになるかを調べる
      retInterval_:= interval(hoge, hoge);
    >>;
  >> else <<
    if(value_=pi) then retInterval_:= piInterval__
    else if(value_=e) then retInterval_:= eInterval__
    else retInterval_:= interval(hoge, hoge);
  >>;
  debugWrite("ans in convertValueToInterval: ", retInterval_);

  return retInterval_;
end;

%---------------------------------------------------------------
% 実数以外の数関連の関数
%---------------------------------------------------------------

procedure hasImaginaryNum(value_)$
  if(not freeof(value_, i)) then t else nil;

procedure hasIndefinableNum(value_)$
begin;
  scalar retFlag_, head_, flagList_, insideInvTrigonometricFunc_;

  if(arglength(value_)=-1) then <<
    return nil;
  >>;

  head_:= head(value_);
  if(hasMinusHead(value_)) then <<
    % 負の数の場合
    retFlag_:= hasIndefinableNum(part(value_, 1));
  >> else if((head_=plus) or (head_=times)) then <<
    flagList_:= union(for each x in getArgsList(value_) join
      if(hasIndefinableNum(x)) then {t} else {});
    retFlag_:= if(flagList_={}) then nil else t;
  >> else if(isInvTrigonometricFunc(head_)) then <<
    insideInvTrigonometricFunc_:= part(value_, 1);
    retFlag_:= if(checkOrderingFormula(insideInvTrigonometricFunc_>=-1) and checkOrderingFormula(insideInvTrigonometricFunc_<=1)) then nil else t;
  >> else <<
    retFlag_:= nil;
  >>;
  return retFlag_;
end;

procedure putIrrationalNumberInterval(value_, interval_)$
  irrationalNumberIntervalList__:= cons({value_, interval_}, irrationalNumberIntervalList__);

procedure getIrrationalNumberInterval(value_)$
  for each x in irrationalNumberIntervalList__ join 
    if(first(x)=value_) then {second(x)} else {};

%---------------------------------------------------------------
% リスト関連の関数
%---------------------------------------------------------------

% @return true , false(t, nilでない!)
procedure isEmpty(lst_)$
  if(lst_ = {}) then true else false;

%---------------------------------------------------------------
% パラメタ関連の関数
%---------------------------------------------------------------

% 式中にパラメタが含まれているかどうかを、parameters__内の変数が含まれるかどうかで判定
procedure hasParameter(expr_)$
  if(collectParameters(expr_) neq {}) then t else nil;

% 式構造中のパラメタを、集める
procedure collectParameters(expr_)$
begin;
  scalar collectedParameters_;

  collectedParameters_:= union({}, for each x in parameters__ join if(not freeof(expr_, x)) then {x} else {});

  return collectedParameters_;
end;

%---------------------------------------------------------------
% 大小判定関連の関数（定数式のみ、パラメタなし）
%---------------------------------------------------------------

checkOrderingFormulaCount_:= 0;
checkOrderingFormulaIrrationalNumberCount_:= 0;

procedure checkOrderingFormula(orderingFormula_)$
% 論理式の比較を行う
%入力: 論理式(特にsqrt(2), greaterp_, sin(2)などを含むようなもの), 精度
%出力: t or nil or -1
%      (xとyがほぼ等しい時 -1)
%geq_= >=, geq; greaterp_= >, greaterp; leq_= <=, leq; lessp_= <, lessp;
begin;
  scalar head_, x, op, y, bak_precision, ans, margin, xInterval_, yInterval_;

  debugWrite("=== in checkOrderingFormula: ", orderingFormula_);
  checkOrderingFormulaCount_:= checkOrderingFormulaCount_+1;
  debugWrite("checkOrderingFormulaCount_: ", checkOrderingFormulaCount_);

  head_:= head(orderingFormula_);
  % 大小に関する論理式以外が入力されたらエラー
  if(hasLogicalOp(head_)) then <<
    debugWrite("ans in checkOrderingFormula: ", ERROR);
    return ERROR;
  >>;

  x:= lhs(orderingFormula_);
  op:= head_;
  y:= rhs(orderingFormula_);

  debugWrite("{x, op, y}: ", {x, op, y});

  if(x=y) then <<
    ans:= if((op = geq) or (op = leq)) then t else nil;
    debugWrite("ans in checkOrderingFormula: ", ans);
    return ans;
  >>;

  if(not freeof({x,y}, INFINITY)) then <<
    ans:= infinityIf(x, op, y);
    debugWrite("ans after infinityIf in checkOrderingFormula: ", ans);
    return ans;
  >>;

  if(not numberp(x) or not(numberp(y))) then <<
    checkOrderingFormulaIrrationalNumberCount_:= checkOrderingFormulaIrrationalNumberCount_+1;
    debugWrite("checkOrderingFormulaIrrationalNumberCount_: ", checkOrderingFormulaIrrationalNumberCount_);
    % 無理数が含まれる場合
    if(optUseApproximateCompare__) then <<
      % 近似値を用いた大小比較
      bak_precision := precision 0;
      on rounded$ precision approxPrecision__$

      % xおよびyが有理数である時
      % 10^(3 + yかxの指数部の値 - 有効桁数)
      if(min(x,y)=0) then
        margin:=10 ^ (3 + floor log10 max(x, y) - approxPrecision__)
      else if(min(x,y)>0) then 
        margin:=10 ^ (3 + floor log10 min(x, y) - approxPrecision__)
      else
        margin:=10 ^ (3 - floor log10 abs min(x, y) - approxPrecision__);

      debugWrite("{margin, x, y, abs(x-y)}: ", {margin, x, y, abs(x-y)});
      %xとyがほぼ等しい時
      if(abs(x-y)<margin) then <<
        off rounded$ precision bak_precision$
        write(-1);
        return -1;
      >>;

      if (op = geq) then
        (if (x >= y) then ans:=t else ans:=nil)
      else if (op = greaterp) then
        (if (x > y) then ans:=t else ans:=nil)
      else if (op = leq) then
        (if (x <= y) then ans:=t else ans:=nil)
      else if (op = lessp) then
        (if (x < y) then ans:=t else ans:=nil);

      off rounded$ precision bak_precision$
    >> else <<
      % 区間値への変換を用いた大小比較
      xInterval_:= convertValueToInterval(x);
      yInterval_:= convertValueToInterval(y);
      debugWrite("xInterval_: ", xInterval_);
      debugWrite("yInterval_: ", yInterval_);
      ans:= compareInterval(xInterval_, op, yInterval_);
    >>;
  >> else <<
    if ((op = geq) or (op = greaterp)) then
      (if (x > y) then ans:=t else ans:=nil)
    else
      (if (x < y) then ans:=t else ans:=nil);
  >>;

  if(ans=unknown) then <<
    % unknownが返った場合は精度を変えて再試行
    intervalPrecision__:= intervalPrecision__+4;
    ans:= checkOrderingFormula(orderingFormula_);
    intervalPrecision__:= intervalPrecision__-4;
  >>;

  debugWrite("checkOrderingFormula arg:", orderingFormula_);
  debugWrite("ans in checkOrderingFormula: ", ans);

  return ans;
end;

% infinity(無限大)を含む不等式の判定
procedure infinityIf(x, op, y)$
begin;
  scalar ans_, infinityTupleDNF_, retTuple_, i_, j_, andAns_;

  debugWrite("in infinityIf: ", {x, op, y});
  % INFINITY > -INFINITYとかの対応
  if(x=INFINITY or y=-INFINITY) then 
    if((op = geq) or (op = greaterp)) then ans_:=t else ans_:=nil
  else if(x=-INFINITY or y=INFINITY) then
    if((op = leq) or (op = lessp)) then ans_:=t else ans_:=nil
  else <<
    % 係数等への対応として、まずinfinity relop valueの形にしてから解きなおす
    infinityTupleDNF_:= exIneqSolve(op(x, y));
    debugWrite("infinityTupleDNF_: ", infinityTupleDNF_);
    if(isFalseDNF(infinityTupleDNF_)) then return nil;
    i_:= 1;
    ans_:= nil;
    while (i_<=length(infinityTupleDNF_) and (ans_ neq t)) do <<
      j_:= 1;
      andAns_:= t;
      while (j_<=length(part(infinityTupleDNF_, i_)) and (andAns_ = t)) do <<
        retTuple_:= part(part(infinityTupleDNF_, i_), j_);
        andAns_:= infinityIf(getVarNameFromTuple(retTuple_), getRelopFromTuple(retTuple_), getValueFromTuple(retTuple_));
        j_:= j_+1;
      >>;
      ans_:= andAns_;
      i_:= i_+1;
    >>;
  >>;

  debugWrite("ans in infinityIf: ", ans_);
  return ans_;
end;

procedure mymin(x,y)$
%入力: 数値という前提
  if(checkOrderingFormula(x<y)) then x else y;

procedure mymax(x,y)$
%入力: 数値という前提
  if(checkOrderingFormula(x>y)) then x else y;

procedure findMinimumValue(x,lst)$
%入力: 現段階での最小値x, 最小値を見つけたい対象のリスト
%出力: リスト中の最小値
  if(lst={}) then x
  else if(mymin(x, first(lst)) = x) then findMinimumValue(x,rest(lst))
  else findMinimumValue(first(lst),rest(lst));

procedure findMaximumValue(x,lst)$
%入力: 現段階での最大値x, 最大値を見つけたい対象のリスト
%出力: リスト中の最大値
  if(lst={}) then x
  else if(mymax(x, first(lst)) = x) then findMaximumValue(x,rest(lst))
  else findMaximumValue(first(lst),rest(lst));

%---------------------------------------------------------------
% TC形式関連の関数
%---------------------------------------------------------------

% TC形式（時刻と条件DNFの組）におけるアクセス用関数
procedure getTimeFromTC(TC_)$
  part(TC_, 1);
procedure getCondDNFFromTC(TC_)$
  part(TC_, 2);

%---------------------------------------------------------------
% 大小判定関連の関数（パラメタを含む）
%---------------------------------------------------------------

procedure compareValueAndParameter(val_, paramExpr_, condDNF_)$
%入力: 比較する対象の値, パラメータを含む式, パラメータに関する条件
%出力: {「値」側が小さくなるためのパラメータの条件, 「パラメータを含む式」が小さくなるためのパラメータの条件}
% checkInfMinTimeの仕様変更により不要に
begin;
  scalar valueLeqParamCondSol_, valueGreaterParamCondSol_, 
         ret_;

  debugWrite("=== in compareValueAndParameter", " ");
  debugWrite("val_: ", val_);
  debugWrite("paramExpr_: ", paramExpr_);
  debugWrite("condDNF_: ", condDNF_);


  % minValue_≦paramの場合の条件式
  valueLeqParamCondSol_:= addCondTupleToCondDNF({val_, leq, paramExpr_}, condDNF_);
  debugWrite("valueLeqParamCondSol_: ", valueLeqParamCondSol_);

  % minValue_＞paramの場合の条件式
  valueGreaterParamCondSol_:= addCondTupleToCondDNF({val_, greaterp, paramExpr_}, condDNF_);
  debugWrite("valueGreaterParamCondSol_: ", valueGreaterParamCondSol_);

  return {valueLeqParamCondSol_, valueGreaterParamCondSol_};
end;

%---------------------------------------------------------------
% 不等式タプル関連の関数
%---------------------------------------------------------------

procedure isLbTuple(tuple_)$
  if((getRelopFromTuple(tuple_)=geq) or (getRelopFromTuple(tuple_)=greaterp)) then t else nil;

procedure isUbTuple(tuple_)$
  if((getRelopFromTuple(tuple_)=leq) or (getRelopFromTuple(tuple_)=lessp)) then t else nil;

procedure getLbTupleListFromConj(conj_)$
  for each x in conj_ join if(isLbTuple(x)) then {x} else {};

procedure getUbTupleListFromConj(conj_)$
  for each x in conj_ join if(isUbTuple(x)) then {x} else {};

% {左辺, relop, 右辺}のタプルから、式を作って返す
procedure makeExprFromTuple(tuple_)$
  myApply(getRelopFromTuple(tuple_), {getVarNameFromTuple(tuple_), getValueFromTuple(tuple_)});

procedure getVarNameFromTuple(tuple_)$
  part(tuple_, 1);

procedure getRelopFromTuple(tuple_)$
  part(tuple_, 2);

procedure getValueFromTuple(tuple_)$
  part(tuple_, 3);

%---------------------------------------------------------------
% 不等式タプルによるDNF関連の関数
%---------------------------------------------------------------

% 論理積にNotをつけてDNFにする
procedure getNotConjDNF(conj_)$
  for each tuple in conj_ collect
    {{getVarNameFromTuple(tuple), getInverseRelop(getRelopFromTuple(tuple)), getValueFromTuple(tuple)}};

% Equalを表す論理積になっているかどうか
procedure isEqualConj(conj_)$
begin;
  scalar value1_, value2_, relop1_, relop2_;

  if(length(conj_) neq 2) then return nil;

  value1_:= getValueFromTuple(first(conj_));
  value2_:= getValueFromTuple(second(conj_));
  if(value1_ neq value2_) then return nil;

  relop1_:= getRelopFromTuple(first(conj_));
  relop2_:= getRelopFromTuple(second(conj_));
  if(((relop1_=geq) and (relop2_=leq)) or ((relop1_=leq) and (relop2_=geq))) then return t
  else return nil;
end;

procedure isSameConj(conj1_, conj2_)$
begin;
  scalar flag_;

  debugWrite("in isSameConj", " ");
  debugWrite("{conj1_, conj2_}: ", {conj1_, conj2_});

  if(length(conj1_) neq length(conj2_)) then return nil;

  flag_:= t;
  for each x in conj1_ do 
    if(freeof(conj2_, x)) then flag_:= nil;
  return flag_;
end;

procedure isFalseConj(conj_)$
  if(conj_={}) then t else nil;

procedure isTrueConj(conj_)$
  if(conj_={true}) then t else nil;

procedure getNotDNF(DNF_)$
begin;
  scalar retDNF_, i_;

  debugWrite("in getNotDNF", DNF_);

  retDNF_:= {{true}};
  i_:= 1;
  while (not isFalseDNF(retDNF_) and i_<=length(DNF_)) do <<
    retDNF_:= addCondDNFToCondDNF(getNotConjDNF(part(DNF_, i_)), retDNF_);
    i_:= i_+1;
  >>;

  debugWrite("ans in getNotDNF: ", retDNF_);
  return retDNF_;
end;

procedure isSameDNF(DNF1_, DNF2_)$
begin;
  scalar flag_, i_, conj1, flagList_;

  debugWrite("in isSameDNF", " ");
  debugWrite("{DNF1_, DNF2_}: ", {DNF1_, DNF2_});

  if(length(DNF1_) neq length(DNF2_)) then return nil;

  flag_:= t;
  i_:= 1;
  while (flag_=t and i_<=length(DNF1_)) do <<
    conj1:= part(DNF1_, i_);
    flagList_:= for each conj2 in DNF2_ join
      if(isSameConj(conj1, conj2)) then {true} else {};
    if(flagList_={}) then flag_:= nil;
    i_:= i_+1;
  >>;
  return flag_;
end;

procedure isFalseDNF(DNF_)$
  if(DNF_={{}}) then t else nil;

procedure isTrueDNF(DNF_)$
  if(DNF_={{true}}) then t else nil;

% 前提：式expr_は1つの関係演算子のみを持つ
procedure makeTupleDNFFromEq(var_, value_)$
  {{ {var_, geq, value_}, {var_, leq, value_} }};

procedure makeTupleDNFFromNeq(var_, value_)$
  {{ {var_, lessp, value_}, {var_, greaterp, value_} }};

% DNF中の、実質等しい（重複した）論理積を削除
procedure simplifyDNF(DNF_)$
begin;
  scalar tmpRetDNF_, simplifiedDNF_;

  % foldLeft((if(isSameConj(first(#1), #2)) then #1 else cons(#2, #1))&, {first(DNF_)}, rest(DNF_))を実現
  tmpRetDNF_:= {first(DNF_)};
  for i:=1 : length(rest(DNF_)) do <<
    tmpRetDNF_:= if(isSameConj(first(tmpRetDNF_), part(rest(DNF_), i))) then tmpRetDNF_
                 else cons(part(rest(DNF_), i), tmpRetDNF_);
  >>;
  simplifiedDNF_:= tmpRetDNF_;

  debugWrite("ans in simplifyDNF: ", simplifiedDNF_);
  return simplifiedDNF_;
end;

procedure addCondDNFToCondDNF(newCondDNF_, condDNF_)$
begin;
  scalar addedCondDNF_, tmpAddedCondDNF_, i_;

  debugWrite("=== in addCondDNFToCondDNF", " ");
  debugWrite("{newCondDNF_, condDNF_}: ", {newCondDNF_, condDNF_});

  % Falseを追加しようとする場合はFalseを返す
  if(isFalseDNF(newCondDNF_)) then return {{}};

  addedCondDNF_:= for each conj in newCondDNF_ join <<
    i_:= 1;
    tmpAddedCondDNF_:= condDNF_;
    while (i_<=length(conj) and not isFalseDNF(tmpAddedCondDNF_)) do <<
      tmpAddedCondDNF_:= addCondTupleToCondDNF(part(conj, i_), tmpAddedCondDNF_);
      i_:= i_+1;
    >>;
    if(not isFalseDNF(tmpAddedCondDNF_)) then simplifyDNF(tmpAddedCondDNF_) else {}
  >>;

  % newCondDNF_内のどのconjを追加してもFalseDNFになるような場合に、最終的に{}が得られているので修正する
  if(addedCondDNF_={})then addedCondDNF_:= {{}};

  debugWrite("=== ans in addCondDNFToCondDNF: ", addedCondDNF_);
  return addedCondDNF_;
end;

procedure addCondTupleToCondDNF(newCondTuple_, condDNF_)$
%入力：追加する（パラメタの）条件タプルnewCondTuple_, （パラメタの）条件を表す論理式のリストcondDNF_
%出力：（パラメタの）条件を表す論理式のDNF
%注意：リストの要素1つ1つは論理和でつながり、要素内は論理積でつながっていることを表す。
begin;
  scalar addedCondDNF_, addedCondConj_;

  debugWrite("=== in addCondTupleToCondDNF", " ");
  debugWrite("{newCondTuple_, condDNF_}: ", {newCondTuple_, condDNF_});

  addedCondDNF_:= union(for each x in condDNF_ join <<
    addedCondConj_:= addCondTupleToCondConj(newCondTuple_, x);
    if(not isFalseConj(addedCondConj_)) then {addedCondConj_} else {}
  >>);

  if(addedCondDNF_={}) then addedCondDNF_:= {{}};
  debugWrite("ans in addCondTupleToCondDNF: ", addedCondDNF_);
  return addedCondDNF_;
end;

procedure addCondTupleToCondConj(newCondTuple_, condConj_)$
begin;
  scalar addedCondConj_, varName_, relop_, value_,
         varTerms_, ubTuple_, lbTuple_, ub_, lb_,
         ubTupleList_, lbTupleList_, ineqSolDNF_;

  debugWrite("in addCondTupleToCondConj", " ");
  debugWrite("{newCondTuple_, condConj_}: ", {newCondTuple_, condConj_});

  % trueを追加しようとする場合、追加しないのと同じ
  if(newCondTuple_=true) then return condConj_;
  % パラメタが入らない場合、単に大小判定をした結果が残る
  % （移項するとパラメタが入らなくなる場合も同様）
  if(not hasParameter(makeExprFromTuple(newCondTuple_)) and freeof(makeExprFromTuple(newCondTuple_), t)) then 
    if(checkOrderingFormula(makeExprFromTuple(newCondTuple_))) then return condConj_
    else return {};
  % falseに追加しようとする場合
  if(isFalseConj(condConj_)) then return {};
  % trueに追加しようとする場合
  if(isTrueConj(condConj_)) then return {newCondTuple_};


  % 場合によっては、タプルのVarName部分とValue部分の両方に変数名が入っていることもある
  % そのため、たとえ1次であっても一旦exIneqSolveで解く必要がある
  if((arglength(getVarNameFromTuple(newCondTuple_)) neq -1) or not numberp(getValueFromTuple(newCondTuple_))) then <<
    % VarName部分が1次の変数名で、かつ、Value部分が数値ならこの処理は不要
    ineqSolDNF_:= exIneqSolve(makeExprFromTuple(newCondTuple_));
    debugWrite("{ineqSolDNF_, (newCondTuple_), (condConj_)}: ", {ineqSolDNF_, newCondTuple_, condConj_});
    if(isFalseDNF(ineqSolDNF_)) then <<
      % falseを表すタプルを論理積に追加しようとした場合はfalseを表す論理積を返す
      addedCondConj_:= {};
      debugWrite("ans in addCondTupleToCondConj: ", addedCondConj_);
      return addedCondConj_;
    >> else if(isTrueDNF(ineqSolDNF_)) then <<
      % trueを表すタプルを論理積に追加しようとした場合はcondConj_を返す
      addedCondConj_:= condConj_;
      debugWrite("ans in addCondTupleToCondConj: ", addedCondConj_);
      return addedCondConj_;
    >>;
    % DNF形式で返るので、改めてタプルを取り出す
    if(length(first(ineqSolDNF_))>1) then <<
      % 「=」を表す形式の場合はgeqとleqの2つのタプルの論理積が返る
      tmpConj_:= condConj_;
      for i:=1 : length(first(ineqSolDNF_)) do <<
        tmpConj_:= addCondTupleToCondConj(part(first(ineqSolDNF_), i), tmpConj_);
      >>;
      return tmpConj_;
    >> else <<
      newCondTuple_:= first(first(ineqSolDNF_));
    >>
  >>;


  varName_:= getVarNameFromTuple(newCondTuple_);
  relop_:= getRelopFromTuple(newCondTuple_);
  value_:= getValueFromTuple(newCondTuple_);
  debugWrite("{varName_, relop_, value_}: ", {varName_, relop_, value_});

  % 論理積の中から、追加する変数と同じ変数の項を探す
  varTerms_:= union(for each term in condConj_ join
    if(not freeof(term, varName_)) then {term} else {});
  debugWrite("varTerms_: ", varTerms_);


  % 下限・上限を得る
  ubTupleList_:= getUbTupleListFromConj(varTerms_);
  lbTupleList_:= getLbTupleListFromConj(varTerms_);
  % 実数のみ比較対象とする（パラメタとの比較は別の処理で行う）
  ubTupleList_:= for each x in ubTupleList_ join if(not hasParameter(getValueFromTuple(x))) then {x} else {};
  lbTupleList_:= for each x in lbTupleList_ join if(not hasParameter(getValueFromTuple(x))) then {x} else {};
  debugWrite("{ubTupleList_, lbTupleList_}: ", {ubTupleList_, lbTupleList_});
  if(ubTupleList_={}) then ubTupleList_:= {{varName_, leq, INFINITY}};
  if(lbTupleList_={}) then lbTupleList_:= {{varName_, geq, -INFINITY}};
  ubTuple_:= first(ubTupleList_);
  lbTuple_:= first(lbTupleList_);

  % 更新が必要かどうかを、追加する不等式と上下限とを比較することで決定
  ub_:= getValueFromTuple(ubTuple_);
  lb_:= getValueFromTuple(lbTuple_);
  debugWrite("{ub_, lb_}: ", {ub_, lb_});
  if((relop_=leq) or (relop_=lessp)) then <<
    % 上限側を調整
    if(mymin(value_, ub_)=value_) then <<
      % 更新を要するかどうかを判定。上限が同じ場合も、等号の有無によっては更新が必要
      if((value_ neq ub_) or freeof(ubTupleList_, lessp)) then
        addedCondConj_:= cons(newCondTuple_, condConj_ \ ubTupleList_)
      else addedCondConj_:= condConj_;
      ub_:= value_;
      ubTuple_:= newCondTuple_;
    >> else <<
      addedCondConj_:= condConj_;
    >>;
  >> else <<
    % 下限側を調整
    if(mymin(value_, lb_)=lb_) then <<
      % 更新を要するかどうかを判定。下限が同じ場合も、等号の有無によっては更新が必要
      if((value_ neq lb_) or freeof(lbTupleList_, greaterp)) then
        addedCondConj_:= cons(newCondTuple_, condConj_ \ lbTupleList_)
      else addedCondConj_:= condConj_;
      lb_:= value_;
      lbTuple_:= newCondTuple_;
    >> else <<
      addedCondConj_:= condConj_;
    >>;
  >>;
  debugWrite("addedCondConj_: ", addedCondConj_);
  debugWrite("lb_: ", lb_);
  debugWrite("lbTuple_: ", lbTuple_);

  % 最後にlb≦ubを確かめ、矛盾する場合は{}を返す（falseを表す論理積）
  % ubがデフォルト（INFINITY）のままなら確認不要
  if(ub_ neq INFINITY) then << 
    if(mymin(lb_, ub_)=lb_) then <<
      % lb=ubの場合、関係演算子がgeqとleqでなければならない
      if((lb_=ub_) and not isEqualConj({ubTuple_, lbTuple_})) then addedCondConj_:= {};
    >> else <<
      addedCondConj_:= {};
    >>;
  >>;


  debugWrite("ans in addCondTupleToCondConj: ", addedCondConj_);
  debugWrite("{(newCondTuple_), (condConj_)}: ", {newCondTuple_, condConj_});
  return addedCondConj_;
end;

%---------------------------------------------------------------
% 論理式関連の関数
%---------------------------------------------------------------

%数式のリストをandで繋いだ論理式に変換する
procedure mymkand(lst)$
for i:=1:length(lst) mkand part(lst,i);

procedure mymkor(lst)$
for i:=1:length(lst) mkor part(lst, i);

%---------------------------------------------------------------
% 求解・無矛盾性判定関連の関数
%---------------------------------------------------------------

% 有理化を行った上で解を返す
% TODO：実数解のみを返すようにする
procedure exSolve(exprs_, vars_)$
begin;
  scalar tmpSol_, retSol_;

  debugWrite("=== in exSolve: ", {exprs_, vars_});
  % 三角関数まわりの方程式を解いた場合、解は1つに限定してしまう
  % TODO：どうにかする？
  off allbranch;
  tmpSol_:= solve(exprs_, vars_);
  on allbranch;

  % 虚数解を除く
  tmpSol_:= for each x in tmpSol_ join
    if(head(x)=list) then <<
      {for each y in x join 
        if(hasImaginaryNum(rhs(y))) then {} else {y}
      }
    >> else <<
      if(hasImaginaryNum(rhs(x))) then {} else {x}
    >>;
  debugWrite("tmpSol_ after removing imaginary number: ", tmpSol_);

  % 実数上で定義できない値（asin(5)など）を除く
  tmpSol_:= for each x in tmpSol_ join
    if(head(x)=list) then <<
      {for each y in x join 
        if(hasIndefinableNum(rhs(y))) then {} else {y}
      }
    >> else <<
      if(hasIndefinableNum(rhs(x))) then {} else {x}
    >>;
  debugWrite("tmpSol_ after removing indefinable value: ", tmpSol_);

  % 有理化
  retSol_:= for each x in tmpSol_ collect
    if(head(x)=list) then map(rationalise, x)
    else rationalise(x);
  debugWrite("=== ans in exSolve: ", retSol_);
  return retSol_;
end;

% 不等式を、左辺に正の変数名のみがある形式に整形し、それを表すタプルを作る
% (不等式の場合、必ず右辺が0になってしまう（自動的に移項してしまう）ことや、
% 不等式の向きが限定されてしまったりすることがあるため)
% 2次不等式までは解ける
% 前提：入力される不等式も変数も1つ
% TODO：複数の不等式が渡された場合への対応？
% TODO：3次以上への対応？
% 入力：1変数の2次以下の不等式1つ
% 出力：{左辺, relop, 右辺}の形式によるDNFリスト
% 等式の場合にも対応。geqとleqを一緒に使うことにより表現する
procedure exIneqSolve(ineqExpr_)$
begin;
  scalar lhs_, relop_, rhs_, adjustedLhs_, adjustedRelop_, 
         reverseRelop_, adjustedIneqExpr_, sol_, adjustedEqExpr_,
         retTuple_, exprVarList_, exprVar_, retTupleDNF_,
         boundList_, ub_, lb_, eqSol_, eqSolValue_, sqrtCondTuple_, lcofRet_, sqrtCoeffList_,
         sqrtCoeff_, insideSqrtExprs_;

  debugWrite("=== in exIneqSolve: ", ineqExpr_);

  lhs_:= lhs(ineqExpr_);
  relop_:= head(ineqExpr_);
  rhs_:= rhs(ineqExpr_);

  % 複数の変数が入っている場合への対応：tとusrVar→parameter→INFINITYの順に見る
  exprVarList_:= union(for each x in union(variables__, {t}) join
    if(not freeof(ineqExpr_, x)) then {x} else {});
  if(exprVarList_={}) then <<
    exprVarList_:= union(for each x in parameters__ join
      if(not freeof(ineqExpr_, x)) then {x} else {});
  exprVarList_:= if(exprVarList_ neq {}) then exprVarList_ else {INFINITY}
  >>;
  exprVar_:= first(exprVarList_);
  debugWrite("{exprVarList_, exprVar_}: ", {exprVarList_, exprVar_});

  % 右辺を左辺に移項する
  adjustedIneqExpr_:= myApply(relop_, {lhs_+(-1)*rhs_, 0});
  adjustedLhs_:= lhs(adjustedIneqExpr_);
  adjustedRelop_:= head(adjustedIneqExpr_);
  debugWrite("{adjusted IneqExpr, Lhs_, Relop_}: ", 
    {adjustedIneqExpr_, adjustedLhs_, adjustedRelop_});

  % 三角関数を含む場合、特別な符号判定が必要
  if(hasTrigonometricFunc(adjustedLhs_)) then <<
    compareIntervalRet_:= compareInterval(convertValueToInterval(adjustedLhs_), adjustedRelop_, makePointInterval(0));
    debugWrite("compareIntervalRet_: ", compareIntervalRet_);
    if(compareIntervalRet_=t) then <<
      retTupleDNF_:= {{true}};
      debugWrite("exIneqSolve arg: ", ineqExpr_);
      debugWrite("=== ans in exIneqSolve: ", retTupleDNF_);
      return retTupleDNF_;
    >> else if(compareIntervalRet_=nil) then <<
      retTupleDNF_:= {{}};
      debugWrite("exIneqSolve arg: ", ineqExpr_);
      debugWrite("=== ans in exIneqSolve: ", retTupleDNF_);
      return retTupleDNF_;
    >> else <<
      % unknownが返った
      retTupleDNF_:= {{unknown}};
      debugWrite("exIneqSolve arg: ", ineqExpr_);
      debugWrite("=== ans in exIneqSolve: ", retTupleDNF_);
      return retTupleDNF_;
    >>;
  >>;

  % この時点でx+value relop 0または-x+value relop 0の形式（1次式）になっているので、
  % -x+value≦0の場合はx-value≧0の形にする必要がある（relopとして逆のものを得る必要がある）
  % サーバーモードだと≧を使った式表現は保持できないようなので、reverseするかどうかだけ分かれば良い
  % minusで条件判定を行うことがなぜかできないので、式に出現する変数名xを調べて、その係数の正負を調べる
  lcofRet_:= lcof(adjustedLhs_, exprVar_);
  if(not numberp(lcofRet_)) then lcofRet_:= 0;
  sqrtCoeffList_:= getSqrtList(adjustedLhs_, exprVar_, COEFF);
  % 前提：根号の中にパラメタがある項は多くても1つ
  % TODO：なんとかする
  if(sqrtCoeffList_={}) then sqrtCoeff_:= 0
  else if(length(sqrtCoeffList_)=1) then sqrtCoeff_:= first(sqrtCoeffList_);
  % TODO：厳密にはxorを使うべきか？
  if(checkOrderingFormula(lcofRet_<0) or checkOrderingFormula(sqrtCoeff_<0)) then <<
    adjustedRelop_:= getReverseRelop(adjustedRelop_);
  >>;
  debugWrite("adjustedRelop_: ", adjustedRelop_);

  % 変数にsqrtがついてる場合は、変数は0以上であるという条件を最後に追加する必要がある
  % TODO: sqrt(x-1)とかへの対応
  insideSqrtExprs_:= getSqrtList(adjustedLhs_, exprVar_, INSIDE);
  debugWrite("insideSqrtExprs_: ", insideSqrtExprs_);
  sqrtCondTupleList_:= for each x in insideSqrtExprs_ collect
    first(first(exIneqSolve(myApply(geq, {x, 0}))));
  debugWrite("sqrtCondTupleList_: ", sqrtCondTupleList_);
  if(sqrtCondTupleList_={}) then sqrtCondTupleList_:= {true};

  
  % ubまたはlbを求める
  off arbvars;
  on multiplicities;
  sol_:= exSolve(equal(adjustedLhs_, 0), exprVar_);
  debugWrite("sol_ from exSolve: ", sol_);
  on arbvars;
  off multiplicities;

  % TODO：複数変数への対応？
  % 複数変数が入ると2重リストになるはずだが、1変数なら不要か？

  if(sol_={}) then <<
    % adjustedLhs_=0に対する実数解がない場合
    % adjustedIneqExpr_が-t^2-1<0やt^2+5<0などのとき
    if((adjustedRelop_=geq) or (adjustedRelop_=greaterp) or (adjustedRelop_=neq)) then retTupleDNF_:= {{true}}
    else retTupleDNF_:= {{}};
  >> else if(length(sol_)>1) then <<
    % 2次方程式を解いた場合。厳密には長さ2のはず
    % TODO：3次以上への一般化？

    boundList_:= {rhs(part(sol_, 1)), rhs(part(sol_, 2))};
    debugWrite("boundList_: ", boundList_);
    if(not hasParameter(boundList_)) then <<
      if(length(union(boundList_))=1) then <<
        % 重解を表す場合
        lb_:= first(union(boundList_));
        ub_:= first(union(boundList_));
      >> else <<
        lb_:= findMinimumValue(INFINITY, boundList_);
        ub_:= first(boundList_ \ {lb_});
      >>;
      debugWrite("lb_ in exIneqSolve: ", lb_);
      debugWrite("ub_ in exIneqSolve: ", ub_);
    
      % relopによって、tupleDNFの構成を変える
      if((adjustedRelop_ = geq) or (adjustedRelop_ = greaterp)) then <<
        % ≧および＞の場合はorの関係になる
        retTupleDNF_:= {{ {exprVar_, getReverseRelop(adjustedRelop_), lb_} }, { {exprVar_, adjustedRelop_, ub_} }};
      >> else if((adjustedRelop_ = geq) or (adjustedRelop_ = greaterp)) then <<
        % ≦および＜の場合はandの関係になる
        retTupleDNF_:= {{ {exprVar_, getReverseRelop(adjustedRelop_), lb_},     {exprVar_, adjustedRelop_, ub_} }};
      >> else <<
        % ≠の場合
        retTupleDNF_:= {{ {exprVar_, lessp, lb_} }, { {exprVar_, greaterp, lb_}, {exprVar_, lessp, ub_} }, { {exprVar_, greaterp, ub_} }};
      >>;
    >> else <<
      % パラメタの解が得られた場合
      % どちらが下限でどちらが上限になるかをどのように調べるか？？
      % 調べるんじゃなくてそういう式を追加すれば良い      


      retTupleDNF_:= hogehogegege;
    >>;
  >> else <<
    adjustedEqExpr_:= first(sol_);
    retTuple_:= {lhs(adjustedEqExpr_), adjustedRelop_, rhs(adjustedEqExpr_)};
    retTupleDNF_:= {{retTuple_}};
  >>;

  debugWrite("retTupleDNF_ before adding sqrtCondTuple_: ", retTupleDNF_);

  if(not isFalseDNF(retTupleDNF_)) then <<
    debugWrite("sqrtCondTupleList_ add loop: ", sqrtCondTupleList_);
    for i:=1 : length(sqrtCondTupleList_) do <<
      debugWrite("add : ", part(sqrtCondTupleList_, i));
      retTupleDNF_:= addCondTupleToCondDNF(part(sqrtCondTupleList_, i), retTupleDNF_);
      debugWrite("retTupleDNF_ in loop of adding sqrtCondTuple: ", retTupleDNF_);
    >>;
  >>;
  debugWrite("exIneqSolve arg: ", ineqExpr_);
  debugWrite("=== ans in exIneqSolve: ", retTupleDNF_);

  return retTupleDNF_;
end;

procedure exRlqe(formula_)$
begin;
  scalar appliedFormula_, header_, argsCount_, appliedArgsList_;
  debugWrite("formula_: ", formula_);

  % 引数を持たない場合（trueなど）
  if(arglength(formula_)=-1) then <<
    appliedFormula_:= rlqe(formula_);
    return appliedFormula_;
  >>;

  header_:= head(formula_);
  debugWrite("header_: ", header_);
  argsCount_:= arglength(formula_);

  if((header_=and) or (header_=or)) then <<
    argsList_:= for i:=1 : argsCount_ collect part(formula_, i);
    appliedArgsList_:= for each x in argsList_ collect exRlqe(x);
    if(header_= and) then appliedFormula_:= rlqe(mymkand(appliedArgsList_));
    if(header_= or) then appliedFormula_:= rlqe(mymkor(appliedArgsList_));
  >> else if(not freeof(formula_, sqrt)) then <<
    % 数式内にsqrtが入っている時のみ、厳密な大小比較が有効となる
    % TODO:当該の数式内に変数が入った際にも正しく処理ができるようにする
    if(checkOrderingFormula(myApply(getInverseRelop(header_), {lhs(formula_), rhs(formula_)}))) then 
      appliedFormula_:= false 
    else appliedFormula_:= rlqe(formula_);
  >> else <<
    appliedFormula_:= rlqe(formula_);
  >>;

  debugWrite("appliedFormula_: ", appliedFormula_);
  return appliedFormula_;
end;

%---------------------------------------------------------------
% ラプラス変換関連の関数
%---------------------------------------------------------------

% 初期条件init○_○lhsを作成
procedure makeInitId(f,i)$
  if(i=0) then
    mkid(mkid(INIT,f),lhs)
  else
    mkid(mkid(mkid(mkid(INIT,f),_),i),lhs);

%laprule_用、mkidしたｆを演算子として返す
procedure setMkidOperator(f,x)$
  f(x);

%laprule_用の自由演算子
operator !~f$

% 微分に関する変換規則laprule_, letは一度で
let {
  laplace(df(~f(~x),x),x) => il!&*laplace(f(x),x) - makeInitId(f,0),
  laplace(df(~f(~x),x,~n),x) => il!&**n*laplace(f(x),x) -
    for i:=n-1 step -1 until 0 sum
      makeInitId(f,n-1-i) * il!&**i,
  laplace(~f(~x),x) => setMkidOperator(mkid(lap,f),il!&)
}$

% ラプラス変換対の作成, オペレータ宣言
% @return {v, v(t), lapv(s)}の対応表
procedure laplaceLetUnit(lapList_)$
begin;
  scalar arg_, LAParg_;

  arg_:= first lapList_;
  LAParg_:= second lapList_;

  % arg_が重複してないか判定
  if(freeof(loadedOperator__,arg_)) then 
    << 
     operator arg_, LAParg_;
     loadedOperator__:= arg_ . loadedOperator__;
    >>;

  return {arg_, arg_(t), LAParg_(s)};
end;

% vars_からdfを除いたものを返す
procedure removedf(vars_)$
begin;
  scalar exceptDfVars_;
  exceptDfVars_:= {};
  for each x in vars_ collect
    if(freeof(x,df)) then exceptDfVars_:= x . exceptDfVars_;
  return exceptDfVars_;
end;

retsolvererror___ := 0;
retoverconstraint___ := 2;
retunderconstraint___ := 3;


procedure getf(x,lst)$
  if(lst={}) then nil else
    if(x=lhs(first(lst))) then rhs(first(lst)) else
      getf(x,rest(lst));

%入力: 変数名, 等式のリストのリスト(ex. {{x=1,y=2},{x=3,y=4},...})
%出力: 変数に対応する値のリスト
procedure lgetf(x,llst)$
  if(llst={}) then {} else
    if(rest(llst)={}) then getf(x,first(llst)) else
      getf(x,first(llst)) . {lgetf(x,rest(llst))};


% ラプラス変換を用いて微分方程式の求解を行う
% @param expr_ "df(v,t,n) = f(t)"の格好をした方程式
% @return retsolvererror___, retoverconstraint___ retunderconstraint___
procedure exDSolve(expr_, initCons_, vars_)$
begin;
  scalar ans_, lapList_, lapTable_, lapPattern_;
  scalar exceptDfVars_, subedExpr_, diffExpr_, lapExpr_, solveVars_, solveAns_, isUnderConstraint_;

  debugWrite("in exDSolve", " ");
  debugWrite("{expr_, initCons_, vars_}: ", {expr_, initCons_, vars_});

  exceptDfVars_:= removedf(vars_);
  lapList_:= for each x in exceptDfVars_ collect {x,mkid(lap,x)};

  % ラプラス変換規則, {{v, v(t), lapv(s)},...}の対応表
  lapTable_:= map(laplaceLetUnit, lapList_);
  debugWrite("lapTable_: ", lapTable_);

  % ht => ht(t)置換
  lapPattern_:= map(first(~w)=second(~w), lapTable_);
  subedExpr_:= sub(lapPattern_, expr_);

  % expr_を等式から差式形式に
  diffExpr_:= {};
  for each x in subedExpr_ do
    if(not freeof(x, equal)) then
      diffExpr_:= append(diffExpr_, {lhs(x) - rhs(x)})
    else diffExpr_:= append(diffExpr_, {lhs(x) - rhs(x)});

  % エラー時の式にはlaplace演算子が含まれる
  if(not freeof(lapExpr_, laplace)) then return ansWrite("exDSolve", retsolvererror___);

  % ラプラス変換
  lapExpr_:=map(laplace(~w,t,s), diffExpr_);
  debugWrite("lapExpr_: ", lapExpr_);

  % 逆ラプラス変換の対象
  solveVars_:= union(map(third, lapTable_), map(lhs, initCons_), {s});
  debugWrite("solveVars_:", solveVars_);

  % 変換対と初期条件を連立して解く
  solveAns_:= solve(union(lapExpr_, initCons_), solveVars_);
  debugWrite("solveAns_: ", solveAns_);

  % solveが解無しの時, 又はsがarbcomplexでない値を持つ時overconstraintと想定
  if(solveAns_={} or freeof(lgetf(s, solveAns_), arbcomplex)) then
    return ansWrite("exDSolve", retoverconstraint___);

  % solveAns_にsolveVars_の解が一つも含まれない時 underconstraintと想定
  isUnderConstraint_:= false;
  for each x in lapTable_ do 
    if(freeof(solveAns_, third(x))) then isUnderConstraint_:= true;
  if(isUnderConstraint_=true) then
    return ansWrite("exDSolve", retunderconstraint___);

  % 逆ラプラス変換
  ans_:= for each x in lapTable_ collect
           (first x) = invlap(lgetf((third x), solveAns_),s,t);

  return ansWrite("exDSolve", ans_);
end;

%---------------------------------------------------------------
% 特定の要素を抽出/削除したり複数の要素を分類したりする関数
%---------------------------------------------------------------

procedure getFrontTwoElemList(lst_)$
  if(length(lst_)<2) then {} else
    for i:=1 : 2 collect part(lst_, i);

procedure removeTrueList(patternList_)$
  for each x in patternList_ join if(rlqe(x)=true) then {} else {x};

% 論理式がtrueであるとき、現在はそのままtrueを返している
% TODO：なんとかする
procedure removeTrueFormula(formula_)$
  if(formula_=true) then true else
    myApply(and, for each x in getArgsList(formula_) join 
      if(rlqe(x)=true) then {} else {x});

% 制約リストから、等式以外を含む制約を抽出する
procedure getOtherExpr(exprs_)$
  for each x in exprs_ join if(hasIneqRelop(x) or hasLogicalOp(x)) then {x} else {};

% NDExpr（exDSolveで扱えないような制約式）であるかどうかを調べる
% 式の中にsinもcosも入っていなければfalse
procedure isNDExpr(expr_)$
  if(freeof(expr_, sin) and freeof(expr_, cos)) then nil else t;

procedure splitExprs(exprs_, vars_)$
begin;
  scalar otherExprs_, NDExprs_, NDExprVars_, DExprs_, DExprVars_;

  otherExprs_:= union(for each x in exprs_ join 
                  if(hasIneqRelop(x) or hasLogicalOp(x)) then {x} else {});
  NDExprs_ := union(for each x in (exprs_ \ otherExprs_) join 
                if(isNDExpr(x)) then {x} else {});
  NDExprVars_ := union(for each x in vars_ join if(not freeof(NDExprs_, x)) then {x} else {});
  DExprs_ := (exprs_ \ otherExprs_) \ NDExprs_;
  DExprVars_:= union(for each x in vars_ join if(not freeof(DExprs_, x)) then {x} else {});
  return {NDExprs_, NDExprVars_, DExprs_, DExprVars_, otherExprs_};
end;

% 前提：入力はinitxlhs=prev(x)の形
% TODO：なんとかする
procedure getInitVars(rcont_)$
  lhs(rcont_);

procedure getIneqBoundTCLists(ineqTCList_)$
begin;
  scalar lbTCList_, ubTCList_, timeExpr_, condExpr_, relop_, exprLhs_, lcofRet_, solveAns_;

  lbTCList_:= {};
  ubTCList_:= {};
  for each x in ineqTCList_ do <<
    timeExpr_:= getTimeFromTC(x);
    condExpr_:= getCondDNFFromTC(x);

    % timeExpr_は (t - value) op 0 または (-t - value) op 0 の形を想定
    relop_:= head(timeExpr_);
    exprLhs_:= lhs(timeExpr_);
    lcofRet_:= lcof(exprLhs_, t);
    if(checkOrderingFormula(lcofRet_<0)) then relop_:= getReverseRelop(relop_);
    solveAns_:= first(solve(exprLhs_=0, t));

    if((relop_ = geq) or (relop_ = greaterp)) then
      lbTCList_:= cons({rhs(solveAns_), condExpr_}, lbTCList_)
    else ubTCList_:= cons({rhs(solveAns_), condExpr_}, ubTCList_);
  >>;
  debugWrite("lbTCList_: ", lbTCList_);
  debugWrite("ubTCList_: ", ubTCList_);
  return {lbTCList_, ubTCList_};
end;

%---------------------------------------------------------------
% HydLa向け関数（通常の限量子に使用可能な変数関連）
%---------------------------------------------------------------

% 左辺が限量可能(dfやprevでない)な変数の式を抽出する
procedure getLhsQuantified(exprs_)$
  union(for each x in exprs_ join
    if(freeof(lhs x, df) and freeof(lhs x, prev)) then {x} else {}
  );

% 変数のリストからdf変数でもprev変数でもない変数のみ抽出する
procedure getQuantifiedVars(vars_)$
  union(for each x in vars_ join
    if(freeof(x, prev) and freeof(x, df)) then {x} else {}
  );

%---------------------------------------------------------------
% HydLa向け関数（df変数関連）
%---------------------------------------------------------------

procedure isDifferentialVar(var_)$
  if(arglength(var_)=-1) then nil
  else if(head(var_)=df) then t
  else nil;

% リストからdf変数を除く
procedure removeDfCons(consList_)$
  union(for each x in consList_ join if(freeof(x, df)) then {x} else {});

% 変数のリストからdf変数のみ抽出する
procedure getDfVars(vars_)$
  union(for each x in vars_ join
    if(freeof(x, prev) and not freeof(x, df)) then {x} else {}
  );

%---------------------------------------------------------------
% HydLa向け関数（prev変数関連）
%---------------------------------------------------------------

procedure isPrevVariable(expr_)$
  if(arglength(expr_)=-1) then nil
  else if(head(expr_)=prev) then t
  else nil;

% prev変数の場合prevを外す
procedure removePrev(var_)$
  if(head(var_)=prev) then part(var_, 1) else var_;

% リストからprev変数を除く
procedure removePrevCons(consList_)$
  union(for each x in consList_ join if(freeof(x, prev)) then {x} else {});

%---------------------------------------------------------------
% HydLa向け関数（init変数関連）
%---------------------------------------------------------------

% REDUCEStringSenderから呼び出す
% @param vars_ init変数のリスト
procedure addInitVariables(vars_)$
begin;

  initVariables__:= union(initVariables__, vars_);
  debugWrite("initVariables__: ", initVariables__);
end;

% initVariables__に含まれる変数かどうか調べる
procedure isInitVariable(var_)$
begin;
  scalar ret_;

  if(arglength(var_) neq -1) then return nil;
  ret_ := nil;
  for each x in initVariables__ do if x = var_ then ret_ := t;
  return ret_;
end;

procedure removeInitCons(consList_)$
begin;

  return union(consList_ \
    for each initVariable in initVariables__ join
      for each x in consList_ join 
        if(freeof(x, initVariable)) then {} else {x}
  );
end;

%---------------------------------------------------------------
% HydLa向け関数（共通して必要）
%---------------------------------------------------------------

rettrue___    := 1;
retfalse___   := 2;
retunknown___ := 3;


% 制約ストアのリセット
procedure resetConstraint()$
begin;

  constraint__ := {};
  variables__ := {};
  pConstraint__:= {};
  prevConstraint__:= {};
  parameters__:= {};
  isTemporary__:= nil;
  initConstraint__ := {};
  initTmpConstraint__:= {};
  tmpConstraint__:= {};
  tmpVariables__:= {};
  prevVariables__:= {};
  guard__:= {};
  guardVars__:= {};
  initVariables__:= {};
  debugWrite("constraint__: ", constraint__);
  debugWrite("variables__: ", variables__);
  debugWrite("pConstraint__: ", pConstraint__);
  debugWrite("prevConstraint__: ", prevConstraint__);
  debugWrite("prevVariables__: ", prevVariables__);
  debugWrite("parameters__: ", parameters__);
  debugWrite("isTemporary__", isTemporary__);
  debugWrite("initConstraint__", initConstraint__);
  debugWrite("initTmpConstraint__", initTmpConstraint__);
  debugWrite("tmpVariables__", tmpVariables__);
  debugWrite("initVariables__", initVariables__);
end;

procedure resetConstraintForVariable()$
begin;

  constraint__ := {};
  variables__ := {};
  tmpVariables__:= {};
  prevVariables__:= {};
  debugWrite("constraint__: ", constraint__);
  debugWrite("variables__: ", variables__);
  debugWrite("tmpVariables__", tmpVariables__);
  debugWrite("prevVariables__: ", prevVariables__);
end;

% TODO co_にprevConstraint__を適用する
procedure addInitConstraint(co_, va_)$
begin;

  debugWrite("prevConstraint__: ", prevConstraint__);
  debugWrite("co_: ", co_);
  debugWrite("va_: ", va_);

  if(isTemporary__) then
  <<
    tmpVariables__:= union(tmpVariables__, va_);
    initTmpConstraint__ := union(initTmpConstraint__, exSub(prevConstraint__, co_));
  >> else
  <<
    variables__ := union(variables__, va_);
    initConstraint__ := union(initConstraint__, exSub(prevConstraint__, co_));
  >>;

  debugWrite("tmpVariables__: ", tmpVariables__);
  debugWrite("initTmpConstraint__: ", initTmpConstraint__);
  debugWrite("variables__: ", variables__);
  debugWrite("initConstraint__: ", initConstraint__);
end;

procedure addPrevConstraint(cons_, vars_)$
begin;

  prevConstraint__:= union(prevConstraint__, cons_);
  prevVariables__:= union(prevVariables__, vars_);

  debugWrite("prevConstraint__: ", prevConstraint__);
  debugWrite("prevVariables__: ", prevVariables__);
end;

procedure addGuard(gu_, vars_)$
begin;
  if(part(gu_,0)=list) then
  <<
    guard__:= union(guard__, gu_); 
  >> else
  <<
    guard__:= union(guard__, {gu_}); 
  >>;

  guardVars__:= union(guardVars__, vars_);
  debugWrite("guard__: ", guard__);
  debugWrite("guardVars__: ", guardVars__);
end;

procedure setGuard(gu_, vars_)$
begin;
  if(part(gu_,0)=list) then
  <<
    guard__:= gu_; 
  >> else
  <<
    guard__:= {gu_};
  >>;

  guardVars__:= vars_;
  debugWrite("guard__: ", guard__);
  debugWrite("guardVars__: ", guardVars__);
end;

procedure startTemporary()$
begin;
  isTemporary__:= t;
end;

procedure endTemporary()$
begin;
  isTemporary__:= nil;
  resetTemporaryConstraint();
end;

procedure resetTemporaryConstraint()$
begin;
  tmpConstraint__:= {};
  initTmpConstraint__:= {};
  tmpVariables__:= {};
  guard__:= {};
  guardVars__:= {};
end;

procedure resetConstraintForParameter(pcons_, pars_)$
begin;
  pConstraint__:= {};
  parameters__:= {};
  addParameterConstraint(pcons_, pars_);
end;

% addConstraintは行わない
% PP/IPで共通のreset時に行う、制約ストアへの制約の追加
procedure addParameterConstraint(pcons_, pars_)$
begin;
  debugWrite("in addParameterConstraint", " ");

  pConstraint__ := union(pConstraint__, pcons_);
  parameters__ := union(parameters__, pars_);
  debugWrite("new pConstraint__: ", pConstraint__);
  debugWrite("new parameters__: ", parameters__);

end;

% 制約ストアへの制約の追加
procedure addConstraint(cons_, vars_)$
begin;

  debugWrite("in addConstraint", " ");
  debugWrite("cons_: ", cons_);
  debugWrite("vars_: ", vars_);
  debugWrite("prevConstraint__: ", prevConstraint__);

  if(isTemporary__) then
  <<
    tmpVariables__:= union(tmpVariables__, vars_);
    tmpConstraint__:= union(tmpConstraint__, exSub(prevConstraint__, cons_));
  >> else
  <<
    variables__:= union(variables__, vars_);
    constraint__:= union(constraint__, exSub(prevConstraint__, cons_));
  >>;

  debugWrite("variables__: ", variables__);
  debugWrite("constraint__: ", constraint__);
  debugWrite("tmpVariables__: ", tmpVariables__);
  debugWrite("tmpConstraint__: ", tmpConstraint__);

  debugWrite("exSub(prevConstraint__, cons_): ", exSub(prevConstraint__, cons_));
  return if(isTemporary__) then tmpConstraint__ else constraint__;
end;

procedure getConstraintStore()$
begin;

  debugWrite("constraint__:", constraint__);
  if(constraint__={}) then return {{}, pConstraint__};

  % 解を1つだけ得る
  % TODO: Orでつながった複数解への対応
  if(head(first(constraint__))=list) then return {first(constraint__), pConstraint__}
  else return {constraint__, pConstraint__};
end;


procedure checkLessThan(lhs_, rhs_)$
begin;
  scalar ret_;

  ret_:= if(mymin(lhs_, rhs_) = lhs_) then rettrue___ else retfalse___;
  debugWrite("ans in checkLessThan: ", ret_);

  return ret_;
end;

procedure simplifyExpr(expr_)$
begin;
  scalar simplifiedExpr_;

  % TODO:simplify関数を使う
%  simplifiedExpr_:= simplify(expr_);
  simplifiedExpr_:= expr_;
  debugWrite("simplifiedExpr_:", simplifiedExpr_);

  return simplifiedExpr_;
end;

procedure exprTimeShift(expr_, time_)$
begin;
  scalar shiftedExpr_;

  shiftedExpr_:= sub(t=t-time_, expr_);
  debugWrite("shiftedExpr_:", shiftedExpr_);

  return shiftedExpr_;
end;

%---------------------------------------------------------------
% HydLa向け関数（PPにおいて必要）
%---------------------------------------------------------------

% @return {trueMap, falseMap} または{false, true}
procedure checkConsistencyPoint()$
begin;
  scalar ans_, ansBool_, trueMap_, falseMap_, cons_, vars_;

  cons_:= union(constraint__, tmpConstraint__, guard__, initConstraint__, initTmpConstraint__);
  vars_:= union(variables__, tmpVariables__, guardVars__);
  
  ansBool_:= checkConsistencyPointMain(cons_, pConstraint__, vars_);

  if(pConstraint__ = {}) then
    return ansWrite("checkConsistencyPoint", {ansBool_, rlqe(not ansBool_)});

  trueMap_:= rlqe(ansBool_ and mymkand(pConstraint__));
  falseMap_:= rlqe(not ansBool_ and mymkand(pConstraint__));

  trueMap_:= if(head(trueMap_) = and) then map(makeConsTuple, getArgsList(trueMap_)) else trueMap_;
  falseMap_:= if(head(falseMap_) = and) then map(makeConsTuple, getArgsList(falseMap_)) else falseMap_;
  ans_:= {trueMap_, falseMap_};

  return ansWrite("checkConsistencyPoint", ans_);
end;

% PPにおける無矛盾性の判定
% 仕様 QE未使用 % (使用するなら, 変数は基本命題的に置き換え)
% @param cons_ 制約集合
% @param pCons_ 記号定数に関する制約の集合, 現状未使用
% @param vars_ 変数集合
% @return true or false
procedure checkConsistencyPointMain(cons_, pCons_, vars_)$
begin;
  scalar eqExprs_, otherExprs_, modeFlagList_, mode_, tmpSol_,
         solvedExprs_, solvedExprsQE_, sol_, ans_;

  debugWrite("checkConsistencyPointMain: ", " ");
  debugWrite("{cons_, pCons_, vars_}: ", {cons_, pCons_, vars_});

  % exSolveの前処理
  if(cons_={}) then
    return ansWrite("checkConsistencyPointMain", true);

  otherExprs_:= getOtherExpr(cons_);
  eqExprs_:= cons_ \ otherExprs_;

  % 未知変数を追加しないようにする
  off arbvars;
  tmpSol_:= exSolve(eqExprs_, vars_);
  on arbvars;
  debugWrite("tmpSol_: ", tmpSol_);

  if(tmpSol_={}) then
    return ansWrite("checkConsistencyPointMain", false);

  % TODO:複数解得られた場合への対応
  if(head(first(tmpSol_))=list) then tmpSol_:= first(tmpSol_);

  % otherExprs_が空の場合sub処理を省略する
  if(otherExprs_={}) then
    return ansWrite("checkConsistencyPointMain", rlqe(not isEmpty(tmpSol_)));

  % subの拡張版を用いる手法
  solvedExprs_:= union(for each x in otherExprs_ join {exSub(tmpSol_, x)});
  debugWrite("solvedExprs_:", solvedExprs_);
  solvedExprsQE_:= exRlqe(mymkand(solvedExprs_));

  debugWrite("solvedExprsQE_:", solvedExprsQE_);
  debugWrite("union(tmpSol_, solvedExprsQE_):", union(tmpSol_, {solvedExprsQE_}));

  sol_:= rlqe(mymkand(union(tmpSol_, {solvedExprsQE_})));
  ans_:= if(sol_ <> false) then return true else return false;

  return ansWrite("checkConsistencyPointMain", ans_);
end;

procedure applyPrevCons(csList_, retList_)$
begin;
  scalar firstCons_, newCsList_, ret_;
%  debugWrite("=== in applyPrevCons", " ");
  if(csList_={}) then return retList_;

  firstCons_:= first(csList_);
%  debugWrite("firstCons_: ", firstCons_);
  if(not freeof(lhs(firstCons_), prev)) then <<
    newCsList_:= union(for each x in rest(csList_) join {exSub({firstCons_}, x)});
    ret_:= applyPrevCons(rest(csList_), retList_);
  >> else if(not freeof(rhs(firstCons_), prev)) then <<
    ret_:= applyPrevCons(cons(getReverseCons(firstCons_), rest(csList_)), retList_);
  >> else <<
    ret_:= applyPrevCons(rest(csList_), cons(firstCons_, retList_));
  >>;
  return ret_;
end;

% createVariableMap内で使う、整形用関数
procedure makeConsTuple(cons_)$
begin;
  scalar varName_, relopCode_, value_, tupleDNF_, retTuple_, adjustedCons_, sol_;
  debugWrite("in makeConsTuple: ", cons_);
  
  % 左辺に変数名のみがある形式にする
  % 前提：等式はすでにこの形式になっている
  if(not hasIneqRelop(cons_)) then <<
    varName_:= lhs(cons_);
    relopCode_:= getExprCode(cons_);
    value_:= rhs(cons_);
  >> else <<
    tupleDNF_:= exIneqSolve(cons_);
    debugWrite("tupleDNF_: ", tupleDNF_);
    % 1次式になってるはずなので、解は1つなはず
    retTuple_:= first(first(tupleDNF_));
    
    varName_:= getVarNameFromTuple(retTuple_);
    relopCode_:= getExprCode(getRelopFromTuple(retTuple_));
    value_:= getValueFromTuple(retTuple_);
  >>;
  debugWrite("ans in makeConsTuple: ", {varName_, relopCode_, value_});
  return {varName_, relopCode_, value_};
end;

% hyrose向けのrlstruct
% @param cons_ 制約のリスト
% @param vars_ prevの覗かれた変数のリスト, df変数の順序を気にしなくて良い
% @return      {vの変数表, vで表現した制約の論理式}
procedure hyroseRlStruct(cons_, vars_)$
begin;
  scalar ansFormula_, ansMap_, formulaList_, varsMap1_;
  debugWrite("{cons_, vars_}: ", {cons_, vars_});
  varsMap1_:= for i:= 1:length(vars_) collect part(vars_, i) = mkid(v,i);
  formulaList_:= for each x in cons_ collect exSub(varsMap1_, x);

  ansFormula_:= mymkand formulaList_;
  ansMap_:= for i:= 1:length(vars_) collect mkid(v,i) = part(vars_, i);
  debugWrite("ans in hyroseRlStruct: ", {ansFormula_, ansMap_});

  return {ansFormula_, ansMap_};
end;

% createVariableMap内で変数表の前処理を行う
% df変数はただの変数として扱い、連立方程式の求解と整形を行う
% exIneqSolveの都合上, 方程式のみ整形操作を行う
% @param cons_ 制約のリスト
% @param vars_ 変数のリスト
% @return      exIneqSolveの食える形になった制約のリスト
procedure solveCS(cons_, vars_)$
begin;
  scalar ans_, structAns_, rationalisedCons_, equalities_, equalityVars_;
  scalar vAns_, formula, vVarsMap_, vVars_, varsExecptOne_;
  debugWrite("in solveCS", " ");

  rationalisedCons_:= map(rationalise, cons_);

  structAns_:= hyroseRlStruct(rationalisedCons_, removePrevCons vars_);
  formula:= first structAns_;
  vVarsMap_:= second structAns_;

  % v変数によるcons_のQE
  vVars_:= for each x in vVarsMap_ collect lhs x;
  vAns_:= {};
  for each x in vVars_ do <<
    varsExecptOne_:= vVars_ \ {x};
    vAns_:= vAns_ union {rlqe(ex(varsExecptOne_, formula))};
  >>;

  % TODO andが入る場合式に展開するが正しい対応か？
  vAns_:= for each x in vAns_ join if(head(x) = and) then getArgsList(x) else {x};

  vAns_:= sub(vVarsMap_, vAns_);
  debugWrite("vAns_: ", vAns_);

  % 等式だけsolveで整形する
  equalities_:= for each x in vAns_ join if(head(x) = equal) then {x} else {};
  equalityVars_:= for each x in vars_ join if(not freeof(equalities_, x)) then {x} else {};
  ans_:= first solve(equalities_, equalityVars_);
  if(head ans_ <> list) then ans_:= {ans_};
  ans_:= for each x in ans_ join if(freeof(x, arbcomplex)) then {x} else {};

  ans_:= ans_ union
    for each x in vAns_ join if(head(x) <> equal) then {x} else {};

  debugWrite("ans in solveCS: ", ans_);
  return ans_;
end;

procedure createVariableMap()$
begin;
  debugWrite("constraint__: ", constraint__);

  return createVariableMapMain(union(constraint__, removePrevCons(initConstraint__)), variables__, pConstraint__);
end;

% 前提：Orでつながってはいない
% TODO：なんとかする
procedure createVariableMapMain(cons_, vars_, pars_)$
begin;
  scalar removedVars_, solvedCons_, consTmpRet_, consRet_, paramDNFList_, paramRet_, tuple_, ret_;

  debugWrite("=== in createVariableMapMain", " ");
  debugWrite("{cons_, vars_, pars_}: ", {cons_, vars_, pars_});

  solvedCons_ := solveCS(cons_, vars_);
  consTmpRet_:= applyPrevCons(solvedCons_, {});
  debugWrite("consTmpRet_: ", consTmpRet_);

  % 式を{(変数名), (関係演算子コード), (値のフル文字列)}の形式に変換する
  consRet_:= map(makeConsTuple, consTmpRet_);
  paramDNFList_:= map(exIneqSolve, pars_);
  paramRet_:= for each x in paramDNFList_ collect <<
    % 1つの項になっているはず
    tuple_:= first(first(x));
    {getVarNameFromTuple(tuple_), getExprCode(getRelopFromTuple(tuple_)), getValueFromTuple(tuple_)}
  >>;
  ret_:= union(consRet_, paramRet_);

  ret_:= getUsrVars(ret_, removePrevCons(vars_));

  debugWrite("=== ans in createVariableMapMain: ", ret_);
  return ret_;
end;

%---------------------------------------------------------------
% HydLa向け関数（IPにおいて必要）
%---------------------------------------------------------------

% TODO!! check_consistency_result_tの要件をしっかり確認すること
% initConstraint__,  initTmpConstraint__, tmpVariables__ 対応版
% TODO: .m::checkConsistencyPoint[constraint && tmpConstraint && guard && initConstraint && initTmpConstraint, pConstraint, Union[variables, tmpVariables, guardVars]] の全変数に対応したい
% @return {true, false} または{false, true}
procedure checkConsistencyInterval()$
begin;
  scalar ans_, ansBool_, trueMap_, falseMap_;
  
  ans_:= checkConsistencyIntervalMain(union(constraint__, tmpConstraint__), guard__,
           union(initConstraint__, initTmpConstraint__), pConstraint__, union(variables__, tmpVariables__, guardVars__));
  debugWrite("ans_ in checkConsistencyIntervalMain: ", ans_);

  ansBool_:= rlqe(part(ans_, 1) = ICI_CONSISTENT___);

  if(part(ans_, 1)=ICI_UNKNOWN___) then ans_:= UNKNOWN
  else if pConstraint__ <> {} then << 
    trueMap_:= rlqe(ansBool_ and mymkand(pConstraint__));
    falseMap_:= rlqe(not ansBool_ and mymkand(pConstraint__));

    % TODO prev変数の処理, t>0としてtの除去
    trueMap_:= if(head(trueMap_) = and) then map(makeConsTuple, getArgsList(trueMap_)) else trueMap_;
    falseMap_:= if(head(falseMap_) = and) then map(makeConsTuple, getArgsList(falseMap_)) else falseMap_;
    ans_:= {trueMap_, falseMap_};
  >> else if(ansBool_ = true) then ans_:= {true, false}
  else ans_:= {false, true};

  debugWrite("ans_ in checkConsistencyInterval: ", ans_);

  return ans_;
end;

ICI_SOLVER_ERROR___:= 0;
ICI_CONSISTENT___:= 1;
ICI_INCONSISTENT___:= 2;
ICI_UNKNOWN___:= 3; 

procedure checkConsistencyIntervalMain(cons_, guardCons_, initCons_, pCons_, vars_)$
begin;
  scalar NDExprs_, NDExprVars_, DExprs_, DExprVars_, otherExprs_, DExprRconts_, DExprRcontsVars_,
         integratedSol_, exDSolveAns_, dfAnsExprs_, splitExprsResult_,
         prevVars_, initVars_, noDifferentialVars_, noPrevVars_, integratedVariableMap_, dfVariableMap_,
         subedGuardQE_, subedGuardEqualList_, ineqSolDNFList_, ineqSolDNF_, isInf_, ans_;

  debugWrite("{variables__, parameters__}: ", {variables__, parameters__});
  debugWrite("{cons_, guardCons_, initCons_, pCons_, vars_}: ", {cons_, guardCons_, initCons_, pCons_, vars_});

  % SinやCosが含まれる場合はラプラス変換不可能なのでNDExpr扱いする
  splitExprsResult_ := splitExprs(removePrevCons(cons_), vars_);

  debugWrite("{NDExprs_, NDExprVars_, DExprs_, DExprVars_, otherExprs_}: ", splitExprsResult_);
  % ここでotherExprs_は不等式やand, orを含む式, 未使用!

  DExprs_ :=    part(splitExprsResult_, 3); % 微分項を含む式
  DExprVars_ := part(splitExprsResult_, 4);
  DExprRconts_:= removeInitCons(initCons_);
  debugWrite("DExprRconts_: ", DExprRconts_);

  if(DExprRconts_ neq {}) then <<
    prevVars_:= for each x in vars_ join if(isPrevVariable(x)) then {x} else {};
    debugWrite("prevVars_: ", prevVars_);
    noPrevVars_:= union(for each x in prevVars_ collect part(x, 1));
    debugWrite("noPrevVars_: ", noPrevVars_);
    DExprRcontsVars_ := union(for each x in noPrevVars_ join if(not freeof(DExprRconts_, x)) then {x} else {});
    debugWrite("DExprRcontsVars_: ", DExprRcontsVars_);
    DExprs_:= union(DExprs_, DExprRconts_);
    DExprVars_:= union(DExprVars_, DExprRcontsVars_);
  >>;

  initVars_:= map(getInitVars, initCons_);
  debugWrite("initVars_: ", initVars_);

  noDifferentialVars_:= union(for each x in DExprVars_ collect if(isDifferentialVar(x)) then part(x, 1) else x);
  debugWrite("noDifferentialVars_: ", noDifferentialVars_);

  %========== exDSolve

  exDSolveAns_:= exDSolve(DExprs_, initCons_, noDifferentialVars_);
  
  if(exDSolveAns_ = retsolvererror___) then    return {ICI_SOLVER_ERROR___};
  if(exDSolveAns_ = retoverconstraint___) then return {ICI_INCONSISTENT___};

  dfVariableMap_:= first(foldLeft(createIntegratedValue, {{},exDSolveAns_}, union(DExprVars_, (vars_ \ initVars_))));
  debugWrite("dfVariableMap_:", dfVariableMap_);
  dfAnsExprs_:= for each x in dfVariableMap_ collect (part(x, 1)=part(x, 2));
  debugWrite("dfAnsExprs_ from exDSolve: ", dfAnsExprs_);

  %========= exDSolve結果(dfAnsExprs_)の整形

  % TODO CHECK: union(dfAnsExprs_, NDExprs_) = {} の場合どうなる?

  % 微分項を含まない方程式
  NDExprs_ :=   part(splitExprsResult_, 1);
  NDExprVars_:= part(splitExprsResult_, 2);

  % NDExpr_を連立
  if(union(dfAnsExprs_, NDExprs_) neq {}) then <<
    solveAns_:= solve(union(dfAnsExprs_, NDExprs_), union(DExprVars_, union(NDExprVars_, (vars_ \ initVars_))));
    debugWrite("solveAns_ after solve with NDExpr_: ", solveAns_);
    if(solveAns_ = {}) then return {ICI_INCONSISTENT___};
  >>;

  % guardCons_がない場合は無矛盾と判定して良い
  if(guardCons_ = {}) then return {ICI_CONSISTENT___};

  %========= exDSolve結果から変数表の作成

  integratedVariableMap_:= first(foldLeft(createIntegratedValue, {{},solveAns_}, union(DExprVars_, union(NDExprVars_, (vars_ \ initVars_)))));
  debugWrite("integratedVariableMap_:", integratedVariableMap_);
  integratedSol_:= for each x in integratedVariableMap_ collect (first(x)=second(x));
  debugWrite("integratedSol_:", integratedSol_);

  % ======= 変数表をguardCons_に適用

  subedGuardCons_:= sub(integratedSol_, guardCons_);
  debugWrite("subedGuardCons_: ", subedGuardCons_);

  % 前提：ParseTree構築時に分割されているはずなのでガードにorが入ることは考えない
  subedGuardList_:=
    if(head(first(subedGuardCons_))=and) then <<
      union(for each x in getArgsList(first(subedGuardCons_)) join <<
        subedGuardQE_:= rlqe(x);
        if(subedGuardQE_=true) then {}
        else if(subedGuardQE_=false) then {false}
        else {x}
      >>)
    >> else <<
      subedGuardQE_:= rlqe(first(subedGuardCons_));
      if(subedGuardQE_=true) then {}
      else if(subedGuardQE_=false) then {false}
      else subedGuardCons_
    >>;
  debugWrite("subedGuardList_: ", subedGuardList_);

  % ガード条件全体がtrueの場合
  if(subedGuardList_={}) then return {ICI_CONSISTENT___};
  % ガード条件内にfalseが入る場合
  if(not freeof(subedGuardList_, false)) then return {ICI_INCONSISTENT___};

  % ======= subedGuardList_の整形

  subedGuardEqualList_:= for each x in subedGuardList_ join 
    if(head(x)=equal) then {x}
    else {};
  debugWrite("subedGuardEqualList_: ", subedGuardEqualList_);
  % ガード条件判定においてはandでつながった等式がある場合、結果はfalse
  if(subedGuardEqualList_ neq {}) then return {ICI_INCONSISTENT___};

  % =======  それぞれの不等式について、1次にしてバックエンドDNF(ineqSolDNFList_)にする。

  % 前提: それぞれの要素間はandの関係である
  ineqSolDNFList_:= 
    union(for each x in (subedGuardList_ \ subedGuardEqualList_) collect <<
      ineqSolDNF_:= exIneqSolve(x);
      debugWrite("ineqSolDNF_: ", ineqSolDNF_);
      if(isFalseDNF(ineqSolDNF_)) then {{}}
      else if(isTrueDNF(ineqSolDNF_)) then {{true}}
      else ineqSolDNF_
    >>);

  debugWrite("ineqSolDNFList_:", ineqSolDNFList_);

  % ======= t>0 と and を取る

  ineqSolDNF_:= foldLeft(addCondDNFToCondDNF, {{{t, greaterp, 0}}}, ineqSolDNFList_);
  debugWrite("ineqSolDNF_:", ineqSolDNF_);

  % 不等式の場合、ここで初めて矛盾が見つかり、ineqSolDNF_がfalseになることがある
  ans_:=
    if(isTrueDNF(ineqSolDNF_)) then              {ICI_CONSISTENT___}
    else if(isFalseDNF(ineqSolDNF_)) then        {ICI_INCONSISTENT___}
    else <<
      isInf_:= checkInfUnitDNF(ineqSolDNF_);
      if(checkInfUnitDNF(ineqSolDNF_)=true) then {ICI_CONSISTENT___}
             else if(isInf_=false) then          {ICI_INCONSISTENT___}
             else                                {ICI_UNKNOWN___}
    >>;

  return ansWrite("checkConsistencyIntervalMain", ans_);
end;

procedure checkInfUnitDNF(tDNF_)$
begin;
  scalar conj_, infCheckAns_, orArgsAnsList_, lbTupleList_, lbTuple_;
  debugWrite("in checkInfUnitDNF: ", tDNF_);

  conj_:= first(tDNF_);
  if(length(tDNF_)>1) then <<
    orArgsAnsList_:= union(for i:=1 : length(tDNF_) collect
      checkInfUnitDNF({part(tDNF_, 1)}));
    debugWrite("orArgsAnsList_: ", orArgsAnsList_);
    infCheckAns_:= if(orArgsAnsList_={false}) then false else true;
  >> else if(isEqualConj(conj_)) then <<
    % Equalを表す論理積の場合
    infCheckAns_:= false;
  >> else <<
    lbTupleList_:= getLbTupleListFromConj(conj_);
    % 前提：下限は1つ
    % TODO：パラメタによる下限もある場合への対応
    lbTuple_:= first(lbTupleList_);
    if((getRelopFromTuple(lbTuple_)=greaterp) and (getValueFromTuple(lbTuple_)=0)) then 
      infCheckAns_:= true
    else infCheckAns_:= false;
  >>;
  debugWrite("ans in checkInfUnitDNF: ", infCheckAns_);

  return infCheckAns_;
end;

% 出力：時刻を表すDNFと条件の組（TC）のリスト
% TODO：ERROR処理
procedure checkInfMinTimeDNF(tDNF_, condDNF_)$
begin;
  scalar minTCList_, conj_, argsAnsTCListList_, minValue_, compareTCListList_, lbTuplelist_, ubTupleList_,
         lbParamTupleList_, ubParamTupleList_, lbValueTupleList_, ubValueTupleList_, lbValue_, ubValue_, 
         paramLeqValueCondDNF_, paramGreaterValueCondDNF_, checkDNF_, diffCondDNF_;
  debugWrite("=== in checkInfMinTimeDNF", " ");
  debugWrite("{tDNF_, condDNF_}", {tDNF_, condDNF_});

  conj_:= first(tDNF_);
  if(length(tDNF_)>1) then <<
    argsAnsTCListList_:= union(for i:=1 : length(tDNF_) collect
      checkInfMinTimeDNF({part(tDNF_, i)}, condDNF_));
    debugWrite("argsAnsTCListList_: ", argsAnsTCListList_);
    minTCList_:= foldLeft(compareMinTimeList, first(argsAnsTCListList_), rest(argsAnsTCListList_));
  >> else if(isEqualConj(conj_)) then <<
    % Equalを表す論理積の場合
    if(not hasParameter(conj_)) then minTCList_:= {{getValueFromTuple(first(conj_)), condDNF_}}
    else <<
      % パラメタの場合、その値が0以下ならば結果はINFINITYになる
      minValue_:= getValueFromTuple(first(conj_));
      checkDNF_:= addCondTupleToCondDNF({minValue_, greaterp, 0}, condDNF_);
      debugWrite("checkDNF_: ", checkDNF_);
      if(not isFalseDNF(checkDNF_)) then << 
        if(isSameDNF(checkDNF_, condDNF_)) then <<
          minTCList_:= {{minValue_, checkDNF_}};
        >> else <<
          diffCondDNF_:= addCondDNFToCondDNF(getNotDNF(checkDNF_), condDNF_);
          debugWrite("diffCondDNF_: ", diffCondDNF_);
          minTCList_:= {{minValue_, checkDNF_}, {INFINITY, diffCondDNF_}};
        >>;
      >> else <<
        minTCList_:= {{INFINITY, condDNF_}};
      >>;
    >>;
  >> else <<
    if(not hasParameter(conj_)) then minTCList_:= {{getValueFromTuple(first(getLbTupleListFromConj(conj_))), condDNF_}}
    else << 
      % パラメタの入った下限もある場合
      % 前提：condDNF_内の定数の種類は1つまで？
      % TODO：なんとかする
      lbTupleList_:= getLbTupleListFromConj(conj_);
      ubTupleList_:= conj_ \ lbTupleList_;
      lbParamTupleList_:= for each x in lbTuplelist_ join if(hasParameter(x)) then {x} else {};
      ubParamTupleList_:= for each x in ubTuplelist_ join if(hasParameter(x)) then {x} else {};
      lbValueTupleList_:= lbTuplelist_ \ lbParamTupleList_;
      ubValueTupleList_:= ubTuplelist_ \ ubParamTupleList_;
      if(lbValueTupleList_={}) then lbValueTupleList_:= {{t, geq, -INFINITY}};
      if(ubValueTupleList_={}) then ubValueTupleList_:= {{t, leq,  INFINITY}};
      lbValue_:= getValueFromTuple(first(lbValueTupleList_));
      ubValue_:= getValueFromTuple(first(ubValueTupleList_));


      minTCList_:= {};
      % パラメタを含む下限がlbValue_より大きいかどうかを調べる
      % TODO：パラメタを含む下限同士の大小判定
      debugWrite("lbParamTupleList_: ", lbParamTupleList_);
      debugWrite("lbValue_: ", lbValue_);
      for each x in lbParamTupleList_ do <<
        debugWrite("x (lbParamTuple): ", x);
        checkDNF_:= addCondTupleToCondDNF({getValueFromTuple(x), greaterp, lbValue_}, condDNF_);
        debugWrite("checkDNF_: ", checkDNF_);
        if(not isFalseDNF(checkDNF_)) then <<
          if(isSameDNF(checkDNF_, condDNF_)) then <<
            minTCList_:= cons({getValueFromTuple(x), checkDNF_}, minTCList_);
          >> else <<
            diffCondDNF_:= addCondDNFToCondDNF(getNotDNF(checkDNF_), condDNF_);
            debugWrite("diffCondDNF_: ", diffCondDNF_);
            if(checkOrderingFormula(lbValue_ >0)) then <<
              minTCList_:= cons({getValueFromTuple(x), checkDNF_}, cons({lbValue_, diffCondDNF_}, minTCList_));
            >> else <<
              minTCList_:= cons({getValueFromTuple(x), checkDNF_}, cons({INFINITY, diffCondDNF_}, minTCList_));
            >>;
          >>;
          debugWrite("minTCList_: ", minTCList_);
        >>;
      >>;
      debugWrite("minTCList_ after add: ", minTCList_);


      debugWrite("========== check param-ub ==========", " ");
      % パラメタを含む上限は下限以上でなくてはならない
      debugWrite("ubParamTupleList_: ", ubParamTupleList_);
      if(minTCList_ neq {}) then <<
        for each x in ubParamTupleList_ do <<
          minTCList_:= union(for each y in minTCList_ join <<
            if(getTimeFromTC(y) neq INFINITY) then <<
              checkDNF_:= addCondTupleToCondDNF({getValueFromTuple(x), geq, getTimeFromTC(y)}, getCondDNFFromTC(y));
              debugWrite("checkDNF_: ", checkDNF_);
              if(not isFalseDNF(checkDNF_)) then {y} else {}
            >> else <<
              % 下限がINFINITYのとき（ある特定の範囲のパラメタによって離散変化が起きないパターン）は上下限確認不要
              % TODO：本当？
              {y}
            >>
          >>);
        >>;
        if(minTCList_={}) then minTCList_:= {{INFINITY, condDNF_}};
      >> else <<
        % パラメタの下限がなかった（lbParamTupleList_が空集合）ときはlbValue_とだけ比較
        for each x in ubParamTupleList_ do <<
          checkDNF_:= addCondTupleToCondDNF({getValueFromTuple(x), geq, lbValue_}, condDNF_);
          if(not isFalseDNF(checkDNF_) and checkOrderingFormula(lbValue_ >0)) then minTCList_:= {{lbValue_, condDNF_}}
          else minTCList_:= {{INFINITY, condDNF_}};
        >>;
      >>;
    >>;
  >>;

  debugWrite("{(tDNF_), (condDNF_)}: ", {tDNF_, condDNF_});
  debugWrite("ans in checkInfMinTimeDNF: ", minTCList_);
  return minTCList_;
end;

IC_SOLVER_ERROR___:= 0;
IC_NORMAL_END___:= 1;

% @return {newRetList_, integRule_}
procedure createIntegratedValue(pairInfo_, variable_)$
begin;
  scalar retList_, integRule_, integExpr_, newRetList_;

  retList_:= first(pairInfo_);
  integRule_:= second(pairInfo_);

  integExpr_:= {variable_, sub(integRule_, variable_)};

  newRetList_:= cons(integExpr_, retList_);

  return {newRetList_, integRule_};
end;

% 次のポイントフェーズに移行する時刻を求める
procedure calculateNextPointPhaseTime(maxTime_, discCause_);
begin;
  scalar ans_;

  ans_:= calculateNextPointPhaseTimeMain(maxTime_, discCause_, constraint__, initConstraint__, pConstraint__, variables__);
  debugWrite("ans_ in calculateNextPointPhaseTime:", ans_);
  return ans_;
end;

% 次のポイントフェーズに移行する時刻を求める
% 戻り値の形式: {time_t, {}(parameter_map_t), true(bool)},...}
procedure calculateNextPointPhaseTimeMain(maxTime_, discCause_, cons_, initCons_, pCons_, vars_);
begin;
  scalar splitExprsResult_, NDExprs_, NDExprVars_, DExprs_, DExprVars_, otherExprs_,
         discCauseList_, condDNF_, minTCondListList_, comparedMinTCondList_, 
         tmpMinTList_, tmpIneqSolDNF_, minTTime_, minTCondDNF_, maxTimeFlag_;

  debugWrite("=== in calculateNextPointPhaseTimeMain", " ");

  % TODO initCons_を使ってない？
  debugWrite("{maxTime_ ,discCause_, cons_, initCons_, pCons_, vars_}: ", {maxTime_ ,discCause_, cons_, initCons_, pCons_, vars_});
  debugWrite("{variables__, parameters__}: ", {variables__, parameters__});

  splitExprsResult_ := splitExprs(removePrevCons(cons_), vars_);
  NDExprs_ := part(splitExprsResult_, 1);
  NDExprVars_ := part(splitExprsResult_, 2);
  DExprs_ := part(splitExprsResult_, 3);
  DExprVars_ := part(splitExprsResult_, 4);
  otherExprs_:= union(part(splitExprsResult_, 5), pCons_);
  % DNF形式にする
  % 空集合なら、{{true}}として扱う（trueを表すDNF）
  if(otherExprs_={}) then condDNF_:= {{true}}
  else <<
    % condDNF_:= foldLeft((addCondDNFToCondDNF(#1, exIneqSolve(#2)))&, {{true}}, otherExprs_);を実現
    tmpIneqSolDNF_:= {{true}};
    for i:=1 : length(otherExprs_) do <<
      tmpIneqSolDNF_:= addCondDNFToCondDNF(tmpIneqSolDNF_, exIneqSolve(part(otherExprs_, i)));
    >>;
    condDNF_:= tmpIneqSolDNF_;
  >>;

  % TODO DExprs_, NDExprs_の処理
  discCauseList_:= union(sub(cons_, discCause_));

  % in calcNextPointPhaseTime
  debugWrite("discCauseList_:", discCauseList_);
  debugWrite("condDNF_: ", condDNF_);

  minTCondListList_:= union(for each x in discCauseList_ collect findMinTime(x, condDNF_));
  debugWrite("minTCondListList_ in calcNextPointPhaseTime: ", minTCondListList_);

  if(not freeof(minTCondListList_, error)) then return {error};

  comparedMinTCondList_:= foldLeft(compareMinTimeList, {{maxTime_, condDNF_}}, minTCondListList_);
  debugWrite("comparedMinTCondList_ in calcNextPointPhaseTime: ", comparedMinTCondList_);

  tmpMinTList_:= union(
    for each x in comparedMinTCondList_ collect <<
      minTTime_:= getTimeFromTC(x);
      minTCondDNF_:= getCondDNFFromTC(x);
      % Condがtrueの場合は空集合として扱う
      if(isTrueDNF(minTCondDNF_)) then minTCondDNF_:= {{}};
      % 演算子部分をコードに置き換える
      % TODO DNFが"x=1 and x=2"と "x+1 or x=2"を正しく区別できているか?
      minTCondDNF_:= for each conj in minTCondDNF_ collect
        for each term in conj collect
          {getVarNameFromTuple(term), getExprCode(getRelopFromTuple(term)), getValueFromTuple(term)};
      maxTimeFlag_:= if(minTTime_ neq maxTime_) then 0 else 1;
      {minTTime_, minTCondDNF_, maxTimeFlag_}
    >>
  );
  debugWrite("=== ans in calculateNextPointPhaseTimeMain: ", tmpMinTList_);

  return tmpMinTList_;
end;

% @param formula_ or が含まれないQE論理式
% @return andに並列につながったDNF形式のQE論理式
procedure FixAndFormula(formula_)$
begin;
  scalar ans_, ansList_;
  if(head(formula_)=and) then <<
    ans_:= true;
    for each x in getArgsList(formula_) do <<
      ans_:= mymkand({ans_, FixAndFormula(x)});
    >>;
  >> else <<
    ans_:= formula_;
  >>;

  if(head(ans_) = and) then <<
    ansList_:= for each x in getArgsList(ans_) join
      if(x <> true) then {x} else {};
    ans_:= myApply(and, ansList_);
  >>;

  debugWrite("ans in FixAndFormula: ", ans_);
  return ans_;
end;

% @param integAsk_: formula(rlqeの食える形)
% @param condDNF_: バックエンドDNF
procedure findMinTime(integAsk_, condDNF_)$
begin;
  scalar integAskQE_, integAskList_, integAskIneqSolDNFList_, integAskIneqSolDNF_,
         minTCList_, tmpSol_, ineqSolDNF_;

  debugWrite("=== in findMinTime", " ");
  debugWrite("integAsk_: ", integAsk_);
  debugWrite("condDNF_: ", condDNF_);

  % t>0と連立してfalseになるような場合はMinTimeを考える必要がない
  if(rlqe(integAsk_ and t>0) = false) then return {{INFINITY, condDNF_}};

  % 入れ子になった and が来た場合に備えDNF形式に整形する
  integAsk_ := FixAndFormula(integAsk_);

  % 前提：ParseTree構築時に分割されているはずなのでガードにorが入ることは考えない
  % TODO：¬gの形だと入ることがあるのでは？？
  integAskList_:= if(head(integAsk_)=and) then <<
    union(for each x in getArgsList(integAsk_) join <<
      integAskQE_:= rlqe(x);
      if(integAskQE_=true) then {error}
      else if(integAskQE_=false) then {false}
      else {x}
    >>)
  >> else <<
    integAskQE_:= rlqe(integAsk_);
    if(integAskQE_=true) then {error}
    else if(integAskQE_=false) then {false}
    else {integAsk_}
  >>;
  debugWrite("integAskList_: ", integAskList_);

  % ガード条件全体がtrueの場合とガード条件内にfalseが入る場合はエラー
  if(integAskList_={error} or not freeof(integAskList_, false)) then return {error};

%  integAskQE_:= rlqe(integAsk_);
%  debugWrite("integAskQE_: ", integAskQE_);
%
%  %%%%%%%%%%%% TODO:この辺から、%%%%%%%%%%%%%%
%  % まず、andでつながったtmp制約をリストに変換
%  if(head(integAskQE_)=and) then integAskList_:= getArgsList(integAskQE_)
%  else integAskList_:= {integAskQE_};
%  debugWrite("integAskList_:", integAskList_);


  integAskIneqSolDNFList_:= union(for each x in integAskList_ collect
                              if(not isIneqRelop(head(x))) then <<
                                tmpSol_:= exSolve(x, t);
                                if(length(tmpSol_)>1) then for each y in tmpSol_ join makeTupleDNFFromEq(lhs(y), rhs(y))
                                else makeTupleDNFFromEq(lhs(first(tmpSol_)), rhs(first(tmpSol_)))
                              >> else <<
                                ineqSolDNF_:= exIneqSolve(x);
                                debugWrite("ineqSolDNF_: ", ineqSolDNF_);
                                if(isFalseDNF(ineqSolDNF_)) then {{}}
                                else if(isTrueDNF(ineqSolDNF_)) then {{error}} % ←
                                else ineqSolDNF_
                              >>
                            );
  debugWrite("integAskIneqSolDNFList_:", integAskIneqSolDNFList_);
  % unknownが含まれたらエラーを返す
  % TODO：要検討
  if(not freeof(integAskIneqSolDNFList_, unknown)) then return {error}
  % trueになったらエラー
  else if(not freeof(integAskIneqSolDNFList_, error)) then return {error};


  debugWrite("findMinTime: add t>0", " ");
  integAskIneqSolDNF_:= foldLeft(addCondDNFToCondDNF, {{{t, greaterp, 0}}}, integAskIneqSolDNFList_);
  debugWrite("findMinTime: end add t>0", " ");
  debugWrite("integAskIneqSolDNF_:", integAskIneqSolDNF_);

  % 不等式の場合、ここで初めて矛盾が見つかり、integAskIneqSolDNF_がfalseになることがある
  if(isFalseDNF(integAskIneqSolDNF_)) then return {{INFINITY, condDNF_}};

  %%%%%%%%%%%% TODO:この辺までを1つの処理にまとめたい%%%%%%%%%%%%


  minTCList_:= checkInfMinTimeDNF(integAskIneqSolDNF_, condDNF_);
  debugWrite("minTCList_ in findMinTime: ", minTCList_);
  debugWrite("=================== end of findMinTime ====================", " ");

  % ERRORが返っていたらerror
  if(not freeof(minTCList_, ERROR)) then return {error};
  return minTCList_;
end;

procedure compareMinTimeList(candidateTCList_, newTCList_)$
begin;
  scalar tmpRet_, arg2_, arg3_, ret_;

  debugWrite("=== in compareMinTimeList", " ");
  debugWrite("{candidateTCList_, newTCList_}: ", {candidateTCList_, newTCList_});

  %ret_:= foldLeft((makeMapAndUnion(candidateTCList_, #1, #2))&, {}, newTCList_);を実現
  tmpRet_:= {};
  for i:=1 : length(newTCList_) do <<
    debugWrite("loop count: ", i);
    debugWrite("tmpRet_: ", tmpRet_);
    arg2_:= tmpRet_;
    arg3_:= part(newTCList_, i);
    tmpRet_:= makeMapAndUnion(candidateTCList_, arg2_, arg3_);
    debugWrite("{i, arg2_, arg3_}: ", {i, arg2_, arg3_});
    debugWrite("tmpRet_ after makeMapAndUnion: ", tmpRet_);
  >>;
  ret_:= tmpRet_;

  debugWrite("ans in compareMinTimeList: ", ret_);
  return ret_;
end;

procedure makeMapAndUnion(candidateTCList_, retTCList_, newTC_)$
begin;
  scalar comparedList_, ret_;

  debugWrite("in makeMapAndUnion", " ");
  debugWrite("{candidateTCList_, retTCList_, newTC_}: ", {candidateTCList_, retTCList_, newTC_});

  % Mapではなく、Joinを使う方が正しそうなのでそうしている
  comparedList_:= for each x in candidateTCList_ join compareParamTime(newTC_, x, MIN);
  debugWrite("comparedList_ in makeMapAndUnion: ", comparedList_);
  ret_:= union(comparedList_, retTCList_);

  debugWrite("ans in makeMapAndUnion: ", ret_);
  return ret_;
end;

procedure compareParamTime(TC1_, TC2_, mode_)$
begin;
  scalar TC1Time_, TC1Cond_, TC2Time_, TC2Cond_,
         intersectionCondDNF_, TC1LessTC2CondDNF_, TC1GeqTC2CondDNF_,
         retTCList_;

  debugWrite("in compareParamTime", " ");
  debugWrite("{TC1_, TC2_, mode_}: ", {TC1_, TC2_, mode_});

  TC1Time_:= getTimeFromTC(TC1_);
  TC1Cond_:= getCondDNFFromTC(TC1_);
  TC2Time_:= getTimeFromTC(TC2_);
  TC2Cond_:= getCondDNFFromTC(TC2_);


  % それぞれの条件部分について論理積を取り、falseなら空集合
  intersectionCondDNF_:= addCondDNFToCondDNF(TC1Cond_, TC2Cond_);
  debugWrite("intersectionCondDNF_: ", intersectionCondDNF_);
  if(isFalseDNF(intersectionCondDNF_)) then return {};

  if(TC1Time_ = infinity) then return {{TC2Time_, intersectionCondDNF_}};
  if(TC2Time_ = infinity) then return {{TC1Time_, intersectionCondDNF_}};

  % 条件の共通部分と時間に関する条件との論理積を取る
  % TC1Time_＜TC2Time_という条件
  debugWrite("========== make TC1LessTC2CondDNF_ ==========", " ");
  TC1LessTC2CondDNF_:= addCondTupleToCondDNF({TC1Time_, lessp, TC2Time_}, intersectionCondDNF_);
  debugWrite("TC1LessTC2CondDNF_: ", TC1LessTC2CondDNF_);
  % TC1Time_≧TC2Time_という条件
  debugWrite("========== make TC1GeqTC2CondDNF_ ==========", " ");
  TC1GeqTC2CondDNF_:= addCondTupleToCondDNF({TC1Time_, geq, TC2Time_}, intersectionCondDNF_);
  debugWrite("TC1GeqTC2CondDNF_: ", TC1GeqTC2CondDNF_);


  retTCList_:= {};
  % それぞれ、falseでなければretTCList_に追加
  if(not isFalseDNF(TC1LessTC2CondDNF_)) then 
    if(mode_=MIN) then retTCList_:= cons({TC1Time_, TC1LessTC2CondDNF_}, retTCList_)
    else if(mode_=MAX) then retTCList_:= cons({TC2Time_, TC1LessTC2CondDNF_}, retTCList_);
  if(not isFalseDNF(TC1GeqTC2CondDNF_)) then 
    if(mode_=MIN) then retTCList_:= cons({TC2Time_, TC1GeqTC2CondDNF_}, retTCList_)
    else if(mode_=MAX) then retTCList_:= cons({TC1Time_, TC1GeqTC2CondDNF_}, retTCList_);

  debugWrite("ans in compareParamTime: ", retTCList_);
  return retTCList_;
end;

defaultPrec_ := 0;

%TODO エラー検出（適用した結果実数以外になった場合等）
procedure applyTime2Expr(expr_, time_)$
begin;
  scalar appliedExpr_;

  appliedExpr_:= sub(t=time_, expr_);
  debugWrite("appliedExpr_:", appliedExpr_);

  return {1, appliedExpr_};
end;

procedure createVariableMapInterval()$
begin;
  return createVariableMapIntervalMain(constraint__, initConstraint__, variables__, parameters__);
end;

% 前提：Orでつながってはいない
% TODO：exDSolveを含めた実行をする
procedure createVariableMapIntervalMain(cons_, initCons_, vars_, pars_)$
begin;
  scalar solvedRet_, consTmpRet_, consRet_, paramDNFList_, paramRet_, tuple_, ret_;
  scalar tmpCons_, rconts_;
  scalar tmpSol_, splitExprsResult_, DExprs_, DExprVars_, 
         initVars_, prevVars_, noPrevVars_, noDifferentialVars_, tmpVarMap_,
         DExprRconts_, DExprRcontsVars_;


  debugWrite("=== in createVariableMapIntervalMain", " ");
  debugWrite("{cons_, initCons_, vars_, pars_}: ", {cons_, initCons_, vars_, pars_});


  tmpCons_ := for each x in initCons_ join if(isInitVariable(lhs x)) then {} else {x};
  if(tmpCons_ neq {}) then return {SOLVER_ERROR___};
  rconts_ := for each x in initCons_ join if(isInitVariable(lhs x)) then {x} else {};
  
  splitExprsResult_ := splitExprs(removePrevCons(cons_), vars_);
  DExprs_ := part(splitExprsResult_, 3);
  DExprVars_ := part(splitExprsResult_, 4);

  DExprRconts_:= removeInitCons(rconts_);
  if(DExprRconts_ neq {}) then <<
    prevVars_:= for each x in vars_ join if(isPrevVariable(x)) then {x} else {};
    noPrevVars_:= union(for each x in prevVars_ collect part(x, 1));
    DExprRcontsVars_ := union(for each x in noPrevVars_ join if(not freeof(DExprRconts_, x)) then {x} else {});
    DExprs_:= union(DExprs_, DExprRconts_);
    DExprVars_:= union(DExprVars_, DExprRcontsVars_);
  >>;

  initCons_:= union(for each x in (rconts_ \ DExprRconts_) collect exSub(cons_, x));
  initVars_:= map(getInitVars, initCons_);
  noDifferentialVars_:= union(for each x in DExprVars_ collect if(isDifferentialVar(x)) then part(x, 1) else x);

  tmpSol_:= exDSolve(DExprs_, initCons_, noDifferentialVars_);
  debugWrite("tmpSol_ solved with exDSolve: ", tmpSol_);
 
  if(tmpSol_ = retsolvererror___) then return {SOLVER_ERROR___}
  else if(tmpSol_ = retoverconstraint___) then return {ICI_INCONSISTENT___};

  tmpVarMap_:= first(foldLeft(createIntegratedValue, {{},tmpSol_}, union(DExprVars_, (vars_ \ initVars_))));
  tmpSol_:= for each x in tmpVarMap_ collect (first(x)=second(x));
  consTmpRet_:= applyPrevCons(union(cons_, tmpSol_), {});    
  debugWrite("consTmpRet_: ", consTmpRet_);

  % 制約の右辺に変数が残っていた場合の適用処理
  solvedRet_:= solveCS(consTmpRet_, vars_);

  % 式を{(変数名), (関係演算子コード), (値のフル文字列)}の形式に変換する
  consRet_:= map(makeConsTuple, solvedRet_);
  paramDNFList_:= map(exIneqSolve, pConstraint__);
  paramRet_:= for each x in paramDNFList_ collect <<
    % 1つの項になっているはず
    tuple_:= first(first(x));
    {getVarNameFromTuple(tuple_), getExprCode(getRelopFromTuple(tuple_)), getValueFromTuple(tuple_)}
  >>;
  ret_:= union(consRet_, paramRet_);

  ret_:= getUsrVars(ret_, removePrevCons(union(DExprVars_, (vars_ \ initVars_))));

  debugWrite("=== ans in createVariableMapIntervalMain: ", ret_);
  return ret_;
end;

% vcs_math_sourceにおけるgetTimeVars
% createVariableMapIntervalのretから記号定数を取り除く
procedure getUsrVars(ret_, vars_);
begin;
  % TODO
  return for each x in ret_ join if(contains(vars_, first x)) then {x} else {}; 
end;

% {df(usrvary,t,2),df(usrvary,t),usrvary}

procedure contains(vars_, var_);
begin;
  return if((for each x in vars_ sum if(x = var_) then 1 else 0) > 0) then t else nil;
end;

%---------------------------------------------------------------
% シミュレーションに直接は関係ないが処理系の都合で必要な関数
%---------------------------------------------------------------

% デバッグ用メッセージ出力関数
% TODO:任意長の引数に対応したい
procedure debugWrite(arg1_, arg2_)$
  if(optUseDebugPrint__) then write(arg1_, arg2_);

% debugWriteのreturn文の時に渡す版
% @return ans_
procedure ansWrite(funcName_, ans_)$
  if(optUseDebugPrint__) then <<
    write("ans in ", funcName_, ": ", ans_);
    ans_
  >> else ans_;

% 関数呼び出しはredevalを経由させる
% <redeval> end:の次が最終行
symbolic procedure redeval(foo_)$
begin scalar ans_;

  debugWrite("<redeval> reval :", (car foo_));
  ans_:= (reval foo_);
  putLineFeed();
  write("<redeval> end:");

  return ans_;
end;

% 出力をシークする
% "<redeval> end:"を頭出しするために使用する
procedure putLineFeed()$
begin;
  write("");
end;


% グローバル変数の初期化とputLineFeed()
symbolic redeval '(resetConstraint);

% ファイル入力の終端定義
;end;