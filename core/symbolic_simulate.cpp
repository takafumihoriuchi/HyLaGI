#include "ModuleSetList.h"

#include "ProgramOptions.h"
#include "SymbolicSimulator.h"

// parser
#include "DefaultNodeFactory.h"

// namespace
using namespace hydla;
using namespace hydla::parse_tree;
using namespace hydla::parser;
using namespace hydla::ch;
using namespace hydla::symbolic_simulator;

/**
 * 記号処理によるシミュレーション
 */
void setup_symbolic_simulator_opts(Opts& opts)
{  
  ProgramOptions &po = ProgramOptions::instance();

  if(po.get<std::string>("output-format") == "t") {
    opts.output_format = fmtTFunction;
  } else if(po.get<std::string>("output-format") == "n"){
    opts.output_format = fmtNumeric; 
  } else {
    // TODO: 例外を投げるようにする
    std::cerr << "invalid option - output format" << std::endl;
    exit(-1);
  }

  opts.mathlink      = po.get<std::string>("mathlink");
  opts.debug_mode    = po.count("debug")>0||po.get<std::string>("area")!="";
  opts.max_time      = po.get<std::string>("time");
  opts.max_step      = po.get<int>("step");
  opts.nd_mode       = po.count("nd")>0;
  opts.dump_in_progress = po.count("dump-in-progress")>0;
  opts.interactive_mode = po.count("in")>0;
  opts.profile_mode  = po.count("profile")>0;
  opts.parallel_mode = po.count("parallel")>0;
  opts.output_interval = po.get<std::string>("output-interval");
  opts.output_precision = po.get<int>("output-precision");
  opts.approx_precision = po.get<int>("approx");
  opts.solver        = po.get<std::string>("solver");
}

void symbolic_simulate(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree)
{
  Opts opts;
  setup_symbolic_simulator_opts(opts);

  SymbolicSimulator ms(opts);
  ms.initialize(parse_tree);
  ms.simulate();
}

