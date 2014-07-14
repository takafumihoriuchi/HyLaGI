#include "ConstraintDifferenceCalculator.h"
#include "VariableFinder.h"

namespace hydla
{
namespace simulator
{

void ConstraintDifferenceCalculator::set_difference_constraints(const ConstraintStore& constraints)
{
  ConstraintStore tmp_constraints;
  module_set_t module_set;
  //TODO: IPでprevを区別する
  for(auto constraint : constraints){
    relation_graph_->get_related_constraints(constraint, tmp_constraints, module_set);
    difference_constraints_.add_constraint_store(tmp_constraints);
  }
}

ConstraintStore ConstraintDifferenceCalculator::get_difference_constraints(){
  return difference_constraints_;
}

bool ConstraintDifferenceCalculator::is_difference(const ConstraintStore constraint_store)
{
  set_difference_constraints(difference_constraints_);
  for(auto constraint : constraint_store){
    if(difference_constraints_.count(constraint)) return true;
  }
  return false;
}

bool ConstraintDifferenceCalculator::is_difference(const constraint_t constraint)
{
  ConstraintStore constraint_store;
  module_set_t module_set;
  relation_graph_->get_related_constraints(constraint, constraint_store, module_set);
  for(auto tmp_constraint : constraint_store){
    if(difference_constraints_.count(tmp_constraint)) return true;
  }
  return false;
}

bool ConstraintDifferenceCalculator::is_difference(const Variable& variable)
{
  VariableFinder finder;
  for(auto constraint : difference_constraints_){
    finder.visit_node(constraint);
  }
  return finder.include_variable(variable) || finder.include_variable_prev(variable);
}

void ConstraintDifferenceCalculator::clear_difference(){
  difference_constraints_.clear();
}

bool ConstraintDifferenceCalculator::is_continuous(const phase_result_sptr_t parent, const constraint_t constraint)
{
  
  VariableFinder finder;
  finder.visit_node(constraint);
  variable_set_t variables(finder.get_all_variable_set());
  for(auto variable : variables){
    if(parent->phase_type == PointPhase){
      auto differential_pair = parent->variable_map.find(Variable(variable.get_name(), variable.get_differential_count()));
      if(differential_pair == parent->variable_map.end() || differential_pair->second.undefined()) return false;
    }
  }
  return true;
}

void ConstraintDifferenceCalculator::set_relation_graph(const boost::shared_ptr<RelationGraph> relation_graph,
    const boost::shared_ptr<GuardRelationGraph> guard_relation_graph){
  relation_graph_ = relation_graph;
  guard_relation_graph_ = guard_relation_graph;
}


}
}
