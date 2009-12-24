#ifndef _INCLUDED_HYDLA_ASK_COLLECTOR_H_
#define _INCLUDED_HYDLA_ASK_COLLECTOR_H_

#include <vector>
#include <set>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "ModuleSet.h"
#include "TreeVisitor.h"

#include "./Types.h"

namespace hydla {
namespace simulator {

/**
 * askノードを集めるビジタークラス
 */
class AskCollector : public parse_tree::TreeVisitor {
public:
  AskCollector(const module_set_sptr& module_set,
               bool debug_mode = false);

  virtual ~AskCollector();

  /** 
   * askノードを集める
   *
   * @param expanded_always  展開済みalwaysノードの集合
   *                           （askの中にあったalwaysが展開されたもの）
   * @param negative_asks    ガード条件がエンテール不可能なaskノードの集合
   * @param positive_asks    ガード条件がエンテール可能なaskノードの集合
   */
  void collect_ask(const  expanded_always_t* expanded_always,                   
                   positive_asks_t*          positive_asks,
                   negative_asks_t*          negative_asks);


  // 制約式
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Constraint> node);

  // Ask制約
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // Tell制約
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);

  // 論理積
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);
  
  // 時相演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Always> node);

private:
  /// 収集をおこなう対象の制約モジュール集合
  module_set_sptr          module_set_; 

  const expanded_always_t* expanded_always_;                   
  
  /// 無効となっているaskのリスト
  negative_asks_t*         negative_asks_;

  /// 有効となっているaskのリスト
  positive_asks_t*         positive_asks_;

  /// デバッグ出力をするかどうか
  bool               debug_mode_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_ASK_COLLECTOR_H_
