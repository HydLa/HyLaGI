//�R���p�C��: g++ -o main main.cpp -lboost_system-gcc43-mt-1_38

#include <iostream>
#include <fstream>

#include <string>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>

#define END_TST "endtst___"
#define END_TST_OFFNAT "endtst___$"

using namespace std;
using namespace boost::asio;
using namespace boost::posix_time;

class asio_cl {
  //TCP/IP��REDUCE�Ƃ̃C���^�t�F�[�X
  //TODO: REDUCE���N�����ĂȂ��ȂǂŐڑ��ł��Ȃ��ꍇ�̕���

  ip::tcp::iostream s;
  boost::asio::io_service io_service;


public:
  asio_cl(){

/*
  system�֐���reduce���N�������reduce���I������܂ŏ������i�܂Ȃ�!
  system("cd /home/yysaki/HydLa/branches/HydLa_v0.5.x/reduce/");
  system("reduce -F-");
*/
  boost::system::error_code error = boost::asio::error::host_not_found;
    s.connect("localhost", "1206");
  }

  int send_to_reduce(string cmd){
    cout << "---- send phase ----" << endl;
    cout << "cmd: " << cmd << endl;
    s << cmd;
    s << "\r\n";

    return 0;
  }

  //TODO: recv���擾�o���Ȃ��ꍇ�̃^�C���A�E�g����
  //TODO: �I�������̎擾
  int recv_from_reduce(){
    int i = 0;
    string line;

    cout << "---- recv phase ----" << endl;
    while(getline(s, line)){
  //    cout << i++ << "�Ԗ�, length: " << line.length() << endl;
      cout << line << endl;
      if(line.length()==0) break;
    }

    return 0;
  }

  int count_char(std::string str, char query){
    int count = 0, i = 0;
    while(true){
      i = str.find(query, i);
      if(i==std::string::npos) break;
//    std::cout << "i[" << count << "]: " << i << std::endl;

      count++; i++;
    }

    return count;
  }

  /*
  * �[�����"solve({x+y=2,3x+2y=5},{x,y});" �ȂǂƓ���
  */
  void scanf_test(){
    cout << "---- scanf phase ----" << endl;
    string scaned;
    while(true){
      cout << "cmd: ";
      cin >> scaned;
      if(scaned=="exit") break;
      send_to_reduce(scaned);
      recv_from_reduce();
    }
  }

/*
Goal �ȉ���3���߂𑗂�v�Z�𐬌�����������recv
     recv��while�̔�����������Aline[0]=='('�Ƃ���
     '('�̐���')'�̐��𐔂��A���ꂼ�ꂪ��v����e�L�X�g��Ԃ�l�Ƃ���


1: vars_:={y,yy,yyy,z,zz};
2: expr_:={zz - yy = 0, yyy + 10 = 0, yy = 0, True, z - y = 0, y - 10 = 0};
3: symbolic reval '(isconsistent vars_ expr_);
*/
  std::string isConsistent_test(){
    cout << "---- isConsistent_test() phase ----" << endl;
    std::string cmd, line;
    cmd = "vars_:={y,yy,yyy,z,zz};";
    send_to_reduce(cmd);
    if(getline(s, line)) std::cout << line;

    cmd = "expr_:={zz - yy = 0, yyy + 10 = 0, yy = 0, True, z - y = 0, y - 10 = 0};";
    send_to_reduce(cmd);
    if(getline(s, line)) std::cout << line;

    cmd = "symbolic reval '(isconsistent vars_ expr_);";
    send_to_reduce(cmd);
    if(getline(s, line)) std::cout << line;

    // "(.*"�̂��̂��Ƃ肠�����I������
    while(line[0]!='(' && line[line.length()-1]!=')'){
      getline(s, line);
      std::cout << line << std::endl;
//      std::cout << line[line.length()-1] << std::endl;
    }

//  string���� '('�𐔂���֐�
//  int i = count_char(line, '(');





    while(count_char(line, '(')!=count_char(line, ')')){
      std::string tmp;
      getline(s, tmp);
      line = line + tmp;
    }


    cout << "---- exit isConsistent_test() phase ----" << endl;
    return line;
  }


/*
  {";", "", "end;"} �̏��ɕ�������擾�������Ƃ��I������Ƃ��āA�I������܂�getline(s, line)������
  TODO: write "endoffile___"; ���t�@�C�����Ɏd���ݏI������Ƃ���
*/
  void file_input_test(){
    std::cout << "<main> file_input_test: " << "lib.red" << std::endl;

    std::string line[3];
    std::string cmd = "in \"lib.red\";";
    send_to_reduce(cmd);

    while(true){
      getline(s, line[0]);
      std::cout << line[0] << std::endl;
//      cout << "line[0]" << line[0] << "         line[1] " << line[1] << "         line[2] " << line[2] << endl;
      if(line[0]=="end;" && line[1]=="" && line[2]==";") break;
      line[2] = line[1];
      line[1] = line[0];
    }
  }


  void func_test(const char* filename){
    std::cout << "<main> func_test: " << filename << std::endl;

    std::ifstream ifs(filename);
    std::string cmd;
//���M��
    while(getline(ifs, cmd)){
      s << cmd;
    }

	std::string line[3];
//��M��
    while(true){
      getline(s, line[0]);
      cout << line[0] << endl;
      if(line[0]==END_TST) break;
      if(line[0]==END_TST_OFFNAT) break;
      if(line[0]=="end;" && line[1]=="" && line[2]==";") break;
      line[2] = line[1];
      line[1] = line[0];
    }
  }
};



int main(int argc, char** argv){
  asio_cl cl;

  cl.recv_from_reduce();

//  "in \"lib.red\";"��send����
  cl.file_input_test();

//  func_test�ďo����
  if(argc==2){
    cl.func_test(argv[1]);
    return 0;
  }else{
    std::cout << "argc!=2" << std::endl;
  }


  std::string ans;
//  "isConsistent(expr, vars)"��send����
//  ans = cl.isConsistent_test();
//  std::cout << "ans= " << ans << std::endl;
//  std::cout << "ans has " << cl.count_char(ans, '(') << "'('s, and "
//          << cl.count_char(ans, ')') << "')'s" << std::endl;

// scanf -> send -> recv -> scanf�̌J��Ԃ�
  cl.scanf_test();

  cl.send_to_reduce("bye;");
  cl.recv_from_reduce();
}

