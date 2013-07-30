#include "REDUCEVCS.h"

#include "../../parser/SExpParser.h"
#include "../../simulator/Dumpers.h"
#include "../SolveError.h"
#include "Logger.h"
#include "REDUCELink.h"
#include "REDUCELinkFactory.h"
#include "REDUCEStringSender.h"
#include "SExpConverter.h"
#include "VariableNameEncoder.h"
#include <cassert>
#include <boost/shared_ptr.hpp>


using namespace hydla::parse_tree;
using namespace hydla::parser;
using namespace hydla::vcs;

namespace hydla {
namespace vcs {
namespace reduce {

void REDUCEVCS::change_mode(hydla::simulator::symbolic::Mode m, int approx_precision)
{
  mode_ = m;
}


REDUCEVCS::REDUCEVCS(const hydla::simulator::Opts &opts, variable_range_map_t &m)
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  REDUCELinkFactory rlf;
  reduce_link_ = reduce_link_t(rlf.createInstance());

  // デバッグプリントの設定
  std::stringstream debug_print_opt_str;
  debug_print_opt_str << "optUseDebugPrint_:=";
  debug_print_opt_str << (opts.debug_mode ? "t" : "nil");
  debug_print_opt_str << ";";
  reduce_link_->send_string((debug_print_opt_str.str()).c_str());

  HYDLA_LOGGER_VCS("--- send depend statements of variables ---");

  std::ostringstream depend_str;
  depend_str << "depend {";
  variable_range_map_t::const_iterator it = m.begin();
  variable_range_map_t::const_iterator end = m.end();
  bool first_element = true;
  for(; it!=end; ++it)
  {
    // 回避 const REDUCEVariable& variable = it->first;
    // 微分回数が0のものだけdepend文を作成
    if(it->first->derivative_count == 0){
      VariableNameEncoder vne;
      if(!first_element) depend_str << ",";
      depend_str << REDUCEStringSender::var_prefix << vne.LowerEncode(it->first->name);
      first_element = false;
    }
  }
  depend_str << "},t;";

  HYDLA_LOGGER_VCS("depend_str: ", depend_str.str()); 
  reduce_link_->send_string((depend_str.str()).c_str());


  // REDUCEの関数定義を送信
  reduce_link_->send_string(vcs_reduce_source());
  reduce_link_->skip_until_redeval();

  SExpConverter::initialize();

  HYDLA_LOGGER_FUNC_END(VCS);
}

REDUCEVCS::~REDUCEVCS(){}

bool REDUCEVCS::reset(const variable_map_t& variable_map, const parameter_map_t& parameter_map)
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);

  reduce_link_->send_string("symbolic redeval '(resetConstraintStore);");
  reduce_link_->skip_until_redeval();

  REDUCEStringSender rss(reduce_link_);
  {
    constraints_t constraints;

    HYDLA_LOGGER_VCS("------Parameter map------\n", parameter_map);
    parameter_map_t::const_iterator it = parameter_map.begin();
    for(; it!=parameter_map.end(); ++it){
      if(it->second.is_unique()){
        const value_t &value = it->second.get_lower_bound().value;
        constraints.push_back(SExpConverter::make_equal(*it->first, get_symbolic_value_t(value).get_node(), true));
      }else{
        for(uint i=0; i < it->second.get_lower_cnt();i++){
          const value_range_t::bound_t &bnd = it->second.get_lower_bound(i);
          const value_t &value = bnd.value;
          parameter_t& param = *it->first;
          if(!bnd.include_bound){
            const symbolic_value_t lower_bound = get_symbolic_value_t(value);
            constraints.push_back(node_sptr(new GreaterEqual(node_sptr(
                      new Parameter(param.get_name(), param.get_derivative_count(), param.get_phase_id())), lower_bound.get_node())));
          }else{
            const symbolic_value_t lower_bound = get_symbolic_value_t(value);
            constraints.push_back(node_sptr(new Greater(node_sptr(
                      new Parameter(param.get_name(), param.get_derivative_count(), param.get_phase_id())), lower_bound.get_node())));
          }
        }
        for(uint i=0; i < it->second.get_lower_cnt();i++){
          const value_range_t::bound_t &bnd = it->second.get_lower_bound(i);
          const value_t &value = bnd.value;
          parameter_t& param = *it->first;
          if(!bnd.include_bound){
            const symbolic_value_t upper_bound = get_symbolic_value_t(value);
            constraints.push_back(node_sptr(new GreaterEqual(node_sptr(
                      new Parameter(param.get_name(), param.get_derivative_count(), param.get_phase_id())), upper_bound.get_node())));
          }else{
            const symbolic_value_t upper_bound = get_symbolic_value_t(value);
            constraints.push_back(node_sptr(new Greater(node_sptr(
                      new Parameter(param.get_name(), param.get_derivative_count(), param.get_phase_id())), upper_bound.get_node())));
          }
        }
      }
    }

    HYDLA_LOGGER_VCS("size:", constraints.size());
    reduce_link_->send_string("pcons_:=");
    rss.put_nodes(constraints);
    reduce_link_->send_string("$");
  }
  reduce_link_->send_string("pars_:=");
  rss.put_pars();
  reduce_link_->send_string("$");

  reduce_link_->send_string("symbolic redeval '(addParameterConstraint pcons_ pars_);");
  reduce_link_->skip_until_redeval();
  
  {
    constraints_t constraints;

    HYDLA_LOGGER_VCS("--- Variable map ---\n", variable_map);
    variable_map_t::const_iterator it  = variable_map.begin();
    variable_map_t::const_iterator end = variable_map.end();
    for(; it!=end; ++it) {
      if(it->second.get() && !it->second->undefined()) {
        constraints.push_back(SExpConverter::make_equal(*it->first, get_symbolic_value_t(it->second).get_node(), true));
      }
    }

    HYDLA_LOGGER_VCS("size:", constraints.size());
    reduce_link_->send_string("cons_:=");
    rss.put_nodes(constraints);
    reduce_link_->send_string("$");
  }
  reduce_link_->send_string("vars_:=");
  // is_unique()だったParameterについても送信する
  rss.put_vars();
  reduce_link_->send_string("$");

  reduce_link_->send_string("symbolic redeval '(addPrevConstraint cons_ vars_);");
  reduce_link_->skip_until_redeval();

  HYDLA_LOGGER_FUNC_END(VCS);
  return true;
}

/**
 * PP時: {prev変数 = 変数名}
 * IP時: {ラプラス変換初期値 = prev変数}, addInitConstraint
 * put_nodeする際ignore_prevをmode_で設定しない唯一のケース(要確認)
 */
void REDUCEVCS::set_continuity(const std::string &name, const int& dc){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  if(mode_==hydla::simulator::symbolic::ConditionsMode){ assert(0); }

  REDUCEStringSender rss(reduce_link_);

  const node_sptr prev_node(make_variable(name, dc, true));
  const node_sptr var_node(make_variable(name, dc));

  HYDLA_LOGGER_VCS("--- send co_ ---");
  reduce_link_->send_string("co_:={");
  if(mode_==hydla::simulator::symbolic::DiscreteMode){
    rss.put_node(var_node, true);
    reduce_link_->send_string("=");
    rss.put_node(prev_node, false);
  }else{ // case hydla::simulator::symbolic::ContinuousMode:
    rss.put_node(var_node, true, true);
    reduce_link_->send_string("=");
    rss.put_node(prev_node, false);
  }

  reduce_link_->send_string("}$");

  HYDLA_LOGGER_VCS("--- send va_ ---");
  reduce_link_->send_string("va_:={");
  rss.put_node(prev_node, false);
  reduce_link_->send_string(",");
  rss.put_node(var_node, true);
  reduce_link_->send_string("}$");

  reduce_link_->send_string("symbolic redeval '(addInitConstraint co_ va_);");

  reduce_link_->skip_until_redeval();

  HYDLA_LOGGER_VCS("--- add_init_variable ---");
  reduce_link_->send_string("vars_:={");
  rss.put_node(var_node, true, true);
  reduce_link_->send_string("}$");

  reduce_link_->send_string("symbolic redeval '(addInitVariables vars_);");
  reduce_link_->skip_until_redeval();

  HYDLA_LOGGER_FUNC_END(VCS);
  return;
}

void REDUCEVCS::add_constraint(const constraints_t& constraints)
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);

  if(mode_==hydla::simulator::symbolic::ConditionsMode){ assert(0); }

  REDUCEStringSender rss(reduce_link_);
  const bool ignore_prev = (mode_==hydla::simulator::symbolic::ContinuousMode || mode_==hydla::simulator::symbolic::ConditionsMode);

  // cons_を渡す
  HYDLA_LOGGER_VCS("--- send cons_ ---");
  reduce_link_->send_string("cons_:={");
  constraints_t::const_iterator it = constraints.begin();
  constraints_t::const_iterator end = constraints.end();
  for(; it!=end; ++it){
    if(it!=constraints.begin()) reduce_link_->send_string(",");
    rss.put_node(*it, ignore_prev);
  }
  reduce_link_->send_string("}$");

  // vars_を渡す
  HYDLA_LOGGER_VCS("--- send vars_ ---");
  reduce_link_->send_string("vars_:=");
  rss.put_vars();
  reduce_link_->send_string("$");

  
  reduce_link_->send_string("symbolic redeval '(addConstraint cons_ vars_);");

  reduce_link_->skip_until_redeval();

  HYDLA_LOGGER_FUNC_END(VCS);
  return;
}

void REDUCEVCS::add_constraint(const node_sptr& constraint){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  if(mode_==hydla::simulator::symbolic::ConditionsMode){ assert(0); }

  REDUCEStringSender rss(reduce_link_);
  const bool ignore_prev = (mode_==hydla::simulator::symbolic::ContinuousMode || mode_==hydla::simulator::symbolic::ConditionsMode);

  // cons_を渡す
  HYDLA_LOGGER_VCS("--- send cons_ ---");
  reduce_link_->send_string("cons_:={");
      rss.put_node(constraint, ignore_prev);
  reduce_link_->send_string("}$");

  // vars_を渡す
  HYDLA_LOGGER_VCS("--- send vars_ ---");
  reduce_link_->send_string("vars_:=");
  rss.put_vars();
  reduce_link_->send_string("$");

  
  reduce_link_->send_string("symbolic redeval '(addConstraint cons_ vars_);");

  reduce_link_->skip_until_redeval();

  HYDLA_LOGGER_FUNC_END(VCS);
  return;
}

CheckConsistencyResult REDUCEVCS::check_consistency(){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  if(mode_==hydla::simulator::symbolic::ConditionsMode){ assert(0); }

  switch(mode_){ 
    case hydla::simulator::symbolic::DiscreteMode:
      // TODO myCheckConsistencyPointが本来、真偽が数式に依存する場合Trueが戻ってくるバグを持つ
      reduce_link_->send_string("expr_:={}$ lcont_:={}$ vars:={}$");
      reduce_link_->send_string("symbolic redeval '(myCheckConsistencyPoint);");
      break;
    case hydla::simulator::symbolic::ConditionsMode:
      // TODO 未実装
      assert(0);
      reduce_link_->send_string("symbolic redeval '(checkFalseConditions);");
      break;
    default: // case hydla::simulator::symbolic::ContinuousMode:
      // TODO 記号定数による場合分けへの対応
      reduce_link_->send_string("symbolic redeval '(myCheckConsistencyInterval);");
      break;
  }

  HYDLA_LOGGER_VCS( "--- REDUCEVCS::check_consistency receive ---");
  reduce_link_->skip_until_redeval();
  const SExpParser sp(reduce_link_->get_as_s_exp_parser());

  // {true, false} または {false, true} の構造
  // TODO 記号定数を戻り値に取る場合の対応
  SExpParser::const_tree_iter_t tree_root_ptr = sp.get_tree_iterator();

  // 第一要素を取得
  SExpParser::const_tree_iter_t ret_code_ptr = tree_root_ptr->children.begin();
  std::string ret_first_str = std::string(ret_code_ptr->value.begin(), ret_code_ptr->value.end());

  CheckConsistencyResult ret;
  if(ret_first_str.find("true")!=std::string::npos){
    ret.true_parameter_maps.push_back(parameter_map_t());
  }else if(ret_first_str.find("false")!=std::string::npos){
    ret.false_parameter_maps.push_back(parameter_map_t());
  }else{
    // TODO: 上記以外の構造への対応
    assert(0);
  }

  HYDLA_LOGGER_FUNC_END(VCS);
  return ret;
}

// TODO: 不等式及び記号定数への対応
SymbolicVirtualConstraintSolver::create_result_t REDUCEVCS::create_maps(){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  if(mode_==hydla::simulator::symbolic::ConditionsMode){ assert(0); }

  REDUCEStringSender rss(reduce_link_);
  /////////////////// 送信処理

  if(mode_==hydla::simulator::symbolic::DiscreteMode){
    reduce_link_->send_string("symbolic redeval '(myConvertCSToVM);");
  }else{
    reduce_link_->send_string("symbolic redeval '(convertCSToVMInterval);");
  }


  /////////////////// 受信処理                     

  reduce_link_->skip_until_redeval();
  const SExpParser sp(reduce_link_->get_as_s_exp_parser());

  // {true, false} または {false, true} の構造
  SExpParser::const_tree_iter_t tree_root_ptr = sp.get_tree_iterator();

  create_result_t create_result;
  // TODO:以下のコードはor_size==1が前提
  //  for(int or_it = 0; or_it < or_size; or_it++){}
  {
    // {{(変数名), (関係演算子コード), (値のフル文字列)}, ...}の形式
    variable_range_map_t map;
    // TODO 不要？
    SExpConverter::clear_parameter_map();

    for(SExpParser::const_tree_iter_t it = tree_root_ptr->children.begin(); it!= tree_root_ptr->children.end(); it++){
      std::string and_cons_string =  sp.get_string_from_tree(it);
      HYDLA_LOGGER_VCS("and_cons_string: ", and_cons_string);

      // 関係演算子のコード
      int relop_code;
      {
        SExpParser::const_tree_iter_t relop_code_ptr = it->children.begin()+1;      
        std::string relop_code_str = std::string(relop_code_ptr->value.begin(),relop_code_ptr->value.end());
        std::stringstream relop_code_ss;
        relop_code_ss << relop_code_str;
        relop_code_ss >> relop_code;
        assert(relop_code>=0 && relop_code<=4);
      }

      // 値
      SExpParser::const_tree_iter_t value_ptr = it->children.begin()+2;

      // 変数名
      std::string var_name;
      // 微分回数
      int var_derivative_count;
      {
        SExpParser::const_tree_iter_t var_ptr = it->children.begin();
        std::string var_head_str = std::string(var_ptr->value.begin(),var_ptr->value.end());

        // prevの先頭にスペースが入ることがあるので除去する
        // TODO:S式パーサを修正してスペース入らないようにする
        if(var_head_str.at(0) == ' ') var_head_str.erase(0,1);

        // prev変数は処理しない
        if(var_head_str=="prev") continue;

        var_derivative_count = sp.get_derivative_count(var_ptr);

        // 微分を含む変数
        if(var_derivative_count > 0){
          var_name = std::string(var_ptr->children.begin()->value.begin(), 
              var_ptr->children.begin()->value.end());
        }
        // 微分を含まない変数
        else{
          assert(var_derivative_count == 0);
          var_name = var_head_str;
        }

        // 変数名の先頭にスペースが入ることがあるので除去する
        // TODO:S式パーサを修正してスペース入らないようにする
        if(var_name.at(0) == ' ') var_name.erase(0,1);

        // 既存の記号定数の場合
        // TODO 要動作確認
        if(var_name.find(REDUCEStringSender::var_prefix, 0) != 0){
          // 'p'は取っておく必要がある
          assert(var_name.at(0) == REDUCEStringSender::par_prefix.at(0));
          var_name.erase(0, 1);

          variable_t* variable_ptr = get_variable(var_name, var_derivative_count);

          value_range_t tmp_range = map[variable_ptr];
          value_t tmp_value = SExpConverter::convert_s_exp_to_symbolic_value(sp, value_ptr);
          SExpConverter::set_range(tmp_value, tmp_range, relop_code);

          map[variable_ptr] = tmp_range;

          continue;
        }

        // "usrVar"を取り除く
        assert(var_name.find(REDUCEStringSender::var_prefix, 0) == 0);
        var_name.erase(0, REDUCEStringSender::var_prefix.length());

        // 大文字小文字表記に変換
        VariableNameEncoder vne;
        var_name = vne.UpperDecode(var_name);
      }

      // TODO: ↓の一行消す
      if(var_name == "t") continue;
      variable_t* variable_ptr = get_variable(var_name, var_derivative_count);

      // 変数表の対応する部分に代入する
      value_t symbolic_value = SExpConverter::convert_s_exp_to_symbolic_value(sp, value_ptr);
      if(symbolic_value->undefined()){
        throw SolveError("invalid value");
      }

      value_range_t tmp_range = map[variable_ptr];
      SExpConverter::set_range(symbolic_value, tmp_range, relop_code);
      map[variable_ptr] = tmp_range;
    }
    create_result.result_maps.push_back(map);
  }
  // TODO ?
  // SExpConverter::clear_parameter_map();

  for(unsigned int i=0; i < create_result.result_maps.size();i++){
    HYDLA_LOGGER_VCS("--- result map ", i, "/", create_result.result_maps.size(), "---\n");
    HYDLA_LOGGER_VCS(create_result.result_maps[i]);
  }

  HYDLA_LOGGER_FUNC_END(VCS);

  //TODO 戻り値を設定
  return create_result;

}

// TODO: 現状，制約に関しては変数自体を送り，変数リストは全部送るという，CalculateNextPPTime専用の仕様になってしまっているので，どうにかする．
void REDUCEVCS::reset_constraint(const variable_map_t& vm, const bool& send_derivatives){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);

  HYDLA_LOGGER_VCS("------Variable map------\n", vm);

  reduce_link_->send_string("symbolic redeval '(resetConstraintForVariable);");
  reduce_link_->skip_until_redeval();

  REDUCEStringSender rss(reduce_link_);
  // ignore_prevは不要では？
  const bool ignore_prev = (mode_==hydla::simulator::symbolic::ContinuousMode || mode_==hydla::simulator::symbolic::ConditionsMode);

  // cons_を渡す
  HYDLA_LOGGER_VCS("--- send cons_ ---");

  reduce_link_->send_string("cons_:={");
  variable_map_t::const_iterator it;
  bool isFirst = true;
  for(it=vm.begin(); it!=vm.end(); ++it){
    if((send_derivatives || it->first->get_derivative_count() == 0) && !it->second->undefined()){
      if(!isFirst) reduce_link_->send_string(",");
      isFirst = false;

      const node_sptr lhs_node(make_variable(it->first->get_name(), it->first->get_derivative_count()));
      rss.put_node(node_sptr(new Equal(lhs_node, get_symbolic_value_t(it->second).get_node())), ignore_prev);
    }
  }
  reduce_link_->send_string("}$");

  // vars_を渡す
  HYDLA_LOGGER_VCS("--- send vars_ ---");
  reduce_link_->send_string("vars_:={");
  for(it=vm.begin(); it!=vm.end(); ++it){
    if(it!=vm.begin()) reduce_link_->send_string(",");
    const node_sptr node(make_variable(it->first->get_name(), it->first->get_derivative_count()));
    rss.put_node(node, ignore_prev);
  }
  reduce_link_->send_string("}$");
  
  reduce_link_->send_string("symbolic redeval '(addConstraint cons_ vars_);");

  reduce_link_->skip_until_redeval();

  HYDLA_LOGGER_FUNC_END(VCS);
  return;
}


void REDUCEVCS::start_temporary(){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  reduce_link_->send_string("symbolic redeval '(startTemporary);");
  reduce_link_->skip_until_redeval();

  HYDLA_LOGGER_FUNC_END(VCS);
  return;
}

void REDUCEVCS::end_temporary(){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  reduce_link_->send_string("symbolic redeval '(endTemporary);");
  reduce_link_->skip_until_redeval();

  HYDLA_LOGGER_FUNC_END(VCS);

  return;
}

// TODO: add_guardなのかset_guardなのかとか，仕様とかをはっきりさせる
void REDUCEVCS::add_guard(const node_sptr& constraint){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);

  REDUCEStringSender rss(reduce_link_);
  const bool ignore_prev = (mode_==hydla::simulator::symbolic::ContinuousMode || mode_==hydla::simulator::symbolic::ConditionsMode);

  // cons_を渡す
  HYDLA_LOGGER_VCS("--- send cons_ ---");
  reduce_link_->send_string("cons_:={");
      rss.put_node(constraint, ignore_prev);
  reduce_link_->send_string("}$");

  // vars_を渡す
  HYDLA_LOGGER_VCS("--- send vars_ ---");
  reduce_link_->send_string("vars_:=");
  rss.put_vars();
  reduce_link_->send_string("$");

  switch(mode_){
    case hydla::simulator::symbolic::DiscreteMode:
      reduce_link_->send_string("symbolic redeval '(addConstraint cons_ vars_);");
      break;
    case hydla::simulator::symbolic::ConditionsMode:
      reduce_link_->send_string("symbolic redeval '(addGuard cons_ vars_);");
      break;
    default: // case hydla::simulator::symbolic::ContinuousMode:
      reduce_link_->send_string("symbolic redeval '(setGuard cons_ vars_);");
      break;
  }

  reduce_link_->skip_until_redeval();

  HYDLA_LOGGER_FUNC_END(VCS);
  return;
}


SymbolicVirtualConstraintSolver::PP_time_result_t REDUCEVCS::calculate_next_PP_time(
    const constraints_t& discrete_cause,
    const time_t& current_time,
    const time_t& max_time){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);

  ////////////////// 送信処理
  
  REDUCEStringSender rss(reduce_link_);
  reduce_link_->send_string("maxTime_:=");
  time_t tmp_time(max_time->clone());
  *tmp_time -= *current_time;
  HYDLA_LOGGER_VCS("%% current time:", *current_time);
  HYDLA_LOGGER_VCS("%% send time:", *tmp_time);
  rss.put_node(get_symbolic_value_t(tmp_time).get_node(), true);
  reduce_link_->send_string("$");

  reduce_link_->send_string("discCause_:={");
  for(constraints_t::const_iterator it = discrete_cause.begin(); it != discrete_cause.end();it++){
    if(it!=discrete_cause.begin()) reduce_link_->send_string(",");
    rss.put_node(*it, true);
  }
  reduce_link_->send_string("}$");

  reduce_link_->send_string("symbolic redeval '(calculateNextPointPhaseTime maxTime_ discCause_);");

  ////////////////// 受信処理

  reduce_link_->skip_until_redeval();
  const SExpParser sp(reduce_link_->get_as_s_exp_parser());
  
  // {{value_t(time_t), {}(parameter_map_t), true(bool)},...} のようなものが戻るはず
  SExpParser::const_tree_iter_t tree_root_ptr = sp.get_tree_iterator();
  PP_time_result_t result;

  for(SExpParser::const_tree_iter_t it = tree_root_ptr->children.begin(); it!= tree_root_ptr->children.end(); it++){
    PP_time_result_t::candidate_t candidate;

    // 時刻を受け取る
    candidate.time = SExpConverter::convert_s_exp_to_symbolic_value(sp, it->children.begin());
    *candidate.time += *current_time;
    HYDLA_LOGGER_VCS("next_phase_time: ", candidate.time);

    // 条件を受け取る
    SExpParser::const_tree_iter_t param_ptr = it->children.begin()+1;
    std::string param_str = std::string(param_ptr->value.begin(), param_ptr->value.end());

    // TODO 空リスト以外の場合に対応
    if(!(param_ptr->children.size()==1 && param_str.find("list")!=std::string::npos)){
      assert(0);
    }
    candidate.parameter_map = parameter_map_t();

    // 終了時刻かどうかを受け取る
    SExpParser::const_tree_iter_t bool_ptr = it->children.begin()+2;
    std::string bool_str = std::string(bool_ptr->value.begin(), bool_ptr->value.end());
    candidate.is_max_time = (bool_str.find("1")!=std::string::npos);

    HYDLA_LOGGER_VCS("is_max_time: ",  candidate.is_max_time);
    HYDLA_LOGGER_VCS("--- parameter map ---\n",  candidate.parameter_map);
    result.candidates.push_back(candidate);
  }

  HYDLA_LOGGER_FUNC_END(VCS);

  return result;
}

void REDUCEVCS::apply_time_to_vm(const variable_map_t& in_vm, 
                                              variable_map_t& out_vm, 
                                              const time_t& time)
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);

  REDUCEStringSender rss(reduce_link_);

  variable_map_t::const_iterator it  = in_vm.begin();
  variable_map_t::const_iterator end = in_vm.end();
  for(; it!=end; ++it) {
    HYDLA_LOGGER_VCS("variable : ", *(it->first));
    // 値
    value_t value;
    if(it->second->undefined()) {
      out_vm[it->first] = value;
      continue;
    }

    // applyTime2Expr(expr_, time_)を渡したい

    reduce_link_->send_string("expr_:=");
    rss.put_node(get_symbolic_value_t(it->second).get_node(), true);
    reduce_link_->send_string("$");


    reduce_link_->send_string("time_:=");
    rss.put_node(get_symbolic_value_t(time).get_node(), true);
    //rss.put_node(get_symbolic_value_t(time).get_node(), true);
    //send_time(time);
    reduce_link_->send_string("$");


    reduce_link_->send_string("symbolic redeval '(applyTime2Expr expr_ time_);");

    
    ////////////////// 受信処理

    reduce_link_->skip_until_redeval();
    const SExpParser sp(reduce_link_->get_as_s_exp_parser());

    // {コード, 値}の構造
    SExpParser::const_tree_iter_t ct_it = sp.get_tree_iterator();

    // コードを取得
    SExpParser::const_tree_iter_t ret_code_it = ct_it->children.begin();
    std::string ret_code_str = std::string(ret_code_it->value.begin(), ret_code_it->value.end());
    HYDLA_LOGGER_VCS("ret_code_str: ",
                     ret_code_str);

    if(ret_code_str=="0") {
      // TODO: 適用に失敗（実数以外になる等）した場合。適切な処理をする
      assert(0);
    }
    else {
      assert(ret_code_str=="1");
      SExpParser::const_tree_iter_t value_it = ct_it->children.begin()+1;
      SExpConverter sc;
      value = sc.convert_s_exp_to_symbolic_value(sp, value_it);
      HYDLA_LOGGER_REST("new value : ", value->get_string());
    }

    out_vm[it->first] = value;
  }

  HYDLA_LOGGER_FUNC_END(VCS);
}

std::string REDUCEVCS::get_constraint_store(){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  std::string ret;

  reduce_link_->send_string("symbolic redeval '(getConstraintStore);");
  reduce_link_->skip_until_redeval();

  ret = reduce_link_->get_s_expr();
  HYDLA_LOGGER_FUNC_END(VCS);
  return ret;
}

// deleted
////value_tを指定された精度で数値に変換する
//std::string REDUCEVCS::get_real_val(const value_t &val, int precision, hydla::simulator::symbolic::OutputFormat opfmt){
//  std::string ret;
//  REDUCEStringSender rss(reduce_link_);
//
//  if(!val.undefined()) {
//    
//    reduce_link_->send_string("on rounded$");
//
//    // getRealVal(value_, prec_)を渡したい
//    reduce_link_->send_string("value_:=");
//    rss.put_node(val, true);
//    reduce_link_->send_string("$");
//    
//    std::stringstream precision_str;
//    precision_str << precision;
//    reduce_link_->send_string("prec_:="+ precision_str.str() +"$");
//    // 計算に用いる精度は6ケタ未満にできない（？）ようなので，表示桁を下げる
//    if(precision < 6){
//      reduce_link_->send_string("print_precision(" + precision_str.str() + ")$");
//    }
//    reduce_link_->send_string("getRealVal(value_, prec_);");
//    
//    reduce_link_->skip_until_redeval();
//    reduce_link_->get_line();
//    ret = reduce_link_->get_line();
//    reduce_link_->send_string("off rounded$");
//    // 精度を元に戻しておく
//    reduce_link_->send_string("precision(defaultPrec_)$");
//  }
//  else {
//    ret = "UNDEF";
//  }
//  return ret;
//}


bool REDUCEVCS::less_than(const time_t &lhs, const time_t &rhs)
{
  assert(0);
  return true;
  //HYDLA_LOGGER_FUNC_BEGIN(VCS);

  //REDUCEStringSender rss(reduce_link_);

  //// checkLessThan(lhs_, rhs_)を渡したい
  //

  //reduce_link_->send_string("lhs_:=");
  //rss.put_node(lhs, true);
  //reduce_link_->send_string("$");


  //reduce_link_->send_string("rhs_:=");
  //rss.put_node(rhs, true);
  //reduce_link_->send_string("$");

  //reduce_link_->send_string("symbolic redeval '(checkLessThan lhs_ rhs_);");


  //////////////////// 受信処理

  //// reduce_link_->read_until_redeval();
  //reduce_link_->skip_until_redeval();

  //std::string ans = reduce_link_->get_s_expr();
  //HYDLA_LOGGER_VCS("check_less_than_ans: ", ans);
  //HYDLA_LOGGER_FUNC_END(VCS);
  //return  boost::lexical_cast<int>(ans) == 1;
}


void REDUCEVCS::simplify(time_t &time) 
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  HYDLA_LOGGER_DEBUG("SymbolicTime::send_time : ", time);
  REDUCEStringSender rss(reduce_link_);

  // simplifyExpr(expr_)を渡したい

  reduce_link_->send_string("expr_:=");
  rss.put_node(get_symbolic_value_t(time).get_node(), true);
  reduce_link_->send_string("$");


  reduce_link_->send_string("symbolic redeval '(simplifyExpr expr_);");


  ////////////////// 受信処理

  reduce_link_->skip_until_redeval();
  const SExpParser sp(reduce_link_->get_as_s_exp_parser());

  SExpParser::const_tree_iter_t time_it = sp.get_tree_iterator();
  SExpConverter sc;
  time = sc.convert_s_exp_to_symbolic_value(sp, time_it);

  HYDLA_LOGGER_FUNC_END(VCS);
}

/*
 * SymbolicValueの時間をずらす
 */
hydla::vcs::SymbolicVirtualConstraintSolver::value_t REDUCEVCS::shift_expr_time(const value_t& val, const time_t& time){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  REDUCEStringSender rss(reduce_link_);

  // exprTimeShift(expr_, time_)を渡したい

  reduce_link_->send_string("expr_:=");
  rss.put_node(get_symbolic_value_t(val).get_node(), true);
  reduce_link_->send_string("$");


  reduce_link_->send_string("time_:=");
  rss.put_node(get_symbolic_value_t(time).get_node(), true);
  reduce_link_->send_string("$");


  reduce_link_->send_string("symbolic redeval '(exprTimeShift expr_ time_);");


  ////////////////// 受信処理

  reduce_link_->skip_until_redeval();
  const SExpParser sp(reduce_link_->get_as_s_exp_parser());

  SExpParser::const_tree_iter_t value_it = sp.get_tree_iterator();
  SExpConverter sc;

  HYDLA_LOGGER_FUNC_END(VCS);
  return  sc.convert_s_exp_to_symbolic_value(sp, value_it);
}

// TODO なんとかする
void REDUCEVCS::approx_vm(variable_range_map_t& vm){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  HYDLA_LOGGER_FUNC_END(VCS);
}

const REDUCEVCS::symbolic_value_t REDUCEVCS::get_symbolic_value_t(value_t value){
  value->accept(*this);
  return visited_;
}


void REDUCEVCS::visit(symbolic_value_t& value){
  visited_ = value;
}

const node_sptr REDUCEVCS::make_variable(const std::string &name, const int& dc, const bool& is_prev) const {
  node_sptr var_node(new Variable(name));
  for (int i = 0; i < dc; i++) {
    var_node = node_sptr(new Differential(var_node));
  }
  if(is_prev){
    var_node = node_sptr(new Previous(var_node));
  }

  return var_node; 
  }
} // namespace reduce
} // namespace vcs
} // namespace hydla 
