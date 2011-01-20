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
	  Logger::enflag=6; //テスト用に常に出力するものを作る
	Logger::instance().set_log_level(Logger::Area);
	} else/* if(po.get<std::string>("ar") == "") {		//局部的出力entail
	  //Logger::flag=2;
	//Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "") {		//局部的出力entail
	  //Logger::flag=3;
	//Logger::instance().set_log_level(Logger::Area);
	} else */if(po.get<std::string>("ar") == "const_co1") {		//条件付制約
	  Logger::constflag=1;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "const_co2") {		//tell制約
	  Logger::constflag=2;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "const_co3") {		//制約集めた後のalways制約
	  Logger::constflag=3;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "const_co4") {		//maxmoduleなどの導出
	  Logger::constflag=4;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "const_co5") {		//always制約fromIP/PP
	  Logger::constflag=5;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "const_co6") {		//consraint_store_t
	  Logger::constflag=6;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "const_co7") {		//Begin Interval add_constraint 情報不足
	  Logger::constflag=7;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "const_co8") {		//PP変数表
	  Logger::constflag=8;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "const_co9") {		//Begin MathematicaVCSPoint::add_constraint(情報不足)
	  Logger::constflag=9;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "const_co10") {		//制約ストアの構築　List関数の要素数(Or/Andで結ばれた解の個数)を得る
	  Logger::constflag=10;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "const_co11") {		//posAsk,negAsk,NACons(採用していないモジュール内のtell制約)＞動かない
	  Logger::constflag=11;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "const_co12") {		//changed askに関して
	  Logger::constflag=12;
	Logger::instance().set_log_level(Logger::Area);
	//} else if(po.get<std::string>("ar") == "") {		//局部的出力entail
	  //Logger::flag=3;
	//Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "mat_s1") {		//mathematicaの通信(sendのs)左連続制約
	  Logger::mathsendflag=1;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "mat_s2") {		//通信・intervalでの
	  Logger::mathsendflag=2;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "mat_s3") {		//制約ストアの送信PP
	  Logger::mathsendflag=3;
	Logger::instance().set_log_level(Logger::Area);
	//} else if(po.get<std::string>("ar") == "") {		//局部的出力entail
	  //Logger::flag=3;
	//Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "mat_c1") {		//局部的出力entail
	  Logger::mathcalcflag=1;
	Logger::instance().set_log_level(Logger::Area);
	//} else if(po.get<std::string>("ar") == "") {		//局部的出力entail
	  //Logger::flag=3;
	//Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "var_v1") {		//IP変数表
	  Logger::varflag=1;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "var_v2") {		//左連続制約について
	  Logger::varflag=2;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "var_v3") {		//pointphaseの結果
	  Logger::varflag=3;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "var_v4") {		//intervalphaseの結果
	  Logger::varflag=4;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "var_v5") {		//次のフェーズにおける変数の値の導出
	  Logger::varflag=5;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "var_v6") {		//PP変数表
	  Logger::varflag=6;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "var_v7") {		//Point::create_variable_map
	  Logger::varflag=7;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "var_v8") {		//変数と値の組を受取るIP
	  Logger::varflag=8;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "var_v9") {		//変数表に登録されている変数名一覧or登録されていないもの
	  Logger::varflag=9;
	Logger::instance().set_log_level(Logger::Area);
	//} else if(po.get<std::string>("ar") == "") {		//局部的出力
	  //Logger::flag=3;
	//Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "time_t1") {		//シミュレーションにおける該当時間・送信時間
	  Logger::timeflag=1;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "time_t2") {		//経過時間・次のフェーズの始まる時間
	  Logger::timeflag=2;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "time_t3") {		//maxtime(動作せず)
	  Logger::timeflag=3;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "time_t4") {		//apply_time_to_vm>>Symbolictime
	  Logger::timeflag=4;
	Logger::instance().set_log_level(Logger::Area);
	//} else if(po.get<std::string>("ar") == "") {		//局部的出力
	  //Logger::flag=3;
	//Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "pt_p1") {		//ASTtree
	  Logger::ptflag=1;
	Logger::instance().set_log_level(Logger::Area);
	//} else if(po.get<std::string>("ar") == "") {		//局部的出力
	  //Logger::flag=3;
	//Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "pt_p4") {		//AskDisjunction
	  Logger::ptflag=4;
	Logger::instance().set_log_level(Logger::Area);
	} else if(po.get<std::string>("ar") == "pt_p5") {		//remove id
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
  } else if(po.count("comprehensive")>0){					//大局的出力モード
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
