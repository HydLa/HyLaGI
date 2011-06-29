#include "SExpParser.h"

#include <iostream>
#include <cstring>
#include <sstream>

#include "SExpGrammar.h"
#include "CommentGrammar.h"

using namespace std;
using namespace boost::spirit::classic;

namespace hydla {
namespace parser {

SExpParser::SExpParser()
{
}

SExpParser::~SExpParser()
{
}

std::string SExpParser::get_string_from_tree(const_tree_iter_t iter) const{

  std::ostringstream ret_str;

  ret_str << std::string(iter->value.begin(), iter->value.end());
  if(iter->children.size()>0){
    ret_str << "(";
    for(size_t i=0; i<iter->children.size();i++){
      if(i!=0) ret_str << ",";
      ret_str << get_string_from_tree(iter->children.begin()+i);
    }
    ret_str << ")";
  }

//  std::cout << "ret_str.str(): " << ret_str.str() << "\n";

  return ret_str.str();
}

void SExpParser::dump_tree(const_tree_iter_t iter, int nest){

  for(int i = 0; i < nest; ++i)
    cout << "  ";

  if (iter->value.id() == SExpGrammar::RI_Identifier)
    cout << "識別子 " << string(iter->value.begin(), iter->value.end());
  else if (iter->value.id() == SExpGrammar::RI_Number)
    cout << "整数 " << string(iter->value.begin(), iter->value.end());
  else if (iter->value.id() == SExpGrammar::RI_String)
    cout << "文字列 "<< "\"" << string(iter->value.begin(), iter->value.end()) << "\"";
  else if (iter->value.id() == SExpGrammar::RI_List)
    cout << "リスト ";
  else if (iter->value.id() == SExpGrammar::RI_Data)
    cout << "データ ";
  else
    cout << "Node '" << string(iter->value.begin(), iter->value.end()) << "'";


  cout << endl;

  for(int j = 0; j < iter->children.size(); ++j)
    dump_tree(iter->children.begin()+j, nest+1);
}

int SExpParser::parse_main(const char* input_str){
  SExpGrammar                   sg;
  CommentGrammar                cg;
  cout << "input_str: " << input_str << "\n";

  pos_iter_t positBegin(input_str, input_str + strlen(input_str));
  pos_iter_t positEnd;

  ast_tree_ = ast_parse(positBegin, positEnd, sg, cg);
  cout << "--------------------------\n";
  if(ast_tree_.full){
    dump_tree(get_tree_iterator(), 0);
  }else{
    cout << "失敗\n";
  }
  return(0);
}

} //namespace parser
} //namespace hydla

