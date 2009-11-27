#ifndef _INCLUDED_HYDLA_PARSE_TREE_H_
#define _INCLUDED_HYDLA_PARSE_TREE_H_

#include <string>
#include <vector>
#include <map>

#include <boost/shared_ptr.hpp>

#include "ParseError.h"
#include "Node.h"

namespace hydla { 
namespace parse_tree {

class ParseTree {
public:
  // 定義の型
  typedef std::string                             difinition_name_t;
  typedef int                                     bound_variable_count_t;
  typedef std::pair<difinition_name_t, 
                    bound_variable_count_t>       difinition_type_t;

  // 制約定義
  typedef boost::shared_ptr<hydla::parse_tree::ConstraintDefinition> constraint_def_map_value_t;
  typedef std::map<difinition_type_t,
                   constraint_def_map_value_t>    constraint_def_map_t;

  // プログラム定義
  typedef boost::shared_ptr<hydla::parse_tree::ProgramDefinition>    program_def_map_value_t;
  typedef std::map<difinition_type_t,
                   program_def_map_value_t>       program_def_map_t;

  ParseTree();
  virtual ~ParseTree();

  /**
   * 制約定義を追加する
   */
  void addConstraintDefinition(const boost::shared_ptr<ConstraintDefinition>& d)
  {
    cons_def_map_.insert(make_pair(create_definition_key(d), d));
  }
  
  /**
   * プログラム定義を追加する
   */
  void addProgramDefinition(boost::shared_ptr<ProgramDefinition> d)
  {
    prog_def_map_.insert(make_pair(create_definition_key(d), d));
  }

  /**
   * パースされたノードツリーの設定
   */
  void setTree(const node_sptr& tree) 
  {
    node_tree_ = tree;
  }

  std::string to_string();
  void preprocess();

  void dispatch(parse_tree::TreeVisitor* visitor)
  {
    if(node_tree_) node_tree_->accept(node_tree_, visitor);
  }

  const variable_map_t& get_variable_map() const 
  {
    return variable_map_;
  }

  const constraint_def_map_t* get_constraint_def_map() const
  {
    return &cons_def_map_;
  }

  const program_def_map_t* get_program_def_map() const
  {
    return &prog_def_map_;
  }

  /**
   * すべてのデータを破棄し、初期状態に戻す
   */
  void clear()
  {
    node_tree_.reset();
    cons_def_map_.clear();
    prog_def_map_.clear();
  }

private:
  difinition_type_t create_definition_key(boost::shared_ptr<Definition> d);

  node_sptr            node_tree_;
  constraint_def_map_t cons_def_map_;
  program_def_map_t    prog_def_map_;
  variable_map_t       variable_map_;
};

} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_H_
