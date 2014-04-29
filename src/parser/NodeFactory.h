#ifndef _INCLUDED_HYDLA_PARSER_NODE_FACTORY_H_
#define _INCLUDED_HYDLA_PARSER_NODE_FACTORY_H_

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "Node.h"

namespace hydla { 
namespace parser {

#define NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(NAME) \
  virtual boost::shared_ptr<hydla::symbolic_expression::NAME> \
  create(hydla::symbolic_expression::NAME) const = 0;

class NodeFactory 
{
public:
  NodeFactory()
  {}

  virtual ~NodeFactory()
  {}

  template<typename NodeType>
  boost::shared_ptr<NodeType> create() const
  {
    return create(NodeType());
  }

protected:
  //定義
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(ProgramDefinition)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(ConstraintDefinition)

  //呼び出し
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(ProgramCaller)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(ConstraintCaller)
  
  //制約式
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Constraint)

  //Tell制約
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Tell)

  //Ask制約
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Ask)

  //比較演算子
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Equal)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(UnEqual)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Less)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(LessEqual)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Greater)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(GreaterEqual)

  //論理演算子
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(LogicalAnd)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(LogicalOr)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Not)

  //算術二項演算子
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Plus)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Subtract)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Times)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Divide)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Power)
  
  //算術単項演算子
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Negative)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Positive)

  //制約階層定義演算子
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Weaker)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Parallel)

  // 時相演算子
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Always)

  //微分
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Differential)
  
  //左極限
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Previous)
  
  //円周率
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Pi)
  //自然対数の底
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(E)
  //関数
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Function)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(UnsupportedFunction)
  
  

  //変数・束縛変数
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Variable)

  //数字
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Number)
  
  //Print
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Print)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(PrintPP)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(PrintIP)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Scan)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Exit)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Abort)

  //SystemVariable
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(SVtimer)

  // True
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(True)

  // System Time
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(SymbolicT)

};                                                     

} //namespace parser
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSER_NODE_FACTORY_H_
