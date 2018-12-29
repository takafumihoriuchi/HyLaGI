#include "SequentialSimulator.h"
#include "Timer.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"
#include "SignalHandler.h"
#include "Logger.h"

namespace hydla {
namespace simulator {

using namespace std;

// メソッド名の後のコロン以降は、初期化指定子リストを意味する。
// ここではSimulatora構造体にoptsを与え、変数printerにbackendを与えている。
SequentialSimulator::SequentialSimulator(Opts &opts) : Simulator(opts), printer(backend)
{
	std::cout << "=> 4.1.0.1:\t Hello, we\'ve just created a new instance of SequentialSimulator\n";
}

SequentialSimulator::~SequentialSimulator()
{
}

phase_result_sptr_t SequentialSimulator::simulate()
{
	std::cout << "=> 5:\t IN SEQUENTIAL SIMULATOR" << std::endl;

	std::string error_str = "";
	std::cout << "=> 5.1:\t calling make_initial_todo()" << std::endl;
	// シミュレーションのスタックに最初の状態を詰め込む
	make_initial_todo();

	try
	{
		std::cout << "=> 5.2:\t calling dfs() with try" << std::endl;
		dfs(result_root_);
	}
	catch(const std::exception &se)
	{
		error_str += "error ";
		error_str += ": ";
		error_str += se.what();
		error_str += "\n";
		HYDLA_LOGGER_DEBUG_VAR(error_str);
		std::cout << error_str;
		exit_status = EXIT_FAILURE;
	}

	std::cout << "=> 5.3:\t returning from simulate()" << std::endl;
	HYDLA_LOGGER_DEBUG("%% simulation ended");
	return result_root_;
}

void SequentialSimulator::dfs(phase_result_sptr_t current)
{
	auto detail = logger::Detail(__FUNCTION__);

	HYDLA_LOGGER_DEBUG_VAR(*current);
	if (signal_handler::interrupted)
	{
		current->simulation_state = INTERRUPTED;
		return;
	}

	phase_simulator_->apply_diff(*current);

	while (!current->todo_list.empty())
	{
		phase_result_sptr_t todo = current->todo_list.front();
		current->todo_list.pop_front();
		profile_vector_->insert(todo);
		if (todo->simulation_state == NOT_SIMULATED)
		{
			process_one_todo(todo);
			if (opts_->dump_in_progress){
				printer.output_one_phase(todo, "------ In Progress ------");
			}
		}

		dfs(todo);
		if (!opts_->nd_mode || (opts_->stop_at_failure && assertion_failed))
		{
			omit_following_todos(current);
			break;
		}
	}

	phase_simulator_->revert_diff(*current);
}

void SequentialSimulator::omit_following_todos(phase_result_sptr_t current)
{
	while (!current->todo_list.empty())
	{
		phase_result_sptr_t not_selected_children = current->todo_list.front();
		current->todo_list.pop_front();
		if (not_selected_children->simulation_state != SIMULATED)
		{
			current->children.push_back(not_selected_children);
		}
		not_selected_children->simulation_state = NOT_SIMULATED;
	}
}

} // namespace simulator
} // namespace hydla
