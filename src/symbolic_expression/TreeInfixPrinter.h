#pragma once

//ツリーを中置記法で出力するクラス

#include "Node.h"
#include "DefaultTreeVisitor.h"

namespace hydla {
namespace symbolic_expression {


class TreeInfixPrinter:
  public DefaultTreeVisitor
{
  public:
  typedef enum{
    PAR_NONE,
    PAR_P_S,        /// parentheses are needed for plus and subtract
    PAR_N,          /// parentheses are needed for negative
    PAR_N_P_S,      /// needed for negative, plus and subtract
    PAR_N_P_S_T_D_P, /// needed for negative, plus, subtract, times, divide, power
  }needParenthesis;

  /**
   * ノードを引数に取り，中値記法形式で出力する
   */
  std::ostream& print_infix(const node_sptr &, std::ostream&);
  /**
   * ノードを引数に取り，中値記法形式にして返す
   */
  std::string get_infix_string(const node_sptr &);


  /**
   * 数式の出力に略記法を用いるかどうかを設定する。
   * 現状だとパラメータのみ。p[x, 0, 1]をx01のようにする。
   */
  static void set_use_shorthand(bool);

  private:

  static bool use_shorthand;
  
  needParenthesis need_par_;
  std::ostream *output_stream_;
  
  void print_binary_node(const BinaryNode &, const std::string &symbol,
                          const needParenthesis &pre_par = PAR_NONE, const needParenthesis &post_par = PAR_NONE);
  void print_unary_node(const UnaryNode &, const std::string &pre, const std::string &post);

  void print_factor_node(const FactorNode &, const std::string &pre, const std::string &post);

  // 制約定義
  virtual void visit(boost::shared_ptr<ConstraintDefinition> node);
  
  // プログラム定義
  virtual void visit(boost::shared_ptr<ProgramDefinition> node);

  // 制約呼び出し
  virtual void visit(boost::shared_ptr<ConstraintCaller> node);
  
  // プログラム呼び出し
  virtual void visit(boost::shared_ptr<ProgramCaller> node);

  // 制約式
  virtual void visit(boost::shared_ptr<Constraint> node);

  // Ask制約
  virtual void visit(boost::shared_ptr<Ask> node);

  // Tell制約
  virtual void visit(boost::shared_ptr<Tell> node);


  // 比較演算子
  virtual void visit(boost::shared_ptr<Equal> node);
  virtual void visit(boost::shared_ptr<UnEqual> node);
  virtual void visit(boost::shared_ptr<Less> node);
  virtual void visit(boost::shared_ptr<LessEqual> node);
  virtual void visit(boost::shared_ptr<Greater> node);
  virtual void visit(boost::shared_ptr<GreaterEqual> node);

  
  // 制約階層定義演算子
  virtual void visit(boost::shared_ptr<Weaker> node);
  virtual void visit(boost::shared_ptr<Parallel> node);

  // 時相演算子
  virtual void visit(boost::shared_ptr<Always> node);


  // 論理演算子
  virtual void visit(boost::shared_ptr<LogicalAnd> node);
  virtual void visit(boost::shared_ptr<LogicalOr> node);

  // 算術二項演算子
  virtual void visit(boost::shared_ptr<Plus> node);
  virtual void visit(boost::shared_ptr<Subtract> node);
  virtual void visit(boost::shared_ptr<Times> node);
  virtual void visit(boost::shared_ptr<Divide> node);
  virtual void visit(boost::shared_ptr<Power> node);

  // 算術単項演算子
  virtual void visit(boost::shared_ptr<Negative> node);
  virtual void visit(boost::shared_ptr<Positive> node);

  // 微分
  virtual void visit(boost::shared_ptr<Differential> node);

  // 左極限
  virtual void visit(boost::shared_ptr<Previous> node);
  
  //Print 
  virtual void visit(boost::shared_ptr<Print> node);
  virtual void visit(boost::shared_ptr<PrintPP> node);
  virtual void visit(boost::shared_ptr<PrintIP> node);
  virtual void visit(boost::shared_ptr<Scan> node);
  virtual void visit(boost::shared_ptr<Exit> node);
  virtual void visit(boost::shared_ptr<Abort> node);

  //SystemVariable
  virtual void visit(boost::shared_ptr<SVtimer> node);

  // 否定
  virtual void visit(boost::shared_ptr<Not> node);
  
  // 変数
  virtual void visit(boost::shared_ptr<Variable> node);

  // 数字
  virtual void visit(boost::shared_ptr<Number> node);
  virtual void visit(boost::shared_ptr<Float> node);

  // 記号定数
  virtual void visit(boost::shared_ptr<Parameter> node);

  // t
  virtual void visit(boost::shared_ptr<SymbolicT> node);
  virtual void
  visit(boost::shared_ptr<ImaginaryUnit> node);

  // 無限大
  virtual void visit(boost::shared_ptr<Infinity> node);
  
  // 自然対数の底
  virtual void visit(boost::shared_ptr<E> node);
  
  // 円周率
  virtual void visit(boost::shared_ptr<Pi> node);
  
  
  // 任意
  virtual void visit(boost::shared_ptr<Function> node);
  virtual void visit(boost::shared_ptr<UnsupportedFunction> node);

  // True
  virtual void visit(boost::shared_ptr<True> node);

  // False
  virtual void visit(boost::shared_ptr<False> node);

  // ExpresssionListElement
  virtual void visit(boost::shared_ptr<ExpressionListElement> node);
  // List 
  /*
  virtual void visit(boost::shared_ptr<ExpressionList> node);
  virtual void visit(boost::shared_ptr<ExpressionListCaller> node);
  virtual void visit(boost::shared_ptr<ProgramList> node);
  virtual void visit(boost::shared_ptr<ProgramListCaller> node);
  virtual void visit(boost::shared_ptr<ProgramListElement> node);
  virtual void visit(boost::shared_ptr<Range> node);
  virtual void visit(boost::shared_ptr<Union> node);
  virtual void visit(boost::shared_ptr<Intersection> node);
  */
};

} // namespace symbolic_expression
} // namespace hydla 

