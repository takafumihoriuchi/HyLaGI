#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_CONSTRAINT_STORE_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_CONSTRAINT_STORE_H_

#include <set>
#include "realpaverbasic.h"
#include <boost/bimap/bimap.hpp>

#include "BPTime.h"
#include "BPTypes.h"

namespace hydla {
namespace bp_simulator {

//typedef boost::bimaps::bimap<std::string, int> var_name_map_t;

/**
 * ����X�g�A
 */
class ConstraintStore
{
public:
  ConstraintStore(bool debug_mode = false);
  ConstraintStore(const ConstraintStore& src);
  ~ConstraintStore();
  void build(const variable_map_t& variable_map);
  void build_variable_map(variable_map_t& variable_map) const;
  std::set<rp_constraint> get_store_exprs_copy() const;
  void add_constraint(rp_constraint c, const var_name_map_t& vars);
  void add_constraint(std::set<rp_constraint>::iterator start, std::set<rp_constraint>::iterator end, const var_name_map_t& vars);
  const var_name_map_t& get_store_vars() const
  {
    return this->vars_;
  }
  void display(const int digits) const;

private:
  rp_vector_variable to_rp_vector() const;
  std::set<rp_constraint> exprs_;
  var_name_map_t vars_;
  bool debug_mode_;
};

} // namespace bp_simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_BP_SIMULATOR_CONSTRAINT_STORE_H_