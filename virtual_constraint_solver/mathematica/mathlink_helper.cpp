#include "mathlink_helper.h"


bool MathLink::receive(){
  HYDLA_LOGGER_EXTERN("#*** Begin MathLink::receive ***\n");
  bool at_end = false;
  bool next_is_massage = false;
  debug_print_.clear();
  input_print_.clear();
  // ���ʂ��󂯎��i�󂯎��܂ő҂�������j
  while(true){
    int pkt = MLNextPacket();
    switch(pkt){
    case RETURNPKT:
      HYDLA_LOGGER_EXTERN("%% Mathlink::receive returnpkt");
      MLGetType();
      at_end = true;
      break;
    case MESSAGEPKT: // Mathematica ���b�Z�[�W���ʎq�isymbol::string�j
    {
      std::string str = get_string();
      HYDLA_LOGGER_EXTERN("%% Mathlink::receive messagepkt\n", str, "\n");
      next_is_massage = true;
      break;
    }
    case TEXTPKT: // Print[]�Ő��������悤��Mathematica ����̃e�L�X�g�o��
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
    case INPUTNAMEPKT: // ���̓��͂Ɋ��蓖�Ă��閼�O�i�ʏ� In[n]:=�j
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