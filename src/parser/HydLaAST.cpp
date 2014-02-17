#include "HydLaAST.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <assert.h>

#include "HydLaGrammar.h"
#include "CommentGrammar.h"
#include "ParseError.h"
#include "Logger.h"

using namespace std;
using namespace boost;
using namespace hydla::grammer_rule;
using namespace hydla::parse_error;

namespace hydla {
namespace parser {
 
HydLaAST::HydLaAST()
{
}
  
HydLaAST::~HydLaAST()
{
}

void HydLaAST::parse(std::istream& stream, SyntaxType type) 
{
  pos_iter_t it_begin(make_multi_pass(istreambuf_iterator<char>(stream)), 
                        make_multi_pass(istreambuf_iterator<char>()));
  pos_iter_t it_end;
  HydLaGrammar hg;
  CommentGrammar cg;
  switch(type)
  {
  case PROGRAM:
    ast_tree_ = ast_parse<node_val_data_factory_t>(it_begin, it_end, hg.use_parser<HydLaGrammar::START_HydLaProgram>(), cg);
    break;
  case CONSTRAINT:
    ast_tree_ = ast_parse<node_val_data_factory_t>(it_begin, it_end, hg.use_parser<HydLaGrammar::START_Constraint>(), cg);
    break;
  case ARITHMETIC_EXPRESSION:
    ast_tree_ = ast_parse<node_val_data_factory_t>(it_begin, it_end, hg.use_parser<HydLaGrammar::START_ArithmeticExpr>(), cg);
  break;
  }
 if(!ast_tree_.full) {
    throw SyntaxError("", ast_tree_.stop.get_position().line);
  }
}


void HydLaAST::parse_file(const std::string& filename, SyntaxType type) 
{
  ifstream in(filename.c_str());
  if (!in) {    
    throw std::runtime_error(string("cannot open \"") + filename + "\"");
  }
  parse(in, type);
}

void HydLaAST::parse_string(const std::string& str, SyntaxType type)
{
  istringstream in(str);
  parse(in, type);
}

std::ostream& HydLaAST::dump(std::ostream& outstream, 
                             const_tree_iter_t iter, 
                             int nest) const
{
  for(int i=0; i<nest; i++) {
    outstream << "  "; // スペース２個分インデント
  }

#define OUT_ID_NAME(X) case X: outstream << " " << #X; break

  long id = iter->value.id().to_long();
  outstream << "ID:" << id;
  switch(id) {
    OUT_ID_NAME(RI_Constraint);
    OUT_ID_NAME(RI_ConstraintCaller);
    OUT_ID_NAME(RI_ConstraintCallee);
    OUT_ID_NAME(RI_ProgramCaller);
    OUT_ID_NAME(RI_ProgramCallee);
    OUT_ID_NAME(RI_Tell);
    OUT_ID_NAME(RI_ConstraintDef);
    OUT_ID_NAME(RI_ProgramDef);
  }

  outstream << " Node:" << string(iter->value.begin(), iter->value.end());

  size_t size = iter->children.size();
  if (size > 0) {
    outstream << " Child Size:" << size << endl;
  } else {
    outstream << endl;
  }

  for(size_t j=0; j<size; j++) {
    dump(outstream, iter->children.begin()+j, nest+1);
  }

  return outstream;
}

std::ostream& operator<<(std::ostream& s, const HydLaAST& ast)
{
  return ast.dump(s);
}

} //namespace parser
} //namespace hydla
