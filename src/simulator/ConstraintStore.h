#ifndef _INCLUDED_HYDLA_SIMULATOR_CONSTRAINT_STORE_H_
#define _INCLUDED_HYDLA_SIMULATOR_CONSTRAINT_STORE_H_

#include "Node.h"
#include <vector>

namespace hydla{
namespace simulator{

typedef parse_tree::node_sptr constraint_t;
typedef std::vector<constraint_t> constraints_t;

class ConstraintStore
{
public:
  ConstraintStore();
  constraints_t::iterator begin();
  constraints_t::iterator end();

  constraints_t::const_iterator begin()const ;
  constraints_t::const_iterator end()const ;

  void add_constraint(const constraint_t &constraint);
  void add_constraint_store(const ConstraintStore &store);

  void clear();
  size_t size()const;

  bool consistent() const;
  // return if this constraint store is always true
  bool valid() const;
  void set_consistency(bool);
private:
  bool is_consistent;
  constraints_t constraints;
};

std::ostream &operator<<(std::ostream &ost, const ConstraintStore &store);

}
}

#endif // include guard