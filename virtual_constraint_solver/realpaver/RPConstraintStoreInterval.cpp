#include "RPConstraintStoreInterval.h"
#include "Logger.h"
#include "realpaver.h"
#include "rp_problem_ext.h"
#include "rp_constraint_ext.h"
#include "RPConstraintSolver.h"

#include <boost/lexical_cast.hpp>

namespace hydla {
namespace vcs {
namespace realpaver {

ConstraintStoreInterval::ConstraintStoreInterval()
{}

ConstraintStoreInterval::ConstraintStoreInterval(const ConstraintStoreInterval& src)
{}

ConstraintStoreInterval::~ConstraintStoreInterval()
{}

ConstraintStoreInterval& ConstraintStoreInterval::operator=(const ConstraintStoreInterval& src)
{
  return *this;
}

void ConstraintStoreInterval::build(const virtual_constraint_solver_t::variable_map_t& variable_map)
{
  typedef var_name_map_t::value_type vars_type_t;
  virtual_constraint_solver_t::variable_map_t::const_iterator it;
  for(it=variable_map.begin(); it!=variable_map.end(); it++) {
    // 変数名を作る ex. "usrVar0ht" "initValue0ht"
    std::string name(var_prefix);
    //std::string name(it->first.name);
    name += boost::lexical_cast<std::string>(it->first.derivative_count);
    name += it->first.name;
    //for(int i=it->first.derivative_count; i>0; i--) name += BP_DERIV_STR;
    std::string ini_name(init_prefix);
    ini_name += boost::lexical_cast<std::string>(it->first.derivative_count);
    ini_name += it->first.name;
    //ini_name += BP_INITIAL_STR;
    // 表に登録
    unsigned int size = this->vars_.size();
    var_property vp(it->first.derivative_count, false),
      vp_p(it->first.derivative_count, true);
    this->vars_.insert(vars_type_t(name, size, vp)); // 登録済みの変数は変更されない
    this->vars_.insert(vars_type_t(ini_name, size+1, vp_p));
    // rp_intervalからrp_constraintを作る
    rp_interval i;
    it->second.get(rp_binf(i), rp_bsup(i));
    // (-oo, +oo) ==> 作らない，登録しない
    if(rp_binf(i)==-RP_INFINITY && rp_bsup(i)==RP_INFINITY) continue;
    if(rp_interval_point(i)) {
      // rp_interval_point ==> 式1つ登録(var = val)
      rp_erep l, r;
      rp_ctr_num cnum;
      rp_constraint c;
      rp_erep_create_var(&l, this->vars_.left.at(ini_name));
      rp_erep_create_cst(&r, "", i);
      rp_ctr_num_create(&cnum, &l, RP_RELATION_EQUAL, &r);
      rp_constraint_create_num(&c, cnum);
      this->exprs_.insert(c);
    } else {
      // else ==> 式2つ登録(inf <= var, var <= sup)
      rp_interval i_tmp;
      rp_erep l, r;
      rp_ctr_num cnum;
      rp_constraint c;
      rp_erep_create_var(&l, this->vars_.left.at(ini_name));
      rp_interval_set_point(i_tmp, rp_binf(i));
      rp_erep_create_cst(&r, "", i_tmp);
      rp_ctr_num_create(&cnum, &l, RP_RELATION_SUPEQUAL, &r);
      rp_constraint_create_num(&c, cnum);
      this->exprs_.insert(c);
      rp_erep_create_var(&l, this->vars_.left.at(ini_name));
      rp_interval_set_point(i_tmp, rp_bsup(i));
      rp_erep_create_cst(&r, "", i_tmp);
      rp_ctr_num_create(&cnum, &l, RP_RELATION_INFEQUAL, &r);
      rp_constraint_create_num(&c, cnum);
      this->exprs_.insert(c);
    }
  }
  unsigned int size = this->vars_.size();
  var_property vp(0, false);
  this->vars_.insert(vars_type_t("t", size, vp)); // 登録済みの変数は変更されない
}

// 消す予定
void ConstraintStoreInterval::build_variable_map(virtual_constraint_solver_t::variable_map_t& variable_map) const
{}

//std::set<rp_constraint> ConstraintStoreInterval::get_store_exprs_copy() const
//{}

//void ConstraintStoreInterval::add_constraint(rp_constraint c, const var_name_map_t& vars)
//{}

//void ConstraintStoreInterval::add_constraint(std::set<rp_constraint>::iterator start, std::set<rp_constraint>::iterator end, const var_name_map_t& vars)
//{}

std::ostream& ConstraintStoreInterval::dump_cs(std::ostream& s) const
{
  rp_vector_variable vec = ConstraintSolver::create_rp_vector(this->vars_);
  std::set<rp_constraint>::const_iterator ctr_it = this->exprs_.begin();
  while(ctr_it != this->exprs_.end()){
    rp::dump_constraint(s, *ctr_it, vec); // digits, mode);
    s << "\n";
    ctr_it++;
  }
  s << "\n";
  rp_vector_destroy(&vec);
  return s;
}

} // namespace realpaver
} // namespace vcs
} // namespace hydla 
