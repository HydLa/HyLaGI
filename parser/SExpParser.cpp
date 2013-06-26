#include "SExpParser.h"

#include <iostream>
#include <cstring>
#include <sstream>

#include "CommentGrammar.h"

using namespace std;
using namespace boost::spirit::classic;

namespace hydla {
namespace parser {

/** ��W����\��S��"list" */
const std::string SExpParser::empty_list_s_exp("list");

SExpParser::SExpParser()
{
}

SExpParser::~SExpParser()
{
}

SExpParser::SExpParser(const char* input_str){
  parse_main(input_str);
}

SExpParser::SExpParser(const SExpParser& sp) : ast_tree_(sp.ast_tree_)
{
}

std::string SExpParser::get_string_from_tree(const_tree_iter_t iter) const{

  std::ostringstream ret_str;

  // �T�[�o�[���[�h�ł�minus�g�p�s��
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

//  std::cout << "ret_str.str(): " << ret_str.str() << "\n";

  return ret_str.str();
}

int SExpParser::get_derivative_count(const_tree_iter_t var_iter) const{

  int var_derivative_count;
  std::string var_str = std::string(var_iter->value.begin(), var_iter->value.end());

  // df�̐擪�ɃX�y�[�X�����邱�Ƃ�����̂ŏ�������
  // TODO:S���p�[�T���C�����ăX�y�[�X����Ȃ��悤�ɂ���
  if(var_str.at(0) == ' ') var_str.erase(0,1);

  // �������܂ޕϐ�
  if(var_str=="df"){
    size_t df_child_size = var_iter->children.size();

    // 1������̏ꍇ�͔����񐔕������ȗ�����Ă���
    if(df_child_size==2){
      var_derivative_count = 1;
    }
    else{
      assert(df_child_size==3);
      std::stringstream dc_ss;
      std::string dc_str = std::string((var_iter->children.begin()+2)->value.begin(),
                                       (var_iter->children.begin()+2)->value.end());
      dc_ss << dc_str;
      dc_ss >> var_derivative_count;
    }
  }
  else {
    var_derivative_count = 0;
  }

  return var_derivative_count;
}

void SExpParser::dump_tree(const_tree_iter_t iter, int nest){

  for(int i = 0; i < nest; ++i)
    cout << "  ";

  if (iter->value.id() == SExpGrammar::RI_Identifier)
    cout << "���ʎq " << string(iter->value.begin(), iter->value.end());
  else if (iter->value.id() == SExpGrammar::RI_Number)
    cout << "���� " << string(iter->value.begin(), iter->value.end());
  else if (iter->value.id() == SExpGrammar::RI_String)
    cout << "������ "<< "\"" << string(iter->value.begin(), iter->value.end()) << "\"";
  else if (iter->value.id() == SExpGrammar::RI_List)
    cout << "���X�g ";
  else if (iter->value.id() == SExpGrammar::RI_Data)
    cout << "�f�[�^ ";
  else
    cout << "Node '" << string(iter->value.begin(), iter->value.end()) << "'";


  cout << endl;

  for(size_t j = 0; j < iter->children.size(); ++j)
    dump_tree(iter->children.begin()+j, nest+1);
}

int SExpParser::parse_main(const char* input_str){
  SExpGrammar                   sg;
  CommentGrammar                cg;
//  cout << "input_str: " << input_str << "\n";

  pos_iter_t positBegin(input_str, input_str + strlen(input_str));
  pos_iter_t positEnd;

  ast_tree_ = ast_parse(positBegin, positEnd, sg, cg);

/*
  cout << "--------------------------\n";
  if(ast_tree_.full){
    dump_tree(get_tree_iterator(), 0);
  }else{
    cout << "���s\n";
  }
*/

  return(0);
}

std::ostream& operator<<(std::ostream& s, const SExpParser::const_tree_iter_t& iter)
{
  SExpParser sp;
  s << sp.get_string_from_tree(iter);
  return s;
}

} //namespace parser
} //namespace hydla

