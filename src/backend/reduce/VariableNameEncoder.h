#pragma once

#include <iostream>
#include <algorithm>

namespace hydla {
namespace backend {
namespace reduce {

/**
 * Hyrose文法で許される変数名をREDUCEで処理出来る文字列に変換するクラス
 * (var_prefix, par_prefixの付け外しなどこれにまとめるとスッキリしそう)
 */
class VariableNameEncoder {
public:
  VariableNameEncoder(){}
  virtual ~VariableNameEncoder(){}

  /** "XYZ" => "!!x!!y!!z"と変換する */
  std::string LowerEncode(const std::string& dist) const {
    std::string ret = dist;
    for(int i=0; i<(int)ret.length(); ++i){
      if('A' <= ret[i] && ret[i] <= 'Z'){
        ret[i] = std::tolower(ret[i]);
        ret.insert(i, "!!");
      }
    }
    return ret;
  }

  /** "!!x!!y!!z" => "XYZ"と変換する */
  std::string UpperDecode(const std::string& dist) const {
    std::string ret = dist;
    char prev = ' ';
    for(int i=0; i<(int)ret.length(); ++i){
      if(ret[i] == '!' && prev == '!'){
        ret[i+1] = std::toupper(ret[i+1]);
        ret.erase(i-1, 2);
      }
      prev = ret[i];
    }
    return ret;
  }
};

}
}
}
