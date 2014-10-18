#pragma once

#include <iostream>
#include <string>
#include <cassert>

#include <boost/shared_ptr.hpp>
#include "PhaseResult.h"
#include "Simulator.h"
#include "ConsistencyChecker.h"
#include "RelationGraph.h"

namespace hydla {
namespace simulator {

class ValueModifier;

struct FindMinTimeCandidate
{
  value_t         time;
  bool            on_time;
  parameter_map_t parameter_map;
};


typedef std::vector<FindMinTimeCandidate> find_min_time_result_t;

struct CompareMinTimeResult
{
  std::vector<parameter_map_t> less_maps, greater_maps, equal_maps;
};


typedef std::vector<parameter_map_t>                       parameter_maps_t;
typedef symbolic_expression::node_sptr                     node_sptr;

class PhaseSimulator{
public:
  PhaseSimulator(Simulator* simulator, const Opts& opts);
  virtual ~PhaseSimulator();

  virtual void initialize(variable_set_t  &v,
                          parameter_map_t &p,
                          variable_map_t  &m,
                          module_set_container_sptr &msc,
                          phase_result_sptr_t root);


  void process_todo(phase_result_sptr_t&);


  void set_backend(backend_sptr_t);

  void apply_diff(const PhaseResult &phase);
  
  /// revert diff
  void revert_diff(const PhaseResult &phase);
  void revert_diff(const ask_set_t &positive_asks, const ask_set_t &negative_asks, const ConstraintStore &always_list, const module_diff_t &module_diff);

  boost::shared_ptr<RelationGraph> relation_graph_;

private:

  std::list<phase_result_sptr_t> simulate_ms(const module_set_t& unadopted_ms, phase_result_sptr_t& state, ask_set_t trigger_asks, ask_set_t positive_asks, ask_set_t negative_asks);

  void replace_prev2parameter(
                              PhaseResult &phase,
                              variable_map_t &vm,
                              parameter_map_t &parameter_map);

  module_diff_t get_module_diff(module_set_t unadopted_ms, module_set_t parent_unadopted);

  variable_map_t get_related_vm(const node_sptr &node, const variable_map_t &vm);

  std::list<phase_result_sptr_t> make_results_from_todo(phase_result_sptr_t& todo);

  void push_branch_states(phase_result_sptr_t &original,
                          CheckConsistencyResult &result);

  pp_time_result_t compare_min_time(const pp_time_result_t &existing, const find_min_time_result_t &newcomer, const ask_t& ask);
  pp_time_result_t compare_min_time(const pp_time_result_t &existing, const pp_time_result_t &newcomer);

  bool calculate_closure(phase_result_sptr_t& state, ask_set_t &trigger_asks, ConstraintStore &diff_sum, ask_set_t &positive_asks, ask_set_t &negative_asks, ConstraintStore always);

  bool judge_continuity(const phase_result_sptr_t &job, const ask_t &ask, const variable_set_t &changing_variables);

 	/// make todos from given phase_result
  void make_next_todo(phase_result_sptr_t& phase);

  Simulator* simulator_;

  const Opts *opts_;

  variable_set_t *variable_set_;
  parameter_map_t *parameter_map_;
  variable_map_t *variable_map_;
  std::set<std::string> variable_names;

  module_set_container_sptr msc_no_init_;

  phase_result_sptr_t result_root;

  bool relation_graph_is_taken_over;  /// indicates whether the state of relation_graph_ is taken over from parent phase

  boost::shared_ptr<ConsistencyChecker> consistency_checker;
  int                                   phase_sum_, todo_id;
  module_set_container_sptr             module_set_container;
  ask_set_t                             all_asks;
  boost::shared_ptr<ValueModifier>      value_modifier;
  value_t                               max_time;

  /// pointer to the backend to be used
  backend_sptr_t backend_;

};


} //namespace simulator
} //namespace hydla
