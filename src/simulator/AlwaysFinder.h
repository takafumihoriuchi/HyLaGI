#pragma once

#include "Node.h"
#include "TreeVisitorForAtomicConstraint.h"
#include "PhaseResult.h"

namespace hydla {
namespace simulator {

class AlwaysFinder : public symbolic_expression::TreeVisitorForAtomicConstraint
{
public:
	AlwaysFinder();

	virtual ~AlwaysFinder();
	
	/** 
	 * find all always included by this node
	 */
	void find_always(symbolic_expression::node_sptr node, always_set_t* always_set, ConstraintStore* non_al, asks_t* pos, asks_t *nonal_asks);

	using TreeVisitorForAtomicConstraint::visit; // suppress warnings
	virtual void visit(boost::shared_ptr<symbolic_expression::Ask> node);
	// Always
	virtual void visit(boost::shared_ptr<symbolic_expression::Always> node);

	virtual void visit_atomic_constraint(boost::shared_ptr<symbolic_expression::Node> node);

private:
	always_set_t *always_set;
	ConstraintStore *non_always;
	asks_t           *positive_asks;
	asks_t           *nonalways_asks;
};

} //namespace simulator
} //namespace hydla 
