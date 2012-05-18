#include "mathlink_helper.h"


bool MathLink::receive(){
  HYDLA_LOGGER_VCS("#*** Begin MathLink::receive ***\n");
  
  token_list_.clear();
  int_list_.clear();
  string_list_.clear();
  bool at_end = false;
  // ���ʂ��󂯎��i�󂯎��܂ő҂�������j
  while(!at_end){
    int pkt = MLNextPacket();
    switch(pkt){
    case RETURNPKT:
      HYDLA_LOGGER_VCS("%% Mathlink::receive returnpkt\n");
      switch(MLGetType()){ // ���s�I�u�W�F�N�g�̌^�𓾂�
      case MLTKSTR: // ������
	      strCase();
        break;
      case MLTKSYM: // �V���{���i�L���j
	      symCase();
        break;
      case MLTKINT: // ����
	      intCase();
        break;
      case MLTKFUNC: // �֐�
        funcCase();
        break;
      default:  // �Â��o�[�W������Mathlink���C�u�����̐����Ȃ�
        HYDLA_LOGGER_VCS("%% Mathlink::receive illegal_token:", pkt, "\n");
        assert(0);
        break;
      }
      HYDLA_LOGGER_VCS("\n");
      at_end = true;
      break;
    case MESSAGEPKT: // Mathematica ���b�Z�[�W���ʎq�isymbol::string�j
    {
      std::string str = get_string_();
      HYDLA_LOGGER_VCS("%% Mathlink::receive messagepkt\n", str, "\n");
      break;
    }
    case TEXTPKT: // Print[]�Ő��������悤��Mathematica ����̃e�L�X�g�o��
    {
      std::string str = get_string_();
      HYDLA_LOGGER_VCS("%% Mathlink::receive textpkt\n", str, "\n");
      break;
    }
    case SYNTAXPKT:
      HYDLA_LOGGER_VCS("%% Mathlink::receive syntaxpkt\n");
      break;
    case INPUTNAMEPKT: // ���̓��͂Ɋ��蓖�Ă��閼�O�i�ʏ� In[n]:=�j
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
    case MLTKFUNC: // �֐��iDerivative[1][f]�ɂ�����Derivative[1]�̂悤�Ɂj
      funcCase();
      break;
    case MLTKSYM: // �V���{���i�L���j
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
        case MLTKERR: // �G���[
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
