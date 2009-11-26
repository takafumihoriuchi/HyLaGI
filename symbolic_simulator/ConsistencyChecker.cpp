#include "ConsistencyChecker.h"
//#include "PacketChecker.h"
#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {


ConsistencyChecker::ConsistencyChecker(MathLink& ml) :
  ml_(ml),
  in_differential_equality_(false),
  in_differential_(false),
  in_prev_(false)
{}

ConsistencyChecker::~ConsistencyChecker()
{}

// Tell制約
void ConsistencyChecker::visit(boost::shared_ptr<Tell> node)                  
{
  //ml_.MLPutFunction("tell", 1);

  node_sptr chnode(node->get_child_node());
  chnode->accept(chnode, this);
  std::cout << std::endl;
}

// 比較演算子
void ConsistencyChecker::visit(boost::shared_ptr<Equal> node)                 
{
  ml_.MLPutFunction("Equal", 2);
        
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "=";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  in_differential_equality_ = false;
}

void ConsistencyChecker::visit(boost::shared_ptr<UnEqual> node)               
{
  ml_.MLPutFunction("UnEqual", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "!=";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  in_differential_equality_ = false;
}

void ConsistencyChecker::visit(boost::shared_ptr<Less> node)                  
{
  ml_.MLPutFunction("Less", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "<";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  in_differential_equality_ = false;
}

void ConsistencyChecker::visit(boost::shared_ptr<LessEqual> node)             
{
  ml_.MLPutFunction("LessEqual", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "<=";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  in_differential_equality_ = false;
}

void ConsistencyChecker::visit(boost::shared_ptr<Greater> node)               
{
  ml_.MLPutFunction("Greater", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << ">";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  in_differential_equality_ = false;
}

void ConsistencyChecker::visit(boost::shared_ptr<GreaterEqual> node)          
{
  ml_.MLPutFunction("GreaterEqual", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << ">=";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  in_differential_equality_ = false;
}

// 論理演算子
void ConsistencyChecker::visit(boost::shared_ptr<LogicalAnd> node)            
{
  ml_.MLPutFunction("And", 2);

  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}

void ConsistencyChecker::visit(boost::shared_ptr<LogicalOr> node)             
{
  //ml_.MLPutFunction("Or", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}
  
// 算術二項演算子
void ConsistencyChecker::visit(boost::shared_ptr<Plus> node)                  
{
  ml_.MLPutFunction("Plus", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "+";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}

void ConsistencyChecker::visit(boost::shared_ptr<Subtract> node)              
{
  ml_.MLPutFunction("Subtract", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "-";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}

void ConsistencyChecker::visit(boost::shared_ptr<Times> node)                 
{
  ml_.MLPutFunction("Times", 2);

  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "*";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}

void ConsistencyChecker::visit(boost::shared_ptr<Divide> node)                
{
  ml_.MLPutFunction("Divide", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "/";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}
  
// 算術単項演算子
void ConsistencyChecker::visit(boost::shared_ptr<Negative> node)              
{       
  ml_.MLPutFunction("Minus", 1);
  std::cout << "-";

  node_sptr chnode(node->get_child_node());
  chnode->accept(chnode, this);
}

void ConsistencyChecker::visit(boost::shared_ptr<Positive> node)              
{
  node_sptr chnode(node->get_child_node());
  chnode->accept(chnode, this);
}

// 微分
void ConsistencyChecker::visit(boost::shared_ptr<Differential> node)          
{

/*
  ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
  ml_.MLPutArgCount(1);      // this 1 is for the 'f'
  ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
  ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
  ml_.MLPutSymbol("Derivative");
  ml_.MLPutInteger(1);
*/

  in_differential_equality_ = true;
  in_differential_ = true;
  node_sptr chnode(node->get_child_node());
  chnode->accept(chnode, this);
  std::cout << "dash";
  if(in_differential_){
    //std::cout << "[t]"; // ht[t]' のようになるのを防ぐため
  }

  in_differential_ = false;
}

// 左極限
void ConsistencyChecker::visit(boost::shared_ptr<Previous> node)              
{
  in_prev_ = true;
  //ml_.MLPutFunction("prev", 1);
  node_sptr chnode(node->get_child_node());
  chnode->accept(chnode, this);
  std::cout << "prev";

  in_prev_ = false;
}
  
// 変数
void ConsistencyChecker::visit(boost::shared_ptr<Variable> node)              
{

  if(in_differential_){
    ml_.MLPutSymbol(("usrVar" + node->get_name() + "dash").c_str());
    vars_.insert(std::pair<std::string, int>("usrVar" + node->get_name() + "dash", 0)); // 本当は1として扱いたい？
  }
  else if(in_prev_){
    ml_.MLPutSymbol(("usrVar" + node->get_name() + "prev").c_str());
    vars_.insert(std::pair<std::string, int>("usrVar" + node->get_name() + "prev", 0)); // 本当は2として扱いたい
  }
  else {
    ml_.MLPutSymbol(("usrVar" + node->get_name()).c_str());
    vars_.insert(std::pair<std::string, int>("usrVar" + node->get_name(), 0));
  }
  std::cout << node->get_name().c_str();
  if(in_differential_equality_){
    if(in_differential_){
      //ml_.MLPutSymbol("t");
    }else{
      //ml_.MLPutSymbol("t");
      //std::cout << "[t]";
    }
  } else {
    //ml_.MLPutInteger(0);
    //std::cout << "[0]";
  }
}

// 数字
void ConsistencyChecker::visit(boost::shared_ptr<Number> node)                
{    
  ml_.MLPutInteger(atoi(node->get_number().c_str()));
  std::cout << node->get_number().c_str();
}


bool ConsistencyChecker::is_consistent(collected_tells_t& collected_tells)
{

/*
  ml_.MLPutFunction("isConsistent", 2);
  ml_.MLPutFunction("List", 3);
  ml_.MLPutFunction("Equal", 2);
  ml_.MLPutSymbol("x");
  ml_.MLPutSymbol("y");
  ml_.MLPutFunction("Equal", 2);
  ml_.MLPutSymbol("x");
  ml_.MLPutInteger(2);
  ml_.MLPutFunction("Equal", 2);
  ml_.MLPutSymbol("y");
  ml_.MLPutInteger(1);

  ml_.MLPutFunction("List", 2);
  ml_.MLPutSymbol("x");
  ml_.MLPutSymbol("y");
  ml_.MLEndPacket();
*/


  // isConsistent[expr, vars]を渡したい
  ml_.MLPutFunction("isConsistent", 2);

  // tell制約の集合からexprを得てMathematicaに渡す
  int tells_size = collected_tells.size();
  ml_.MLPutFunction("List", tells_size);
  collected_tells_t::iterator tells_it = collected_tells.begin();
  while((tells_it) != collected_tells.end())
  {
    visit((*tells_it));
    tells_it++;
  }


  // varsを渡す
  int var_num = vars_.size();
  ml_.MLPutFunction("List", var_num);
  std::map<std::string, int>::iterator vars_it = vars_.begin();
  const char* sym;
  std::cout << "vars: ";
  while(vars_it!=vars_.end())
  {
    sym = ((*vars_it).first).c_str();
    switch((*vars_it).second)
    {
    case 0:
      ml_.MLPutSymbol(sym);
      break;
    case 1:
      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
      ml_.MLPutArgCount(1);      // this 1 is for the 'f'
      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
      ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
      ml_.MLPutSymbol("Derivative");
      ml_.MLPutInteger(1);
      ml_.MLPutSymbol(sym);
      //ml_.MLPutSymbol("t");
      break;
    case 2:
      ml_.MLPutFunction("prev", 1);
      ml_.MLPutSymbol(sym);
      break;
    default:
      ;
    }
    std::cout << sym << " ";
    vars_it++;
  }

  // ml_.MLEndPacket();

  // 要素の全削除
  vars_.clear();

  std::cout << std::endl;


/*
// 返ってくるパケットを解析
PacketChecker pc(ml_);
pc.check();
*/

  ml_.skip_pkt_until(RETURNPKT);
  
  int num;
  ml_.MLGetInteger(&num);
  std::cout << "ConsistencyChecker#num:" << num << std::endl;
  
  // Mathematicaから1（Trueを表す）が返ればtrueを、0（Falseを表す）が返ればfalseを返す
  return num==1;
}


} //namespace symbolic_simulator
} // namespace hydla
