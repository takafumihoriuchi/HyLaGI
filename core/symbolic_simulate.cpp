#include "ModuleSetList.h"

#include "ProgramOptions.h"
#include "SymbolicSimulator.h"
#include "SequentialSimulator.h"
#include "InteractiveSimulator.h"
#include "SymbolicOutputter.h"

// parser
#include "DefaultNodeFactory.h"

#include <boost/lexical_cast.hpp>

// namespace
using namespace hydla;
using namespace hydla::parse_tree;
using namespace hydla::parser;
using namespace hydla::ch;
using namespace hydla::symbolic_simulator;
using namespace hydla::simulator;

/**
 * 記号処理によるシミュレーション
 */
void setup_symbolic_simulator_opts(Opts& opts)
{  
  ProgramOptions &po = ProgramOptions::instance();

  /*
  if(po.get<std::string>("tm") == "n") {
    opts.time_measurement = tFmtNot;
  } else if(po.get<std::string>("tm") == "s") {
    opts.time_measurement = tFmtStd;
  } else if(po.get<std::string>("tm") == "c") {
    opts.time_measurement = tFmtCsv;
  } else {
    throw std::runtime_error(std::string("invalid option - time measurement"));
  }
  */

  opts.mathlink      = "-linkmode launch -linkname '" + po.get<std::string>("math-name") + " -mathlink'";
  opts.debug_mode    = po.get<std::string>("debug")!="";
  opts.max_time      = po.get<std::string>("time");
  opts.max_phase      = po.get<int>("phases");
  opts.nd_mode       = po.count("nd")>0;
  opts.dump_in_progress = po.count("dump-in-progress")>0;
  opts.interactive_mode = po.count("in")>0;
  opts.profile_mode  = po.count("profile")>0;
  opts.output_interval = po.get<std::string>("output-interval");
  opts.output_precision = po.get<int>("output-precision");
  opts.exclude_error = po.count("fail-stop") == 0;
  opts.solver        = po.get<std::string>("solver");
  opts.optimization_level = po.get<int>("optimization-level");
  if(opts.optimization_level < 0 || opts.optimization_level > 3){
    throw std::runtime_error(std::string("invalid option - optimization_level"));
  }
  
  
  std::string output_variables = po.get<std::string>("output-variables");
  std::string::size_type prev = 0, now;
  while( (now = output_variables.find('_', prev)) != std::string::npos){
    std::string name = output_variables.substr(prev, now-prev);
    if(now == output_variables.length()-1)break;
    now++;
    prev = output_variables.find('_', now);
    //std::cout << output_variables.substr(now, prev-now) << std::endl;
    if(prev == std::string::npos) prev = output_variables.length();
    int d_count;
    try{ d_count = boost::lexical_cast<int>(output_variables.substr(now, prev-now));
    }catch(boost::bad_lexical_cast e){
      std::cerr << "invalid option format - output variables" << std::endl;
      exit(-1);
    }
    name.append(d_count, '\'');
    opts.output_variables.insert(name);
    prev++;
  }
}

void symbolic_simulate(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree)
{
  Opts opts;
  setup_symbolic_simulator_opts(opts);

  if(opts.interactive_mode){ 
    //opts->max_time = "100";
    InteractiveSimulator ss(opts);
    ss.set_phase_simulator(new SymbolicSimulator(opts));
    ss.initialize(parse_tree);
    ss.simulate();
  }else{
    SequentialSimulator ss(opts);
    ss.set_phase_simulator(new SymbolicSimulator(opts));
    ss.initialize(parse_tree);
    hydla::output::SymbolicOutputter outputter(opts.output_variables);
    outputter.output_result_tree(ss.simulate());
  }
}

