#include "IncrementalModuleSet.h"
#include "TreeInfixPrinter.h"
#include "../common/Timer.h"
#include "../common/Logger.h"

#include <iostream>
#include <algorithm>
#include <boost/lambda/lambda.hpp>

using namespace boost::lambda;

namespace hydla {
namespace hierarchy {

IncrementalModuleSet::IncrementalModuleSet()
{}

IncrementalModuleSet::IncrementalModuleSet(ModuleSet ms):
  ModuleSetContainer(ms)
{}

IncrementalModuleSet::IncrementalModuleSet(ModuleSet ms, node_sptr c):
  ModuleSetContainer(ms)
{
  for(auto m : ms)
  {
    module_conditions_[m].push_back(c);
    node_sptr tmp = m.second->clone();
    ListBoundVariableUnifier unifier;
    unifier.unify(tmp);

    module_t module = module_t(symbolic_expression::TreeInfixPrinter().get_infix_string(tmp),tmp);
    related_modules_[module].add_module(m);
    unifiers_[m] = unifier;
  }
}

IncrementalModuleSet::IncrementalModuleSet(const IncrementalModuleSet& im)
{
  stronger_modules_ = im.stronger_modules_;
  weaker_modules_ = im.weaker_modules_;
  required_ms_ = im.required_ms_;
  maximal_module_set_ = im.maximal_module_set_;
}

IncrementalModuleSet::~IncrementalModuleSet()
{}

/**
 * 対象のモジュール集合から除去可能なモジュールの集合を返す
 * @param current_ms 対象のモジュール集合 
 * @param ms 矛盾の原因となったモジュール集合
 */
std::vector<ModuleSet> IncrementalModuleSet::get_removable_module_sets(ModuleSet &current_ms, const ModuleSet &ms)
{
  HYDLA_LOGGER_DEBUG("%% candidate modules : ", ms.get_name(), "\n");
  // msは矛盾集合
  std::vector<ModuleSet> removable;

  for( auto it : ms ){
    bool has_weaker = false;
    for( auto weaker : weaker_modules_[it] ){
      if(ms.find(weaker) != ms.end()){
        has_weaker = true;
        break;
      }
    }
    if(has_weaker) continue;
    // one of the removable module set
    ModuleSet removable_for_it;
    // a vector which has all modules which is weaker than it
    std::vector<module_t> childs;
    childs.push_back(it);

    while(childs.size() > 0){
      // pop the top of childs
      auto roop_it = childs.front();
      removable_for_it.add_module(roop_it);
      // to prevent recheck, erase childs front
      childs.erase(childs.begin());
      // if there are modules which is weaker than roop_it
      if(weaker_modules_.find(roop_it) != weaker_modules_.end()){
        for( auto wmit : weaker_modules_[roop_it] ){
	  // wmit is a module which is weaker than roop_it
	  // if wmit is included by current module set
          if(current_ms.find(wmit) != current_ms.end()){
	    // push wmit to childs
            childs.push_back(wmit);
          }
        }
      }
    }
    if(same_modules_.count(it)) removable_for_it.insert(same_modules_[it]);
    removable.push_back(removable_for_it);
  }

  // string for debug
  std::string str = "";
  for(auto it : removable){
    str += it.get_name();
    str += " , ";
  }
  HYDLA_LOGGER_DEBUG("%% removable modules : ", str, "\n");
  return removable;
}

IncrementalModuleSet::module_set_set_t IncrementalModuleSet::get_full_ms_list() const{
  module_set_set_t mss;
  mss.insert(maximal_module_set_);
  return mss;
}

ModuleSet IncrementalModuleSet::unadopted_module_set(){
  ModuleSet ret = maximal_module_set_;
  ret.erase(get_weaker_module_set());
  return ret;
}


void IncrementalModuleSet::add_parallel(IncrementalModuleSet& parallel_module_set) 
{
  add_order_data(parallel_module_set);

  // 今まで出現したすべてのモジュールの集合を保持
  maximal_module_set_.insert(parallel_module_set.maximal_module_set_);
}

void IncrementalModuleSet::add_weak(IncrementalModuleSet& weak_module_set_list) 
{
  add_order_data(weak_module_set_list);

  // thisに含まれるすべてのモジュールが
  // weak_module_setに含まれるすべてのモジュールよりも強いという情報を保持

  ModuleSet strong_modules;
  ModuleSet weak_modules;
  for(auto this_module : maximal_module_set_ ){
    if(weaker_modules_.count(this_module)) continue;
    strong_modules.add_module(this_module);
  }
  for(auto weaker_module : weak_module_set_list.maximal_module_set_ ){
    if(stronger_modules_.count(weaker_module)) continue;
    weak_modules.add_module(weaker_module);
  }
  for(auto sm : strong_modules){
    for(auto wm : weak_modules){
      stronger_modules_[wm].add_module(sm);
      weaker_modules_[sm].add_module(wm);
    }
  }

  // 今まで出現したすべてのモジュールの集合を保持
  maximal_module_set_.insert(weak_module_set_list.maximal_module_set_);
}

void IncrementalModuleSet::add_order_data(IncrementalModuleSet& ims)
{
  for(auto sm : ims.stronger_modules_){
    stronger_modules_[sm.first].insert(sm.second);
  }
  for(auto wm : ims.weaker_modules_){
    weaker_modules_[wm.first].insert(wm.second);
  }
  for(auto mc : ims.module_conditions_){
    module_conditions_[mc.first].insert(module_conditions_[mc.first].end(),mc.second.begin(),mc.second.end());
  }
  for(auto rm : ims.related_modules_){
    bool merge = false;
    for(auto m : related_modules_){
      if(m.first.second->is_same_struct(*rm.first.second, true)){
        related_modules_[m.first].insert(rm.second);
        merge = true;
        break;
      }
    }
    if(!merge){
      related_modules_[rm.first].insert(rm.second);
    }
  }
  for(auto u : ims.unifiers_)
  {
    unifiers_[u.first] = u.second;
  }
}

std::ostream& IncrementalModuleSet::dump_module_sets_for_graphviz(std::ostream& s)
{
  reset();
  s << "digraph module_set { " << std::endl;
  s << "  edge [dir=back];" << std::endl;
  module_set_set_t mss;
  while(has_next()){
    ModuleSet tmp = get_weaker_module_set();
    generate_new_ms(mss, tmp);
    for(auto ms : ms_to_visit_){
      if(tmp.including(ms)){
        ModuleSet parent = tmp;
        ModuleSet child = ms;
        parent.insert(required_ms_);
        child.insert(required_ms_);
        s << "  \"" << parent.get_name() << "\" -> \"" << child.get_name() << "\"" << std::endl;
      }
    }
  }
  s << "}" << std::endl;
  reset();
  return s;
}

std::ostream& IncrementalModuleSet::dump_priority_data_for_graphviz(std::ostream& s) const
{
  s << "digraph priority_data { " << std::endl;
  s << "  edge [dir=back];" << std::endl;
  for(auto m : required_ms_){
    s << "  \"" << m.first << "\" [shape=box];" << std::endl;
  }
  for(auto m : weaker_modules_){
    for(auto wm : m.second){
      s << "  \"" << m.first.first << "\" -> \"" << wm.first << "\";" << std::endl;
    }
  }
  for(auto m : same_modules_){
    for(auto sm : m.second){
      s << "  \"" << m.first.first << "\" -> \"" << sm.first << "\" [style=dotted];" << std::endl;
      s << "  \"" << sm.first << "\" -> \"" << m.first.first << "\" [style=dotted];" << std::endl;
    }
  }
  s << "}" << std::endl;
  return s;
}
std::ostream& IncrementalModuleSet::dump(std::ostream& s) const
{
  s << "========== dump Incremental Module Set ==========" << std::endl;
  s << "********** required modules **********" << std::endl;
  for(auto m : required_ms_)
  {
    s << m.first << std::endl;
  }
  s << std::endl;
  s << "********** module priorities **********" << std::endl;
  for(auto m : weaker_modules_)
  {
    for(auto wm : m.second)
    {
      s << wm.first << " << " << m.first.first << "," << std::endl;
    }
  }
  for(auto m : same_modules_)
  {
    for(auto sm : m.second)
    {
      s << m.first.first << " << " << sm.first << "," << std::endl;
      s << sm.first << " << " << m.first.first << "," << std::endl;
    }
  }
  s << std::endl;
  if(!module_conditions_.empty())
  {
    s << "********** module conditions **********" << std::endl;
    for(auto m : module_conditions_)
    {
      s << m.first.first << " : ";
      for(auto c : m.second)
      {
        s << symbolic_expression::TreeInfixPrinter().get_infix_string(c) << ", ";
      }
      s << std::endl;
    }
    s << std::endl;
  }
  if(!related_modules_.empty())
  {
    s << "********** related modules **********" << std::endl;
    for(auto m : related_modules_)
    {
      s << m.first.first << "    : ";
      for(auto c : m.second)
      {
        s << c.first << ", ";
      }
      s << std::endl;
    }
    s << std::endl;
  }
  if(!unifiers_.empty())
  {
    for(auto u : unifiers_)
    {
      s << u.first.first << " : " << std::endl;
      u.second.dump(s);
      s << std::endl;
    }
  }
  return s;
}

std::ostream& IncrementalModuleSet::dump_node_names(std::ostream& s) const
{
  return s;
}

std::ostream& IncrementalModuleSet::dump_node_trees(std::ostream& s) const
{
  return s;
}

/**
 * この mark_nodes は調べているモジュール集合が
 * 無矛盾だった場合に呼ばれ、
 * そのモジュール集合が包含しているモジュール集合を
 * 探索対象から外す
 */

bool IncrementalModuleSet::check_same_ms_generated(module_set_set_t &new_mss, ModuleSet &ms)
{
  return new_mss.count(ms);
}

void IncrementalModuleSet::update_by_new_mss(module_set_set_t &new_mss)
{
  ms_to_visit_.clear();
  for(auto new_ms : new_mss){
    ms_to_visit_.insert(new_ms);
  }
}

ModuleSet IncrementalModuleSet::get_circular_ms(ModuleSet root, module_t &origin, module_t &module)
{
  ModuleSet ret;
  if(origin == module){
    ret.add_module(module);
    return ret;
  }
  if(root.find(module) != root.end()){
    return ret;
  }
  root.add_module(module);
  ModuleSet ms;
  if(weaker_modules_.count(module)){
    for(auto weak : weaker_modules_[module]){
      ms = get_circular_ms(root,origin,weak);
      if(!ms.empty()){
        ret.insert(ms);
      }
    }
  }
  if(!ret.empty()) ret.add_module(module);
  return ret;
}

void IncrementalModuleSet::init()
{
  full_module_set_set_.clear();
  full_module_set_set_.insert(maximal_module_set_);

  for(auto m : maximal_module_set_){
    if(!stronger_modules_.count(m)){ 
      required_ms_.add_module(m);
    }
  }
  maximal_module_set_.erase(required_ms_);

  for(auto m : maximal_module_set_){
    ModuleSet ms;
    ms.add_module(m);
    for(auto weak : weaker_modules_[m]){
      ModuleSet circular = get_circular_ms(ms, m, weak);
      if(!circular.empty()){
        same_modules_[m].insert(circular);
      }
    }
  }
  for(auto m : maximal_module_set_){
    if(same_modules_.count(m)){
      stronger_modules_[m].erase(same_modules_[m]);
      weaker_modules_[m].erase(same_modules_[m]);
      same_modules_[m].erase(m);
    }
  }

  HYDLA_LOGGER_DEBUG("%% required modules : ", required_ms_.get_name());
  dump(std::cout);
}

void IncrementalModuleSet::remove_included_ms_by_current_ms(){
  /// current は現在のモジュール集合
  ModuleSet current = get_weaker_module_set();
  module_set_set_t::iterator lit = ms_to_visit_.begin();
  while(lit!=ms_to_visit_.end()){
    /**
     * ms_to_visit_内のモジュール集合で
     * currentが包含するモジュール集合を削除
     */
    if(current.including(*lit)){
      lit = ms_to_visit_.erase(lit);
    }
    else lit++;
  }
} 

/**
 * この mark_nodes は調べているモジュール集合が
 * 矛盾した場合に呼ばれ、新たなモジュール集合を生成する
 * @param mms そのフェーズにおおける極大無矛盾集合
 * @param ms 矛盾の原因となったモジュール集合 
 */
void IncrementalModuleSet::generate_new_ms(const module_set_set_t& mcss, const ModuleSet& ms){
  HYDLA_LOGGER_DEBUG("%% inconsistent module set : ", ms.get_name());
  std::string for_debug = "";
  for( auto mit : mcss ){
    for_debug += mit.get_name() + " , ";
  }
  HYDLA_LOGGER_DEBUG("%% maximal consistent module set : ", for_debug);
  // 制約の優先度を崩さない範囲で除去可能なモジュールを要素として持つ集合を得る
  // 結果用のモジュール集合の集合
  module_set_set_t new_mss;
  ModuleSet inconsistent_ms = ms;
  inconsistent_ms.erase(required_ms_);
  for( auto lit : ms_to_visit_ ){
    // 探索対象のモジュール集合がmsを含んでいる場合
    // そのモジュール集合は矛盾するため
    // 新たなモジュール集合を生成する
    if(lit.including(inconsistent_ms)){
      // get vector of removable module set
      std::vector<ModuleSet> rm = get_removable_module_sets(lit,inconsistent_ms);
      for(auto removable_module_set : rm){
        // remove removable module set of lit.
        // make new module set
        // The set has no module which is in removable module set
        ModuleSet new_ms = lit;
        new_ms.erase(removable_module_set);
        // check weather the new_ms is included by maximal consistent module sets.
        bool checked = false;
        for( auto mcs : mcss ){
         // 生成されたモジュール集合が極大無矛盾集合に包含されている場合checkedをtrueにしておく
          if(mcs.disjoint(new_ms)){
            checked = true;
            break;
          }
        }
       // checkedがtrueでない場合、生成したモジュール集合を探索対象に追加
        if(!checked){
          new_mss.insert(new_ms);
          HYDLA_LOGGER_DEBUG("%% new ms : ", new_ms.get_name());
        }
      }	
    }else{
      // 探索対象がmsを含んでいない場合そのまま残しておく
      if(!check_same_ms_generated(new_mss, lit)){
        new_mss.insert(lit);
      }
    }
  }
  // update ms_to_visit_ by generated module sets
  update_by_new_mss(new_mss);
}

/// 最も要素数の多いモジュール集合を返す
  ModuleSet IncrementalModuleSet::get_max_module_set() const{
    ModuleSet ret = maximal_module_set_;
    ret.insert(required_ms_);
    return ret;
  }

// 探索対象を引数のmssが示す状態にする
  void IncrementalModuleSet::reset(const module_set_set_t &mss){
    ms_to_visit_ = mss;
  }

// 探索対象を初期状態に戻す
  void IncrementalModuleSet::reset(){
    ms_to_visit_ = get_full_ms_list();
  }

} // namespace hierarchy
} // namespace hydla
