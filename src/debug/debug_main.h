#pragma once

#include "Node.h"
#include "ParseTree.h"


// constraint hierarchy
#include "ModuleSetContainerCreator.h"
#include "IncrementalModuleSet.h"

#include "ConstraintStore.h"

#include "solve_sym.h"

namespace hydla { 
  namespace debug {
    class Debug{
    public:
      Debug(); 
      Debug(const Debug& db); 
      /**
       * 構文木を JSON 形式で出力する
       */
      static void dump_debug(Debug *db, std::string string, boost::shared_ptr<parse_tree::ParseTree> pt, boost::shared_ptr<hierarchy::IncrementalModuleSet> msc);
      static void debug_call(std::vector<std::vector<std::vector<std::string>>> constraint_map, std::vector<std::vector<std::string>> v_map, std::vector<std::vector<std::string>> h_map);

    };
  }
}
