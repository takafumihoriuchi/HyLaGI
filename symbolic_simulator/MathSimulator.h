#ifndef _INCLUDED_MATH_SIMULATOR_H_
#define _INCLUDED_MATH_SIMULATOR_H_

#include <string>

#include "ParseTree.h"

#include "mathlink_helper.h"
#include "Simulator.h"

#include "SymbolicVariable.h"
#include "SymbolicValue.h"
#include "SymbolicTime.h"

namespace hydla {
namespace symbolic_simulator {

typedef hydla::simulator::VariableMap<SymbolicVariable, SymbolicValue> variable_map_t;
typedef hydla::simulator::PhaseState<SymbolicVariable, SymbolicValue, SymbolicTime> phase_state_t;
typedef boost::shared_ptr<phase_state_t> phase_state_sptr;
typedef hydla::simulator::Simulator<phase_state_t> simulator_t;

class MathSimulator : public simulator_t
{
public:

  typedef enum OutputFormat_ {
    fmtTFunction,
    fmtNumeric,
  } OutputFormat;

  typedef struct Opts_ {
    std::string mathlink;
    bool debug_mode;
    std::string max_time;
    bool profile_mode;
    bool parallel_mode;
    OutputFormat output_format;
  } Opts;

  MathSimulator();
  virtual ~MathSimulator();

  bool simulate(boost::shared_ptr<hydla::ch::ModuleSetContainer> msc,
                boost::shared_ptr<hydla::parse_tree::ParseTree> pt,
                Opts& opts);

//  virtual bool test_module_set(hydla::ch::module_set_sptr ms);


  /**
   * Point Phaseの処理
   */
  virtual bool point_phase(hydla::ch::module_set_sptr& ms, phase_state_sptr& state);
  
  /**
   * Interval Phaseの処理
   */
  virtual bool interval_phase(hydla::ch::module_set_sptr& ms, phase_state_sptr& state);

private:
  void init(Opts& opts);

  MathLink ml_; 
  //boost::shared_ptr<hydla::ch::ModuleSetContainer> module_set_container_;
};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_MATH_SIMULATOR_H_
