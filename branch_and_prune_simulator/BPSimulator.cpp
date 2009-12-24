#include "BPSimulator.h"
#include "ConsistencyChecker.h"
#include "EntailmentChecker.h"

#include <iostream>
#include <boost/foreach.hpp>

// constraint_hierarchy
#include "ModuleSet.h"

using namespace hydla::simulator;

namespace hydla {
namespace bp_simulator {

BPSimulator::BPSimulator(const Opts& opts) :
  simulator_t(opts.debug_mode),
  opts_(opts)
{}

BPSimulator::~BPSimulator()
{}

/* ダンプ用 */
namespace {
  class NodeDump {
  public:
    template<typename T>
    void operator()(T& it) 
    {
      std::cout << *it << "\n";
    }
  };
}

void BPSimulator::do_initialize()
{}

/**
 * Point Phaseの処理
 */
bool BPSimulator::point_phase(const module_set_sptr& ms, 
                              const phase_state_const_sptr& state)
{
  if(is_debug_mode()) {
    std::cout << "#***** begin point phase *****\n"
              << "#** module set **\n"
              << ms->get_name()
              << "\n"
              << *ms 
              << std::endl;
  }

  // TODO: 無駄な宣言
  tells_t         tell_list;
  positive_asks_t positive_asks;
  negative_asks_t negative_asks;

  return this->do_point_phase(ms, state, tell_list, positive_asks, negative_asks);
}

/**
 * Point Phaseの実質的な処理
 * askのエンテールに基づき分割再帰される可能性がある．
 * 制約ストアは最後まで更新せず，有効なtell制約はtell_listに保持される．
 * 最後にストアとtell_listを合わせて新状態を生成する．
 *
 * @param ms モジュール集合
 * @param state Point Phase開始時の状態
 * @param tell_list tell制約リスト
 * @param positive_asks エンテールされたask制約リスト
 * @param negative_asks エンテールされていないask制約リスト
 *
 * @return Point Phaseを満たす解が存在するか
 */
bool BPSimulator::do_point_phase(const module_set_sptr& ms,
                    const phase_state_const_sptr& state,
                    tells_t tell_list,
                    positive_asks_t positive_asks,
                    negative_asks_t negative_asks)
{
  TellCollector tell_collector(ms);
  AskCollector  ask_collector(ms);
  //ConstraintStoreBuilderPoint csbp; //TODO: kenshiroが作成
  ConsistencyChecker consistency_checker(is_debug_mode());
  EntailmentChecker entailment_checker;   //TODO: kenshiroが作成

  // TODO: stateから制約ストアを作る

  bool expanded   = true;
  while(expanded) {
    // tell制約を集める
    tell_collector.collect_all_tells(&tell_list, &state->expanded_always, &positive_asks);
    if(is_debug_mode()) {
      std::cout << "#** collected tells **\n";  
      std::for_each(tell_list.begin(), tell_list.end(), NodeDump());
    }

    // 制約が充足しているかどうかの確認
    if(!consistency_checker.is_consistent(tell_list)){
      if(is_debug_mode()) std::cout << "#*** inconsistent\n";
      return false;
    }
    if(is_debug_mode()) std::cout << "#*** consistent\n";

    // ask制約を集める
    ask_collector.collect_ask(&state->expanded_always, 
                              &positive_asks, &negative_asks);
    if(is_debug_mode()) {
      std::cout << "#** positive asks **\n";  
      std::for_each(positive_asks.begin(), positive_asks.end(), NodeDump());

      std::cout << "#** negative asks **\n";  
      std::for_each(negative_asks.begin(), negative_asks.end(), NodeDump());
    }

    //ask制約のエンテール処理
    expanded = false;
    {
      negative_asks_t::iterator it  = negative_asks.begin();
      negative_asks_t::iterator end = negative_asks.end();
      while(it!=end) {
        // 他に値boxが必要では？
        Trivalent res = entailment_checker.check_entailment(*it, tell_list);
        switch(res) {
          case TRUE:
            expanded = true;
            positive_asks.insert(*it);
            negative_asks.erase(it++);
            break;
          case UNKNOWN:
            // TRUEの場合とFALSEの場合に分割再帰
            // bool rt = do_point_phase(ms, state, ...); // TRUE
            // bool rf = do_point_phase(ms, state, ...); // FALSE
            // 後始末？
            // return (rt | rf);
            return true;
          case FALSE:
            it++;
            break;
        }
      }
    }
  } // while(expanded)

  // ?
  //if(!csbp.build_constraint_store(&new_tells, &state->constraint_store)) {
  //  return false;
  //}

  //// IntervalPhaseへ
  //state->phase = IntervalPhase;
  //state_queue_.push(*state);

  return true;
}

/**
 * Interval Phaseの処理
 */
bool BPSimulator::interval_phase(const module_set_sptr& ms, 
                                 const phase_state_const_sptr& state)
{
  return true;
}

} // bp_simulator
} // hydla
