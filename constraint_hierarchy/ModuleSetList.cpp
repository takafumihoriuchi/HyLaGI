#include "ModuleSetList.h"

#include <iostream>
#include <algorithm>
#include <boost/lambda/lambda.hpp>

using namespace boost::lambda;

namespace hydla {
namespace ch {

class ModuleSetComparator {
public:
  bool operator()(const module_set_sptr &lhs, const module_set_sptr &rhs) 
  {
    return lhs->compare(*rhs) > 0;
  }
};

ModuleSetList::ModuleSetList()
{}

ModuleSetList::ModuleSetList(module_set_sptr m) :
  module_set_list_(1, m)
{}

ModuleSetList::~ModuleSetList()
{}

void ModuleSetList::add_parallel(ModuleSetList& parallel_module_set_list) 
{
  // parallel(X, Y) = X �� Y �� {x �� y | x��X, y��Y}

  module_set_list_t::const_iterator p_it = 
    parallel_module_set_list.module_set_list_.begin();
  module_set_list_t::const_iterator p_end = 
    parallel_module_set_list.module_set_list_.end();

  // X
  module_set_list_t new_list(module_set_list_);
    
  // Y
  new_list.insert(new_list.end(), p_it, p_end);

  // {x �� y | x��X, y��Y}
  for(; p_it!=p_end; ++p_it) {
    module_set_list_t::iterator this_it =  module_set_list_.begin();
    module_set_list_t::iterator this_end = module_set_list_.end();
    
    for(; this_it!=this_end; ++this_it) {
      module_set_sptr ms(new ModuleSet(**this_it,  **p_it));
      new_list.push_back(ms);
    }
  }

  sort(new_list.begin(), new_list.end(), ModuleSetComparator());

  module_set_list_.swap(new_list);
}

void ModuleSetList::add_weak(ModuleSetList& weak_module_set_list) 
{
  // ordered(X, Y) = Y �� {x �� y | x��X, y��Y}
      
  // Y
  module_set_list_t new_list(module_set_list_);

  // {x �� y | x��X, y��Y}
  module_set_list_t::const_iterator p_it = 
    weak_module_set_list.module_set_list_.begin();
  module_set_list_t::const_iterator p_end = 
    weak_module_set_list.module_set_list_.end();

  for(; p_it!=p_end; ++p_it) {
    module_set_list_t::iterator this_it =  module_set_list_.begin();
    module_set_list_t::iterator this_end = module_set_list_.end();
  
    for(; this_it!=this_end; ++this_it) {
      module_set_sptr ms(new ModuleSet(**this_it,  **p_it));
      new_list.push_back(ms);
    }
  }

  sort(new_list.begin(), new_list.end(), ModuleSetComparator());

  module_set_list_.swap(new_list);
}

std::ostream& ModuleSetList::dump(std::ostream& s)
{
  module_set_list_t::iterator it = module_set_list_.begin();
  module_set_list_t::iterator end = module_set_list_.end();
  
  s << "{";
  if(it!=end) s << **(it++);
  while(it!=end) {
    s << ", " << **(it++);
  }
  s << "}";
  return s;
}

bool ModuleSetList::dispatch(ModuleSetTester* tester, int threads)
{
  module_set_list_t::iterator it  = module_set_list_.begin();
  module_set_list_t::iterator end = module_set_list_.end();
  while(it!=end) {
    if(tester->test_module_set(*it++)) return true;
  }
  return false;
}


std::ostream& operator<<(std::ostream& s, ModuleSetList& m)
{
  return m.dump(s);
}

} // namespace ch
} // namespace hydla
