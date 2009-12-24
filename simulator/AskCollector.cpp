#include "AskCollector.h"

#include <assert.h>

#include <iostream>
#include <algorithm>

#include <boost/bind.hpp>
#include <boost/iterator/indirect_iterator.hpp>

using namespace std;
using namespace hydla::parse_tree;

namespace hydla {
namespace simulator {
  
namespace {
class NodeDumper {
public:
  template<typename T>
  void operator()(T& it) 
  {
    std::cout << *it << "\n";
  }
};
}

AskCollector::AskCollector(const module_set_sptr& module_set,
                           bool debug_mode) :
  module_set_(module_set),
  debug_mode_(debug_mode)
{}

AskCollector::~AskCollector()
{}

void AskCollector::collect_ask(const expanded_always_t* expanded_always,                   
                               positive_asks_t*         positive_asks,
                               negative_asks_t*         negative_asks)
{
  assert(expanded_always);
  assert(negative_asks);
  assert(positive_asks);

  negative_asks_   = negative_asks;
  positive_asks_   = positive_asks;

  // 各ノードの探索
  module_set_->dispatch(this);
  for_each(expanded_always->begin(), 
           expanded_always->end(),
           bind(&Always::accept, _1, _1, this));

  if(debug_mode_) {
    std::cout << "#** positive asks **\n";  
    std::for_each(positive_asks->begin(), positive_asks->end(), NodeDumper());

    std::cout << "#** negative asks **\n";  
    std::for_each(negative_asks->begin(), negative_asks->end(), NodeDumper());
  }
}

// 制約式
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Constraint> node)
{
  accept(node->get_child());
}

// Ask制約
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{
  if(positive_asks_->find(node) != positive_asks_->end()) {
    // 既に展開済みのaskノードであった場合
    accept(node->get_child());
  } else {
    // まだ展開されていないaskノードであった場合
    negative_asks_->insert(node);  
  }
}

// Tell制約
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Tell> node)
{
  // do nothing
}

// 論理積
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// 時相演算子
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Always> node)
{
  accept(node->get_child());
}

} //namespace simulator
} //namespace hydla 
