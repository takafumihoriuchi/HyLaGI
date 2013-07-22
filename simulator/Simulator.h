#ifndef _INCLUDED_HYDLA_SIMULATOR_H_
#define _INCLUDED_HYDLA_SIMULATOR_H_

#include <deque>

#include "Node.h"
#include "ParseTree.h"
#include "ModuleSetContainer.h"

#include "PhaseResult.h"
#include "DefaultParameter.h"
#include "ContinuityMapMaker.h"

namespace hydla {
namespace simulator {

typedef enum{
  NO_APPROX,
  NUMERIC_APPROX,
  INTERVAL_APPROX
}ApproximationMode;

typedef enum{
  DFS,
  BFS
}SearchMethod;

typedef struct Opts_ {
  std::string mathlink;
  bool debug_mode;
  std::string max_time;
  bool nd_mode;
  bool interactive_mode;
  bool ha_convert_mode;
  bool dump_relation;
  bool profile_mode;
  bool parallel_mode;
  int parallel_number;
  bool reuse;
  bool dump_in_progress;
  bool stop_at_failure;
  bool ignore_warnings;
  std::string output_interval;
  int output_precision;
  ApproximationMode approx_mode;
  int approx_precision;
  int approx_threshold;
  std::string solver;
  hydla::parse_tree::node_sptr assertion;
  std::set<std::string> output_variables;
  int optimization_level;
  std::string analysis_mode;
  std::string analysis_file;
  int timeout;
  int timeout_phase;
  int timeout_case;
  int timeout_calc;
  int max_loop_count;
  int max_phase;
  int max_phase_expanded;
  SearchMethod search_method;
} Opts;

typedef boost::shared_ptr<hydla::ch::ModuleSet>           module_set_sptr;
typedef hydla::ch::ModuleSetContainer                     module_set_container_t;
typedef boost::shared_ptr<module_set_container_t>  module_set_container_sptr;
typedef hydla::ch::ModuleSetContainer::module_set_list_t  module_set_list_t;
typedef boost::shared_ptr<hydla::parse_tree::ParseTree>  parse_tree_sptr;
typedef boost::shared_ptr<const hydla::ch::ModuleSet>    module_set_const_sptr;
typedef boost::shared_ptr<hydla::ch::ModuleSetContainer> module_set_container_sptr;

typedef std::map<boost::shared_ptr<hydla::parse_tree::Ask>, bool> entailed_prev_map_t;
typedef std::map<variable_t*, simulator::ValueRange, VariableComparator>      variable_range_map_t;
typedef std::vector<variable_range_map_t>      variable_range_maps_t;

typedef std::map<std::string, unsigned int> profile_t;


/**
 * シミュレーションすべきフェーズを表す構造体
 */
struct SimulationTodo{
  typedef std::map<hydla::ch::ModuleSet, variable_range_maps_t > ms_cache_t;

  Phase                     phase;
  int                       id;
  time_t                    current_time;
  parameter_map_t           parameter_map;
  positive_asks_t           positive_asks;
  negative_asks_t           negative_asks;
  expanded_always_t         expanded_always;
  entailed_prev_map_t       judged_prev_map;

  /// 前のフェーズ
  phase_result_sptr_t parent;

  /// フェーズ内で一時的に追加する制約．分岐処理などに使用
  constraints_t temporary_constraints;
  /// 使用する制約モジュール集合．（フェーズごとに，非always制約を含むか否かの差がある）
  module_set_container_sptr module_set_container;
  /// 未判定のモジュール集合を保持しておく．分岐処理時，同じ集合を複数回調べることが無いように
  /// TODO:現状，これがまともに使われていない気がする．つまり，何か間違っている可能性があるし，無駄は確実にある
  module_set_list_t ms_to_visit;
  /// プロファイリング結果
  profile_t profile;
  /// 所属するケースの計算時間
  int elapsed_time;
  /// map to cache result of calculation for each module_set
  ms_cache_t ms_cache;

  std::set<std::string> changed_variables;

  /**
   * reset members to calculate from the start of the phase
   * TODO: expanded_alwaysどうしよう．
   * TODO: 記号定数も元に戻すべき？こちらは現状ではそこまで問題ないはず
   */
  void reset_from_start_of_phase(){
    ms_cache.clear();
    temporary_constraints.clear();
    ms_to_visit = module_set_container->get_full_ms_list();
    positive_asks.clear();
    negative_asks.clear();
    judged_prev_map.clear();
  }
};


typedef boost::shared_ptr<SimulationTodo>     simulation_todo_sptr_t;
// プロファイリング結果全体
// 各Todoごとにかかった時間（現状では，Todoそのものを保存している）
typedef std::vector<simulation_todo_sptr_t> entire_profile_t;

class PhaseSimulator;

typedef PhaseResult                                       phase_result_t;
typedef PhaseSimulator                                    phase_simulator_t;

typedef boost::shared_ptr<SimulationTodo>                simulation_todo_sptr_t;


struct TodoContainer
{
  virtual void push_todo(simulation_todo_sptr_t& todo)
  {
    container_.push_back(todo);
  }
  
  virtual simulation_todo_sptr_t pop_todo()
  {
    simulation_todo_sptr_t todo = container_.back();
    container_.pop_back();
    return todo;
  }
  
  virtual bool empty()
  {
    return container_.empty();
  }
  
  virtual int size()
  {
    return container_.size();
  }
  
  protected:
  std::deque<simulation_todo_sptr_t> container_;
};

typedef TodoContainer                                    todo_container_t;

typedef std::list<variable_t>                            variable_set_t;

typedef std::list<parameter_t>                     parameter_set_t;

class Simulator
{
public:  

  Simulator(Opts& opts);
  
  virtual ~Simulator(){}

  /**
   * simulate using given parse_tree
   * @return root of the tree which expresses the result-trajectories
   */
  virtual phase_result_const_sptr_t simulate() = 0;
  
  virtual void initialize(const parse_tree_sptr& parse_tree);
  
  
  /**
   * set the phase simulator which this simulator uses in each phase
   * @param ps a pointer to an instance of phase_simlulator_t (caller must not delete the instance)
   */
  virtual void set_phase_simulator(phase_simulator_t *ps);
  
  /**
   * @return set of introduced parameters and their ranges of values
   */
  parameter_map_t get_parameter_map(){return *original_parameter_map_;}
  
  phase_result_sptr_t get_result_root(){return result_root_;}
  
  /**
   * push the initial state of simulation into the stack
   */
  virtual simulation_todo_sptr_t make_initial_todo();
  
  /**
   * @return pointer to introduced parameter
   */
  parameter_t* introduce_parameter(variable_t* var, phase_result_sptr_t& phase, ValueRange& range);
  
  // TODO: publicメンバが多すぎる気がする
  
  /**
   * template of variable_range maps
   */
  boost::shared_ptr<variable_range_map_t> original_range_map_;
  
  /*
   * set of variables
   */
  boost::shared_ptr<variable_set_t> variable_set_;
  

  /*
   * set of introduced parameters
   */
  boost::shared_ptr<parameter_set_t> parameter_set_;
  
  /*
   * map of introduced parameters and their original ranges of values
   */
  boost::shared_ptr<parameter_map_t> original_parameter_map_;
  

protected:
  
  /**
   * シミュレーション時に使用される変数表のオリジナルの作成
   */
  virtual void init_variable_map(const parse_tree_sptr& parse_tree);
  
  void init_module_set_container(const parse_tree_sptr& parse_tree);
  
  void reset_result_root();


  parse_tree_sptr parse_tree_;
  
  /**
   * PhaseSimulator to use
   */ 
  boost::shared_ptr<phase_simulator_t > phase_simulator_;

  /**
   * a container for candidate module sets
   */
  module_set_container_sptr msc_original_;
  
  /**
   * mcs_original_から非always制約を除いたもの
   */
  module_set_container_sptr msc_no_init_;

  /** 
   * root of the tree of result trajectories
   */
  phase_result_sptr_t result_root_;

  Opts*     opts_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_H_
