#include "Simulator.h"
#include "PhaseSimulator.h"
#include "SymbolicValue.h"

namespace {
  struct NodeDumper {
      
    template<typename T>
    NodeDumper(T it, T end) 
    {
      for(; it!=end; ++it) {
        ss << **it << "\n";
      }
    }

    friend std::ostream& operator<<(std::ostream& s, const NodeDumper& nd)
    {
      s << nd.ss.str();
      return s;
    }

    std::stringstream ss;
  };
}




namespace hydla {
namespace simulator {


std::ostream& operator<<(std::ostream& s, const constraints_t& a)
{
  s << NodeDumper(a.begin(), a.end());
  return s;
}

std::ostream& operator<<(std::ostream& s, const ask_set_t& a)
{
  s << NodeDumper(a.begin(), a.end());
  return s;
}

std::ostream& operator<<(std::ostream& s, const tells_t& a)
{
  s << NodeDumper(a.begin(), a.end());
  return s;
}

std::ostream& operator<<(std::ostream& s, const collected_tells_t& a)
{
  s << NodeDumper(a.begin(), a.end());
  return s;
}

std::ostream& operator<<(std::ostream& s, const expanded_always_t& a)
{
  s << NodeDumper(a.begin(), a.end());
  return s;
}




Simulator::Simulator(Opts& opts):opts_(&opts){}

/**
 * 使用するPhaseSimulatorを設定する．
 * この関数に渡すPhaseSimulatorのインスタンスはnewで作成し，呼び出し側でdeleteしないようにする
 */
void Simulator::set_phase_simulator(phase_simulator_t *ps){
  phase_simulator_.reset(ps);
}

void Simulator::initialize(const parse_tree_sptr& parse_tree)
{
  init_module_set_container(parse_tree);

  phase_id_ = 1;
  opts_->assertion = parse_tree->get_assertion_node();
  result_root_.reset(new phase_result_t());
  
  parse_tree_ = parse_tree;
  init_variable_map(parse_tree);
  continuity_map_t  cont(parse_tree->get_variable_map());
  phase_simulator_->initialize(variable_set_, parameter_set_,
   variable_map_, msc_no_init_->get_max_module_set(), cont);

  push_initial_state();
}


void Simulator::init_module_set_container(const parse_tree_sptr& parse_tree)
{    
  if(opts_->nd_mode||opts_->interactive_mode) {
    //全解探索モードなど
    ModuleSetContainerInitializer::init<ch::ModuleSetGraph>(
        parse_tree, msc_original_, msc_no_init_, parse_tree_);
  }
  else {
    //通常実行モード
    ModuleSetContainerInitializer::init<ch::ModuleSetList>(
        parse_tree, msc_original_, msc_no_init_, parse_tree_);
  }
}

void Simulator::init_variable_map(const parse_tree_sptr& parse_tree)
{
  typedef hydla::parse_tree::ParseTree::variable_map_const_iterator vmci;

  vmci it  = parse_tree->variable_map_begin();
  vmci end = parse_tree->variable_map_end();
  for(; it != end; ++it)
  {
    for(int d=0; d<=it->second; ++d) {
      variable_t v;
      v.name             = it->first;
      v.derivative_count = d;
      variable_set_.push_front(v);
      variable_map_[&(variable_set_.front())] = value_t();
    }
  }
}


void Simulator::push_initial_state()
{
  //初期状態を作ってスタックに入れる
  simulation_phase_sptr_t state(new simulation_phase_t());
  state->elapsed_time = 0;
  phase_result_sptr_t &pr = state->phase_result;
  pr.reset(new phase_result_t());
  pr->cause_of_termination = NONE;
    
  pr->phase        = simulator::PointPhase;
  pr->step         = 0;
  pr->current_time = value_t(new hydla::symbolic_simulator::SymbolicValue("0"));
  state->module_set_container = msc_original_;
  state->ms_to_visit = msc_original_->get_full_ms_list();
  pr->parent = result_root_;
  push_simulation_phase(state);
}

}
}
