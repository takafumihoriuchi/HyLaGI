#include "PhaseResult.h"
#include "PhaseSimulator.h"
#include "PropertyNode.h"
#include "../symbolic_expression/Node.h"
#include "LTLNode.h"
#include "ConsistencyChecker.h"
#include "ValueModifier.h"
#include <iostream>
#include <vector>
#include <string>
using namespace std;
using namespace hydla;
using namespace simulator;
using namespace symbolic_expression;

LTLNode::LTLNode(phase_result_sptr_t set_phase,PropertyNode* set_property):Automaton("Property" + (set_property->name) + " Phase" + to_string(set_phase->id)){
  phase = set_phase;
  property = set_property;
  parent_node = NULL;
  checked_next_link = false;
}

bool LTLNode::acceptanceState(){
  return (property->type == ACCEPTANCE_STATE);
}

bool LTLNode::acceptanceCycle(){
  return (property->type == ACCEPTANCE_CYCLE);
}

// void LTLNode::addLink(LTLNode* child){
//   link.push_back(child);
//   if(!(child->parent_node != NULL)){
//     ltl_node_list_t tmp_path = path;
//     tmp_path.push_back(child);
//     child->parent_node = this;
//     child->path = tmp_path;
//   }
// }

// void LTLNode::trace(){
//   cout << " -> " << "\"" << name << "\"";
//   if(parent_node != NULL){
//     parent_node->trace();
//   }
// }

// void LTLNode::dump(){
//   if(write == 0){
//     write++;
//     cout << "\"" << name << "\"" << " ";
//     if(property->type != NOMAL){
//       if(red>0){
//         cout << "[peripheries=2 color=red];" << endl;
//       }else{
//         cout << "[peripheries=2];" << endl;
//       }
//     }else{
//       if(red>0){
//         cout << "[color=red];" << endl;
//       }else{
//         cout << ";" << endl;
//       }
//     }
//     for(ltl_node_list_t::iterator it = link.begin();it != link.end();it++){
//       if(name == (*it)->name){
//         if(red>0 && (*it)->red>0){
//           cout << "\"" << name << "\"" << " -> " << "\"" << name << "\"" << "[color=red];" << endl;
//         }else{
//           cout << "\"" << name << "\"" << " -> " << "\"" << name << "\"" << ";" << endl;
//         }
//       }else{
//         if(red>0 && (*it)->red>0){
//           cout << "\"" << name << "\"" << " -> " << "\"" << (*it)->name << "\"" << "[color=red];" << endl;
//         }else{
//           cout << "\"" << name << "\"" << " -> " << "\"" << (*it)->name << "\"" << ";" << endl;
//         }
//         (*it)->dump();
//       }
//     }
//   }
// }

// void LTLNode::dot(){
//   if(property->type != ZERO){
//     write_reset();
//     cout << "digraph g{" << endl;
//     cout << "\"init\"[shape=\"point\"];" << endl;
//     cout << "\"init\"" << " -> " << "\"" << name << "\"" << ";" << endl;
//     dump();
//     cout << "}" << endl;
//     write_reset();
//   }else{
//     ltl_node_list_t::iterator it = link.begin();
//     (*it)->dot();
//   }
// }

// void LTLNode::write_reset(){
//   if(write > 0){
//     write = 0;
//     for(ltl_node_list_t::iterator it = link.begin();it != link.end();it++){
//       (*it)->write_reset();
//     }
//   }
// }

// void LTLNode::setRed(){
//   for(ltl_node_list_t::iterator it = path.begin();it != path.end();it++){
//     (*it)->red = 1;
//   }
// }

// bool LTLNode::will_include(LTLNode* check,backend_sptr_t backend){
//   // A->will_include(B) <=> A ) B
//   bool ret;
//   //property_automaton
//   int old_property = property->name;
//   int now_property = check->property->name;
//   if(old_property != now_property){
//     cout << "different property automaton :\n\t \"" << name << "\" : \"" << check->name << "\"" << endl;
//     return false;
//   }
//   //phase typeの比較
//   if(phase->phase_type != check->phase->phase_type){
//     cout << "different phase type :\n\t \"" << name << "\" : \"" << check->name << "\"" << endl;
//     return false;
//   }
//   //phase の変数表の大きさの比較
//   if(phase->variable_map.size() != check->phase->variable_map.size()){
//     cout << "different size of variable map :\n\t \"" << name << "\" : \"" << check->name << "\"" << endl;
//     return false;
//   }

//   //compareing set of variables
//   bool include_ret;
//   backend->call("checkInclude", 6, "vlnmvtmpvlnmvtmp", "b",
//                 &(phase->current_time), &(phase->variable_map), &(phase->parameter_map),
//                 &(check->phase->current_time), &(check->phase->variable_map), &(check->phase->parameter_map), &include_ret);
//   if(include_ret){
//     cout << "\n\"" << name << "\" includes \"" << check->name << "\"\n" << endl;
//   }
//   else{
//     cout << "not included :\n\t \"" << name << "\" : \"" << check->name << "\"" << endl;
//   }

//   // //変数表の時刻を戻す
//   // //In this part, substitute var_t for t=current_time.
//   // value_t time_old, time_now;
//   // variable_map_t old_vm, now_vm;
//   // ValueModifier modifier(*backend);
//   // time_old = phase->current_time;
//   // time_now = check->phase->current_time;
//   // old_vm = modifier.substitute_time(time_old,phase->variable_map);
//   // now_vm = modifier.substitute_time(time_now,check->phase->variable_map);

//   // // comparing variable
//   // auto var_old = old_vm.begin();
//   // auto var_now = now_vm.begin();
//   // cout << "compare variables" << endl;
//   // int parametercount = 0;
//   // while(var_old != old_vm.end() && var_now != now_vm.end()){
//   //   ret = false;
//   //   //もし変数名が違う場合
//   //   if(var_old->first != var_now->first){
//   //     cout << "not include : name " << endl;
//   //     return false;
//   //   }
//   //   value_t tmp_variable_now = var_now->second.get_unique_value();
//   //   value_t tmp_variable_old = var_old->second.get_unique_value();

//   //   cout << var_old->first << "\t: " << var_old->second << "\t <=> \t" << var_now->second << endl;
//   //   // counting the number of parameters
//   //   if(search_parameter(tmp_variable_old) || search_parameter(tmp_variable_now)){
//   //     parametercount++;
//   //   }
//   //   if(parametercount > 1){
//   //     cout << "not include : parameter " << endl;
//   //     return false;
//   //   }

//   //   backend->call("checkInclude", 4, "vlnvlnmpmp", "b", &tmp_variable_old, &tmp_variable_now, &(phase->parameter_map), &(check->phase->parameter_map), &ret);
//   //   cout << "\t:" ;
//   //   if(ret) cout << "true" << endl;
//   //   else  cout << "false" << endl;
//   //   if(!ret){
//   //     cout << "not include : value " << endl;
//   //     return false;
//   //   }
//   //   var_old++;
//   //   var_now++;
//   // }
//   // cout << "\"" << name << "\" includes \"" << check->name << "\"" << endl;
//   return include_ret;
// }

// LTLNode* LTLNode::detectAcceptanceCycle(LTLNode* parent_node_node,backend_sptr_t backend){
//   LTLNode* ret = NULL;
//   for(path_list_t::iterator acceptance_path = parent_node_node->acceptance_pathes.begin();acceptance_path != parent_node_node->acceptance_pathes.end();acceptance_path++){
//     for(ltl_node_list_t::iterator it = acceptance_path->begin();it != acceptance_path->end();it++){
//       if((*it)->will_include(this,backend)){
//         ret = *it;
//       }
//     }
//   }
//   return ret;
// }

// LTLNode* LTLNode::detectLoop(LTLNode* parent_node_node,backend_sptr_t backend){
//   LTLNode* ret = NULL;
//   for(ltl_node_list_t::iterator it = parent_node_node->path.begin();it != parent_node_node->path.end();it++){
//     if((*it)->will_include(this,backend)){
//       ret = *it;
//     }
//   }
//   return ret;
// }

// bool LTLNode::search_parameter(value_t var){
//   // cout << var.get_node()->get_string();
//   if(var.get_node()->get_string().find("Parameter[",0) != string::npos){
//     // cout << " : contain" << endl;
//     return true;
//   }
//   // cout << " : not contain" << endl;
//   return false;
// }

// stack< pair<bool,int> > d
// voname search(){
//   //現在の状態の保持
//   int s = now_state;
//   int toggle = search_mode;
//   //次の状態の作成全ての遷移エッジについて考える
//   iterator it = transition[s].iterator();
//   //一つずつ見ていく
//   while(it.end){
//     //遷移エッジから新しい状態をつくる
//     int sd = ;
//     //二段階目のループか比べる(新しい状態の検査)
//     if(toggle){
//     }
//     //loopなど(新しい状態の検査)
//     if(! instatespace()) {
//       addstatespace(sd+toggle);
//       //stack利用
//       search();
//     }
//     //一段階目のループ検査(現在の状態)
//     if(infinalstate(s) !toggle()){
//       //一段階目
//       toggle = true;
//       seed = s;
//       search();
//     }
//   }
// }

// phase_result_sptr_t* simulate(phase_result_sptr_t* phase){
//   phase_result_sptr_t* ret = now phase_result_sptr_t(phase->value + 1);
//   phase->set_next(ret);
//   return ret;
// }
