#include "MathematicaExpressionConverter.h"
#include "PacketSender.h"

#include <stdlib.h>
#include <cassert>
#include <iostream>
#include <sstream>

namespace hydla {
namespace vcs {
namespace mathematica {

MathematicaExpressionConverter::string_map_t MathematicaExpressionConverter::string_map_;

using namespace hydla::parse_tree;

//名前としては，SymbolicValue→symbolic，Mathematicaの文字列→matheくらいで統一すべきかも．どっちもExpressionなわけだし．


void MathematicaExpressionConverter::initialize(){
  //ノードと文字列の対応関係を作っておく．
  string_map_.insert(std::make_pair("Plus", function_and_node(for_binary_node, NODE_PLUS)));
  string_map_.insert(std::make_pair("Subtract", function_and_node(for_binary_node, NODE_SUBTRACT)));
  string_map_.insert(std::make_pair("Times", function_and_node(for_binary_node, NODE_TIMES)));
  string_map_.insert(std::make_pair("Divide", function_and_node(for_binary_node, NODE_DIVIDE)));
  string_map_.insert(std::make_pair("Power", function_and_node(for_binary_node, NODE_POWER)));
  string_map_.insert(std::make_pair("Rational", function_and_node(for_binary_node, NODE_DIVIDE)));//ちょっと意味は異なるみたいだけど，結果は同じになるはずなのでこれで
  string_map_.insert(std::make_pair("Derivative", function_and_node(for_derivative, NODE_DIFFERENTIAL)));
  string_map_.insert(std::make_pair("prev", function_and_node(for_unary_node, NODE_PREVIOUS)));
  string_map_.insert(std::make_pair("Sqrt", function_and_node(for_unary_node, NODE_SQRT)));
}

MathematicaExpressionConverter::value_t MathematicaExpressionConverter::convert_math_string_to_symbolic_value(const std::string &expr){

  HYDLA_LOGGER_REST("#*** convert string to value ***\n",
                     expr);

  value_t value;
  std::string::size_type now = 0;
  value.set(convert_math_string_to_symbolic_tree(expr, now));
  HYDLA_LOGGER_REST("#*** convert result value ***\n",
                     value.get_string());
  return value;
}

MathematicaExpressionConverter::node_sptr MathematicaExpressionConverter::convert_math_string_to_symbolic_tree(const std::string &expr, std::string::size_type &now){
  now = expr.find_first_not_of(" ", now);
  std::string::size_type prev = now;
  if((expr[now]>='0'&&expr[now]<='9')){//正の数値の場合
    now = expr.find_first_of("],", now);
    return node_sptr(new hydla::parse_tree::Number(expr.substr(prev,now-prev)));
  }
  if(expr[now]=='-'){ //負の数値の場合
    now = expr.find_first_of("],", now);
    return node_sptr(new hydla::parse_tree::Negative(node_sptr(new hydla::parse_tree::Number(expr.substr(prev+1,now-prev-1)))));
  }

  now = expr.find_first_of("[],", prev);
  string_map_t::iterator it = string_map_.find(expr.substr(prev,now-prev));
  if(it == string_map_.end()){
    if(expr.substr(prev,now-prev)=="t"){//時刻
      return node_sptr(new hydla::parse_tree::SymbolicT());
    }
    if(expr[prev]=='p'){//定数名
      return node_sptr(new hydla::parse_tree::Parameter(expr.substr(prev+1,now-prev-1)));
    }
    if(expr.substr(prev,now-prev)==PacketSender::var_prefix){//変数名
      return node_sptr(new hydla::parse_tree::Variable(expr.substr(prev,now-prev)));
    }
    
    //謎ノード．Numberノードに入れておけば，ソルバがMathematicaである限り処理を継続することはできるはずだが・・・将来的には全対応したい
    int depth = 0;
    while(1){
      if(expr[now]=='['){
        depth++;
      }else{
        depth--;
        if(depth<=0){
          if(now == std::string::npos){
            now = expr.length() - 1;
          }else if(depth<0){
            now--;
          }
          break;
        }
      }
      now = expr.find_first_of("[]", now+1);
      assert(now != std::string::npos);
    }
    return node_sptr(new hydla::parse_tree::Number(expr.substr(prev, (now++)-prev+1)));
  }
  
  //何か子ノードがあるはず．Derivativeの場合は少し特別か  
  now++;//[の分前進
  return (*(it->second.function))(expr, now, it->second.node);
}

//これ以降のfor〜については，列挙型使わず，もっと楽にできそうな気もする
MathematicaExpressionConverter::node_sptr 
  MathematicaExpressionConverter::for_derivative(
    const std::string &expr,
    std::string::size_type &now,
    const MathematicaExpressionConverter::nodeType &nt){
  //まず微分回数
  std::string::size_type prev = now;
  int derivative_count;
  now = expr.find("]", 0);
  derivative_count = atoi(expr.substr(prev,now-prev).c_str());
  now+=2;//][
  //次に中身
  node_sptr tmp_node = convert_math_string_to_symbolic_tree(expr, now);
  for(int i=0;i<derivative_count-1;i++){
    tmp_node.reset(new hydla::parse_tree::Differential(tmp_node));
  }
  now++;//]
  return node_sptr(new hydla::parse_tree::Differential(tmp_node));
}

MathematicaExpressionConverter::node_sptr
  MathematicaExpressionConverter::for_unary_node(
    const std::string &expr,
    std::string::size_type &now,
    const MathematicaExpressionConverter::nodeType &nt){
  //中身
  node_sptr tmp_node = convert_math_string_to_symbolic_tree(expr, now);
  now++;//]
  switch(nt){
    default:
    assert(0);
    return node_sptr(new hydla::parse_tree::Previous(tmp_node));
    
    case NODE_PREVIOUS:
    return node_sptr(new hydla::parse_tree::Previous(tmp_node));
    
    case NODE_SQRT:
    return node_sptr(new hydla::parse_tree::Power
      (tmp_node, 
       node_sptr(new hydla::parse_tree::Divide(
          node_sptr(new hydla::parse_tree::Number("1")),
          node_sptr(new hydla::parse_tree::Number("2")) ) ) ) );
  }
}

MathematicaExpressionConverter::node_sptr
  MathematicaExpressionConverter::for_binary_node(
    const std::string &expr,
    std::string::size_type &now,
    const MathematicaExpressionConverter::nodeType &nt){
  //BinaryNodeを作るための関数だけど，PlusとTimesはリストで複数引数取れるみたいだから特別にループ

  //左
  node_sptr lhs = convert_math_string_to_symbolic_tree(expr, now);
  now++;//,
  while(1){
   //右
   node_sptr rhs = convert_math_string_to_symbolic_tree(expr, now);
   now++;//]や,
   switch(nt){
     case NODE_PLUS:
     if(expr[now-1]==']'){//ここで終了
       return node_sptr(new hydla::parse_tree::Plus(lhs, rhs));
     }
     else{
       lhs = node_sptr(new hydla::parse_tree::Plus(lhs, rhs));
     }
     break;

     case NODE_TIMES:
     if(expr[now-1]==']')//ここで終了
       return node_sptr(new hydla::parse_tree::Times(lhs, rhs));
     else
       lhs = node_sptr(new hydla::parse_tree::Times(lhs, rhs));
     break;

     case NODE_SUBTRACT:
     return node_sptr(new hydla::parse_tree::Subtract(lhs, rhs));
     case NODE_DIVIDE:
     return node_sptr(new hydla::parse_tree::Divide(lhs, rhs));
     case NODE_POWER:
     return node_sptr(new hydla::parse_tree::Power(lhs, rhs));

     default:
     assert(0);
     return node_sptr(new hydla::parse_tree::Power(lhs, rhs));
   }
  }
}

std::string MathematicaExpressionConverter::get_relation_math_string(value_range_t::Relation rel){
  switch(rel){
    case value_range_t::EQUAL:
      return "Equal";
    case value_range_t::NOT_EQUAL:
      return "UnEqual";
    case value_range_t::GREATER_EQUAL:
      return "GreaterEqual";
    case value_range_t::LESS_EQUAL:
      return "LessEqual";
    case value_range_t::GREATER:
      return "Greater";
    case value_range_t::LESS:
      return "Less";
    default:
      assert(0);
      return "";
  }
}

MathematicaExpressionConverter::value_range_t::Relation MathematicaExpressionConverter::get_relation_from_code(const int &relop_code){
  switch(relop_code){
    case 0: // Equal
      return value_range_t::EQUAL;
      break;

    case 1: // Less
      return value_range_t::LESS;    
      break;

    case 2: // Greater
      return value_range_t::GREATER;
      break;

    case 3: // LessEqual
      return value_range_t::LESS_EQUAL;
      break;

    case 4: // GreaterEqual
      return value_range_t::GREATER_EQUAL;
    default:
      assert(0);
      return value_range_t::GREATER_EQUAL;
  }
}

void MathematicaExpressionConverter::set_parameter_on_value(MathematicaExpressionConverter::value_t &val,const std::string &par_name){
  val.set(node_sptr(new hydla::parse_tree::Parameter(par_name)));
  return;
}


std::string MathematicaExpressionConverter::convert_node_to_math_string(const node_sptr &node){
  string_for_math_string_.clear();
  differential_count_=0;
  in_prev_=0;
  accept(node);
  return string_for_math_string_;
}

std::string MathematicaExpressionConverter::convert_symbolic_value_to_math_string(const value_t &val){
  return convert_node_to_math_string(val.get_node());
}

#define START_P "["

#define DEFINE_VISIT_BINARY(NODE_NAME, FUNC_NAME)                             \
void MathematicaExpressionConverter::visit(boost::shared_ptr<NODE_NAME> node) \
{                                                                         \
  string_for_math_string_.append(#FUNC_NAME START_P);                             \
  accept(node->get_lhs());                                                \
  string_for_math_string_.append(", ");                                   \
  accept(node->get_rhs());                                                \
  string_for_math_string_.append("]");                                    \
}

#define DEFINE_VISIT_UNARY(NODE_NAME, FUNC_NAME)                             \
void MathematicaExpressionConverter::visit(boost::shared_ptr<NODE_NAME> node) \
{                                                                         \
  string_for_math_string_.append(#FUNC_NAME START_P);                             \
  accept(node->get_child());                                              \
  string_for_math_string_.append("]");                                    \
}

#define DEFINE_VISIT_FACTOR(NODE_NAME, FUNC_NAME)                             \
void MathematicaExpressionConverter::visit(boost::shared_ptr<NODE_NAME> node) \
{                                                                         \
  string_for_math_string_.append(#FUNC_NAME);                       \
}


// 比較演算子

DEFINE_VISIT_BINARY(Equal, Equal)
DEFINE_VISIT_BINARY(UnEqual, Unequal)
DEFINE_VISIT_BINARY(Less, Less)
DEFINE_VISIT_BINARY(LessEqual, LessEqual)
DEFINE_VISIT_BINARY(Greater, Greater)
DEFINE_VISIT_BINARY(GreaterEqual, GreaterEqual)



// 論理演算子
DEFINE_VISIT_BINARY(LogicalAnd, And)
DEFINE_VISIT_BINARY(LogicalOr, Or)

  
// 算術二項演算子
DEFINE_VISIT_BINARY(Plus, Plus)
DEFINE_VISIT_BINARY(Subtract, Subtract)
DEFINE_VISIT_BINARY(Times, Times)
DEFINE_VISIT_BINARY(Divide, Divide)
DEFINE_VISIT_BINARY(Power, Power)

  
// 算術単項演算子

DEFINE_VISIT_UNARY(Negative, Minus)
void MathematicaExpressionConverter::visit(boost::shared_ptr<Positive> node)              
{
  accept(node->get_child());
}

// 微分
void MathematicaExpressionConverter::visit(boost::shared_ptr<Differential> node)          
{
  differential_count_++;
  accept(node->get_child());
  differential_count_--;
}

// 左極限
void MathematicaExpressionConverter::visit(boost::shared_ptr<Previous> node)              
{
  in_prev_++;
  accept(node->get_child());
  in_prev_--;
}

// 否定
DEFINE_VISIT_UNARY(Not, Not)
  
// 三角関数
DEFINE_VISIT_UNARY(Sin, Sin)
DEFINE_VISIT_UNARY(Cos, Cos)
DEFINE_VISIT_UNARY(Tan, Tan)
// 逆三角関数
DEFINE_VISIT_UNARY(Asin, ArcSin)
DEFINE_VISIT_UNARY(Acos, ArcCos)
DEFINE_VISIT_UNARY(Atan, ArcTan)
// 円周率
DEFINE_VISIT_FACTOR(Pi, Pi)
// 対数
DEFINE_VISIT_BINARY(Log, Log)
DEFINE_VISIT_UNARY(Ln, Log)
// 自然対数の底
DEFINE_VISIT_FACTOR(E, E)

// 任意の文字列
void MathematicaExpressionConverter::visit(boost::shared_ptr<ArbitraryBinary> node){
  string_for_math_string_.append(node->get_string() + "[");
  accept(node->get_lhs());                                              
  string_for_math_string_.append(", ");                                   
  accept(node->get_rhs());                                                
  string_for_math_string_.append("]");                                    
}
void MathematicaExpressionConverter::visit(boost::shared_ptr<ArbitraryUnary> node){
  string_for_math_string_.append(node->get_string() + "[");
  accept(node->get_child());                                              
  string_for_math_string_.append("]");                                    
}
void MathematicaExpressionConverter::visit(boost::shared_ptr<ArbitraryFactor> node){
  string_for_math_string_.append(node->get_string());
}
  
// 変数
void MathematicaExpressionConverter::visit(boost::shared_ptr<Variable> node)              
{
  std::ostringstream tmp;
  if(differential_count_){
    tmp << "Derivative[" << differential_count_ << "][";
  }
  
  if(in_prev_){
    tmp << "prev[" << PacketSender::var_prefix << node->get_name() << "]";
  }else{
    tmp << PacketSender::var_prefix << node->get_name();
  }
  
  if(differential_count_){
   tmp << "]";
  }

  var_info_t new_var = 
    boost::make_tuple(node->get_name(), 
                      differential_count_, 
                      in_prev_ && !ignore_prev_);
  vars_.insert(new_var);
  string_for_math_string_.append(tmp.str());
}

// 数字
void MathematicaExpressionConverter::visit(boost::shared_ptr<Number> node)                
{    
  string_for_math_string_.append(node->get_number());
}


// 記号定数
void MathematicaExpressionConverter::visit(boost::shared_ptr<Parameter> node)                
{    
  string_for_math_string_.append(PacketSender::par_prefix + node->get_name());
}

// t
DEFINE_VISIT_FACTOR(SymbolicT, t)

} // namespace mathematica
} // namespace vcs
} // namespace hydla 

