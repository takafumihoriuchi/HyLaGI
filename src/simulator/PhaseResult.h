#pragma once

#include <vector>

#include <boost/shared_ptr.hpp>

#include "Variable.h"
#include "ValueRange.h"
#include "ModuleSet.h"
#include "ModuleSetContainer.h"
#include "ConstraintStore.h"
#include "Parameter.h"

namespace hydla {
namespace simulator {

class PhaseResult;
struct SimulationTodo;

/**
 * type for cause of termination of simulation
 */
typedef enum{
  TIME_LIMIT,
  STEP_LIMIT,
  SOME_ERROR,
  INCONSISTENCY,
  ASSERTION,
  OTHER_ASSERTION,
  TIME_OUT_REACHED,
  NOT_UNIQUE_IN_INTERVAL,
  NOT_SELECTED,
  INTERRUPTED,
  NONE
}CauseForTermination;


/**
 * type of a phase
 */
typedef enum {
  PointPhase,
  IntervalPhase,
} PhaseType;


typedef std::vector<boost::shared_ptr<symbolic_expression::Tell> > tells_t;
typedef boost::shared_ptr<symbolic_expression::Ask>                ask_t;
typedef std::set<ask_t >                                           ask_set_t;
typedef hierarchy::ModuleSet                              module_set_t;

typedef boost::shared_ptr<PhaseResult>                    phase_result_sptr_t;
typedef boost::shared_ptr<const PhaseResult>              phase_result_const_sptr_t;
typedef std::vector<phase_result_sptr_t >                 phase_result_sptrs_t;

typedef Value                                             value_t;
typedef ValueRange                                        range_t;
typedef Variable                                          variable_t;
typedef Parameter                                         parameter_t;

typedef std::map<variable_t, range_t, VariableComparator>                    variable_map_t;

typedef std::map<parameter_t, range_t, ParameterComparator>                   parameter_map_t;

typedef boost::shared_ptr<hierarchy::ModuleSetContainer> module_set_container_sptr;

typedef std::set<std::string> change_variables_t;


struct FullInformation
{
  ask_set_t                    positive_asks;
  ask_set_t                    negative_asks;
};

/**
 * A class to express the result of each phase.
 */
class PhaseResult {

public:
  PhaseType                    phase_type;
  int id;
  value_t                      current_time, end_time;

  variable_map_t               variable_map;
  parameter_map_t              parameter_map;

  ConstraintStore              expanded_constraints;
  
  ask_set_t                    get_all_positive_asks();
  ask_set_t                    get_all_negative_asks();
  ask_set_t                    diff_positive_asks, diff_negative_asks;

  void                         set_full_information(FullInformation *); /// instance pointed by given pointer will be freed internally
  
  int                          step;
  module_set_t                 module_set;
  ConstraintStore              current_constraints;
  ConstraintStore              changed_constraints;

  CauseForTermination cause_for_termination;
  /// A set of succeeding phases
  phase_result_sptrs_t children;
  /// A preceding phase
  PhaseResult *parent;

  PhaseResult();
  PhaseResult(const SimulationTodo& todo, const CauseForTermination& cause = NONE);
  ~PhaseResult();

private:
  void generate_full_information();
  FullInformation             *full_information;
};

std::ostream& operator<<(std::ostream& s, const PhaseResult& pr);

std::ostream& operator<<(std::ostream& s, const variable_map_t& vm);

std::ostream& operator<<(std::ostream& s, const parameter_map_t& pm);
std::ostream& operator<<(std::ostream& s, const ask_set_t& a);
std::ostream& operator<<(std::ostream& s, const tells_t& a);

std::ostream& operator<<(std::ostream& s, const ConstraintStore& a);

std::ostream& operator<<(std::ostream& s, const change_variables_t& a);

} // namespace simulator
} // namespace hydla 

