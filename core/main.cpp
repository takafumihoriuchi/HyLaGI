#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include <fstream>

#ifdef _MSC_VER
#include <windows.h>
#endif

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

// core
#include "version.h"
#include "ProgramOptions.h"

// common
#include "Logger.h"

// constraint hierarchy
#include "ModuleSetContainerCreator.h"
#include "ModuleSetList.h"
#include "ModuleSetGraph.h"

// parser
#include "DefaultNodeFactory.h"

// namespace
using namespace boost;
using namespace hydla;
using namespace hydla::logger;
using namespace hydla::parser;
using namespace hydla::parse_tree;
using namespace hydla::ch;

// prototype declarations
int main(int argc, char* argv[]);
void hydla_main(int argc, char* argv[]);
void symbolic_simulate(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree);
void symbolic_legacy_simulate(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree);
void branch_and_prune_simulate(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree);
bool dump(boost::shared_ptr<ParseTree> pt);

//
int main(int argc, char* argv[]) 
{
#ifdef _MSC_VER
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  int ret = 0;

  try {
    hydla_main(argc, argv);
  }
  catch(std::exception &e) {
    std::cerr << "error : " << e.what() << std::endl;
    ret = -1;
  } 
#if !(defined(_MSC_VER) && defined(_DEBUG))
  catch(...) {
    std::cerr << "fatal error!!" << std::endl;
    ret = -1;
  }
#else
    system("pause");
#endif

  return ret;
}

void hydla_main(int argc, char* argv[])
{
  ProgramOptions &po = ProgramOptions::instance();
  po.parse(argc, argv);
  
    if(po.get<std::string>("ar") == "test") {		//局部的出力entail
	  Logger::enflag=5; //テスト用に常に出力するものを作る
	Logger::instance().set_log_level(Logger::Area);
	} else/* if(po.get<std::string>("ar") == "") {		//局部的出力entail
	  //Logger::flag=2;
	//Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "") {		//局部的出力entail
	  //Logger::flag=3;
	//Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "") {		//局部的出力entail
	  //Logger::flag=3;
	//Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "") {		//局部的出力entail
	  //Logger::flag=3;
	//Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "") {		//局部的出力entail
	  //Logger::flag=3;
	//Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "") {		//局部的出力entail
	  //Logger::flag=3;
	//Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "") {		//局部的出力entail
	  //Logger::flag=3;
	//Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "") {		//局部的出力entail
	  //Logger::flag=3;
	//Logger::instance().set_log_level(Logger::Area);
	} else */if(po.get<std::string>("ar") == "const_1") {		//局部的出力entail
	  Logger::constflag=1;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "const_2") {		//局部的出力entail
	  Logger::constflag=2;
	Logger::instance().set_log_level(Logger::Area);
	//} else if(po.get<std::string>("ar") == "") {		//局部的出力entail
	  //Logger::flag=3;
	//Logger::instance().set_log_level(Logger::Area);
	//} else if(po.get<std::string>("ar") == "") {		//局部的出力entail
	  //Logger::flag=3;
	//Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "var_1") {		//局部的出力entail
	  Logger::varflag=1;
	Logger::instance().set_log_level(Logger::Area);
	//} else if(po.get<std::string>("ar") == "") {		//局部的出力entail
	  //Logger::flag=3;
	//Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "pt_p5") {		//局部的出力entail
	  Logger::ptflag=5;
	Logger::instance().set_log_level(Logger::Area);
    } else if(po.get<std::string>("ar") == "parse_tree"||po.get<std::string>("ar") == "pt") {		//parse_tree
	  Logger::ptflag=0;
	Logger::instance().set_log_level(Logger::Area);
    } else if(po.get<std::string>("ar") == "ent_e1") {			//局部的出力entail_pp_
	  Logger::enflag=1;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "ent_e2") {			//局部的出力entail_??_
	  Logger::enflag=2;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "ent_e3") {			//局部的出力entail_ip_
	  Logger::enflag=3;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "ent_e4") {			//局部的出力entail_??_
	  Logger::enflag=4;
	Logger::instance().set_log_level(Logger::Area);
    } else if(po.get<std::string>("ar") == "ent_e5") {			//局部的出力entail_??_
	  Logger::enflag=5;
	Logger::instance().set_log_level(Logger::Area);
    } else if(po.get<std::string>("ar") == "entail"||po.get<std::string>("ar") == "ent") {	//局部的出力entail判定
	  Logger::enflag=0;
	Logger::instance().set_log_level(Logger::Area);
    } else if(po.get<std::string>("ar") == "inco"||po.get<std::string>("ar") == "inconsistent") {	//局部的出力無矛盾性判定
	  Logger::conflag=0;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "inco_c1") {		//局部的出力inconsistent_pp_
	  Logger::conflag=1;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "inco_c2") {		//局部的出力inconsistent_ip_
	  Logger::conflag=2;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "inco_c3") {		//局部的出力inconsistent_??_
	  Logger::conflag=3;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "inco_c4") {		//局部的出力inconsistent_??_
	  Logger::conflag=4;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "inco_ent_"||po.get<std::string>("ar") == "ent_inco_") {		//局部的出力inconsistent
	  Logger::conflag=0;Logger::enflag=0;
	Logger::instance().set_log_level(Logger::Area);
	}else if(po.count("debug")>0) {
    Logger::instance().set_log_level(Logger::Debug);
  } else if(po.count("comprehensive")>0){					//追加予定の大局的出力モード
	Logger::instance().set_log_level(Logger::Summary);
  } else {
    Logger::instance().set_log_level(Logger::Warn);
  }
	
  if(po.count("help")) {
    po.help_msg(std::cout);
    return;
  }

  if(po.count("version")) {
    std::cout << Version::description() << std::endl;
    return;
  }

  // ParseTreeの構築
  boost::shared_ptr<ParseTree> pt(new ParseTree);
  if(po.count("input-file")) {
    std::string filename(po.get<std::string>("input-file"));
    std::ifstream in(filename.c_str());
    if (!in) {    
      throw std::runtime_error(std::string("cannot open \"") + filename + "\"");
    }
    pt->parse<DefaultNodeFactory>(in);
  } else {
    pt->parse<DefaultNodeFactory>(std::cin);
  }
  
  // いろいろと表示
  if(dump(pt)) {
    return;
  }
  
  // シミュレーション開始
  std::string method(po.get<std::string>("method"));
  if(method == "s" || method == "SymbolicSimulator") {
    symbolic_simulate(pt);
  } 
  else if(method == "b" || method == "BandPSimulator") {
    branch_and_prune_simulate(pt);
  } 
  else if(method == "l" || method == "SymbolicLegacySimulator") {
    symbolic_legacy_simulate(pt);
  } 
  else {
    // TODO: 例外を投げるようにする
    std::cerr << "invalid method" << std::endl;
    return;
  }
}

bool dump(boost::shared_ptr<ParseTree> pt)
{
  ProgramOptions &po = ProgramOptions::instance();

  if(po.count("dump-parse-tree")>0) {
    pt->to_graphviz(std::cout);
    return true;
  }    

  if(po.count("dump-module-set-list")>0) {
    ModuleSetContainerCreator<ModuleSetList> mcc;
    boost::shared_ptr<ModuleSetList> msc(mcc.create(pt));
    msc->dump_node_names(std::cout);
    return true;
  }

//   if(po.count("dump-module-set-list-noinit")>0) {
//     ModuleSetContainerCreator<ModuleSetList> mcc;
//     boost::shared_ptr<ModuleSetList> msc(mcc.create(pt_no_init_node));
//     msc->dump_node_names(std::cout);
//     return true;
//   }

  if(po.count("dump-module-set-graph")>0) {
    ModuleSetContainerCreator<ModuleSetGraph> mcc;
    boost::shared_ptr<ModuleSetGraph> msc(mcc.create(pt));
    msc->dump_graphviz(std::cout);
    return true;
  }

//   if(po.count("dump-module-set-graph-noinit")>0) {
//     ModuleSetContainerCreator<ModuleSetGraph> mcc;
//     boost::shared_ptr<ModuleSetGraph> msc(mcc.create(pt_no_init_node));
//     msc->dump_graphviz(std::cout);
//     return true;
//   }

  return false;
}
