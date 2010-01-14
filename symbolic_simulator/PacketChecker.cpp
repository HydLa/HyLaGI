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
  // 結果を受け取る（受け取るまで待ち続ける）
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
      switch(ml_.MLGetType()){ // 現行オブジェクトの型を得る
      case MLTKSTR: // 文字列
	      strCase();
        break;
      case MLTKSYM: // シンボル（記号）
	      symCase();
        break;
      case MLTKINT: // 整数
	      intCase();
        break;
      case MLTKOLDINT: // 古いバージョンのMathlinkライブラリの整数
        std::cout << "oldint" << std::endl;
        break;
      case MLTKERR: // エラー
        std::cout << "err" << std::endl;
        break;
      case MLTKFUNC: // 合成関数
        funcCase();
        break;
      case MLTKREAL: // 近似実数
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

    case MESSAGEPKT: // Mathematica メッセージ識別子（symbol::string）
      std::cout << "messagepkt" << std::endl;
      std::cout << "#message:" << ml_.get_string() << std::endl;
      break;
    case TEXTPKT: // Print[]で生成されるようなMathematica からのテキスト出力
      std::cout << "textpkt" << std::endl;
      std::cout << "#text:" << ml_.get_string() << std::endl;
      break;
    case SYNTAXPKT:
      std::cout << "syntaxpkt" << std::endl;
      break;
    case INPUTNAMEPKT: // 次の入力に割り当てられる名前（通常 In[n]:=）
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
  // Returnパケット以降のチェック
  std::cout << "in check2()" << std::endl;
  switch(ml_.MLGetType()){ // 現行オブジェクトの型を得る
  case MLTKSTR: // 文字列
    strCase();
    break;
  case MLTKSYM: // シンボル（記号）
    symCase();
    break;
  case MLTKINT: // 整数
    intCase();
    break;
  case MLTKOLDINT: // 古いバージョンのMathlinkライブラリの整数
    std::cout << "oldint" << std::endl;
    break;
  case MLTKERR: // エラー
    std::cout << "err" << std::endl;
    break;
  case MLTKFUNC: // 合成関数
    funcCase();
    break;
  case MLTKREAL: // 近似実数
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
  case MLTKFUNC: // 関数（Derivative[1][f]におけるDerivative[1]のように）
    std::cout << "#funcCase:case MLTKFUNC" << std::endl;
    
    funcCase();
    break;
  case MLTKSYM: // シンボル（記号）
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
      case MLTKOLDINT: // 古いバージョンのMathlinkライブラリの整数
        std::cout << "#funcCase:oldint" << std::endl;
        break;
      case MLTKERR: // エラー
        std::cout << "#funcCase:err" << std::endl;
        break;
      case MLTKREAL: // 近似実数
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
