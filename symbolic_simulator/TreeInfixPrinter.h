#ifndef TREE_INFIX__PRINTER_H_
#define TREE_INFIX__PRINTER_H_


//c[๐uL@ลoอท้NX

#include "Node.h"
#include "TreeVisitor.h"

namespace hydla {
namespace symbolic_simulator {


class TreeInfixPrinter:
  public hydla::parse_tree::TreeVisitor
{
  public:
  typedef enum{
    PAR_NONE,
    PAR_N,
    PAR_N_P_S,
    PAR_N_P_S_T_D_P,
  }needParenthesis;

  typedef hydla::parse_tree::node_sptr node_sptr;

  
  //valueฦมฤถ๑ษฯทท้
  std::ostream& print_infix(const node_sptr &, std::ostream&);

  private:
  
  needParenthesis need_par_;
  std::ostream *output_stream_;
  
  void print_binary_node(const hydla::parse_tree::BinaryNode &, const std::string &symbol,
                          const needParenthesis &pre_par = PAR_NONE, const needParenthesis &post_par = PAR_NONE);
  void print_unary_node(const hydla::parse_tree::UnaryNode &, const std::string &pre, const std::string &post);



  // ง๑่`
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintDefinition> node);
  
  // vO่`
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ProgramDefinition> node);

  // ง๑ฤัoต
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node);
  
  // vOฤัoต
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node);

  // ง๑ฎ
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Constraint> node);

  // Askง๑
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // Tellง๑
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);


  // ไrZq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Equal> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Less> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Greater> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node);

  
  // ง๑Kw่`Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Weaker> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parallel> node);

  // Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Always> node);


  // _Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalOr> node);

  // Zp๑Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Times> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Divide> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Power> node);

  // ZpPZq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);

  // ๗ช
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // ถษภ
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);
  
  // ฯ
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // 
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);

  // L่
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parameter> node);

  // t
  virtual void visit(boost::shared_ptr<hydla::parse_tree::SymbolicT> node);
};

} // namespace symbolic_simulator
} // namespace hydla 

#endif //_INCLUDED_HYDLA_VCS_MATHEMATICA_EXPRESSION_CONVERTER_H_
