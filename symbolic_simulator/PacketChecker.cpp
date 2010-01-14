#include "PacketChecker.h"
#include <iostream>

namespace hydla {
namespace symbolic_simulator {


PacketChecker::PacketChecker(MathLink& ml) :
  ml_(ml)
{}

PacketChecker::~PacketChecker()
{}



void PacketChecker::check(){
  int pkt;
  // ���ʂ��󂯎��i�󂯎��܂ő҂�������j
  std::cout << "in check()" << std::endl;
  while(true){
    if((pkt = ml_.MLNextPacket()) == ILLEGALPKT){
      std::cout << "illegal packet" << std::endl;
      break;
    }
    std::cout << "not illegal:" << pkt << std::endl;
    switch(pkt){
    case RETURNPKT:
       std::cout << "returnpkt" << std::endl;
      switch(ml_.MLGetType()){ // ���s�I�u�W�F�N�g�̌^�𓾂�
      case MLTKSTR: // ������
	      strCase();
        break;
      case MLTKSYM: // �V���{���i�L���j
	      symCase();
        break;
      case MLTKINT: // ����
	      intCase();
        break;
      case MLTKOLDINT: // �Â��o�[�W������Mathlink���C�u�����̐���
        std::cout << "oldint" << std::endl;
        break;
      case MLTKERR: // �G���[
        std::cout << "err" << std::endl;
        break;
      case MLTKFUNC: // �����֐�
        funcCase();
        break;
      case MLTKREAL: // �ߎ�����
        std::cout << "real" << std::endl;
        break;
      case MLTKOLDREAL:
        std::cout << "oldreal" << std::endl;
        break;
      case MLTKOLDSTR:
        std::cout <<"oldstr" << std::endl;
        break;
      case MLTKOLDSYM:
        std::cout << "oldsym" << std::endl;
        break;
      default:
        std::cout <<"unknown_token" << std::endl;
      }
      break;

    case MESSAGEPKT: // Mathematica ���b�Z�[�W���ʎq�isymbol::string�j
      std::cout << "messagepkt" << std::endl;
      std::cout << "#message:" << ml_.get_string() << std::endl;
      break;
    case TEXTPKT: // Print[]�Ő��������悤��Mathematica ����̃e�L�X�g�o��
      std::cout << "textpkt" << std::endl;
      std::cout << "#text:" << ml_.get_string() << std::endl;
      break;
    case SYNTAXPKT:
      std::cout << "syntaxpkt" << std::endl;
      break;
    case INPUTNAMEPKT: // ���̓��͂Ɋ��蓖�Ă��閼�O�i�ʏ� In[n]:=�j
      std::cout << "inputnamepkt" << std::endl;
      std::cout << "#inputname:" << ml_.get_string() << std::endl;
      break;
    default:
       std::cout << "unknown_outer" << std::endl;
    } 
    ml_.MLNewPacket();
    std::cout << "new packet" << std::endl;
  }
  std::cout << "while end" << std::endl;
  std::cout << std::endl;

}

void PacketChecker::check2(){
  // Return�p�P�b�g�ȍ~�̃`�F�b�N
  std::cout << "in check2()" << std::endl;
  switch(ml_.MLGetType()){ // ���s�I�u�W�F�N�g�̌^�𓾂�
  case MLTKSTR: // ������
    strCase();
    break;
  case MLTKSYM: // �V���{���i�L���j
    symCase();
    break;
  case MLTKINT: // ����
    intCase();
    break;
  case MLTKOLDINT: // �Â��o�[�W������Mathlink���C�u�����̐���
    std::cout << "oldint" << std::endl;
    break;
  case MLTKERR: // �G���[
    std::cout << "err" << std::endl;
    break;
  case MLTKFUNC: // �����֐�
    funcCase();
    break;
  case MLTKREAL: // �ߎ�����
    std::cout << "real" << std::endl;
    break;
  case MLTKOLDREAL:
    std::cout << "oldreal" << std::endl;
    break;
  case MLTKOLDSTR:
    std::cout <<"oldstr" << std::endl;
    break;
  case MLTKOLDSYM:
    std::cout << "oldsym" << std::endl;
    break;
  default:
    std::cout <<"unknown_token" << std::endl;
  }
  std::cout << std::endl;
}

void PacketChecker::strCase(){
  std::cout << "str" << std::endl;
  std::cout << "#string:\"" << ml_.get_string() << "\"" << std::endl;
}

void PacketChecker::symCase(){
  std::cout << "symbol" << std::endl;
  std::cout << "#symname:" << ml_.get_string() << std::endl;
}

void PacketChecker::intCase(){
  std::cout << "int" << std::endl;
  int n;
  if(! ml_.MLGetInteger(&n)){
    std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
    throw MathLinkError("MLGetInteger", ml_.MLError());
  }
  std::cout << "#n:" << n << std::endl;
}

void PacketChecker::funcCase(){
  std::cout << "func" << std::endl;
  int funcarg;
  if(! ml_.MLGetArgCount(&funcarg)){
    std::cout << "#funcCase:MLGetArgCount:unable to get the number of arguments from ml" << std::endl;
    throw MathLinkError("MLGetArgCount", ml_.MLError());
  }
  std::cout << "#funcarg:" << funcarg << std::endl;
  switch(ml_.MLGetNext()){ //
  case MLTKFUNC: // �֐��iDerivative[1][f]�ɂ�����Derivative[1]�̂悤�Ɂj
    std::cout << "#funcCase:case MLTKFUNC" << std::endl;
    
    funcCase();
    break;
  case MLTKSYM: // �V���{���i�L���j
    std::cout << "#funcCase:case MLTKSYM" << std::endl;
    std::cout << "#funcname:" << ml_.get_symbol() << std::endl;
    break;
  default:
    ;
  }
  for(int i=0; i<funcarg; i++){
    switch (ml_.MLGetNext()){
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
      case MLTKOLDINT: // �Â��o�[�W������Mathlink���C�u�����̐���
        std::cout << "#funcCase:oldint" << std::endl;
        break;
      case MLTKERR: // �G���[
        std::cout << "#funcCase:err" << std::endl;
        break;
      case MLTKREAL: // �ߎ�����
        std::cout << "#funcCase:real" << std::endl;
        break;
      case MLTKOLDREAL:
        std::cout << "#funcCase:oldreal" << std::endl;
        break;
      case MLTKOLDSTR:
        std::cout <<"#funcCase:oldstr" << std::endl;
        break;
      case MLTKOLDSYM:
        std::cout << "#funcCase:oldsym" << std::endl;
        break;
      default:
        std::cout <<"#funcCase:unknown_token" << std::endl;      
    }
  }
}

} //namespace symbolic_simulator
} // namespace hydla
