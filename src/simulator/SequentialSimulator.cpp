#include "SequentialSimulator.h"
#include "Timer.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"
#include "SignalHandler.h"
#include "Logger.h"

namespace hydla {
namespace simulator {

using namespace std;

SequentialSimulator::SequentialSimulator(Opts &opts):Simulator(opts), printer()
{
}

SequentialSimulator::~SequentialSimulator()
{
}

phase_result_sptr_t SequentialSimulator::simulate()
{
  std::string error_str = "";
  make_initial_todo();

  try
  {
    //TODO: implement BFS
    dfs(result_root_);
  }
  catch(const std::runtime_error &se)
  {
    error_str += "error ";
    error_str += ": ";
    error_str += se.what();
    error_str += "\n";
    HYDLA_LOGGER_DEBUG_VAR(error_str);
    std::cout << error_str;
  }


  if(signal_handler::interrupted){
    // // TODO: 各未実行フェーズを適切に処理
    // while(!todo_stack_->empty())
    // {
    //   simulation_job_sptr_t todo(todo_stack_->pop_todo());
    //   todo->parent->simulation_state = INTERRUPTED;
    //   // TODO: restart simulation from each interrupted phase
    // }
  }
  
  HYDLA_LOGGER_DEBUG("%% simulation ended");
  return result_root_;
}

void SequentialSimulator::dfs(phase_result_sptr_t current)
{
  if(signal_handler::interrupted)
  {
    current->simulation_state = INTERRUPTED;
    return;
  }
  phase_simulator_->apply_diff(*current);
  while(!current->todo_list.empty())
  {
    phase_result_sptr_t todo = current->todo_list.front();
    current->todo_list.pop_front();
    profile_vector_->insert(todo);
    if(todo->simulation_state == NOT_SIMULATED)
    {
      process_one_todo(todo);
    }
    /* TODO: assertion違反が検出された場合の対応
       if(phase->simulation_state == ASSERTION)
       {
       HYDLA_LOGGER_DEBUG("%% Failure of assertion is detected");
       if(opts_->stop_at_failure)break;
       else continue;
       }
    */
    if(opts_->dump_in_progress){
      printer.output_one_phase(todo);
    }
    dfs(todo);
  }
  phase_simulator_->revert_diff(*current);
}


} // simulator
} // hydla
