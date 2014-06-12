#include "JsonWriter.h"
#include "Logger.h"
#include <fstream>
#include "Utility.h"
#include "Constants.h"

using namespace std;
using namespace picojson;
using namespace hydla::utility;

namespace hydla{
namespace io{

void JsonWriter::write(const simulator_t &simulator, const std::string &name)
{
  object json_object;
  json_object["variables"] = for_vs(simulator.get_variable_set());
  json_object["parameters"] = for_pm(simulator.get_parameter_map());

  phase_result_const_sptr_t root = simulator.get_result_root();
  picojson::array children;
  json_object["first_phases"] = make_children(root);

  value json(json_object);
 
  std::ofstream ofs;
  ofs.open(name.c_str());
  ofs << json.serialize();
  ofs.close(); 
}

void JsonWriter::write_phase(const phase_result_const_sptr_t &phase, const std::string &name)
{
  std::ofstream ofs;
  ofs.open(name.c_str());
  ofs << for_phase(phase).serialize();
  ofs.close();  
}

value JsonWriter::for_phase(const phase_result_const_sptr_t &phase)
{
  //TODO: positive_asksとかnegative_asksとかも書く
  object phase_object;
  phase_object["id"] = value((long)phase->id);
  if(phase->phase_type == simulator::PointPhase)
  {
    phase_object["type"] = value(string("PP"));
    object time_object;
    time_object["time_point"] =
      value(phase->current_time.get_string());
    phase_object["time"] = value(time_object);
  }
  else if(phase->phase_type == simulator::IntervalPhase)
  {
    phase_object["type"] = value(string("IP"));

    object time_object;
    time_object["start_time"] = 
      value(phase->current_time.get_string());
    if(!phase->end_time.undefined())
    {
      time_object["end_time"] = 
        value(phase->end_time.get_string());
    }
    phase_object["time"] = value(time_object);
  }
  phase_object["variable_map"] = for_vm(phase->variable_map);
  phase_object["parameter_map"] = for_pm(phase->parameter_map);
  phase_object["children"] = make_children(phase);
  if(phase->children.size() == 0){
    phase_object["cause_for_termination"] = value(get_string_for_cause(phase->cause_for_termination));
  }
  return value(phase_object);
}


value JsonWriter::make_children(const phase_result_const_sptr_t &phase)
{
  picojson::array children;
  for(vector<phase_result_sptr_t>::const_iterator it = phase->children.begin(); it != phase->children.end(); it++)
  {
    children.push_back(for_phase(*it));
  }
  return value(children);
}

value JsonWriter::for_range(const value_range_t &range)
{
  object range_object;

  if(range.unique())
  {
    range_object["unique_value"] = value(range.get_unique_value().get_string());
  }
  else
  {
    picojson::array lbs;
    for(uint i = 0; i < range.get_lower_cnt(); i++)
    {
      const value_range_t::bound_t &bound = range.get_lower_bound(i);
      object lb;
      lb["value"] = value(bound.value.get_string());
      lb["closed"] = value(bound.include_bound);
      lbs.push_back(value(lb));
    }
    range_object["lower_bounds"] = value(lbs);
 
    picojson::array ubs;
    for(uint i = 0; i < range.get_upper_cnt(); i++)
    {
      const value_range_t::bound_t &bound = range.get_upper_bound(i);
      object ub;
      ub["value"] = value(bound.value.get_string());
      ub["closed"] = value(bound.include_bound);
      ubs.push_back(value(ub));
    }
    range_object["upper_bounds"] = value(ubs);
  }
  return value(range_object);
}


value JsonWriter::for_vm(const variable_map_t &vm)
{
  object vm_obj;
  for(variable_map_t::const_iterator it = vm.begin(); it != vm.end(); it++)
  {
    const std::string &key = it->first.get_string();
    const value_range_t &range = it->second;
    vm_obj[key] = for_range(range);
  }
  return value(vm_obj);
}

value JsonWriter::for_vs(const variable_set_t &vs)
{
  picojson::array vs_array;
  for(variable_set_t::const_iterator it = vs.begin(); it != vs.end(); it++)
  {
    vs_array.push_back(value(it->get_string()));
  }
  return value(vs_array);
}

value JsonWriter::for_pm(const parameter_map_t &pm)
{
  object pm_obj;
  for(parameter_map_t::const_iterator it = pm.begin(); it != pm.end(); it++)
  {
    std::string key = it->first.to_string();
    pm_obj[key] = for_range(it->second);
  }
  return value(pm_obj);
}


}
}