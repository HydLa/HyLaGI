/**
 *
 * 解候補モジュール集合のテストケース
 *
 */

#include <iostream>

#include <boost/test/auto_unit_test.hpp>

#include "HydLaAST.h"
#include "NodeFactory.h"
#include "ParseTreeGenerator.h"
#include "ParseError.h"

using namespace std;
using namespace hydla;
using namespace hydla::parser;
using namespace hydla::parse_tree;
using namespace hydla::parse_error;

using namespace boost;

//using namespace hydla::ch;

BOOST_AUTO_TEST_CASE(module_set_list_test)
{
//  shared_ptr<NodeFactory> nf(new NodeFactory());
//  HydLaParser hp(nf);
//  hp.parse_string(INPUT);
//
//  ModuleSetContainerCreator<ModuleSetList> mscc;
//  boost::shared_ptr<ModuleSetList> msl(
//    mscc.create_module_set_container(&hp.parse_tree()));
//msl->
//
//  virtual bool dispatch(boost::function<bool (hydla::ch::module_set_sptr)> callback_func, 
//                        int threads = 1) = 0;

  HydLaAST ast;
  ParseTreeGenerator<DefaultNodeFactory> ptg;


    ast.parse_string("x=1.");
     boost::shared_ptr<ParseTree> pt1(ptg.generate(ast.get_tree_iterator()));

    ast.parse_string("1=x.");
     boost::shared_ptr<ParseTree> pt2(ptg.generate(ast.get_tree_iterator()));

    BOOST_CHECK_EQUAL(pt1->is_same_tree_struct(*pt2), true);

}
