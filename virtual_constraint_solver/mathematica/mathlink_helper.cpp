#include "mathlink_helper.h"


bool MathLink::receive(){
  HYDLA_LOGGER_VCS("#*** Begin MathLink::receive ***");
  token_list_.clear();
  int_list_.clear();
  string_list_.clear();
  bool at_end = false;
  // 結果を受け取る（受け取るまで待ち続ける）
  while(!at_end){
    int pkt = MLNextPacket();
    switch(pkt){
    case RETURNPKT:
      HYDLA_LOGGER_VCS("%% Mathlink::receive returnpkt");
      switch(MLGetType()){ // 現行オブジェクトの型を得る
      case MLTKSTR: // 文字列
	      strCase();
        break;
      case MLTKSYM: // シンボル（記号）
	      symCase();
        break;
      case MLTKINT: // 整数
	      intCase();
        break;
      case MLTKFUNC: // 関数
        funcCase();
        break;
      default:  // 古いバージョンのMathlinkライブラリの整数など
        HYDLA_LOGGER_VCS("%% Mathlink::receive illegal_token:", pkt);
        assert(0);
        break;
      }
      at_end = true;
      break;
    case MESSAGEPKT: // Mathematica メッセージ識別子（symbol::string）
    {
      std::string str = get_string_();
      HYDLA_LOGGER_VCS("%% Mathlink::receive messagepkt");
      HYDLA_LOGGER_VCS("%% Mathlink::receive #message: ", str);
      break;
    }
    case TEXTPKT: // Print[]で生成されるようなMathematica からのテキスト出力
    {
      std::string str = get_string_();
      HYDLA_LOGGER_VCS("%% Mathlink::receive textpkt");
      HYDLA_LOGGER_VCS("%% Mathlink::receive #text: ", str);
      break;
    }
    case SYNTAXPKT:
      HYDLA_LOGGER_VCS("%% Mathlink::receive syntaxpkt");
      break;
    case INPUTNAMEPKT: // 次の入力に割り当てられる名前（通常 In[n]:=）
    {
      std::string str = get_string_();
      HYDLA_LOGGER_VCS("%% Mathlink::receive inputnamepkt");
      HYDLA_LOGGER_VCS("%% Mathlink::receive #inputname: ", str);
      break;
    }
    case ILLEGALPKT:
      HYDLA_LOGGER_VCS("%% Mathlink::receive illegalpkt");
      break;
    default:
      HYDLA_LOGGER_VCS("%% Mathlink::receive unknownpkt: ", pkt);
      break;
    }
    MLNewPacket();
  }
  HYDLA_LOGGER_VCS("#*** End MathLink::receive ***");
  return true;
}


void MathLink::strCase(){
  std::string str = get_string_();
  string_list_.push_back(str);
  token_list_.push_back(MLTKSTR);
  HYDLA_LOGGER_VCS("\"", str, "\"");
}

void MathLink::symCase(){
  std::string str = get_string_();
  string_list_.push_back(str);
  token_list_.push_back(MLTKSYM);
  HYDLA_LOGGER_VCS(str);
}

void MathLink::intCase(){
  int n = get_integer_();
  int_list_.push_back(n);
  token_list_.push_back(MLTKINT);
  HYDLA_LOGGER_VCS(n);
}

void MathLink::funcCase(){
  int funcarg = get_arg_count_();
  HYDLA_LOGGER_VCS("#function argc: ", funcarg);
  token_list_.push_back(MLTKFUNC);
  int_list_.push_back(funcarg);
  switch(MLGetNext()){ //
    case MLTKFUNC: // 関数（Derivative[1][f]におけるDerivative[1]のように）
      funcCase();
      break;
    case MLTKSYM: // シンボル（記号）
    {
      std::string str = get_string_();
      HYDLA_LOGGER_VCS("#function name: ", str);
      token_list_.push_back(MLTKSYM);
      string_list_.push_back(str);
      break;
    }
    default:
    ;
  }
  int token;
  for(int i=0; i<funcarg; i++){
    switch (token = MLGetNext()){
      case MLTKSTR:
        strCase();
        break;
      case MLTKSYM:
        symCase();
        break;
      case MLTKINT:
        intCase();
        break;
      case MLTKFUNC:
        funcCase();
        break;
      case MLTKERR: // エラー
        HYDLA_LOGGER_VCS("#funcCase:err");
      default:
        HYDLA_LOGGER_VCS("#funcCase:unknown_token", token);
    }
  }
}
