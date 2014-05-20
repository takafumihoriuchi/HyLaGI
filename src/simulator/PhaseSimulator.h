#ifndef _INCLUDED_HYDLA_PHASE_SIMULATOR_H_
#define _INCLUDED_HYDLA_PHASE_SIMULATOR_H_

#include <iostream>
#include <string>
#include <cassert>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "Timer.h"
#include "Logger.h"
#include "PhaseResult.h"
#include "RelationGraph.h"
#include "Simulator.h"
#include "UnsatCoreFinder.h"
#include "AnalysisResultChecker.h"

namespace hydla {

namespace simulator {

typedef std::vector<parameter_map_t>                       parameter_maps_t;

struct CheckConsistencyResult
{
  ConstraintStore consistent_store, inconsistent_store;
};


typedef enum{
  CONDITIONS_TRUE,
  CONDITIONS_FALSE,
  CONDITIONS_VARIABLE_CONDITIONS
} ConditionsResult;

class PhaseSimulator{

public:
  typedef std::vector<simulation_todo_sptr_t> todo_list_t;
  typedef std::vector<phase_result_sptr_t> result_list_t;

  typedef std::map<module_set_sptr, hydla::symbolic_expression::node_sptr> condition_map_t;
  typedef hydla::symbolic_expression::node_sptr node_sptr;

  PhaseSimulator(Simulator* simulator, const Opts& opts);
  PhaseSimulator(PhaseSimulator&);

  virtual ~PhaseSimulator();

  virtual void set_backend(backend_sptr_t back);

  void set_break_condition(symbolic_expression::node_sptr break_cond);
  symbolic_expression::node_sptr get_break_condition();

  virtual void initialize(variable_set_t &v, parameter_map_t &p, variable_map_t &m, continuity_map_t& c, parse_tree_sptr pt, const module_set_container_sptr& msc);

  virtual void init_arc(const parse_tree_sptr& parse_tree);


  variable_map_t apply_time_to_vm(const variable_map_t &vm, const value_t &tm);

  /**
   * calculate phase results from given todo
   * @param todo_cont container of todo into which PhaseSimulator pushes todo if case analyses are needed
   *                  if it's null, PhaseSimulator uses internal container and handle all cases derived from given todo
   */
  result_list_t calculate_phase_result(simulation_todo_sptr_t& todo, todo_container_t* todo_cont = NULL);



	/**
   * HASimulator用
   */
	void substitute_parameter_condition(phase_result_sptr_t pr, parameter_map_t pm);

  int get_phase_sum()const{return phase_sum_;}

  void set_select_function(int (*f)(result_list_t&)){select_phase_ = f;}


  typedef hierarchy::module_set_sptr              modulse_set_sptr;
  typedef std::set< std::string > change_variables_t;

/*
  virtual void find_unsat_core(const module_set_sptr& ms,
      simulation_todo_sptr_t&,
      const variable_map_t& vm);
*/


 	/**
   * make todos from given phase_result
   * this function doesn't change the 'phase' argument except the end time of phase
   */
  virtual todo_list_t make_next_todo(phase_result_sptr_t& phase, simulation_todo_sptr_t& current_todo);


  /// pointer to the backend to be used
  backend_sptr_t backend_;
  bool breaking;

protected:

  typedef enum{
    ENTAILED,
    CONFLICTING,
    BRANCH_VAR,
    BRANCH_PAR
  } CheckEntailmentResult;

  result_list_t simulate_ms(const module_set_sptr& ms, const variable_map_t&, simulation_todo_sptr_t& state);

  /**
   * 与えられたsimulation_todo_sptr_tの情報を引き継いだ，
   * 新たなsimulation_todo_sptr_tの作成
   */
  simulation_todo_sptr_t create_new_simulation_phase(const simulation_todo_sptr_t& old) const;

  /**
   * PPモードとIPモードを切り替える
   */
  void set_simulation_mode(const PhaseType& phase);


  Simulator* simulator_;

  void replace_prev2parameter(
                              phase_result_sptr_t &phase,
                              variable_map_t &vm,
                              parameter_map_t &parameter_map);

  const Opts *opts_;

  variable_set_t *variable_set_;
  parameter_map_t *parameter_map_;
  variable_map_t *variable_map_;
  negative_asks_t prev_guards_;

  int phase_sum_;

  /**
   * graph of relation between module_set for IP and PP
   */
  boost::shared_ptr<RelationGraph> relation_graph_;

  /**
   * 解候補モジュール集合のコンテナ
   * （非always制約を除いたバージョン）
   */
  module_set_container_sptr msc_no_init_;

  todo_container_t* todo_container_;

  phase_result_sptr_t make_new_phase(const phase_result_sptr_t& original);


  void push_branch_states(simulation_todo_sptr_t &original,
    CheckConsistencyResult &result);


  /// ケースの選択時に使用する関数ポインタ
  int (*select_phase_)(result_list_t&);
  symbolic_expression::node_sptr break_condition_;
  parse_tree_sptr parse_tree_;

private:

  result_list_t make_results_from_todo(simulation_todo_sptr_t& todo);

  phase_result_sptr_t make_new_phase(simulation_todo_sptr_t& todo, const ConstraintStore& store);

  std::set<module_set_sptr> checkd_module_set_;

  /**
   * 与えられた制約モジュール集合の閉包計算を行い，無矛盾性を判定するとともに対応する変数表を返す．
   */
  virtual ConstraintStore calculate_constraint_store(const module_set_sptr& ms,
                           simulation_todo_sptr_t& state);

  void apply_discrete_causes_to_guard_judgement( ask_set_t& discrete_causes,
                                                 positive_asks_t& positive_asks,
                                                 negative_asks_t& negative_asks,
                                                 ask_set_t& unknown_asks );

  void set_changing_variables( const phase_result_sptr_t& parent_phase,
                             const module_set_sptr& present_ms,
                             const positive_asks_t& positive_asks,
                             const negative_asks_t& negative_asks,
                             change_variables_t& changing_variables );

  void set_changed_variables(phase_result_sptr_t& phase);

  change_variables_t get_difference_variables_from_2tells(const tells_t& larg, const tells_t& rarg);

  bool apply_entailment_change( const ask_set_t::iterator it,
                                const ask_set_t& previous_asks,
                                const bool in_IP,
                                change_variables_t& changing_variables,
                                ask_set_t& notcv_unknown_asks,
                                ask_set_t& unknown_asks );

  void apply_previous_solution(const change_variables_t& changing_variables,
                             const bool in_IP,
                             const phase_result_sptr_t parent,
                             continuity_map_t& continuity_map,
                             const value_t& current_time );


  bool calculate_closure(simulation_todo_sptr_t& state,
    const module_set_sptr& ms);

  /**
   * Check whether a guard is entailed or not.
   * If the entailment depends on the condition of variables or parameters, return BRANHC_VAR or BRANCH_PAR.
   * If the return value is BRANCH_PAR, the value of cc_result consists of cases the guard is entailed and cases the guard is not entailed.
   */
  CheckEntailmentResult check_entailment(
    CheckConsistencyResult &cc_result,
    const symbolic_expression::node_sptr& guard,
    const continuity_map_t& cont_map,
    const PhaseType& phase);

  CheckConsistencyResult check_consistency(const PhaseType &phase);

  bool has_variables(symbolic_expression::node_sptr node, const change_variables_t &variables, bool include_prev);

  void add_continuity(const continuity_map_t&, const PhaseType &phase);

  virtual module_set_list_t calculate_mms(
    simulation_todo_sptr_t& state,
    const variable_map_t& vm);
/*
  virtual void mark_nodes_by_unsat_core(const modulse_set_sptr& ms,
      simulation_todo_sptr_t&,
    const variable_map_t& vm);
*/

  //virtual ConstraintStoreResult check_conditions(const module_set_sptr& ms, simulation_todo_sptr_t&, const variable_map_t &, bool b);
  void replace_prev2parameter(phase_result_sptr_t& state,
                              ConstraintStore& store,
                              parameter_map_t &parameter_map);


  continuity_map_t variable_derivative_map_;

  boost::shared_ptr<AnalysisResultChecker > analysis_result_checker_;
  boost::shared_ptr<UnsatCoreFinder > unsat_core_finder_;

  PhaseType current_phase_;


};


} //namespace simulator
} //namespace hydla

#endif // _INCLUDED_HYDLA_PHASE_SIMULATOR_H_
