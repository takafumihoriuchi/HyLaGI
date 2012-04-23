#ifndef _INCLUDED_HYDLA_SIMULATOR_TYPES_H_
#define _INCLUDED_HYDLA_SIMULATOR_TYPES_H_

#include <vector>
#include <set>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "ModuleSet.h"
#include "ModuleSetContainer.h"

namespace hydla {
namespace simulator {

/**
 * 処理のフェーズ
 */
typedef enum Phase_ {
  PointPhase,
  IntervalPhase,
} Phase;

/**
 * 変化したask制約の状態
 */
typedef enum AskState_ {
  Positive2Negative,
  Negative2Positive,
} AskState;

/**
 * 
 */
typedef enum{
  TIME_LIMIT,
  SOME_ERROR,
  INCONSISTENCY,
  ASSERTION,
  NONE
}CauseOfTermination;

typedef hydla::parse_tree::node_id_t                      node_id_t;
typedef boost::shared_ptr<hydla::ch::ModuleSet>           module_set_sptr;
typedef hydla::ch::ModuleSetContainer                     module_set_container_t;
typedef boost::shared_ptr<module_set_container_t>  module_set_container_sptr;
typedef hydla::ch::ModuleSetContainer::module_set_list_t  module_set_list_t;

typedef std::vector<boost::shared_ptr<hydla::parse_tree::Node> > constraints_t;
typedef std::vector<boost::shared_ptr<hydla::parse_tree::Tell> > tells_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Tell> >    collected_tells_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Always> >  expanded_always_t;
typedef std::vector<node_id_t>                                   expanded_always_id_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Ask> >     ask_set_t;
typedef ask_set_t                                                positive_asks_t;
typedef ask_set_t                                                negative_asks_t;
typedef std::vector<std::pair<AskState, node_id_t> >             changed_asks_t;
typedef std::vector<tells_t>                                     not_adopted_tells_list_t;
typedef std::map<std::string, int>                               continuity_map_t;


std::ostream& operator<<(std::ostream& s, const ask_set_t& a);
std::ostream& operator<<(std::ostream& s, const tells_t& a);
std::ostream& operator<<(std::ostream& s, const collected_tells_t& a);
std::ostream& operator<<(std::ostream& s, const expanded_always_t& a);
std::ostream& operator<<(std::ostream& s, const expanded_always_id_t& a);
std::ostream& operator<<(std::ostream& s, const continuity_map_t& continuity_map);

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_TYPES_H_

