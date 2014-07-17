#pragma once

#include "RelationGraph.h"
#include "AskRelationGraph.h"
#include "PhaseResult.h"

namespace hydla {
namespace simulator {

class ConstraintStore;

class ConstraintDifferenceCalculator{

public:
  typedef hierarchy::ModuleSet module_set_t;

  /*
   * Calculate which constraints is difference between a current todo and the parent.
   */
  void calculate_difference_constraints(const phase_result_sptr_t parent, const boost::shared_ptr<RelationGraph> relation_graph);

  void add_diference_constraints(const constraint_t constraint, const boost::shared_ptr<RelationGraph> relation_graph);

  /**
   * Get connected constraints which are changed
   */
  ConstraintStore get_difference_constraints();

  /**
   * return whether all variables of constraint are continuous
   */
  bool is_continuous(const phase_result_sptr_t parent, const constraint_t constraint);

  void collect_ask(const boost::shared_ptr<AskRelationGraph> ask_relation_graph,
      const ask_set_t &positive_asks,
      const ask_set_t &negative_asks,
      ask_set_t &unknown_asks);

private:
  ConstraintStore difference_constraints_;

  void set_symmetric_difference(
    const ConstraintStore& parent_constraints,
    const ConstraintStore& current_constraints,
    ConstraintStore& result );

};

} //namespace simulator
} //namespace hydla