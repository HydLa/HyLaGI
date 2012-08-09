#ifndef _INCLUDED_HYDLA_VCS_REDUCE_VARIABLENAMEENCODER_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_VARIABLENAMEENCODER_H_

#include <iostream>
#include <algorithm>

namespace hydla {
namespace vcs {
namespace reduce {

/**
 * Hyrose���@�ŋ������ϐ�����REDUCE�ŏ����o���镶����ɕϊ�����N���X
 * (var_prefix, par_prefix�̕t���O���Ȃǂ���ɂ܂Ƃ߂�ƃX�b�L��������)
 */
class VariableNameEncoder {
public:
  VariableNameEncoder(){}
  virtual ~VariableNameEncoder(){}

  /**
   * "XYZ" => "_x_y_z"�ƕϊ�����
   */
  std::string LowerEncode(const std::string& dist) const {
    std::string ret = dist;
    for(int i=0;i<(int)ret.length();i++){
      if( 'A'<=ret[i] && ret[i]<='Z' ){
        ret[i] = std::tolower(ret[i]);
        ret.insert(i, "_");
      }
    }
    return ret;
  }

  /**
   * "_x_y_z" => "XYZ"�ƕϊ�����
   */
  std::string UpperDecode(const std::string& dist) const {
    std::string ret = dist;
    for(int i=0;i<(int)ret.length();i++){
      if( ret[i]=='_'){
        ret.erase(i,1);
        ret[i] = std::toupper(ret[i]);
      }
    }
    return ret;
  }
};

} //namespace reduce
} //namespace vcs
} //namespace hydla

#endif /* _INCLUDED_HYDLA_VCS_REDUCE_VARIABLENAMEENCODER_H_ */
