#ifndef _INCLUDED_HYDLA_PARSE_TREE_GRAPHVIZ_DUMPER_H_
#define _INCLUDED_HYDLA_PARSE_TREE_GRAPHVIZ_DUMPER_H_

#include <ostream>

#include <boost/shared_ptr.hpp>

#include "Node.h"

namespace hydla { 
namespace parse_tree {
  
/**
 * ParseTreeをdot言語形式でダンプする
 */
class ParseTreeGraphvizDumper {
public:
  ParseTreeGraphvizDumper()
  {}

  virtual ~ParseTreeGraphvizDumper()
  {}

  std::ostream& dump(std::ostream& s);


};

} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_GRAPHVIZ_DUMPER_H_