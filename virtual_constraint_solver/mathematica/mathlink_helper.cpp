#include "mathlink_helper.h"


bool MathLink::receive(){
  HYDLA_LOGGER_EXTERN("#*** Begin MathLink::receive ***\n");
  bool at_end = false;
  bool next_is_massage = false;
  debug_print_.clear();
  input_print_.clear();
  // 結果を受け取る（受け取るまで待ち続ける）
  while(true){
    int pkt = MLNextPacket();
    switch(pkt){
    case RETURNPKT:
      HYDLA_LOGGER_EXTERN("%% Mathlink::receive returnpkt");
      MLGetType();
      at_end = true;
      break;
    case MESSAGEPKT: // Mathematica メッセージ識別子（symbol::string）
    {
      std::string str = get_string();
      HYDLA_LOGGER_EXTERN("%% Mathlink::receive messagepkt\n", str, "\n");
      next_is_massage = true;
      break;
    }
    case TEXTPKT: // Print[]で生成されるようなMathematica からのテキスト出力
    {
      std::string str = get_string();
      HYDLA_LOGGER_EXTERN("%% Mathlink::receive textpkt\n", str, "\n");
      if(input_print_.empty()){
        input_print_ = str;
      }else if(next_is_massage){
        debug_print_ += str + "\n";
        next_is_massage = false;
      }else{
        debug_print_ += str + "\n";
      }
      break;
    }
    case SYNTAXPKT:
      HYDLA_LOGGER_EXTERN("%% Mathlink::receive syntaxpkt\n");
      break;
    case INPUTNAMEPKT: // 次の入力に割り当てられる名前（通常 In[n]:=）
    {
      std::string str = get_string();
      HYDLA_LOGGER_EXTERN("%% Mathlink::receive inputnamepkt\n", str, "\n");
      break;
    }
    case ILLEGALPKT:
      HYDLA_LOGGER_EXTERN("%% Mathlink::receive illegalpkt\n");
      throw MathLinkError("receive illegalpkt", 0);
      break;
    default:
      HYDLA_LOGGER_EXTERN("%% Mathlink::receive unknownpkt: ", pkt, "\n");
      throw MathLinkError("receive unknownpkt", 0);
      break;
    }
    if(at_end)break;
    MLNewPacket();
  }
  HYDLA_LOGGER_EXTERN("#*** End MathLink::receive ***\n");
  return true;
}

std::string MathLink::get_debug_print(){
  return debug_print_;
}


std::string MathLink::get_input_print(){
  return input_print_;
}