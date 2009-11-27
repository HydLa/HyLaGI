#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "HydLaParser.h"
#include "HydLaGrammarRule.h"
#include "HydLaGrammar.h"

#include "ParseError.h"

#include "CommentGrammar.h"

using namespace std;
using boost::shared_ptr;

using namespace hydla::parse_tree;
using namespace hydla::parse_error;
using namespace hydla::grammer_rule;

namespace hydla {
namespace parser {

void HydLaParser::create_ast(std::istream& iter)
{
  pos_iter_t positBegin(make_multi_pass(istreambuf_iterator<char>(iter)), 
                        make_multi_pass(istreambuf_iterator<char>()));
  pos_iter_t positEnd;

  HydLaGrammar hg;
  CommentGrammar cg;
  ast_tree_ = ast_parse<node_val_data_factory_t>(positBegin, positEnd, hg, cg);

  if(!ast_tree_.full) {
    throw SyntaxError("", ast_tree_.stop.get_position().line);
  }
}

} // namespace parser
} // namespace hydla
