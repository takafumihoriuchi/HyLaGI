#include "EntailmentChecker.h"

#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace bp_simulator {


EntailmentChecker::EntailmentChecker() :
  in_differential_equality_(false),
  in_differential_(false)
{}

EntailmentChecker::~EntailmentChecker()
{}

// Tell制約
void EntailmentChecker::visit(boost::shared_ptr<Tell> node)                  
{
  ////ml_.MLPutFunction("tell", 1);

  //node_sptr chnode(node->get_child_node());
  //chnode->accept(chnode, this);
  //std::cout << std::endl;
}

// 比較演算子
void EntailmentChecker::visit(boost::shared_ptr<Equal> node)                 
{
  //ml_.MLPutFunction("Equal", 2);
  //      
  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //std::cout << "=";
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  //in_differential_equality_ = false;
}

void EntailmentChecker::visit(boost::shared_ptr<UnEqual> node)               
{
  //ml_.MLPutFunction("UnEqual", 2);
  //  
  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //std::cout << "!=";
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  //in_differential_equality_ = false;
}

void EntailmentChecker::visit(boost::shared_ptr<Less> node)                  
{
  //ml_.MLPutFunction("Less", 2);
  //  
  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //std::cout << "<";
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  //in_differential_equality_ = false;
}

void EntailmentChecker::visit(boost::shared_ptr<LessEqual> node)             
{
  //ml_.MLPutFunction("LessEqual", 2);
  //  
  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //std::cout << "<=";
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  //in_differential_equality_ = false;
}

void EntailmentChecker::visit(boost::shared_ptr<Greater> node)               
{
  //ml_.MLPutFunction("Greater", 2);
  //  
  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //std::cout << ">";
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  //in_differential_equality_ = false;
}

void EntailmentChecker::visit(boost::shared_ptr<GreaterEqual> node)          
{
  //ml_.MLPutFunction("GreaterEqual", 2);
  //  
  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //std::cout << ">=";
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  //in_differential_equality_ = false;
}

// 論理演算子
void EntailmentChecker::visit(boost::shared_ptr<LogicalAnd> node)            
{
  ////ml_.MLPutFunction("And, 2);

  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}

void EntailmentChecker::visit(boost::shared_ptr<LogicalOr> node)             
{
  ////ml_.MLPutFunction("Or", 2);
  //  
  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}
  
// 算術二項演算子
void EntailmentChecker::visit(boost::shared_ptr<Plus> node)                  
{
  //ml_.MLPutFunction("Plus", 2);
  //  
  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //std::cout << "+";
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}

void EntailmentChecker::visit(boost::shared_ptr<Subtract> node)              
{
  //ml_.MLPutFunction("Subtract", 2);
  //  
  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //std::cout << "-";
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}

void EntailmentChecker::visit(boost::shared_ptr<Times> node)           
{
  //ml_.MLPutFunction("Times", 2);

  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //std::cout << "*";
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}

void EntailmentChecker::visit(boost::shared_ptr<Divide> node)                
{
  //ml_.MLPutFunction("Divide", 2);
  //  
  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //std::cout << "/";
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}
  
// 算術単項演算子
void EntailmentChecker::visit(boost::shared_ptr<Negative> node)
{
  //ml_.MLPutFunction("Minus", 1);
  //std::cout << "-";

  //node_sptr chnode(node->get_child_node());
  //chnode->accept(chnode, this);
}

void EntailmentChecker::visit(boost::shared_ptr<Positive> node)              
{
  //node_sptr chnode(node->get_child_node());
  //chnode->accept(chnode, this);
}

// 微分
void EntailmentChecker::visit(boost::shared_ptr<Differential> node)          
{
  //ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
  //ml_.MLPutArgCount(1);      // this 1 is for the 'f'
  //ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
  //ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
  //ml_.MLPutSymbol("Derivative");
  //ml_.MLPutInteger(1);


  //in_differential_equality_ = true;
  //in_differential_ = true;
  //node_sptr chnode(node->get_child_node());
  //chnode->accept(chnode, this);
  //std::cout << "'";
  //if(in_differential_){
  //  //std::cout << "[t]"; // ht[t]' のようになるのを防ぐため
  //}

  //in_differential_ = false;
}

// 左極限
void EntailmentChecker::visit(boost::shared_ptr<Previous> node)              
{
  ////ml_.MLPutFunction("prev", 1);
  //node_sptr chnode(node->get_child_node());
  //chnode->accept(chnode, this);
  //std::cout << "-";
}
  
// 変数
void EntailmentChecker::visit(boost::shared_ptr<Variable> node)              
{
  //ml_.MLPutSymbol(node->get_name().c_str());
  //if(in_differential_){
  //  vars_.insert(std::pair<std::string, int>(node->get_name() + "'", 1));
  //}
  //else {
  //  vars_.insert(std::pair<std::string, int>(node->get_name(), 0));
  //}
  //std::cout << node->get_name().c_str();
  //if(in_differential_equality_){
  //  if(in_differential_){
  //    //ml_.MLPutSymbol("t");
  //  }else{
  //    //ml_.MLPutSymbol("t");
  //    //std::cout << "[t]";
  //  }
  //} else {
  //  //ml_.MLPutInteger(0);
  //  //std::cout << "[0]";
  //}
}

// 数字
void EntailmentChecker::visit(boost::shared_ptr<Number> node)                
{
  //ml_.MLPutInteger(atoi(node->get_number().c_str()));
  //std::cout << node->get_number().c_str();
}

/**
 * collected_tellsからnegative_askのガード条件がentailされるどうか調べる
 * TRUEならcollected_tellsにask制約の後件を追加する
 * 
 * Input:
 *  negative_ask まだ展開されていないask制約
 *  collected_tells tell制約のリスト（展開されたask制約の「=>」の右辺がここに追加される）
 * Output:
 *  entailされるかどうか {TRUE, UNKNOWN, FALSE}
 */
Trivalent EntailmentChecker::check_entailment(
  const boost::shared_ptr<Ask>& negative_ask,
  tells_t& collected_tells)
{
  // collected_tellsをrp_constraint化 -> S
  // negative_ask前件をrp_constraint化 -> g
  // !(negative_ask前件)をrp_constraint化 -> ng
  // if []solve(X, S & g, D) == empty -> FALSE
  // elseif []solve(X, S & ng, D) == empty -> TRUE
  // else -> UNKNOWN
  return FALSE;
}

} //namespace bp_simulator
} // namespace hydla
