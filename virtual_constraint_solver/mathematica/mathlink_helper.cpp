#include "mathlink_helper.h"


bool MathLink::receive(){
  HYDLA_LOGGER_EXTERN("#*** Begin MathLink::receive ***\n");
  bool at_end = false;
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
      break;
    }
    case TEXTPKT: // Print[]�Ő��������悤��Mathematica ����̃e�L�X�g�o��
    {
      std::string str = get_string();
      HYDLA_LOGGER_EXTERN("%% Mathlink::receive textpkt\n", str, "\n");
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
      break;
    default:
      HYDLA_LOGGER_EXTERN("%% Mathlink::receive unknownpkt: ", pkt, "\n");
      break;
    }
    if(at_end)break;
    MLNewPacket();
  }
  HYDLA_LOGGER_EXTERN("#*** End MathLink::receive ***\n");
  return true;
}