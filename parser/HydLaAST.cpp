#include "HydLaAST.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <assert.h>

#include "HydLaGrammar.h"
#include "CommentGrammar.h"
#include "ParseError.h"

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

void HydLaAST::parse(std::istream& stream) 
{
  pos_iter_t positBegin(make_multi_pass(istreambuf_iterator<char>(stream)), 
                        make_multi_pass(istreambuf_iterator<char>()));
  pos_iter_t positEnd;

  HydLaGrammar hg;
  CommentGrammar cg;
  ast_tree_ = ast_parse<node_val_data_factory_t>(positBegin, positEnd, hg, cg);
  
  

 if(!ast_tree_.full) {
    throw SyntaxError("", ast_tree_.stop.get_position().line);
  }
}

void HydLaAST::parse_flie(const std::string& filename) 
{
  ifstream in(filename.c_str());
  if (!in) {    
    throw std::runtime_error(string("cannot open \"") + filename + "\"");
  }

  parse(in);
}

void HydLaAST::parse_string(const std::string& str)
{    
  stringstream in(str);
  parse(in);
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
