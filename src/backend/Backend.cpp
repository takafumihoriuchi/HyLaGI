#include "Backend.h"
#include <stdarg.h>
#include "InterfaceError.h"
#include <sstream>
#include <boost/lexical_cast.hpp>

#include "../symbolic_expression/TreeInfixPrinter.h"

using namespace std;

namespace hydla{
namespace backend{
using namespace simulator;
using namespace parse_tree;
using namespace symbolic_expression;

const std::string Backend::var_prefix = "u";
// use different prefix to distinct variables and constants
const std::string par_prefix = "p";

// check equivalence ignoring whether upper or lower case
static bool equal_ignoring_case(std::string lhs, std::string rhs)
{
  const char *l = lhs.c_str(), *r = rhs.c_str();
  int d;
  while(*l != '\0' && *r != '\0'){
    d = (tolower(*l++) - tolower(*r++));
    if ( d != 0)
    {
      return false;
    }
  }
  return true;
}


Backend::Backend(Link* link)
{
  link_.reset(link);
}

Backend::~Backend()
{
}

void Backend::invalid_fmt(const char* fmt, int idx)
{
  std::stringstream sstr;
  sstr << "invalid format \"" << fmt << "\" at " << idx;
  throw InterfaceError(sstr.str().c_str());
}

int Backend::read_args_fmt(const char* args_fmt, const int& idx, void *arg)
{
  int i = idx;
  switch(args_fmt[i])
  {
  case 'i':
  {
    int* num = (int*)arg;
    link_->put_integer(*num);
  }
  break;

  case 's':
  {
    const char* sym = (const char *)arg;
    link_->put_symbol(sym);
  }
  break;


  case 'e':
  {
    symbolic_expression::node_sptr* node = (symbolic_expression::node_sptr *)arg;
    variable_form_t form;
    if(!get_form(args_fmt[++i], form))
    {
      invalid_fmt(args_fmt, i);
    }
    else
    {
      send_node(*node, form);
    }
  }
  break;
    

  case 'c':
    switch(args_fmt[++i])
    {
    case 's':
    {
      variable_form_t form;
      if(!get_form(args_fmt[++i], form))
      {
        invalid_fmt(args_fmt, i);
      }
      else
      {
        constraint_store_t *cs = (constraint_store_t *)arg;
        link_->put_converted_function("And", cs->size());
        for(auto constraint : *cs)
        {
          send_node(constraint, form);
        }
      }
    }
    break;
    default:
      invalid_fmt(args_fmt, i);
      break;
    }
    break;
  case 'm':
    switch(args_fmt[++i])
    {
    case 'v':
    {
      variable_map_t* vm = (variable_map_t*)arg;
      bool send_derivative = true;
      char form_char;
      if(args_fmt[++i] == '0')
      {
        send_derivative = false;
        form_char = args_fmt[++i];
      }
      else
      {
        form_char = args_fmt[i];
      }
      variable_form_t form;
      if(!get_form(form_char, form))
      {
        invalid_fmt(args_fmt, i);
        break;
      }
      send_variable_map(*vm, form, send_derivative);
      break;
    }
    case 'p':
    {
      parameter_map_t* pm = (parameter_map_t*)arg;
      send_parameter_map(*pm);
      break;
    }
    default:
      invalid_fmt(args_fmt, i);
      break;
    }
    break;
  case 'p':     
  {
    parameter_t* par = (parameter_t *)arg;
    link_->put_parameter(par_prefix + par->get_name(), par->get_differential_count(), par->get_phase_id());
  }
  break;

  case 'v':
  {
    if(args_fmt[++i] == 'l')
    {
      value_t *val = (value_t *)arg;
      variable_form_t vf;
      if(!get_form(args_fmt[++i], vf))
      {
        invalid_fmt(args_fmt, i);
        break;
      }
      send_value(*val, vf);
    }
    else
    {
      variable_t *var = (variable_t*)arg;
      variable_form_t vf;
      if(!get_form(args_fmt[i], vf))
      {
        invalid_fmt(args_fmt, i);
        break;
      }
      send_variable(var->get_name(), var->get_differential_count(), vf);
    }
  }
  break;


  default:      
    invalid_fmt(args_fmt, i);
    break;
  }
  return i - idx;
}

int Backend::read_ret_fmt(const char *ret_fmt, const int& idx, void* ret)
{
  int i = idx;
  switch(ret_fmt[i])
  {
  case 'r':
  {
    MidpointRadius *mr = (MidpointRadius *)ret;
    *mr = receive_midpoint_radius();
  }
  break;
  case 'i':
  {
    int* num = (int *)ret;
    *num = link_->get_integer();
  }
  break;

  case 'e':
  {
    symbolic_expression::node_sptr* node = (symbolic_expression::node_sptr*)ret;
    variable_form_t form;
    if(!get_form(ret_fmt[++i], form))
    {
      invalid_fmt(ret_fmt, i);
    }
    else
    {
      *node = receive_node();
    }
  }
  break;
  
  case 'b':
  {
    bool* b = (bool*)ret;
    receive_bool(*b);
    break;
  }

  
  case 'f':
  {
    find_min_time_result_t *f = (find_min_time_result_t *)ret;
    *f = receive_find_min_time_result();
    break;
  }

                   
  case 'm':
    switch(ret_fmt[++i])
    {
    case 'v':
    {
      variable_map_t* vm = (variable_map_t*)ret;
      variable_form_t form;
      if(!get_form(ret_fmt[++i], form))
      {
        invalid_fmt(ret_fmt, i);
        break;
      }
      receive_map(*vm);
      break;
    }
    case 'p':
    {
      parameter_map_t* pm = (parameter_map_t*)ret;
      receive_parameter_map(*pm);
      break;
    }
    default:
      invalid_fmt(ret_fmt, i);
      break;
    }
    break;
  case 'c':
    switch(ret_fmt[++i])
    {
    case 'c':
    {
      check_consistency_result_t* cc = (check_consistency_result_t *)ret;
      *cc = receive_cc();
      break;
    }
    case 'v':
    {
      // for cv
      create_vm_t* cv = (create_vm_t*)ret;
      *cv = receive_cv();
      break;
    }
    case 'p':
    {
      // for cp
      compare_min_time_result_t* cp = (compare_min_time_result_t*)ret;
      *cp = receive_compare_min_time_result();
      break;
    }
    case 's':
    {
      // for cs
      ConstraintStore* cs = (ConstraintStore*)ret;
      *cs = receive_cs();
      break;
    }
    default:
      invalid_fmt(ret_fmt, i);     
      break;
    }
    break;

  case 'v':
  {
    if(ret_fmt[++i] == 'l')
    {
      value_t *val = (value_t *)ret;
      *val = receive_value();
    }
    else invalid_fmt(ret_fmt, i);
    break;
  }
    

  default:      
    invalid_fmt(ret_fmt, i);
    break;
  }
  return i - idx;
}

int Backend::call(const char* name, int arg_cnt, const char* args_fmt, const char* ret_fmt, ...)
{
  link_->pre_send();
  link_->put_converted_function(name, arg_cnt);
  va_list args;
  va_start(args, ret_fmt);
  for(int i = 0; args_fmt[i] != '\0'; i++)
  {
    void* arg = va_arg(args, void *);
    i += read_args_fmt(args_fmt, i, arg);
  }
  link_->pre_receive();
  HYDLA_LOGGER_DEBUG("input: \n", link_->get_input_print());
  for(int i = 0; ret_fmt[i] != '\0'; i++)
  {
    void* ret = va_arg(args, void *);
    i += read_ret_fmt(ret_fmt, i, ret);
  }
  link_->post_receive();

  // TODO: 例外投げた場合もva_endを呼び出すように
  va_end(args);
  return 0;
}

bool Backend::get_form(const char &form_c, variable_form_t &form)
{
  switch(form_c)
  {
  case 'p':
    form = Link::VF_PREV;
    return true;
  case 'c':
    form = Link::VF_IGNORE_PREV;
    return true;
  case 'n':
    form = Link::VF_NONE;
    return true;
  case 'z':
    form = Link::VF_ZERO;
    return true;
  case 't':
    form = Link::VF_TIME;
    return true;
  default:
    return false;
  }
}

int Backend::send_node(const symbolic_expression::node_sptr& node, const variable_form_t &form)
{
  differential_count_ = 0;
  in_prev_ = false;
  apply_not_ = false;
  variable_arg_ = form;
  accept(node);
  return 0;
}

int Backend::send_variable_map(const variable_map_t& vm, const variable_form_t& vf, const bool &send_derivative)
{
  int size_to_sent = 0;
  for(variable_map_t::const_iterator it = vm.begin(); it != vm.end(); it++)
  {
    const variable_t var = it->first;
    const value_range_t &range = it->second;
    if(!send_derivative && var.get_differential_count() > 0 )continue;
    if(range.unique())
    {
      size_to_sent++;
    }
    else
    {
      size_to_sent += range.get_lower_cnt();
      size_to_sent += range.get_upper_cnt();
    }
  }
  link_->put_converted_function("List", size_to_sent);
  for(variable_map_t::const_iterator it = vm.begin(); it != vm.end(); it++)
  {
    const variable_t var = it->first;
    const value_range_t &range = it->second;
    if(!send_derivative && var.get_differential_count() > 0 )continue;
    if(range.unique())
    {
      link_->put_converted_function("Equal", 2);
      send_variable(var, vf);
      send_value(range.get_unique_value(), vf);
    }
    else
    {
      for(uint i = 0; i < range.get_lower_cnt(); i++)
      {
        const value_range_t::bound_t &bnd = range.get_lower_bound(i);
        if(bnd.include_bound)
        {
          link_->put_converted_function("GreaterEqual", 2);
        }
        else
        {
          link_->put_converted_function("Greater", 2);
        }
        send_variable(var, vf);
        send_value(bnd.value, vf);
      }

      for(uint i = 0; i < range.get_upper_cnt(); i++)
      {
        const value_range_t::bound_t &bnd = range.get_upper_bound(i);
        if(bnd.include_bound)
        {
          link_->put_converted_function("LessEqual", 2);
        }
        else
        {
          link_->put_converted_function("Less", 2);
        }
        send_variable(var, vf);
        send_value(bnd.value, vf);
      }      
    }
  }
  return 0;
}

int Backend::send_parameter_map(const parameter_map_t& parameter_map)
{
  parameter_map_t::const_iterator it = parameter_map.begin();
  int size=0;
  for(; it!=parameter_map.end(); ++it)
  {
    const value_range_t &range = it->second;
    if(range.unique()){
      size++;
    }else{
      size += range.get_lower_cnt() + range.get_upper_cnt();
    }
  }
  
  link_->put_converted_function("List", size);
  it = parameter_map.begin();
  for(; it!=parameter_map.end(); ++it)
  {
    if(it->second.unique()){
      const value_t &value = it->second.get_unique_value();
      const parameter_t& param = it->first;
      link_->put_function("Equal", 2);
      link_->put_parameter(par_prefix + param.get_name(), param.get_differential_count(), param.get_phase_id());
      send_value(value, Link::VF_PREV);
    }else{
      for(uint i=0; i < it->second.get_lower_cnt();i++)
      {
        const value_range_t::bound_t &bnd = it->second.get_lower_bound(i);
        const value_t &value = bnd.value;
        const parameter_t& param = it->first;
        if(!bnd.include_bound)
        {
          link_->put_converted_function("Greater", 2);
        }
        else
        {
          link_->put_converted_function("GreaterEqual", 2);
        }
        link_->put_parameter(par_prefix + param.get_name(), param.get_differential_count(), param.get_phase_id());
        send_value(value, Link::VF_PREV);
      }
      for(uint i=0; i < it->second.get_upper_cnt();i++)
      {
        const value_range_t::bound_t &bnd = it->second.get_upper_bound(i);
        const value_t &value = bnd.value;
        const parameter_t& param = it->first;
        if(!bnd.include_bound)
        {
          link_->put_converted_function("Less", 2);
        }
        else
        {
          link_->put_converted_function("LessEqual", 2);
        }
        link_->put_parameter(par_prefix + param.get_name(), param.get_differential_count(), param.get_phase_id());
        send_value(value, Link::VF_PREV);
      }
    }
  }
  return 0;
}

void Backend::visit(boost::shared_ptr<Ask> node)                   
{
  throw InterfaceError("ask node cannot be sent to backend");
}

void Backend::visit(boost::shared_ptr<Tell> node)
{
  accept(node->get_child());
}

#define DEFINE_VISIT_BINARY(NODE_NAME, FUNC_NAME)                       \
void Backend::visit(boost::shared_ptr<NODE_NAME> node)        \
{                                                                       \
  link_->put_converted_function(#FUNC_NAME, 2);                                  \
  accept(node->get_lhs());                                              \
  accept(node->get_rhs());                                              \
}

#define DEFINE_VISIT_BINARY_NOT(NODE_NAME, FUNC_NAME, NOT_NAME)        \
void Backend::visit(boost::shared_ptr<NODE_NAME> node)        \
{                                                                       \
  if(!apply_not_)                                                        \
    link_->put_converted_function(#FUNC_NAME, 2);                       \
  else                                                                  \
    link_->put_converted_function(#NOT_NAME, 2);                        \
  accept(node->get_lhs());                                                \
  accept(node->get_rhs());                                              \
}


#define DEFINE_VISIT_UNARY(NODE_NAME, FUNC_NAME)                        \
void Backend::visit(boost::shared_ptr<NODE_NAME> node)        \
{                                                                       \
  link_->put_converted_function(#FUNC_NAME, 1);                                  \
  accept(node->get_child());                                            \
}

#define DEFINE_VISIT_FACTOR(NODE_NAME, FUNC_NAME)                       \
void Backend::visit(boost::shared_ptr<NODE_NAME> node)        \
{                                                                       \
  link_->put_symbol(#FUNC_NAME);                                       \
}

DEFINE_VISIT_BINARY_NOT(Equal, Equal, Unequal)
DEFINE_VISIT_BINARY_NOT(UnEqual, Unequal, Equal)
DEFINE_VISIT_BINARY_NOT(Less, Less, GreaterEqual)
DEFINE_VISIT_BINARY_NOT(LessEqual, LessEqual, Greater)
DEFINE_VISIT_BINARY_NOT(Greater, Greater, LessEqual)
DEFINE_VISIT_BINARY_NOT(GreaterEqual, GreaterEqual, Less)

/// 論理演算子
DEFINE_VISIT_BINARY_NOT(LogicalAnd, And, Or)
DEFINE_VISIT_BINARY_NOT(LogicalOr, Or, And)

/// 算術二項演算子
DEFINE_VISIT_BINARY(Plus, Plus)
DEFINE_VISIT_BINARY(Subtract, Subtract)
DEFINE_VISIT_BINARY(Times, Times)
DEFINE_VISIT_BINARY(Divide, Divide)
DEFINE_VISIT_BINARY(Power, Power)

  
/// 算術単項演算子

DEFINE_VISIT_UNARY(Negative, Minus)
void Backend::visit(boost::shared_ptr<Positive> node)              
{
  accept(node->get_child());
}

/// 微分
void Backend::visit(boost::shared_ptr<Differential> node)          
{
  differential_count_++;
  accept(node->get_child());
  differential_count_--;
}

/// 左極限
void Backend::visit(boost::shared_ptr<Previous> node)              
{
  in_prev_ = true;
  accept(node->get_child());
  in_prev_ = false;
}


/// 否定
void Backend::visit(boost::shared_ptr<Not> node)              
{
  apply_not_ = !apply_not_;
  accept(node->get_child());
  apply_not_ = !apply_not_;
}

/// 関数
void Backend::visit(boost::shared_ptr<Function> node)              
{
  string name;
  int arg_cnt = node->get_arguments_size();
  bool converted;
  name = link_->convert_function(node->get_string(), true, converted);
  if(!converted)
  {
    throw InterfaceError(node->get_string() + " is not suppported in " + link_->backend_name());
  }
  link_->put_function(name, arg_cnt);
  for(int i=0; i < arg_cnt;i++){
    accept(node->get_argument(i));
  }
}

void Backend::visit(boost::shared_ptr<UnsupportedFunction> node)              
{
  link_->put_function(node->get_string().c_str(), 1);
  link_->put_function("Evaluate", node->get_arguments_size()); // for "HoldForm" in Mathematica
  for(int i=0; i<node->get_arguments_size();i++){
    accept(node->get_argument(i));
  }
}

/// 円周率
DEFINE_VISIT_FACTOR(Infinity, Infinity)
/// 円周率
DEFINE_VISIT_FACTOR(Pi, Pi)
/// 自然対数の底
DEFINE_VISIT_FACTOR(E, E)

void Backend::visit(boost::shared_ptr<symbolic_expression::ExpressionListElement> node)
{
  // 式リストの要素の送信
  variable_form_t va;
  if(variable_arg_== Link::VF_NONE && in_prev_)
  {
    va = Link::VF_PREV;
  }
  else{
    if(variable_arg_ == Link::VF_IGNORE_PREV)
    {
      va = Link::VF_NONE;
    }
    else
    {
      va = variable_arg_;
    }
  }

  boost::shared_ptr<symbolic_expression::ExpressionListCaller> caller = boost::dynamic_pointer_cast<symbolic_expression::ExpressionListCaller>(node->get_lhs()->clone());
  if(caller)
  {
    send_list_caller(caller->get_name(), differential_count_, va, node->get_rhs());
  }
  boost::shared_ptr<symbolic_expression::ExpressionList> el = boost::dynamic_pointer_cast<symbolic_expression::ExpressionList>(node->get_lhs()->clone());
  if(el)
  {
    link_->put_function("Part",2);
    accept(node->get_lhs());
    accept(node->get_rhs());
  }
}

void Backend::visit(boost::shared_ptr<symbolic_expression::ExpressionList> node)
{
  link_->put_function("List",node->get_arguments_size());
  for(int i = 0; i < node->get_arguments_size(); i++)
  {
    accept(node->get_argument(i));
  }
}

// 変数
void Backend::visit(boost::shared_ptr<symbolic_expression::Variable> node)              
{
  // 変数の送信
  variable_form_t va;
  if(variable_arg_== Link::VF_NONE && in_prev_)
  {
    va = Link::VF_PREV;
  }
  else{
    if(variable_arg_ == Link::VF_IGNORE_PREV)
    {
      va = Link::VF_NONE;
    }
    else
    {
      va = variable_arg_;
    }
  }

  send_variable(node->get_name(), differential_count_, va);
}

// 数字
void Backend::visit(boost::shared_ptr<Number> node)                
{
  // link_->put_integer(atoi(node->get_number().c_str())); //数値がでかいとオーバーフローする
  // link_->put_symbol(node->get_number().c_str()); // put_symbolだと送れない
  link_->put_number(node->get_number().c_str());
}

void Backend::visit(boost::shared_ptr<Float> node)              {
  link_->put_float(node->get_number());
}  


// 記号定数
void Backend::visit(boost::shared_ptr<symbolic_expression::Parameter> node)
{
  link_->put_parameter(par_prefix + node->get_name(), node->get_differential_count(), node->get_phase_id());
}

// t
void Backend::visit(boost::shared_ptr<SymbolicT> node)                
{    
  link_->put_symbol("t");
}


int Backend::send_value(const value_t &val, const variable_form_t& var)
{
  send_node(val.get_node(), var);
  return 0;
}

int Backend::send_variable(const variable_t &var, const variable_form_t &variable_arg)
{
  return send_variable(var.get_name(), var.get_differential_count(), variable_arg);
}

int Backend::send_list_caller(const std::string& name, int diff_count, const variable_form_t &variable_arg, symbolic_expression::node_sptr node)
{
  variable_form_t tmp_form = variable_form_t(variable_arg);
  std::string prefix = (variable_arg == Link::VF_PREV)?par_prefix:var_prefix;
  link_->put_pre_list_caller(prefix + name, diff_count, variable_arg);
  differential_count_ = 0;
  accept(node);
  differential_count_ = diff_count;
  link_->put_post_list_caller(diff_count, tmp_form);
  return 0;
}

int Backend::send_variable(const std::string& name, int diff_count, const variable_form_t &variable_arg)
{
  std::string prefix = (variable_arg == Link::VF_PREV)?par_prefix:var_prefix;
  if(name.substr(0,1) == "U") prefix = "l";
  link_->put_variable(prefix + name, diff_count, variable_arg);
  return 0;
}


// コマンド文
void Backend::visit(boost::shared_ptr<symbolic_expression::PrintPP> node){link_->put_symbol("True");}
void Backend::visit(boost::shared_ptr<symbolic_expression::PrintIP> node){link_->put_symbol("True");}
void Backend::visit(boost::shared_ptr<symbolic_expression::Scan> node){link_->put_symbol("True");}

void Backend::visit(boost::shared_ptr<symbolic_expression::True> node){link_->put_symbol("True");}
void Backend::visit(boost::shared_ptr<symbolic_expression::False> node){link_->put_symbol("False");}

void Backend::set_range(const value_t &val, value_range_t &range, const int& relop){
  switch(relop){
    case 0://Equal
    range.set_unique_value(val);
    break;
    case 1://Less
    range.set_upper_bound(val, false);
    break;
    case 2://Greater
    range.set_lower_bound(val, false);
    break;
    case 3://LessEqual
    range.set_upper_bound(val, true);
    break;
    case 4://GreaterEqual
    range.set_lower_bound(val, true);
    break;
  }
}

ConstraintStore Backend::receive_cs()
{
  ConstraintStore cs;
  std::string name;
  int count;
  link_->get_function(name, count);
  for(int i = 0; i < count; i++)
  {
    symbolic_expression::node_sptr constraint;
  
    constraint = receive_node();
    // TODO: avoid string comparison
    string constraint_string = get_infix_string(constraint);
    if(constraint_string == "False")
    {
      cs.set_consistency(false);
    }
    else if(constraint_string != "True")
    {
      cs.add_constraint(constraint);
    }
  }
  return cs;
}


create_vm_t Backend::receive_cv()
{
  create_vm_t ret;
  std::string name;
  int cnt;
  link_->get_function(name, cnt);
  for(int i = 0; i < cnt; i++)
  {
    variable_map_t map; receive_map(map);
    ret.push_back(map);
  }
  return ret;
}

compare_min_time_result_t Backend::receive_compare_min_time_result()
{
  std::string name;
  int list_size; 
  link_->get_function(name, list_size);
  compare_min_time_result_t result;
  link_->get_function(name, list_size);
  for(int i = 0; i < list_size; i++)
  {
    parameter_map_t pm;
    receive_parameter_map(pm);
    result.less_maps.push_back(pm);
  }
  link_->get_function(name, list_size);
  for(int i = 0; i < list_size; i++)
  {
    parameter_map_t pm;
    receive_parameter_map(pm);
    result.greater_maps.push_back(pm);
  }
  link_->get_function(name, list_size);
  for(int i = 0; i < list_size; i++)
  {
    parameter_map_t pm;
    receive_parameter_map(pm);
    result.equal_maps.push_back(pm);
  }
  return result;
}



find_min_time_result_t Backend::receive_find_min_time_result()
{
  std::string name;
  int find_min_time_size; 
  // List
  link_->get_function(name, find_min_time_size);
  find_min_time_result_t result;
  for(int time_it = 0; time_it < find_min_time_size; time_it++){
    FindMinTimeCandidate candidate;
    int dummy_buf;
    // List
    link_->get_function(name, dummy_buf);
    candidate.time = receive_value();
    candidate.on_time = (bool)link_->get_integer();

    // 条件を受け取る
    receive_parameter_map(candidate.parameter_map);
    result.push_back(candidate);
  }
  return result;
}


std::string Backend::remove_prefix(const std::string &original, const std::string &prefix)
{
  if(original.length() <= prefix.length())throw InterfaceError("invalid name: " + original);
  return original.substr(prefix.length());
}

check_consistency_result_t Backend::receive_cc()
{
  check_consistency_result_t ret;
  std::string outer_name;
  int outer_cnt;
  link_->get_function(outer_name, outer_cnt);
  assert(outer_cnt == 2);
  ret.consistent_store = receive_cs();
  ret.inconsistent_store = receive_cs();
  return ret;
}

symbolic_expression::node_sptr Backend::receive_function()
{
// TODO: UnsupportedFunctionを含む関数は，バックエンドを切り替えられないので各Valueごとにそのことを示すフラグを持たせた方が良いかも
  int arg_count;
  symbolic_expression::node_sptr ret;
  std::string symbol;
  bool converted;
  link_->get_function(symbol, arg_count);
  symbol = link_->convert_function(symbol, false, converted);
  if(equal_ignoring_case(symbol, "Sqrt")){//1引数関数
    ret = symbolic_expression::node_sptr(new Divide(symbolic_expression::node_sptr(new Number("1")), symbolic_expression::node_sptr(new Number("2")))); 
    ret = symbolic_expression::node_sptr(new symbolic_expression::Power(receive_node(), ret));
  }
  else if(equal_ignoring_case(symbol, "par")){
    std::string name;
    name = remove_prefix(link_->get_symbol(), par_prefix);
    std::string d_str;
    d_str = link_->get_string();
    int differential_count = boost::lexical_cast<int, std::string>(d_str);
    std::string id_str;
    id_str = link_->get_string();
    int id = boost::lexical_cast<int, std::string>(id_str);
    ret = symbolic_expression::node_sptr(new symbolic_expression::Parameter(name, differential_count, id));
  }
  else if(equal_ignoring_case(symbol, "prev")){
    std::string name;
    name = remove_prefix(link_->get_symbol(), par_prefix);
    std::string d_str = link_->get_string();
    int differential_count = boost::lexical_cast<int, std::string>(d_str);
    symbolic_expression::node_sptr tmp_var = symbolic_expression::node_sptr(new symbolic_expression::Variable(name));
    for(int i = 0; i < differential_count; i++) tmp_var = symbolic_expression::node_sptr(new symbolic_expression::Differential(tmp_var));
    ret = symbolic_expression::node_sptr(new symbolic_expression::Previous(tmp_var));
  }
  else if(equal_ignoring_case(symbol, "minus")){
    ret = symbolic_expression::node_sptr(new symbolic_expression::Negative(receive_node()));
  }
  else if(equal_ignoring_case(symbol, "Plus")
          || equal_ignoring_case(symbol, "Subtract")
          || equal_ignoring_case(symbol, "Times")
          || equal_ignoring_case(symbol, "Divide")
          || equal_ignoring_case(symbol, "Power")
          || equal_ignoring_case(symbol, "Rational")
          || equal_ignoring_case(symbol, "And")
          || equal_ignoring_case(symbol, "Or")
          || equal_ignoring_case(symbol, "Equal")
          || equal_ignoring_case(symbol, "Unequal")
          || equal_ignoring_case(symbol, "Less")
          || equal_ignoring_case(symbol, "LessEqual")
          || equal_ignoring_case(symbol, "Greater")
          || equal_ignoring_case(symbol, "GreaterEqual"))        
  { // 加減乗除など，二項演算子で書かれる関数
    symbolic_expression::node_sptr lhs, rhs;
    ret = receive_node();
    for(int arg_it=1;arg_it<arg_count;arg_it++){
      lhs = ret;
      rhs = receive_node();
      if(equal_ignoring_case(symbol, "Plus"))
        ret = symbolic_expression::node_sptr(new symbolic_expression::Plus(lhs, rhs));
      else if(equal_ignoring_case(symbol, "Subtract"))
        ret = symbolic_expression::node_sptr(new symbolic_expression::Subtract(lhs, rhs));
      else if(equal_ignoring_case(symbol, "Times"))
        ret = symbolic_expression::node_sptr(new symbolic_expression::Times(lhs, rhs));
      else if(equal_ignoring_case(symbol, "Divide"))
        ret = symbolic_expression::node_sptr(new symbolic_expression::Divide(lhs, rhs));
      else if(equal_ignoring_case(symbol, "Power"))
        ret = symbolic_expression::node_sptr(new symbolic_expression::Power(lhs, rhs));
      else if(equal_ignoring_case(symbol, "Rational"))
        ret = symbolic_expression::node_sptr(new symbolic_expression::Divide(lhs, rhs));
      else if(equal_ignoring_case(symbol, "And"))
        ret = symbolic_expression::node_sptr(new symbolic_expression::LogicalAnd(lhs, rhs));
      else if(equal_ignoring_case(symbol, "Or"))
        ret = symbolic_expression::node_sptr(new symbolic_expression::LogicalOr(lhs, rhs));
      else if(equal_ignoring_case(symbol, "Equal"))
        ret = symbolic_expression::node_sptr(new symbolic_expression::Equal(lhs, rhs));
      else if(equal_ignoring_case(symbol, "Unequal"))
        ret = symbolic_expression::node_sptr(new symbolic_expression::UnEqual(lhs, rhs));
      else if(equal_ignoring_case(symbol, "Less"))
        ret = symbolic_expression::node_sptr(new symbolic_expression::Less(lhs, rhs));
      else if(equal_ignoring_case(symbol, "LessEqual"))
        ret = symbolic_expression::node_sptr(new symbolic_expression::LessEqual(lhs, rhs));
      else if(equal_ignoring_case(symbol, "Greater"))
        ret = symbolic_expression::node_sptr(new symbolic_expression::Greater(lhs, rhs));
      else if(equal_ignoring_case(symbol, "GreaterEqual"))
        ret = symbolic_expression::node_sptr(new symbolic_expression::GreaterEqual(lhs, rhs));
    }
  }
  else if(equal_ignoring_case(symbol, "derivative"))
  {
    std::string d_str = link_->get_string();
    int variable_differential_count = boost::lexical_cast<int, std::string>(d_str.c_str());
    std::string variable_name = remove_prefix(link_->get_symbol(), var_prefix);
    ret = symbolic_expression::node_sptr(new symbolic_expression::Variable(variable_name));
    for(int i = 0; i < variable_differential_count; i++)
    {
      ret = symbolic_expression::node_sptr(new symbolic_expression::Differential(ret));
    } 
  }
  else{
    // その他の関数
    boost::shared_ptr<symbolic_expression::ArbitraryNode> f;
    if(converted)
    {
      // 対応している関数
      f.reset(new symbolic_expression::Function(symbol));
    }
    else{
      // 謎の関数
      f.reset(new symbolic_expression::UnsupportedFunction(symbol));
    }

    for(int arg_it=0; arg_it < arg_count; arg_it++){
      f->add_argument(receive_node());
    }
    ret = f;
  }
  return ret;
}

value_t Backend::receive_value()
{
  value_t val(receive_node());
  return val;
}

symbolic_expression::node_sptr Backend::receive_node(){
  symbolic_expression::node_sptr ret;
  Link::DataType type = link_->get_type();
  switch(type){
  case Link::DT_STR: // 文字列
    {
      std::string str = link_->get_string();
      ret = symbolic_expression::node_sptr(new symbolic_expression::Number(str));
      break;
    }
  case Link::DT_SYM: // シンボル（記号）
    {
      std::string symbol = link_->get_symbol();
      if(symbol=="t")
        ret = symbolic_expression::node_sptr(new symbolic_expression::SymbolicT());
      else if(symbol=="Pi")
        ret = symbolic_expression::node_sptr(new symbolic_expression::Pi());
      else if(symbol=="E")
        ret = symbolic_expression::node_sptr(new symbolic_expression::E());
      else if(symbol=="inf")
        ret = symbolic_expression::node_sptr(new symbolic_expression::Infinity());
      else if(symbol=="True")
        ret = symbolic_expression::node_sptr(new symbolic_expression::True());
      else if(symbol=="False")
        ret = symbolic_expression::node_sptr(new symbolic_expression::False());
      else if(symbol.length() > var_prefix.length() && symbol.substr(0, var_prefix.length()) == var_prefix)
        ret = symbolic_expression::node_sptr(new symbolic_expression::Variable(symbol.substr(var_prefix.length())));
      break;
    }
  case Link::DT_INT: // オーバーフローする可能性があるなら文字列使う
    {
      std::stringstream sstr;
      int num = link_->get_integer();
      sstr << num;
      ret = symbolic_expression::node_sptr(new symbolic_expression::Number(sstr.str() ) );
      break;
    }
  case Link::DT_FUNC: // 合成関数
      ret = receive_function();
      break;

    default:
      break;
  }
  if(ret == NULL){
    invalid_ret();
  }
  return ret;
}


void Backend::invalid_ret()
{
  throw InterfaceError("invalid return value. \ninput:\n" + link_->get_input_print() + "\n\ntrace:\n" + link_->get_debug_print());
}

void Backend::receive_bool(bool &b)
{
  std::string s_name = link_->get_symbol();
  if(s_name == "True") b = true;
  else
  {
    if(s_name == "False")b = false;
    else throw InterfaceError("invalid return value");
  }
}

int Backend::receive_map(variable_map_t& map)
{
  value_t symbolic_value;
  std::string f_name;
  int and_size, size;
  link_->get_function(f_name, and_size);
  for(int i = 0; i < and_size; i++)
  {
    //{{変数名，微分回数}, 関係演算子コード，数式}で来るはず
    link_->get_function(f_name, size); //List
    link_->get_function(f_name, size); //List
    std::string variable_name = link_->get_symbol();
    int d_cnt = link_->get_integer();
    // 関係演算子のコード
    int rel = link_->get_integer();

    symbolic_value = value_t(receive_node());

    // TODO:次の一行消す
    if(variable_name == "t")continue;
    variable_t variable(variable_name.substr(var_prefix.length()), d_cnt);

    value_range_t tmp_range = map[variable];
    set_range(symbolic_value, tmp_range, rel);
    if(symbolic_value.undefined()){
      throw InterfaceError("invalid value");
    }
    map[variable] = tmp_range;  
  }
  return 0;
}



int Backend::receive_parameter_map(parameter_map_t& map)
{
  string func_name;
  int condition_size; link_->get_function(func_name, condition_size);
  for(int cond_it = 0; cond_it < condition_size; cond_it++){
    string str_buf;
    int int_buf;
    link_->get_function(str_buf, int_buf); // List
    link_->get_function(str_buf, int_buf); // parameter
    if(str_buf == "p")
    {
      std::string name = remove_prefix(link_->get_symbol(), par_prefix);
      int differential_count = link_->get_integer();
      int id = link_->get_integer();
      parameter_t tmp_param(name, differential_count, id);
      value_range_t tmp_range = map[tmp_param];
      int relop_code = link_->get_integer();
      value_t tmp_value = value_t(receive_node());
      set_range(tmp_value, tmp_range, relop_code);
      map[tmp_param] = tmp_range;
    }
    else
    {
      //ignore
      for(int i = 0; i < int_buf; i++)
      {
        link_->get_next();
      }
      link_->get_integer();
      receive_node();
    }
  }
  return 0;
}

MidpointRadius Backend::receive_midpoint_radius()
{
  MidpointRadius mr;
  string func_name;
  int size;
  link_->get_function(func_name, size);
  if(func_name != "midpointRadius" || size != 2)
  {
    throw InterfaceError("invalid as midpoint_radius");
  }
  mr.midpoint = receive_value();
  mr.radius = receive_value();
  return mr;
}

}
}
