#ifndef _INCLUDED_HYDLA_VCS_REDUCE_VARIABLENAMEENCODER_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_VARIABLENAMEENCODER_H_

#include <iostream>
#include <algorithm>

namespace hydla {
namespace vcs {
namespace reduce {

/**
 * Hyrose文法で許される変数名をREDUCEで処理出来る文字列に変換するクラス
 * (var_prefix, par_prefixの付け外しなどこれにまとめるとスッキリしそう)
 */
class VariableNameEncoder {
public:
  VariableNameEncoder(){}
  virtual ~VariableNameEncoder(){}

  /**
   * "XYZ" => "_x_y_z"と変換する
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
   * "_x_y_z" => "XYZ"と変換する
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
