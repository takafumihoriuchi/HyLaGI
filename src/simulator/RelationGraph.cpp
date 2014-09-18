#include "RelationGraph.h"
#include <iostream>
#include "VariableFinder.h"
#include "Logger.h"
#include "SimulateError.h"

using namespace std;

namespace hydla {
namespace simulator {


RelationGraph::RelationGraph(const module_set_t &ms)
{
  for(auto module : ms)
  {
    add(module);
  }
  ignore_prev = true;
  up_to_date = false;
}


RelationGraph::~RelationGraph()
{
  for(auto var : variable_nodes){
    delete var;
  }

  for(auto constraint : constraint_nodes){
    delete constraint;
  }
}

void RelationGraph::add(module_t &mod)
{
  current_module = mod;
  visit_mode = ADDING;
  accept(mod.second);
  up_to_date = false;
}

void RelationGraph::dump_graph(ostream & os) const
{
  os << "graph g {\n";
  os << "graph [ranksep = 1.0 ,rankdir = LR];\n";
  for(auto constraint_node : constraint_nodes) {
    string constraint_name = constraint_node->get_name();
    os << "  \"" << constraint_name << "\" [shape = box]\n";
    for(auto edge : constraint_node->edges){
      string variable_name = edge.variable_node->get_name();
      os << "  \"" 
        << constraint_name 
        << "\" -- \"" 
        << variable_name 
        << "\"";
      if(edge.ref_prev)
      {
        os << " [style = dotted]";
      }
      os <<  ";\n";
    }
  }
  os << "}" << endl;
}


void RelationGraph::dump_active_graph(ostream & os) const
{
  os << "graph g {" << endl;
  os << "graph [ranksep = 2.0 ,rankdir = LR];" << endl;
  for(auto constraint_node : constraint_nodes) {
    if(constraint_node->is_active())
    {
      string constraint_name = constraint_node->get_name();
      os << "  \"" << constraint_name;
      os << "\" [shape = box];" << endl;
      for(auto edge : constraint_node->edges){
        if(!(ignore_prev && edge.ref_prev) )
        {
          string variable_name = edge.variable_node->get_name();
          os << "  \"" << constraint_name 
             <<   "\" -- \"" 
             << variable_name
             << "\"";
          if(edge.ref_prev)
          {
            os << "[label = \"prev\"]";
          }
          os <<  ";" << endl;
        }
      }

    }
  }

  os << "}" << endl;
}


RelationGraph::EdgeToConstraint::EdgeToConstraint(ConstraintNode *cons, bool prev)
  : constraint_node(cons), ref_prev(prev){}

RelationGraph::EdgeToVariable::EdgeToVariable(VariableNode *var, bool prev)
  : variable_node(var), ref_prev(prev){}

string RelationGraph::VariableNode::get_name() const
{
  return variable.get_string();
}

string RelationGraph::ConstraintNode::get_name() const
{
  string ret = symbolic_expression::get_infix_string(constraint);
  // if too long, cut latter part
  const string::size_type max_length = 10;
  if(ret.length() > max_length)
  {
    ret = ret.substr(0, max_length) + ("...");
  }
  return ret + " (" + module.first + ")";
}

bool RelationGraph::ConstraintNode::is_active() const
{
  return expanded && module_adopted;
}


bool RelationGraph::referring(const Variable& var)
{
  if(!up_to_date) check_connected_components();
  return referred_variables.count(var) > 0;
}


void RelationGraph::initialize_node_visited()
{
  for(auto constraint_node : constraint_nodes){
    constraint_node->visited = false;
  }
}



void RelationGraph::get_related_constraints_vector(const ConstraintStore &constraint_store, vector<ConstraintStore> &constraints_vector, vector<module_set_t> &module_set_vector){
  if(!up_to_date) check_connected_components();
  initialize_node_visited();
  constraints_vector.clear();
  module_set_vector.clear();
  for(auto constraint : constraint_store)
  {
    auto constraint_it = constraint_node_map.find(constraint);
    if(constraint_it == constraint_node_map.end())
    {
      VariableFinder finder;
      finder.visit_node(constraint);
      variable_set_t variables;
      variables = finder.get_all_variable_set();
      for(auto variable : variables)
      {
        if(!variable_node_map.count(variable))continue;

        ConstraintStore connected_constraints;
        module_set_t connected_ms;
        VariableNode *var_node = variable_node_map[variable];
        variable_set_t vars;
        visit_node(var_node, connected_constraints, connected_ms, vars);
        if(connected_constraints.size() > 0)
        {
          constraints_vector.push_back(connected_constraints);
          module_set_vector.push_back(connected_ms);
        }
      }
    }
    else
    {
      ConstraintNode *constraint_node = constraint_it->second;
      if(!constraint_node->visited)
      {
        if(constraint_node->is_active())
        {
          ConstraintStore connected_constraints;
          module_set_t connected_ms;
          variable_set_t vars;
          visit_node(constraint_node, connected_constraints, connected_ms, vars);
          constraints_vector.push_back(connected_constraints);
          module_set_vector.push_back(connected_ms);
        }
        else
        {
          // adjacent node may be active (this case is mainly caused by negative asks)
          ConstraintStore connected_constraints;
          module_set_t connected_ms;
          variable_set_t vars;
          for(auto edge : constraint_node->edges)
          {
            if(!ignore_prev || !edge.ref_prev) visit_node(edge.variable_node, connected_constraints, connected_ms, vars);
          }
          if(connected_constraints.size() > 0)
          {
            constraints_vector.push_back(connected_constraints);
            module_set_vector.push_back(connected_ms);
          }
        }
      }
    }
  }
}


void RelationGraph::get_related_constraints(constraint_t constraint, ConstraintStore &constraints, module_set_t &module_set){
  if(!up_to_date) check_connected_components();
  initialize_node_visited();
  constraints.clear();
  module_set.clear();
  auto constraint_it = constraint_node_map.find(constraint);
  if(constraint_it == constraint_node_map.end())
  {
    VariableFinder finder;
    finder.visit_node(constraint);
    variable_set_t variables;
    variables = finder.get_all_variable_set();
    for(auto variable : variables)
    {
      if(!variable_node_map.count(variable))continue;
      VariableNode *var_node = variable_node_map[variable];
      variable_set_t vars;
      visit_node(var_node, constraints, module_set, vars);
    }
  }
  else
  {
    variable_set_t vars;
    ConstraintNode *constraint_node = constraint_it->second;
    visit_node(constraint_node, constraints, module_set, vars);
  }
}


void RelationGraph::get_related_constraints(const Variable &var, ConstraintStore &constraints, module_set_t &module_set){
  if(!up_to_date) check_connected_components();
  initialize_node_visited();
  constraints.clear();
  module_set.clear();
  if(!variable_node_map.count(var))return;
  VariableNode *var_node = variable_node_map[var];
  if(var_node == nullptr)throw HYDLA_SIMULATE_ERROR("VariableNode is not found");
  variable_set_t vars;
  visit_node(var_node, constraints, module_set, vars);
}


void RelationGraph::check_connected_components(){
  connected_constraints_vector.clear();
  connected_modules_vector.clear();
  connected_variables_vector.clear();
  referred_variables.clear();
  initialize_node_visited();

  for(auto constraint_node : constraint_nodes){
    module_set_t ms;
    ConstraintStore constraints;
    variable_set_t vars;
    if(!constraint_node->visited && constraint_node->is_active()){
      visit_node(constraint_node, constraints, ms, vars);
      connected_constraints_vector.push_back(constraints);
      connected_modules_vector.push_back(ms);
      connected_variables_vector.push_back(vars);
      referred_variables.insert(vars.begin(), vars.end());
    }
  }
  up_to_date = true;
}

void RelationGraph::visit_node(ConstraintNode* node, ConstraintStore &constraints, module_set_t &ms, variable_set_t &vars){
  node->visited = true;
  ms.add_module(node->module);
  constraints.add_constraint(node->constraint);
  for(auto edge : node->edges)
  {
    if(!ignore_prev || !edge.ref_prev) visit_node(edge.variable_node, constraints, ms, vars);
  }
}

void RelationGraph::visit_node(VariableNode* node, ConstraintStore &constraints, module_set_t &ms, variable_set_t &vars){
  vars.insert(node->variable);
  for(auto edge : node->edges)
  {
    if( (!ignore_prev || !edge.ref_prev)
        && !edge.constraint_node->visited && edge.constraint_node->is_active()) visit_node(edge.constraint_node, constraints, ms, vars);
  }
}

int RelationGraph::get_connected_count()
{
  if(!up_to_date) check_connected_components();
  return connected_constraints_vector.size();
}

void RelationGraph::set_adopted(const module_t &mod, bool adopted)
{
  if(!module_constraint_nodes_map.count(mod))throw HYDLA_SIMULATE_ERROR("module " + mod.first + " is not found");
  for(auto constraint_node : module_constraint_nodes_map[mod])
  {
    if(adopted != constraint_node->module_adopted)
    {
      constraint_node->module_adopted = adopted;
    }
  }
  up_to_date = false;
}

void RelationGraph::set_adopted(const module_set_t &ms, bool adopted)
{
  for(auto module : ms)
  {
    set_adopted(module, adopted);
  }
}


void RelationGraph::set_expanded(constraint_t cons, bool expanded)
{
  visit_mode = expanded?EXPANDING:UNEXPANDING;
  accept(cons);
  up_to_date = false;
}


void RelationGraph::set_expanded_all(bool expanded)
{
  for(auto node : constraint_nodes)
  {
    if(expanded != node->expanded)
    {
      node->expanded = expanded;
    }
  }
  up_to_date = false;
}

RelationGraph::variable_set_t RelationGraph::get_variables(unsigned int index)
{
  if(!up_to_date) check_connected_components();
  if(index >= connected_variables_vector.size())throw HYDLA_SIMULATE_ERROR("index is out of range");
  return connected_variables_vector[index];
}

ConstraintStore RelationGraph::get_constraints(unsigned int index)
{
  if(!up_to_date) check_connected_components();
  if(index >= connected_constraints_vector.size())throw HYDLA_SIMULATE_ERROR("index is out of range");
  return connected_constraints_vector[index];
}

ConstraintStore RelationGraph::get_constraints()
{
  if(!up_to_date) check_connected_components();
  ConstraintStore constraints;
  for(auto constraint_node : constraint_nodes)
  {
    if(constraint_node->is_active())
    {
      constraints.add_constraint(constraint_node->constraint);
    }
  }
  return constraints;
}

ConstraintStore RelationGraph::get_expanded_constraints()
{
  if(!up_to_date) check_connected_components();
  ConstraintStore constraints;
  for(auto constraint_node : constraint_nodes)
  {
    if(constraint_node->expanded)
    {
      constraints.add_constraint(constraint_node->constraint);
    }
  }
  return constraints;
}

ConstraintStore RelationGraph::get_adopted_constraints()
{
  if(!up_to_date) check_connected_components();
  ConstraintStore constraints;
  for(auto constraint_node : constraint_nodes)
  {
    if(constraint_node->module_adopted)
    {
      constraints.add_constraint(constraint_node->constraint);
    }
  }
  return constraints;
}

RelationGraph::module_set_t RelationGraph::get_modules(unsigned int index)
{
  if(!up_to_date) check_connected_components();
  if(index >= connected_modules_vector.size())throw HYDLA_SIMULATE_ERROR("index is out of range");
  return connected_modules_vector[index];
}

void RelationGraph::set_ignore_prev(bool ignore)
{
  ignore_prev = ignore;
  up_to_date = false;
}

void RelationGraph::visit_atomic_constraint(boost::shared_ptr<symbolic_expression::BinaryNode> node)
{
  if(visit_mode == ADDING)
  {
    VariableFinder finder;
    finder.visit_node(node);
    variable_set_t variables;
    
    ConstraintNode* cons;
    if(constraint_node_map.count(node))
    {
      cons = constraint_node_map[node];
    }
    else
    {
      cons = new ConstraintNode(node, current_module);
      constraint_nodes.push_back(cons);
      constraint_node_map[node] = cons;
      module_constraint_nodes_map[current_module].push_back(cons);
    }

    variables = finder.get_variable_set();
    for(auto variable : variables)
    {
      VariableNode* var_node = add_variable_node(variable);
      cons->edges.push_back(EdgeToVariable(var_node, false));
      var_node->edges.push_back(EdgeToConstraint(cons, false));
    }

    variable_set_t prev_variables;
    prev_variables = finder.get_prev_variable_set();
    for(auto variable : prev_variables)
    {
      if(variables.count(variable))continue;
      VariableNode* var_node = add_variable_node(variable);
      cons->edges.push_back(EdgeToVariable(var_node, true));
      var_node->edges.push_back(EdgeToConstraint(cons, true));
    }
  }
  else if(visit_mode == EXPANDING)
  {
    auto constraint_node_it = constraint_node_map.find(node);
    if(constraint_node_it != constraint_node_map.end())
    {
      ConstraintNode* constraint_node = constraint_node_it->second;
      if(!constraint_node->expanded)
      {
        constraint_node->expanded = true;
      }
    }
    else HYDLA_LOGGER_WARN("(@RelationGraph) try to expand unknown node: ", get_infix_string(node));
  }
  else if(visit_mode == UNEXPANDING)
  {

    auto constraint_node_it = constraint_node_map.find(node);
    if(constraint_node_it != constraint_node_map.end())
    {
      ConstraintNode* constraint_node = constraint_node_it->second;
      if(constraint_node->expanded)
      {
        constraint_node->expanded = false;
      }
    }
    else HYDLA_LOGGER_WARN("(@RelationGraph) try to unexpand unknown node: ", get_infix_string(node));
  }
}

RelationGraph::VariableNode* RelationGraph::add_variable_node(Variable &var)
{
  if(variable_node_map.count(var))
  {
    return variable_node_map[var];
  }
  else
  {
    VariableNode* ret = new VariableNode(var);
    variable_nodes.push_back(ret);
    variable_node_map[var] = ret;
    return ret;
  }
}


void RelationGraph::visit(boost::shared_ptr<symbolic_expression::Ask> node)
{
  if(visit_mode == ADDING)
  {
    accept(node->get_rhs());
  }
}



} //namespace simulator
} //namespace hydla 
