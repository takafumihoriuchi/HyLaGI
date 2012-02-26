#include "TreeVisitor.h"

#include <assert.h>

namespace hydla { 
namespace parse_tree {

TreeVisitor::TreeVisitor()
{}

TreeVisitor::~TreeVisitor()
{}

// è`
void TreeVisitor::visit(boost::shared_ptr<ConstraintDefinition> node)  {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<ProgramDefinition> node)     {assert(0);}

// ÄÑoµ
void TreeVisitor::visit(boost::shared_ptr<ConstraintCaller> node)      {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<ProgramCaller> node)         {assert(0);}

// §ñ®
void TreeVisitor::visit(boost::shared_ptr<Constraint> node)            {assert(0);}

// Ask§ñ
void TreeVisitor::visit(boost::shared_ptr<Ask> node)                   {assert(0);}

// Tell§ñ
void TreeVisitor::visit(boost::shared_ptr<Tell> node)                  {assert(0);}

// ärZq
void TreeVisitor::visit(boost::shared_ptr<Equal> node)                 {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<UnEqual> node)               {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Less> node)                  {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<LessEqual> node)             {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Greater> node)               {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<GreaterEqual> node)          {assert(0);}

// _Zq
void TreeVisitor::visit(boost::shared_ptr<LogicalAnd> node)            {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<LogicalOr> node)             {assert(0);}
  
// ZpñZq
void TreeVisitor::visit(boost::shared_ptr<Plus> node)                  {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Subtract> node)              {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Times> node)                 {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Divide> node)                {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Power> node)                 {assert(0);}
  
// ZpPZq
void TreeVisitor::visit(boost::shared_ptr<Negative> node)              {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Positive> node)              {assert(0);}
  
// §ñKwè`Zq
void TreeVisitor::visit(boost::shared_ptr<Weaker> node)                {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Parallel> node)              {assert(0);}

// Zq
void TreeVisitor::visit(boost::shared_ptr<Always> node)                {assert(0);}

// ~ü¦
void TreeVisitor::visit(boost::shared_ptr<Pi> node)                {assert(0);}
// ©RÎÌê
void TreeVisitor::visit(boost::shared_ptr<E> node)                {assert(0);}
// CÓÌ¶ñ
void TreeVisitor::visit(boost::shared_ptr<ArbitraryNode> node)                {assert(0);}
  
// ÷ª
void TreeVisitor::visit(boost::shared_ptr<Differential> node)          {assert(0);}

// ¶ÉÀ
void TreeVisitor::visit(boost::shared_ptr<Previous> node)              {assert(0);}

// Ûè
void TreeVisitor::visit(boost::shared_ptr<Not> node)              {assert(0);}
  
// Ï
void TreeVisitor::visit(boost::shared_ptr<Variable> node)              {assert(0);}

// 
void TreeVisitor::visit(boost::shared_ptr<Number> node)                {assert(0);}

// Lè
void TreeVisitor::visit(boost::shared_ptr<Parameter> node)                {assert(0);}

// t
void TreeVisitor::visit(boost::shared_ptr<SymbolicT> node)                {assert(0);}
	
// Ö
void TreeVisitor::visit(boost::shared_ptr<Function> node)                {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<UnsupportedFunction> node)                {assert(0);}

// Print
void TreeVisitor::visit(boost::shared_ptr<Print> node)              {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<PrintPP> node)              {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<PrintIP> node)              {assert(0);}
    
void TreeVisitor::visit(boost::shared_ptr<Scan> node)              {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Exit> node)              {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Abort> node)              {assert(0);}

} //namespace parse_tree
} //namespace hydla
