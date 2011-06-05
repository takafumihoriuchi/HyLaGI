
#include <cassert>
#include <boost/algorithm/string/predicate.hpp>

#include "REDUCEVCSPoint.h"

// TODO RTreeVisitorの引っ越し
#include "../../parser/RTreeVisitor.h"

/*
#include "mathlink_helper.h"
#include "PacketErrorHandler.h"
#include "PacketChecker.h"
 */

#include "../mathematica/MathematicaExpressionConverter.h"

#include "Logger.h"

using namespace hydla::vcs;
using namespace hydla::parse_tree;
using namespace hydla::logger;

namespace hydla {
namespace vcs {
namespace reduce {

//REDUCEVCSPoint::REDUCEVCSPoint(MathLink* ml) :ml_(ml)
REDUCEVCSPoint::REDUCEVCSPoint(REDUCELink* cl) :
  cl_(cl)
{
  std::cout << "Begin REDUCEVCSPoint::REDUCEVCSPoint(REDUCEClient* cl)" << std::endl;

}

REDUCEVCSPoint::~REDUCEVCSPoint()
{
  std::cout << "Begin REDUCEVCSPoint::~REDUCEVCSPoint()" << std::endl;

}

// MathematicaVCSPointより
bool REDUCEVCSPoint::reset()
{
  std::cout << "Begin REDUCEVCSPoint::reset()" << std::endl;
  // TODO: チョイ考える
  assert(0);
  //   constraint_store_.first.clear();
  //   constraint_store_.second.clear();
  return true;
}

// MathematicaVCSPointより
bool REDUCEVCSPoint::reset(const variable_map_t& variable_map)
{
  if(Logger::varflag==6){
  HYDLA_LOGGER_AREA("#*** Reset Constraint Store ***");

  if(variable_map.size() == 0)
  {
    HYDLA_LOGGER_AREA("no Variables");
    return true;
  }
  HYDLA_LOGGER_AREA("------Variable map------\n", variable_map);
  }

  HYDLA_LOGGER_SUMMARY("#*** Reset Constraint Store ***");

  if(variable_map.size() == 0)
  {
    HYDLA_LOGGER_SUMMARY("no Variables");
    return true;
  }
  HYDLA_LOGGER_SUMMARY("------Variable map------\n", variable_map);

  std::set<MathValue> and_cons_set;

  MathematicaExpressionConverter mec;

  variable_map_t::variable_list_t::const_iterator it =
    variable_map.begin();
  variable_map_t::variable_list_t::const_iterator end =
    variable_map.end();
  for(; it!=end; ++it)
  {
    const MathVariable& variable = (*it).first;
    const value_t&    value = it->second;

    if(!value.is_undefined()) {
      std::ostringstream val_str;
      val_str << "Equal[";

      if(variable.derivative_count > 0)
      {
        val_str << "Derivative["
                << variable.derivative_count
                << "][prev["
                << PacketSender::var_prefix
                << variable.name
                << "]]";
      }
      else
      {
        val_str << "prev["
                << PacketSender::var_prefix
                << variable.name
                << "]";
      }

      val_str << ","
              << mec.convert_symbolic_value_to_math_string(value)
              << "]"; // Equalの閉じ括弧

      MathValue new_math_value;
      new_math_value.set(val_str.str());
      and_cons_set.insert(new_math_value);

      // 制約ストア内の変数一覧を作成
      constraint_store_.second.insert(
        boost::make_tuple(variable.name,
                          variable.derivative_count,
                          true));
    }
  }

  constraint_store_.first.insert(and_cons_set);

  HYDLA_LOGGER_DEBUG(*this);

  return true;
}

// MathematicaVCSPointより
bool REDUCEVCSPoint::reset(const variable_map_t& variable_map, const parameter_map_t& parameter_map){
  assert(0);
  return false;
  if(!reset(variable_map)){
    return false;
  }

  if(parameter_map.size() == 0)
  {
    HYDLA_LOGGER_SUMMARY("no Parameters");
    return true;
  }
  HYDLA_LOGGER_SUMMARY("------Parameter map------\n", parameter_map);


  std::set<MathValue> and_cons_set;
  par_names_.clear();

  parameter_map_t::variable_list_t::const_iterator it =
    parameter_map.begin();
  parameter_map_t::variable_list_t::const_iterator end =
    parameter_map.end();
  for(; it!=end; ++it)
  {
    const value_range_t&    value = it->second;
    if(!value.is_undefined()) {
      value_range_t::or_vector::const_iterator or_it = value.or_begin(), or_end = value.or_end();
      for(;or_it != or_end; or_it++){
        value_range_t::and_vector::const_iterator and_it = or_it->begin(), and_end = or_it->end();
        for(; and_it != and_end; and_it++){
          std::ostringstream val_str;

          // MathVariable側に関する文字列を作成
          val_str << MathematicaExpressionConverter::get_relation_math_string(and_it->relation) << "[" ;

          val_str << PacketSender::par_prefix
                  << it->first.get_name();

          val_str << ","
                  << and_it->value
                  << "]"; // 閉じ括弧
          MathValue new_math_value;
          new_math_value.set(val_str.str());
          and_cons_set.insert(new_math_value);
          par_names_.insert(it->first.get_name());
        }
      }
    }
  }
  parameter_store_.first.insert(and_cons_set);
  return true;
}
//TODO 定数返しの修正
bool REDUCEVCSPoint::create_variable_map(variable_map_t& variable_map, parameter_map_t& parameter_map)
{
  assert(0);
  return false;
}

//TODO 定数返しの修正
void REDUCEVCSPoint::create_max_diff_map(
    PacketSender& ps, max_diff_map_t& max_diff_map)
{
  assert(0);
}

//TODO 定数返しの修正
void REDUCEVCSPoint::add_left_continuity_constraint(
    PacketSender& ps, max_diff_map_t& max_diff_map)
{
  assert(0);
}

//TODO 定数返しの修正
VCSResult REDUCEVCSPoint::add_constraint(const tells_t& collected_tells, const appended_asks_t &appended_asks)
{
  std::cout << "Begin REDUCEVCSPoint::add_constraint" << std::endl;
  RTreeVisitor rtv = RTreeVisitor(1);

  // tell制約の集合からexprを得てREDUCEに渡す
  std::cout << "collected_tells" << std::endl;

  tells_t::const_iterator tells_it  = collected_tells.begin();
  tells_t::const_iterator tells_end = collected_tells.end();
  for(; tells_it!=tells_end; ++tells_it) {
    if(Logger::constflag==9){
      HYDLA_LOGGER_AREA("put node: ", *(*tells_it)->get_child());
    }
    HYDLA_LOGGER_DEBUG("put node: ", *(*tells_it)->get_child());
    std::cout << rtv.get_expr((*tells_it)->get_child()) << std::endl;
  }

  // appended_asksからガード部分を得てREDUCEに渡す
  std::cout << "appended_asks" << std::endl;

  appended_asks_t::const_iterator append_it  = appended_asks.begin();
  appended_asks_t::const_iterator append_end = appended_asks.end();
  for(; append_it!=append_end; ++append_it) {
    HYDLA_LOGGER_DEBUG("put node (guard): ", *(append_it->ask->get_guard()), "  entailed:", append_it->entailed);
    std::cout << rtv.get_expr(append_it->ask->get_guard()) << std::endl;
  }



  //////////////////// 送信処理

  // send_stringのstringはどのように区切って送信してもOK
  cl_->send_string("expr_:={df(y,t,2) = -10,");
  cl_->send_string("y = 10, df(y,t,1) = 0, prev(y) = y, df(prev(y),t,1) = df(y,t,1)};");
  cl_->send_string("pexpr_:= {};");
  cl_->send_string("vars_:={y, prev(y), df(y,t,1), df(prev(y),t,1), df(y,t,2), y, prev(y), df(y,t,1), df(prev(y),t,1)};");
  cl_->send_string("symbolic redeval '(isconsistent vars_ pexpr_ expr_);");
  cl_->read_until_redeval();

  std::string ans = cl_->get_s_expr();
  std::cout << "add_constraint_ans: " << ans << std::endl;


  // VCSR_FALSE後終了
  std::cout << "End REDUCEVCSPoint::add_constraint" << std::endl;

  assert(0);

  return VCSR_FALSE;

}

//TODO 定数返しの修正
VCSResult REDUCEVCSPoint::check_entailment(const ask_node_sptr& negative_ask, const appended_asks_t &appended_asks)
{

  assert(0);

  return VCSR_FALSE;
}

//TODO 定数返しの修正
VCSResult REDUCEVCSPoint::integrate(
    integrate_result_t& integrate_result,
    const positive_asks_t& positive_asks,
    const negative_asks_t& negative_asks,
    const time_t& current_time,
    const time_t& max_time,
    const not_adopted_tells_list_t& not_adopted_tells_list,
    const appended_asks_t& appended_asks)
{

  assert(0);
  return VCSR_FALSE;
}

//TODO 定数返しの修正
void REDUCEVCSPoint::send_cs() const
{
  assert(0);
}

//TODO 定数返しの修正
void REDUCEVCSPoint::send_ps() const
{
  assert(0);
}

//TODO 定数返しの修正
void REDUCEVCSPoint::send_pars() const{
  assert(0);
}

//TODO 定数返しの修正
void REDUCEVCSPoint::send_cs_vars() const
{
   assert(0);
}

// MathematicaVCSPointより
std::ostream& REDUCEVCSPoint::dump(std::ostream& s) const
{
  s << "#*** Dump REDUCEVCSPoint ***\n"
      << "--- constraint store ---\n";

  //
  std::set<std::set<MathValue> >::const_iterator or_cons_it =
      constraint_store_.first.begin();
  while((or_cons_it) != constraint_store_.first.end())
  {
    std::set<MathValue>::const_iterator and_cons_it =
        (*or_cons_it).begin();
    while((and_cons_it) != (*or_cons_it).end())
    {
      s << (*and_cons_it).get_string() << " ";
      and_cons_it++;
    }
    s << "\n";
    or_cons_it++;
  }

  // 制約ストア内に存在する変数のダンプ
  s << "-- vars --\n";
  constraint_store_vars_t::const_iterator vars_it =
      constraint_store_.second.begin();
  while((vars_it) != constraint_store_.second.end())
  {
    s << *(vars_it) << "\n";
    vars_it++;
  }

  return s;
}

// MathematicaVCSPointより
std::ostream& operator<<(std::ostream& s, const REDUCEVCSPoint& m)
{
  return m.dump(s);
}


} // namespace reduce
} // namespace simulator
} // namespace hydla

