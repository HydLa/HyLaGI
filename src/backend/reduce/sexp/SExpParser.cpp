#include "SExpParser.h"

#include <cstring>
#include <sstream>

using namespace boost::spirit::classic;
using namespace hydla::parser;

namespace hydla {
namespace parser {
SExpParser::tree_info_t SExpParser::parse(const std::string& input_str){
  SExpGrammar                   sg;

  const char* str = input_str.c_str();

  pos_iter_t positBegin(str, str + strlen(str));
  pos_iter_t positEnd;
  tree_info_t ast_tree = ast_parse(positBegin, positEnd, sg);

  return (ast_tree);
}

std::string SExpParser::get_string_from_tree(const_tree_iter_t iter){ 
  std::ostringstream ret_str;

  // サーバーモードではminus使用不可
  std::string str = std::string(iter->value.begin(), iter->value.end());
  if(str == "minus") ret_str << "-";
  else ret_str << str;

  if(iter->children.size()>0){
    ret_str << "(";
    for(size_t i=0; i<iter->children.size();i++){
      if(i!=0) ret_str << ",";
      ret_str << get_string_from_tree(iter->children.begin()+i);
    }
    ret_str << ")";
  }

  return ret_str.str();
}

} // namespace parser
} // namespace hydla


std::ostream& operator<<(std::ostream& s, const hydla::parser::SExpParser::const_tree_iter_t& iter){
  s << SExpParser::get_string_from_tree(iter);
  return s;
}
