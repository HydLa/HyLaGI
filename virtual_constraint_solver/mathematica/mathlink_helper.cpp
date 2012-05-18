#include "mathlink_helper.h"


bool MathLink::receive(){
  HYDLA_LOGGER_VCS("#*** Begin MathLink::receive ***\n");
  
  token_list_.clear();
  int_list_.clear();
  string_list_.clear();
  bool at_end = false;
  // 結果を受け取る（受け取るまで待ち続ける）
  while(!at_end){
    int pkt = MLNextPacket();
    switch(pkt){
    case RETURNPKT:
      HYDLA_LOGGER_VCS("%% Mathlink::receive returnpkt\n");
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
        HYDLA_LOGGER_VCS("%% Mathlink::receive illegal_token:", pkt, "\n");
        assert(0);
        break;
      }
      HYDLA_LOGGER_VCS("\n");
      at_end = true;
      break;
    case MESSAGEPKT: // Mathematica メッセージ識別子（symbol::string）
    {
      std::string str = get_string_();
      HYDLA_LOGGER_VCS("%% Mathlink::receive messagepkt\n", str, "\n");
      break;
    }
    case TEXTPKT: // Print[]で生成されるようなMathematica からのテキスト出力
    {
      std::string str = get_string_();
      HYDLA_LOGGER_VCS("%% Mathlink::receive textpkt\n", str, "\n");
      break;
    }
    case SYNTAXPKT:
      HYDLA_LOGGER_VCS("%% Mathlink::receive syntaxpkt\n");
      break;
    case INPUTNAMEPKT: // 次の入力に割り当てられる名前（通常 In[n]:=）
    {
      std::string str = get_string_();
      HYDLA_LOGGER_VCS("%% Mathlink::receive inputnamepkt\n", str, "\n");
      break;
    }
    case ILLEGALPKT:
      HYDLA_LOGGER_VCS("%% Mathlink::receive illegalpkt\n");
      break;
    default:
      HYDLA_LOGGER_VCS("%% Mathlink::receive unknownpkt: ", pkt, "\n");
      break;
    }
    MLNewPacket();
  }
  HYDLA_LOGGER_VCS("#*** End MathLink::receive ***\n");
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
  token_list_.push_back(MLTKFUNC);
  int_list_.push_back(funcarg);
  switch(MLGetNext()){ //
    case MLTKFUNC: // 関数（Derivative[1][f]におけるDerivative[1]のように）
      funcCase();
      break;
    case MLTKSYM: // シンボル（記号）
    {
      std::string str = get_string_();
      HYDLA_LOGGER_VCS(str);
      token_list_.push_back(MLTKSYM);
      string_list_.push_back(str);
      break;
    }
    default:
    ;
  }
  HYDLA_LOGGER_VCS("<", funcarg, ">[");
  int token;
  int i=0;
  if(funcarg>0){
    while(true){
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
      if(++i>=funcarg)break;
      HYDLA_LOGGER_VCS(",");
    }
  }
  HYDLA_LOGGER_VCS("]");
}
