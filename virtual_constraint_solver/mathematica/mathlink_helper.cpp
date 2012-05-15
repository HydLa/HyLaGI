#include "mathlink_helper.h"


bool MathLink::receive(){
  HYDLA_LOGGER_VCS("#*** Begin MathLink::receive ***");
  token_list_.clear();
  int_list_.clear();
  string_list_.clear();
  bool at_end = false;
  // ���ʂ��󂯎��i�󂯎��܂ő҂�������j
  while(!at_end){
    int pkt = MLNextPacket();
    switch(pkt){
    case RETURNPKT:
      HYDLA_LOGGER_VCS("%% Mathlink::receive returnpkt");
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
        HYDLA_LOGGER_VCS("%% Mathlink::receive illegal_token:", pkt);
        assert(0);
        break;
      }
      at_end = true;
      break;
    case MESSAGEPKT: // Mathematica ���b�Z�[�W���ʎq�isymbol::string�j
    {
      std::string str = get_string_();
      HYDLA_LOGGER_VCS("%% Mathlink::receive messagepkt");
      HYDLA_LOGGER_VCS("%% Mathlink::receive #message: ", str);
      break;
    }
    case TEXTPKT: // Print[]�Ő��������悤��Mathematica ����̃e�L�X�g�o��
    {
      std::string str = get_string_();
      HYDLA_LOGGER_VCS("%% Mathlink::receive textpkt");
      HYDLA_LOGGER_VCS("%% Mathlink::receive #text: ", str);
      break;
    }
    case SYNTAXPKT:
      HYDLA_LOGGER_VCS("%% Mathlink::receive syntaxpkt");
      break;
    case INPUTNAMEPKT: // ���̓��͂Ɋ��蓖�Ă��閼�O�i�ʏ� In[n]:=�j
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
    case MLTKFUNC: // �֐��iDerivative[1][f]�ɂ�����Derivative[1]�̂悤�Ɂj
      funcCase();
      break;
    case MLTKSYM: // �V���{���i�L���j
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
      case MLTKERR: // �G���[
        HYDLA_LOGGER_VCS("#funcCase:err");
      default:
        HYDLA_LOGGER_VCS("#funcCase:unknown_token", token);
    }
  }
}
